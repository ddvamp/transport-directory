#ifndef DDV_TRANSPORT_DIRECTORY_IMPL_H_
#define DDV_TRANSPORT_DIRECTORY_IMPL_H_ 1

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "transport_directory_config.h"
#include "transport_directory_detail.h"
#include "transport_directory_info.h"

namespace transport {

class TransportDirectoryImpl {
private:
	using StopId = detail::StopId;
	using BusId = detail::BusId;

public:
	TransportDirectoryImpl(config::Config &&);

	[[nodiscard]] std::optional<info::Bus> getBus(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Stop> getStop(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Route> getRoute(
		std::string const &from, std::string const &to) const;
	[[nodiscard]] info::Map getMap() const;

private:
	void addBus(config::Bus &&);
	void addStop(config::Stop &&);

	[[nodiscard]] std::size_t countUniqueId(
		std::vector<StopId> const &route) const;
	[[nodiscard]] double computeRoadRouteLength(
		std::vector<StopId> const &route) const noexcept;
	[[nodiscard]] double computeGeoRouteLength(
		std::vector<StopId> const &route) const noexcept;
	[[nodiscard]] auto computeRouteLengths(
		std::vector<StopId> const& route) const noexcept;

	[[nodiscard]] info::Bus makeBusInfo(detail::Bus const &) const;
	[[nodiscard]] info::Stop makeStopInfo(detail::Stop const &) const;
	[[nodiscard]] info::Route makeRouteInfo(detail::Route const &) const;

	void init(std::size_t stops_count, std::size_t buses_count);
	void calculateGeoDistances() noexcept;
	void computeRoutes();
	void fillRoutes();
	void executeWFI();

private:
	std::unordered_map<std::string, BusId> bus_ids_;
	std::vector<detail::Bus> buses_;
	std::unordered_map<std::string, StopId> stop_ids_;
	std::vector<detail::Stop> stops_;

	std::vector<double> distances_;
	std::vector<double> geo_distances_;
	std::vector<detail::Route> routes_;

	config::RoutingSettings routing_settings_;
	config::RenderSettings render_settings_;

	mutable std::string map_;

private:
	detail::Bus &registerBus(std::string name);
	detail::Stop &registerStop(std::string name);

	[[nodiscard]] std::size_t getBusesCount() const noexcept;
	[[nodiscard]] std::size_t getStopsCount() const noexcept;

	[[nodiscard]] decltype(auto) getBusesList() noexcept;
	[[nodiscard]] decltype(auto) getBusesList() const noexcept;

	[[nodiscard]] decltype(auto) getStopsList() noexcept;
	[[nodiscard]] decltype(auto) getStopsList() const noexcept;

	[[nodiscard]] double &getDistance(StopId from, StopId to) noexcept;
	[[nodiscard]] double const &getDistance(
		StopId from, StopId to) const noexcept;

	[[nodiscard]] double &getGeoDistance(StopId from, StopId to) noexcept;
	[[nodiscard]] double const &getGeoDistance(
		StopId from, StopId to) const noexcept;

	[[nodiscard]] detail::Bus &getBus(BusId) noexcept;
	[[nodiscard]] detail::Bus const &getBus(BusId) const noexcept;

	[[nodiscard]] detail::Stop &getStop(StopId) noexcept;
	[[nodiscard]] detail::Stop const &getStop(StopId) const noexcept;

	[[nodiscard]] detail::Route &getRoute(StopId from, StopId to) noexcept;
	[[nodiscard]] detail::Route const &getRoute(
		StopId from, StopId to) const noexcept;
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

inline decltype(auto) TransportDirectoryImpl::
	getBusesList() noexcept
{
	return buses_;
}

inline decltype(auto) TransportDirectoryImpl::
	getBusesList() const noexcept
{
	return buses_;
}

inline decltype(auto) TransportDirectoryImpl::
	getStopsList() noexcept
{
	return stops_;
}

inline decltype(auto) TransportDirectoryImpl::
	getStopsList() const noexcept
{
	return stops_;
}

inline double &TransportDirectoryImpl::
	getDistance(StopId from, StopId to) noexcept
{
	return distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getDistance(StopId from, StopId to) const noexcept
{
	return distances_[from * stops_.size() + to];
}

inline double &TransportDirectoryImpl::
	getGeoDistance(StopId from, StopId to) noexcept
{
	return geo_distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getGeoDistance(StopId from, StopId to) const noexcept
{
	return geo_distances_[from * stops_.size() + to];
}

inline detail::Bus &TransportDirectoryImpl::
	getBus(BusId id) noexcept
{
	return buses_[id];
}

inline detail::Bus const &TransportDirectoryImpl::
	getBus(BusId id) const noexcept
{
	return buses_[id];
}

inline detail::Stop &TransportDirectoryImpl::
	getStop(StopId id) noexcept
{
	return stops_[id];
}

inline detail::Stop const &TransportDirectoryImpl::
	getStop(StopId id) const noexcept
{
	return stops_[id];
}

inline detail::Route &TransportDirectoryImpl::
	getRoute(StopId from, StopId to) noexcept
{
	return routes_[from * stops_.size() + to];
}

inline detail::Route const &TransportDirectoryImpl::
	getRoute(StopId from, StopId to) const noexcept
{
	return routes_[from * stops_.size() + to];
}

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_IMPL_H_ */
