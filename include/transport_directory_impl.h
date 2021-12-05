#ifndef TRANSPORT_DIRECTORY_IMPL_H_
#define TRANSPORT_DIRECTORY_IMPL_H_ 1

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "geo_math.h"
#include "transport_directory_info.h"
#include "transport_directory_setup.h"

namespace transport {

using ID = std::uint16_t;

namespace detail {

struct Stop {
	std::string name;
	geo::Point coords;
	std::unordered_set<ID> adjacent;
	std::unordered_set<ID> buses;
};

struct Bus {
	std::string name;
	std::vector<ID> route;
};

struct Route {
	struct Span {
		ID from;
		ID bus;
		std::uint16_t spans_count;
	};

	struct Transfer {
		ID from;
		ID middle;
		ID to;
	};

	using Item = std::variant<Span, Transfer>;

	double time;
	Item item;
};

} // namespace transport::detail

class TransportDirectoryImpl {
public:
	TransportDirectoryImpl(config::Config config);

	[[nodiscard]] std::optional<info::Bus> getBus(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Stop> getStop(
		std::string const &name) const;
	[[nodiscard]] std::optional<info::Route> getRoute(
		std::string const &source, std::string const &destination) const;

private:
	void addBus(config::Bus bus);
	void addStop(config::Stop stop);

	ID registerBus(std::string const &bus);
	ID registerStop(std::string const &stop);

	[[nodiscard]] double &getDistance(ID from, ID to);
	[[nodiscard]] double const &getDistance(ID from, ID to) const;

	[[nodiscard]] double &getGeoDistance(ID from, ID to);
	[[nodiscard]] double const &getGeoDistance(ID from, ID to) const;

	[[nodiscard]] detail::Route &getRoute(ID from, ID to);
	[[nodiscard]] detail::Route const &getRoute(ID from, ID to) const;

	[[nodiscard]] std::size_t countUniqueID(
		std::vector<ID> const &route) const;
	[[nodiscard]] double computeRoadRouteLength(
		std::vector<ID> const &route) const;
	[[nodiscard]] double computeGeoRouteLength(
		std::vector<ID> const &route) const;

	[[nodiscard]] info::Bus makeBusInfo(detail::Bus const &bus) const;
	[[nodiscard]] info::Stop makeStopInfo(detail::Stop const &stop) const;
	[[nodiscard]] info::Route makeRouteInfo(detail::Route const &route) const;

	void calculateGeoDistances();
	void computeRoutes();
	void fillRoutes();
	void executeWFI();

private:
	std::unordered_map<std::string, ID> buses_ids_;
	std::vector<detail::Bus> buses_;
	std::unordered_map<std::string, ID> stops_ids_;
	std::vector<detail::Stop> stops_;
	std::vector<double> distances_;
	std::vector<double> geo_distances_;
	std::vector<detail::Route> routes_;
	config::RoutingSettings settings_;
};

} // namespace transport

#endif /* TRANSPORT_DIRECTORY_IMPL_H_ */
