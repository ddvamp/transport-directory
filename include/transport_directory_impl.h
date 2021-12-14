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
		std::vector<ID> const &route) const;
	[[nodiscard]] double computeRoadRouteLength(
		std::vector<ID> const &route) const;
	[[nodiscard]] double computeGeoRouteLength(
		std::vector<ID> const &route) const;

	[[nodiscard]] info::Bus makeBusInfo(detail::Bus const &) const;
	[[nodiscard]] info::Stop makeStopInfo(detail::Stop const &) const;
	[[nodiscard]] info::Route makeRouteInfo(detail::Route const &) const;

	void calculateGeoDistances();
	void computeRoutes();
	void fillRoutes();
	void executeWFI();

private:
	std::unordered_map<std::string, ID> bus_ids_;
	std::vector<detail::Bus> buses_;
	std::unordered_map<std::string, ID> stop_ids_;
	std::vector<detail::Stop> stops_;
	std::vector<double> distances_;
	std::vector<double> geo_distances_;
	std::vector<detail::Route> routes_;
	config::RoutingSettings routing_settings_;
	config::RenderSettings render_settings_;
	mutable std::string map_;

private:
	ID registerBus(std::string const &name);
	ID registerStop(std::string const &name);

	[[nodiscard]] double &getDistance(ID from, ID to);
	[[nodiscard]] double const &getDistance(ID from, ID to) const;

	[[nodiscard]] double &getGeoDistance(ID from, ID to);
	[[nodiscard]] double const &getGeoDistance(ID from, ID to) const;

	[[nodiscard]] detail::Route &getRoute(ID from, ID to);
	[[nodiscard]] detail::Route const &getRoute(ID from, ID to) const;
};

inline ID TransportDirectoryImpl::
	registerStop(std::string const &name)
{
	return stop_ids_.try_emplace(name, stop_ids_.size()).first->second;
}

inline ID TransportDirectoryImpl::
	registerBus(std::string const &name)
{
	return bus_ids_.try_emplace(name, bus_ids_.size()).first->second;
}

inline double &TransportDirectoryImpl::
	getDistance(ID from, ID to)
{
	return distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getDistance(ID from, ID to) const
{
	return distances_[from * stops_.size() + to];
}

inline double &TransportDirectoryImpl::
	getGeoDistance(ID from, ID to)
{
	return geo_distances_[from * stops_.size() + to];
}

inline double const &TransportDirectoryImpl::
	getGeoDistance(ID from, ID to) const
{
	return geo_distances_[from * stops_.size() + to];
}

inline detail::Route &TransportDirectoryImpl::
	getRoute(ID from, ID to)
{
	return routes_[from * stops_.size() + to];
}

inline detail::Route const &TransportDirectoryImpl::
	getRoute(ID from, ID to) const
{
	return routes_[from * stops_.size() + to];
}

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_IMPL_H_ */
