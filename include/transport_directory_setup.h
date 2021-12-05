#ifndef TRANSPORT_DIRECTORY_SETUP_H_
#define TRANSPORT_DIRECTORY_SETUP_H_ 1

#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace transport::config {

struct RoutingSettings {
	double wait_time;
	double velocity;
};

using Distances = std::vector<std::pair<std::string, double>>;

struct Stop {
	std::string name;
	double latitude;
	double longitude;
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

#endif /* TRANSPORT_DIRECTORY_SETUP_H_ */
