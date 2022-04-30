#ifndef DDV_TRANSPORT_DIRECTORY_IMPL_H_
#define DDV_TRANSPORT_DIRECTORY_IMPL_H_ 1

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "transport_directory_detail.h"
#include "transport_directory_info.h"
#include "transport_directory_config.h"

namespace transport {

class TransportDirectoryImpl {
private:
	using StopID = detail::StopID;
	using BusID = detail::BusID;

public:
	TransportDirectoryImpl(config::Config);

	[[nodiscard]] std::optional<info::Bus> getBus(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Stop> getStop(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Route> getRoute(
		std::string const &from, std::string const &to) const;
	[[nodiscard]] info::Map getMap() const;

private:
	void addBus(config::Bus);
	void addStop(config::Stop);

	[[nodiscard]] std::size_t countUniqueID(
		std::vector<StopID> const &route) const;
	[[nodiscard]] double computeRoadRouteLength(
		std::vector<StopID> const &route) const noexcept;
	[[nodiscard]] double computeGeoRouteLength(
		std::vector<StopID> const &route) const noexcept;

	[[nodiscard]] info::Bus makeBusInfo(detail::Bus const &) const;
	[[nodiscard]] info::Stop makeStopInfo(detail::Stop const &) const;
	[[nodiscard]] info::Route makeRouteInfo(detail::Route const &) const;

	void calculateGeoDistances();
	void computeRoutes();
	void fillRoutes();
	void executeWFI();

private:
	std::unordered_map<std::string, BusID> bus_ids_;
	std::vector<detail::Bus> buses_;
	std::unordered_map<std::string, StopID> stop_ids_;
	std::vector<detail::Stop> stops_;

	std::vector<double> distances_;
	std::vector<double> geo_distances_;
	std::vector<detail::Route> routes_;

	config::RoutingSettings routing_settings_;
	config::RenderSettings render_settings_;

	mutable std::string map_;

private:
	[[nodiscard]] std::size_t getBusesCount() const noexcept;
	[[nodiscard]] std::size_t getStopsCount() const noexcept;

	detail::Bus &registerBus(std::string name);
	detail::Stop &registerStop(std::string name);

	[[nodiscard]] double &getDistance(StopID from, StopID to) noexcept;
	[[nodiscard]] double const &getDistance(StopID from, StopID to) const noexcept;

	[[nodiscard]] double &getGeoDistance(StopID from, StopID to) noexcept;
	[[nodiscard]] double const &getGeoDistance(StopID from, StopID to) const noexcept;

	[[nodiscard]] detail::Bus &getBus(BusID) noexcept;
	[[nodiscard]] detail::Bus const &getBus(BusID) const noexcept;

	[[nodiscard]] detail::Stop &getStop(StopID) noexcept;
	[[nodiscard]] detail::Stop const &getStop(StopID) const noexcept;

	[[nodiscard]] detail::Route &getRoute(StopID from, StopID to) noexcept;
	[[nodiscard]] detail::Route const &getRoute(StopID from, StopID to) const noexcept;
};

inline std::size_t TransportDirectoryImpl::
	getBusesCount() const noexcept
{
	return buses_.size();
}

inline std::size_t TransportDirectoryImpl::
	getStopsCount() const noexcept
{
	return stops_.size();
}

inline detail::Bus &TransportDirectoryImpl::
	registerBus(std::string name)
{
	auto [it, is_new] = bus_ids_.try_emplace(std::move(name), bus_ids_.size());
	auto &bus = getBus(it->second);
	if (is_new) {
		bus.name = it->first;
		bus.id = it->second;
	}
	return bus;
}

inline detail::Stop &TransportDirectoryImpl::
	registerStop(std::string name)
{
	auto [it, is_new] = stop_ids_.try_emplace(std::move(name), stop_ids_.size());
	auto &stop = getStop(it->second);
	if (is_new) {
		stop.name = it->first;
		stop.id = it->second;
	}
	return stop;
}

inline double &TransportDirectoryImpl::
	getDistance(StopID from, StopID to) noexcept
{
	return distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getDistance(StopID from, StopID to) const noexcept
{
	return distances_[from * stops_.size() + to];
}

inline double &TransportDirectoryImpl::
	getGeoDistance(StopID from, StopID to) noexcept
{
	return geo_distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getGeoDistance(StopID from, StopID to) const noexcept
{
	return geo_distances_[from * stops_.size() + to];
}

inline detail::Bus &TransportDirectoryImpl::
	getBus(BusID id) noexcept
{
	return buses_[id];
}

inline detail::Bus const &TransportDirectoryImpl::
	getBus(BusID id) const noexcept
{
	return buses_[id];
}

inline detail::Stop &TransportDirectoryImpl::
	getStop(StopID id) noexcept
{
	return stops_[id];
}

inline detail::Stop const &TransportDirectoryImpl::
	getStop(StopID id) const noexcept
{
	return stops_[id];
}

inline detail::Route &TransportDirectoryImpl::
	getRoute(StopID from, StopID to) noexcept
{
	return routes_[from * stops_.size() + to];
}

inline detail::Route const &TransportDirectoryImpl::
	getRoute(StopID from, StopID to) const noexcept
{
	return routes_[from * stops_.size() + to];
}

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_IMPL_H_ */
