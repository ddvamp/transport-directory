#include <algorithm>
#include <cmath>
#include <limits>
#include <stack>
#include <utility>
#include <variant>

#include "geo_math.h"
#include "transport_directory_impl.h"
#include "transport_directory_renderer.h"

namespace transport {

using detail::Route;

std::size_t TransportDirectoryImpl::
	countUniqueID(std::vector<StopID> const &route) const
{
	std::vector ids(getStopsCount(), 0);
	for (auto id : route) {
		++ids[id];
	}
	return ids.size() - static_cast<std::size_t>(
		std::count(ids.begin(), ids.end(), 0)
	);
}

double TransportDirectoryImpl::
	computeRoadRouteLength(std::vector<StopID> const &route) const noexcept
{
	double length{};
	for (std::size_t i = 1; i < route.size(); ++i) {
		length += getDistance(route[i - 1], route[i]);
	}
	return length;
}

double TransportDirectoryImpl::
	computeGeoRouteLength(std::vector<StopID> const &route) const noexcept
{
	double length{};
	for (std::size_t i = 1; i < route.size(); ++i) {
		length += getGeoDistance(route[i - 1], route[i]);
	}
	return length;
}

TransportDirectoryImpl::TransportDirectoryImpl(config::Config config)
	: routing_settings_{config.routing_settings}
	, render_settings_{std::move(config.render_settings)}
{
	auto iter = std::partition(config.items.begin(), config.items.end(),
		[](config::Item const &item) {
			return std::holds_alternative<config::Stop>(item);
		});

	stops_.resize(static_cast<std::size_t>(iter - config.items.begin()));
	distances_.resize(getStopsCount() * getStopsCount(),
		std::numeric_limits<double>::infinity());
	for (auto first = config.items.begin(), last = iter;
		first != last; ++first) {
		addStop(std::get<config::Stop>(std::move(*first)));
	}
	
	buses_.resize(static_cast<std::size_t>(config.items.end() - iter));
	for (auto first = iter, last = config.items.end();
		first != last; ++first) {
		addBus(std::get<config::Bus>(std::move(*first)));
	}

	calculateGeoDistances();
	computeRoutes();
}

void TransportDirectoryImpl::addBus(config::Bus bus)
{
	auto &new_bus = registerBus(std::move(bus.name));
	new_bus.route.reserve(bus.route.size());
	for (auto &stop_name : bus.route) {
		auto &stop = registerStop(std::move(stop_name));
		new_bus.route.push_back(stop.id);
		stop.buses.insert(new_bus.id);
	}
	new_bus.is_roundtrip = bus.is_roundtrip;
}

void TransportDirectoryImpl::addStop(config::Stop stop)
{
	auto &new_stop = registerStop(std::move(stop.name));
	new_stop.coords = stop.coords;
	for (auto &[adjacent_name, distance] : stop.distances) {
		auto &adjacent = registerStop(std::move(adjacent_name));
		new_stop.adjacent.insert(adjacent.id);
		getDistance(new_stop.id, adjacent.id) = distance;
		if (adjacent.adjacent.insert(new_stop.id).second) {
			getDistance(adjacent.id, new_stop.id) = distance;
		}
	}
}

void TransportDirectoryImpl::calculateGeoDistances()
{
	geo_distances_.resize(getStopsCount() * getStopsCount());
	for (StopID from{}; from < getStopsCount(); ++from) {
		for (StopID to = from; to < getStopsCount(); ++to) {
			getGeoDistance(from, to) =
				getGeoDistance(to, from) =
				geo::computeGeoDistance(
					getStop(from).coords,
					getStop(to).coords
				);
		}
	}
}

void TransportDirectoryImpl::computeRoutes()
{
	routes_.resize(getStopsCount() * getStopsCount(), {
		.time = std::numeric_limits<double>::infinity(),
		.item = {},
	});
	fillRoutes();
	executeWFI();
}

void TransportDirectoryImpl::fillRoutes()
{
	for (BusID id{}; id < getBusesCount(); ++id) {
		auto const &route = getBus(id).route;
		std::vector span_time(route.size(), 0.0);
		for (std::size_t i = 1; i < route.size(); ++i) {
			auto dtime = getDistance(route[i - 1], route[i]) /
				routing_settings_.velocity;
			for (auto j = i; j-- != 0; ) {
				auto from = route[j];
				auto to = route[i];
				auto time = span_time[j] += dtime;
				if (time < getRoute(from, to).time) {
					getRoute(from, to) = {
						.time = time,
						.item = detail::Route::Span{
							.from = from,
							.bus = id,
							.spans_count = static_cast<std::uint16_t>(i - j),
						},
					};
				}
			}
		}
	}
}

void TransportDirectoryImpl::executeWFI()
{
	for (StopID middle{}; middle < getStopsCount(); ++middle) {
		for (StopID from{}; from < getStopsCount(); ++from) {
			for (StopID to{}; to < getStopsCount(); ++to) {
				auto time =
					getRoute(from, middle).time +
					routing_settings_.wait_time +
				   	getRoute(middle, to).time;
				if (time < getRoute(from, to).time) {
					getRoute(from, to) = {
						.time = time,
						.item = detail::Route::Transfer{
							.from = from,
						   	.middle = middle,
							.to = to,
						},
					};
				}
			}
		}
	}
}

std::optional<info::Bus> TransportDirectoryImpl::getBus(
	std::string const &name) const
{
	auto it = bus_ids_.find(name);
	if (it == bus_ids_.end()) {
		return std::nullopt;
	}
	return makeBusInfo(getBus(it->second));
}

std::optional<info::Stop> TransportDirectoryImpl::getStop(
	std::string const &name) const
{
	auto it = stop_ids_.find(name);
	if (it == stop_ids_.end()) {
		return std::nullopt;
	}
	return makeStopInfo(getStop(it->second));
}

std::optional<info::Route> TransportDirectoryImpl::getRoute(
	std::string const &source, std::string const &destination) const
{
	auto from = stop_ids_.at(source);
	auto to = stop_ids_.at(destination);
	if (from == to) {
		return std::optional<info::Route>{std::in_place};
	}
	if (not std::isfinite(getRoute(from, to).time)) {
		return std::nullopt;
	}
	return makeRouteInfo(getRoute(from, to));
}

info::Map TransportDirectoryImpl::getMap() const
{
	if (map_.empty()) {
		map_ = TransportDirectoryRenderer{
			buses_,
			stops_,
			render_settings_
		}.renderMap();
	}
	return {.data = map_};
}

info::Bus TransportDirectoryImpl::makeBusInfo(detail::Bus const &bus) const
{
	return {
		.stops_count = bus.route.size(),
		.unique_stops_count = countUniqueID(bus.route),
		.road_route_length = computeRoadRouteLength(bus.route),
		.geo_route_length = computeGeoRouteLength(bus.route),
	};
}

info::Stop TransportDirectoryImpl::makeStopInfo(detail::Stop const &stop) const
{
	info::Stop response;
	response.buses.reserve(stop.buses.size());
	for (auto id : stop.buses) {
		response.buses.emplace_back(getBus(id).name);
	}
	std::sort(response.buses.begin(), response.buses.end());
	return response;
}

info::Route TransportDirectoryImpl::makeRouteInfo(Route const &route) const
{
	info::Route response;
	std::stack<Route const *> items;
	auto const *item = &route;
	while (true) {
		if (std::holds_alternative<Route::Transfer>(item->item)) {
			auto const &transfer = std::get<Route::Transfer>(item->item);
			items.push(&getRoute(transfer.middle, transfer.to));
			item = &getRoute(transfer.from, transfer.middle);
		} else {
			auto const &span = std::get<Route::Span>(item->item);
			response.total_time += routing_settings_.wait_time + item->time;
			response.items.push_back({
				.stop_name = getStop(span.from).name,
				.wait_time = routing_settings_.wait_time,
				.bus_name = getBus(span.bus).name,
				.travel_time = item->time,
				.spans_count = span.spans_count,
			});
			if (items.empty()) {
				break;
			}
			item = items.top();
			items.pop();
		}
	}
	return response;
}

} // namespace transport
