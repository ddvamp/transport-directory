#ifndef TRANSPORT_DIRECTORY_SETUP_H_
#define TRANSPORT_DIRECTORY_SETUP_H_ 1

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace transport {

struct Coordinates {
	double latitude;
	double longitude;
};

struct RoutingSettings {
	double wait_time;
	double velocity;
};

namespace config {

using Distances = std::vector<std::pair<std::string, double>>;

struct Stop {
	std::string name;
	Coordinates coords;
	Distances distances;
};

using Route = std::vector<std::string>;

struct Bus {
	std::string name;
	Route route;
};

using Item = std::variant<Stop, Bus>;
using Items = std::vector<Item>;

struct Config {
	Items items;
	RoutingSettings settings;
};

} // namespace transport::config

} // namespace transport

#endif /* TRANSPORT_DIRECTORY_SETUP_H_ */
