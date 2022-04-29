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
		std::vector<StopID> const &route) const;
	[[nodiscard]] double computeGeoRouteLength(
		std::vector<StopID> const &route) const;

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
	BusID registerBus(std::string const &name);
	StopID registerStop(std::string const &name);

	[[nodiscard]] double &getDistance(StopID from, StopID to);
	[[nodiscard]] double const &getDistance(StopID from, StopID to) const;

	[[nodiscard]] double &getGeoDistance(StopID from, StopID to);
	[[nodiscard]] double const &getGeoDistance(StopID from, StopID to) const;

	[[nodiscard]] detail::Route &getRoute(StopID from, StopID to);
	[[nodiscard]] detail::Route const &getRoute(StopID from, StopID to) const;
};

inline auto TransportDirectoryImpl::
	registerStop(std::string const &name) -> StopID
{
	return stop_ids_.try_emplace(name, stop_ids_.size()).first->second;
}

inline auto TransportDirectoryImpl::
	registerBus(std::string const &name) -> BusID
{
	return bus_ids_.try_emplace(name, bus_ids_.size()).first->second;
}

inline double &TransportDirectoryImpl::
	getDistance(StopID from, StopID to)
{
	return distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getDistance(StopID from, StopID to) const
{
	return distances_[from * stops_.size() + to];
}

inline double &TransportDirectoryImpl::
	getGeoDistance(StopID from, StopID to)
{
	return geo_distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getGeoDistance(StopID from, StopID to) const
{
	return geo_distances_[from * stops_.size() + to];
}

inline detail::Route &TransportDirectoryImpl::
	getRoute(StopID from, StopID to)
{
	return routes_[from * stops_.size() + to];
}

inline detail::Route const &TransportDirectoryImpl::
	getRoute(StopID from, StopID to) const
{
	return routes_[from * stops_.size() + to];
}

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_IMPL_H_ */
