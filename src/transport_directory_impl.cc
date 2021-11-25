#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stack>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <variant>


#include "transport_directory_impl.h"

namespace {

[[nodiscard]] double convertDegreesToRadians(double n) noexcept
{
	constexpr double kPi = 3.141592653589793;
	return n * (kPi / 180.0);
}

[[nodiscard]] transport::Coordinates convertGeoCoordinatesToRadians(
	transport::Coordinates p) noexcept
{
	return {
		.latitude = convertDegreesToRadians(p.latitude),
		.longitude = convertDegreesToRadians(p.longitude),
	};
}

[[nodiscard]] double computeGeoDistance(transport::Coordinates x,
	transport::Coordinates y) noexcept
{
	constexpr double kEarthRadius = 6371000;
	x = convertGeoCoordinatesToRadians(x);
	y = convertGeoCoordinatesToRadians(y);
	auto a = std::cos(x.latitude + y.latitude);
	auto b = std::cos(x.latitude - y.latitude);
	auto c = std::cos(x.longitude - y.longitude);
	return std::acos((a + b) * (1 + c) / 2 - a) * kEarthRadius;
}

std::size_t countUniqueItems(std::vector<transport::detail::Id> const &items)
{
	return std::unordered_set(items.begin(), items.end()).size();
}

} // namespace

namespace transport {

using detail::Id;
using detail::Route;

Id TransportDirectoryImpl::registerStop(std::string const &stop)
{
	return stops_ids_.try_emplace(stop, stops_ids_.size()).first->second;
}

Id TransportDirectoryImpl::registerBus(std::string const &bus)
{
	return buses_ids_.try_emplace(bus, buses_ids_.size()).first->second;
}

double &TransportDirectoryImpl::getDistance(Id from, Id to)
{
	return distances_[from * stops_.size() + to];
}

double const &TransportDirectoryImpl::getDistance(Id from, Id to) const
{
	return distances_[from * stops_.size() + to];
}

Route & TransportDirectoryImpl::getRoute(Id from, Id to)
{
	return routes_[from * stops_.size() + to];
}

Route const & TransportDirectoryImpl::getRoute(Id from, Id to) const
{
	return routes_[from * stops_.size() + to];
}

double TransportDirectoryImpl::computeRoadRouteLength(
	std::vector<Id> const &route) const
{
	double length{};
	for (std::size_t i = 1; i < route.size(); ++i) {
		length += getDistance(route[i - 1], route[i]);
	}
	return length;
}

double TransportDirectoryImpl::computeGeoRouteLength(
	std::vector<Id> const &route) const
{
	double length{};
	for (std::size_t i = 1; i < route.size(); ++i) {
		length += computeGeoDistance(stops_[route[i - 1]].coords,
			stops_[route[i]].coords);
	}
	return length;
}

TransportDirectoryImpl::TransportDirectoryImpl(config::Config config) :
	settings_{config.settings}
{
	auto iter = std::partition(config.items.begin(), config.items.end(),
		[](config::Item const &item) {
			return std::holds_alternative<config::Stop>(item);
		});

	auto stops_count = static_cast<std::size_t>(iter - config.items.begin());
	stops_.resize(stops_count);
	distances_.resize(stops_count * stops_count,
		std::numeric_limits<double>::infinity());
	routes_.resize(stops_count * stops_count, Route{
		.time = std::numeric_limits<double>::infinity(),
		.item = {},
	});
	for (auto first = config.items.begin(), last = iter;
		first != last; ++first) {
		auto &stop = std::get<config::Stop>(*first);
		auto id = registerStop(stop.name);
		stops_[id].name = std::move(stop.name);
		stops_[id].coords = stop.coords;
		for (auto &[adjacent, distance] : stop.distances) {
			auto adjacent_id = registerStop(adjacent);
			stops_[id].adjacent.insert(adjacent_id);
			getDistance(id, adjacent_id) = distance;
			if (stops_[adjacent_id].adjacent.insert(id).second) {
				getDistance(adjacent_id, id) = distance;
			}
		}
	}
	
	auto buses_count = config.items.size() - stops_count;
	buses_.resize(buses_count);
	for (auto first = iter, last = config.items.end();
		first != last; ++first) {
		auto &bus = std::get<config::Bus>(*first);
		auto id = registerBus(bus.name);
		buses_[id].name = std::move(bus.name);
		buses_[id].route.reserve(bus.route.size());
		for (auto &stop : bus.route) {
			auto stop_id = registerStop(stop);
			buses_[id].route.push_back(stop_id);
			stops_[stop_id].buses.insert(id);
		}
	}
	computeRoutes();
}

void TransportDirectoryImpl::computeRoutes()
{
	fillRoutes();
	executeWFI();
}

void TransportDirectoryImpl::fillRoutes()
{
	for (Id id{}; id < buses_ids_.size(); ++id) {
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
					getRoute(from, to) = Route{
						.time = time,
						.item = Route::Span{
							.from = from,
							.to = to,
							.bus = id,
							.span_count = static_cast<std::uint32_t>(i - j),
						},
					};
				}
			}
		}
	}
}

void TransportDirectoryImpl::executeWFI()
{
	for (Id middle{}; middle < stops_ids_.size(); ++middle) {
		for (Id from{}; from < stops_ids_.size(); ++from) {
			for (Id to{}; to < stops_ids_.size(); ++to) {
				auto const &lhs = getRoute(from, middle);
				auto const &rhs = getRoute(middle, to);
				auto time = lhs.time + settings_.wait_time + rhs.time;
				if (time < getRoute(from, to).time) {
					getRoute(from, to) = Route{
						.time = time,
						.item = Route::Transfer{
							.from = &lhs,
							.to = &rhs,
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
	auto const &bus = buses_[it->second];
	return info::Bus{
		.stops_count = bus.route.size(),
		.unique_stops_count = countUniqueItems(bus.route),
		.road_route_length = computeRoadRouteLength(bus.route),
		.geo_route_length = computeGeoRouteLength(bus.route),
	};
}

std::optional<info::Stop> TransportDirectoryImpl::getStop(
	std::string const &name) const
{
	auto it = stops_ids_.find(name);
	if (it == stops_ids_.end()) {
		return std::nullopt;
	}
	auto const &stop = stops_[it->second];
	info::Stop response;
	response.buses.reserve(stop.buses.size());
	for (auto id : stop.buses) {
		response.buses.emplace_back(buses_[id].name);
	}
	std::sort(response.buses.begin(), response.buses.end());
	return response;
}

std::optional<info::Route> TransportDirectoryImpl::getRoute(
	std::string const &from, std::string const &to) const
{
	auto fst = stops_ids_.at(from);
	auto snd = stops_ids_.at(to);
	if (fst == snd) {
		return std::optional<info::Route>{std::in_place};
	}
	auto route = &getRoute(fst, snd);
	if (not std::isfinite(route->time)) {
		return std::nullopt;
	}
	std::optional<info::Route> response{std::in_place};
	for (std::stack<Route const *> items; ; ) {
		if (auto p = std::get_if<Route::Transfer>(&route->item)) {
			items.push(p->to);
			route = p->from;
		} else {
			auto const &item = std::get<Route::Span>(route->item);
			response->total_time += settings_.wait_time + route->time;
			response->items.push_back(info::Route::Span{
				.stop_name = stops_[item.from].name,
				.wait_time = settings_.wait_time,
				.bus_name = buses_[item.bus].name,
				.travel_time = route->time,
				.spans_count = item.span_count,
			});
			if (items.empty()) {
				break;
			}
			route = items.top();
			items.pop();
		}
	}
	return response;
}

} // namespace transport
