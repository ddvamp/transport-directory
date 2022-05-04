#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
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
	return static_cast<std::size_t>(
		std::ranges::count_if(ids, [](int v) noexcept { return v != 0; })
	);
}

double TransportDirectoryImpl::
	computeRoadRouteLength(std::vector<StopID> const &route) const noexcept
{
	double length{};
	for (auto from = route.front(); auto to : route | std::views::drop(1)) {
		length += getDistance(std::exchange(from, to), to);
	}
	return length;
}

double TransportDirectoryImpl::
	computeGeoRouteLength(std::vector<StopID> const &route) const noexcept
{
	double length{};
	for (auto from = route.front(); auto to : route | std::views::drop(1)) {
		length += getGeoDistance(std::exchange(from, to), to);
	}
	return length;
}

auto TransportDirectoryImpl::
	computeRouteLengths(std::vector<StopID> const &route) const noexcept
{
	struct Lengths {
		double road;
		double geo;
	} lengths{};
	for (auto from = route.front(); auto to : route | std::views::drop(1)) {
		lengths.road += getDistance(from, to);
		lengths.geo += getGeoDistance(std::exchange(from, to), to);
	}
	return lengths;
}

TransportDirectoryImpl::TransportDirectoryImpl(config::Config &&config)
	: routing_settings_{std::move(config.routing_settings)}
	, render_settings_{std::move(config.render_settings)}
{
	auto buses = std::ranges::partition(config.items,
		[](config::Item const &item) noexcept {
			return std::holds_alternative<config::Stop>(item);
		});
	decltype(buses) stops = {config.items.begin(), buses.begin()};

	init(stops.size(), buses.size());

	for (auto &stop : stops) {
		addStop(std::get<config::Stop>(std::move(stop)));
	}
	for (auto &bus : buses) {
		addBus(std::get<config::Bus>(std::move(bus)));
	}

	calculateGeoDistances();
	computeRoutes();
}

void TransportDirectoryImpl::init(
	std::size_t stops_count, std::size_t buses_count)
{
	stops_.resize(stops_count);
	geo_distances_.resize(stops_count * stops_count);
	distances_.resize(
		stops_count * stops_count,
		std::numeric_limits<double>::infinity()
	);
	routes_.resize(stops_count * stops_count, {
		.time = std::numeric_limits<double>::infinity(),
		.item = {},
	});
	buses_.resize(buses_count);
}

void TransportDirectoryImpl::addBus(config::Bus &&bus)
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

void TransportDirectoryImpl::addStop(config::Stop &&stop)
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

detail::Bus &TransportDirectoryImpl::
	registerBus(std::string name)
{
	auto [it, is_new] =
		bus_ids_.try_emplace(std::move(name), bus_ids_.size());
	auto &bus = getBus(it->second);
	if (is_new) {
		bus.name = it->first;
		bus.id = it->second;
	}
	return bus;
}

detail::Stop &TransportDirectoryImpl::
	registerStop(std::string name)
{
	auto [it, is_new] = 
		stop_ids_.try_emplace(std::move(name), stop_ids_.size());
	auto &stop = getStop(it->second);
	if (is_new) {
		stop.name = it->first;
		stop.id = it->second;
	}
	return stop;
}

void TransportDirectoryImpl::calculateGeoDistances() noexcept
{
	auto &&stops = std::as_const(*this).getStopsList();
	for (auto const &from : stops) {
		for (auto const &to : stops) {
			if (from.id <= to.id) {
				getGeoDistance(from.id, to.id) =
					getGeoDistance(to.id, from.id) =
					geo::computeGeoDistance(from.coords, to.coords);
			}
		}
	}
}

void TransportDirectoryImpl::computeRoutes()
{
	fillRoutes();
	executeWFI();
}

// вычисление кратчайших маршрутов без пересадок
void TransportDirectoryImpl::fillRoutes()
{
	for (auto const &bus : std::as_const(*this).getBusesList()) {
		std::vector span_time(bus.route.size(), 0.0);
		for (std::size_t i = 1; i < bus.route.size(); ++i) {
			auto from = bus.route[i - 1];
			auto to = bus.route[i];
			auto dtime = getDistance(from, to) /
				routing_settings_.velocity;
			for (auto j = i; j-- != 0; ) {
				from = bus.route[j];
				auto &route = getRoute(from, to);
				auto time = span_time[j] += dtime;
				if (time < route.time) {
					route = {
						.time = time,
						.item = detail::Route::Span{
							.from = from,
							.bus = bus.id,
							.spans_count = static_cast<std::uint16_t>(i - j),
						},
					};
				}
			}
		}
	}
}

// алгоритм Флойда–Уоршелла вычисления всех кратчайших маршрутов
void TransportDirectoryImpl::executeWFI()
{
	auto &&stops = std::as_const(*this).getStopsList();
	for (auto const &middle : stops) {
		for (auto const &from : stops) {
			for (auto const &to : stops) {
				auto &route = getRoute(from.id, to.id);
				auto old_time = route.time;
				auto new_time =
					getRoute(from.id, middle.id).time +
					routing_settings_.wait_time +
				   	getRoute(middle.id, to.id).time;
				if (new_time < old_time) {
					route = {
						.time = new_time,
						.item = detail::Route::Transfer{
							.from = from.id,
						   	.middle = middle.id,
							.to = to.id,
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
	auto from_it = stop_ids_.find(source);
	if (from_it == stop_ids_.end()) {
		return std::nullopt;
	}
	auto to_it = stop_ids_.find(destination);
	if (to_it == stop_ids_.end()) {
		return std::nullopt;
	}
	if (from_it == to_it) {
		return std::optional<info::Route>{std::in_place};
	}
	auto const &route = getRoute(from_it->second, to_it->second);
	if (not std::isfinite(route.time)) {
		return std::nullopt;
	}
	return makeRouteInfo(route);
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
	auto lengths = computeRouteLengths(bus.route);
	return {
		.stops_count = bus.route.size(),
		.unique_stops_count = countUniqueID(bus.route),
		.road_route_length = lengths.road,
		.geo_route_length = lengths.geo,
	};
}

info::Stop TransportDirectoryImpl::makeStopInfo(detail::Stop const &stop) const
{
	info::Stop response;
	response.buses.reserve(stop.buses.size());
	for (auto id : stop.buses) {
		response.buses.emplace_back(getBus(id).name);
	}
	std::ranges::sort(response.buses);
	return response;
}

info::Route TransportDirectoryImpl::makeRouteInfo(Route const &route) const
{
	info::Route response;
	std::stack<Route const *> items;
	auto const *item = &route;
	bool route_is_over = false;
	while (not route_is_over) {
		// если очередная часть пути составная
		// откладывание второй части пути и
		// переход к обработке первой части
		if (auto const* transfer = std::get_if<Route::Transfer>(&item->item)) {
			items.push(&getRoute(transfer->middle, transfer->to));
			item = &getRoute(transfer->from, transfer->middle);
			continue;
		}
		// обработка части пути
		auto const &span = std::get<Route::Span>(item->item);
		response.total_time += routing_settings_.wait_time + item->time;
		response.items.push_back({
			.stop_name = getStop(span.from).name,
			.wait_time = routing_settings_.wait_time,
			.bus_name = getBus(span.bus).name,
			.travel_time = item->time,
			.spans_count = span.spans_count,
		});
		// переход к обработке следующей части пути
		if (items.empty()) {
			route_is_over = true;
		} else {
			item = items.top();
			items.pop();
		}
	}
	return response;
}

} // namespace transport
