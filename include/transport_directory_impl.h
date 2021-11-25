#ifndef TRANSPORT_DIRECTORY_IMPL_H_
#define TRANSPORT_DIRECTORY_IMPL_H_ 1

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "transport_directory_info.h"
#include "transport_directory_setup.h"

namespace transport {

namespace detail {

using Id = std::uint32_t;

struct Stop {
	std::string name;
	Coordinates coords;
	std::set<Id> adjacent;
	std::set<Id> buses;
};

struct Bus {
	std::string name;
	std::vector<Id> route;
};

struct Route {
	struct Span {
		Id from;
		Id to;
		Id bus;
		std::uint32_t span_count;
	};

	struct Transfer {
		Route const *from;
		Route const *to;
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
		std::string const &from, std::string const &to) const;

private:
	using Id = detail::Id;
	using Route = detail::Route;

	Id registerStop(std::string const &stop);
	Id registerBus(std::string const &bus);
	[[nodiscard]] double &getDistance(Id from, Id to);
	[[nodiscard]] double const &getDistance(Id from, Id to) const;
	[[nodiscard]] Route &getRoute(Id from, Id to);
	[[nodiscard]] Route const &getRoute(Id from, Id to) const;
	[[nodiscard]] double computeRoadRouteLength(
		std::vector<Id> const &route) const;
	[[nodiscard]] double computeGeoRouteLength(
		std::vector<Id> const &route) const;

	void computeRoutes();
	void fillRoutes();
	void executeWFI();

private:
	std::unordered_map<std::string, Id> stops_ids_;
	std::vector<detail::Stop> stops_;
	std::unordered_map<std::string, Id> buses_ids_;
	std::vector<detail::Bus> buses_;
	std::vector<double> distances_;
	std::vector<detail::Route> routes_;
	RoutingSettings settings_;
};

} // namespace transport

#endif /* TRANSPORT_DIRECTORY_IMPL_H_ */
