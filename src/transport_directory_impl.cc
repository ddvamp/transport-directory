#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stack>
#include <type_traits>
#include <utility>
#include <variant>

#include "transport_directory_impl.h"

namespace transport {

using detail::Route;

ID TransportDirectoryImpl::registerStop(std::string const &stop)
{
	return stops_ids_.try_emplace(stop, stops_ids_.size()).first->second;
}

ID TransportDirectoryImpl::registerBus(std::string const &bus)
{
	return buses_ids_.try_emplace(bus, buses_ids_.size()).first->second;
}

double &TransportDirectoryImpl::getDistance(ID from, ID to)
{
	return distances_[from * stops_.size() + to];
}

double const &TransportDirectoryImpl::getDistance(ID from, ID to) const
{
	return distances_[from * stops_.size() + to];
}

double &TransportDirectoryImpl::getGeoDistance(ID from, ID to)
{
	return geo_distances_[from * stops_.size() + to];
}

double const &TransportDirectoryImpl::getGeoDistance(ID from, ID to) const
{
	return geo_distances_[from * stops_.size() + to];
}

Route &TransportDirectoryImpl::getRoute(ID from, ID to)
{
	return routes_[from * stops_.size() + to];
}

Route const &TransportDirectoryImpl::getRoute(ID from, ID to) const
{
	return routes_[from * stops_.size() + to];
}

std::size_t TransportDirectoryImpl::countUniqueID(
	std::vector<ID> const &route) const
{
	std::vector<int> ids(stops_ids_.size());
	for (ID id : route) {
		++ids[id];
	}
	return ids.size() - static_cast<std::size_t>(
		std::count(ids.begin(), ids.end(), 0));
}

double TransportDirectoryImpl::computeRoadRouteLength(
	std::vector<ID> const &route) const
{
	double length{};
	for (std::size_t i = 1; i < route.size(); ++i) {
		length += getDistance(route[i - 1], route[i]);
	}
	return length;
}

double TransportDirectoryImpl::computeGeoRouteLength(
	std::vector<ID> const &route) const
{
	double length{};
	for (std::size_t i = 1; i < route.size(); ++i) {
		length += getGeoDistance(route[i - 1], route[i]);
	}
	return length;
}

TransportDirectoryImpl::TransportDirectoryImpl(config::Config config)
	: settings_{config.settings}
{
	auto iter = std::partition(config.items.begin(), config.items.end(),
		[](config::Item const &item) {
			return std::holds_alternative<config::Stop>(item);
		});

	stops_.resize(static_cast<std::size_t>(iter - config.items.begin()));
	distances_.resize(stops_.size() * stops_.size(),
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

void TransportDirectoryImpl::addStop(config::Stop stop)
{
	ID id = registerStop(stop.name);
	stops_[id].name = std::move(stop.name);
	stops_[id].coords = {
		.latitude = stop.latitude,
		.longitude = stop.longitude,
	};
	for (auto &[adjacent, distance] : stop.distances) {
		ID adjacent_id = registerStop(adjacent);
		stops_[id].adjacent.insert(adjacent_id);
		getDistance(id, adjacent_id) = distance;
		if (stops_[adjacent_id].adjacent.insert(id).second) {
			getDistance(adjacent_id, id) = distance;
		}
	}
}

void TransportDirectoryImpl::addBus(config::Bus bus)
{
	ID id = registerBus(bus.name);
	buses_[id].name = std::move(bus.name);
	buses_[id].route.reserve(bus.route.size());
	for (auto &stop : bus.route) {
		ID stop_id = registerStop(stop);
		buses_[id].route.push_back(stop_id);
		stops_[stop_id].buses.insert(id);
	}
}

void TransportDirectoryImpl::calculateGeoDistances()
{
	geo_distances_.resize(stops_.size() * stops_.size());
	for (ID from{}; from < stops_ids_.size(); ++from) {
		for (ID to = from; to < stops_ids_.size(); ++to) {
			getGeoDistance(from, to) = getGeoDistance(to, from) =
				computeGeoDistance(stops_[from].coords, stops_[to].coords);
		}
	}
}

void TransportDirectoryImpl::computeRoutes()
{
	routes_.resize(stops_.size() * stops_.size(), {
		.time = std::numeric_limits<double>::infinity(),
		.item = {},
	});
	fillRoutes();
	executeWFI();
}

void TransportDirectoryImpl::fillRoutes()
{
	for (ID id{}; id < buses_ids_.size(); ++id) {
		auto const &route = buses_[id].route;
		std::vector span_time(route.size(), 0.0);
		for (std::size_t i = 1; i < route.size(); ++i) {
			auto dtime = getDistance(route[i - 1], route[i]) /
				settings_.velocity;
			for (auto j = i; j-- != 0; ) {
				auto from = route[j];
				auto to = route[i];
				auto time = span_time[j] += dtime;
				if (time < getRoute(from, to).time) {
					getRoute(from, to) = {
						.time = time,
						.item = Route::Span{
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
	for (ID middle{}; middle < stops_ids_.size(); ++middle) {
		for (ID from{}; from < stops_ids_.size(); ++from) {
			for (ID to{}; to < stops_ids_.size(); ++to) {
				auto time =
					getRoute(from, middle).time +
					settings_.wait_time +
				   	getRoute(middle, to).time;
				if (time < getRoute(from, to).time) {
					getRoute(from, to) = {
						.time = time,
						.item = Route::Transfer{
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
	auto it = buses_ids_.find(name);
	if (it == buses_ids_.end()) {
		return std::nullopt;
	}
	return makeBusInfo(buses_[it->second]);
}

std::optional<info::Stop> TransportDirectoryImpl::getStop(
	std::string const &name) const
{
	auto it = stops_ids_.find(name);
	if (it == stops_ids_.end()) {
		return std::nullopt;
	}
	return makeStopInfo(stops_[it->second]);
}

std::optional<info::Route> TransportDirectoryImpl::getRoute(
	std::string const &source, std::string const &destination) const
{
	ID from = stops_ids_.at(source);
	ID to = stops_ids_.at(destination);
	if (from == to) {
		return std::optional<info::Route>{std::in_place};
	}
	if (not std::isfinite(getRoute(from, to).time)) {
		return std::nullopt;
	}
	return makeRouteInfo(getRoute(from, to));
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
	for (ID id : stop.buses) {
		response.buses.emplace_back(buses_[id].name);
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
			response.total_time += settings_.wait_time + item->time;
			response.items.push_back({
				.stop_name = stops_[span.from].name,
				.wait_time = settings_.wait_time,
				.bus_name = buses_[span.bus].name,
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
