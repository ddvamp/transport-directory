#ifndef REQUEST_H_
#define REQUEST_H_ 1

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "json.h"
#include "transport_directory.h"

namespace request {

struct Bus {
	std::string name;
	std::size_t id;

	[[nodiscard]] json::Object
	process(transport::TransportDirectory const &directory) const;
};

struct Stop {
	std::string name;
	std::size_t id;

	[[nodiscard]] json::Object
	process(transport::TransportDirectory const &directory) const;
};

struct Route {
	std::string from;
	std::string to;
	std::size_t id;

	[[nodiscard]] json::Object
	process(transport::TransportDirectory const &directory) const;
};

using Request = std::variant<Stop, Bus, Route>;

[[nodiscard]] Request parseFrom(json::Object const &node);

[[nodiscard]] json::Array
processAll(transport::TransportDirectory const &directory,
	json::Array const &nodes);

} // namespace request

#endif /* REQUEST_H_ */
