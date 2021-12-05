#include <string_view>
#include <unordered_map>

#include "description.h"

using namespace transport::config;

using json::Array;
using json::Object;

namespace description {

namespace {

[[nodiscard]] Distances parseDistances(Object const &nodes)
{
	Distances distances;
	distances.reserve(nodes.size());
	for (auto const &[stop, distance] : nodes) {
		distances.emplace_back(stop, distance.asDouble());
	}
	return distances;
}

[[nodiscard]] Route parseRoute(Array const &nodes, bool is_roundtrip)
{
	Route stops;
	stops.reserve(is_roundtrip ? nodes.size() : 2 * nodes.size() - 1);
	for (auto const &stop : nodes) {
		stops.push_back(stop.asString());
	}
	if (is_roundtrip) {
		return stops;
	}
	for (auto i = nodes.size() - 1; i-- != 0; ) {
		stops.push_back(stops[i]);
	}
	return stops;
}

[[nodiscard]] Item parseItem(Object const &node)
{
	static std::unordered_map<std::string_view, decltype(&parseItem)> const
	parser = {
		{"Bus", [](Object const &n) { return Item{parseBus(n)}; }},
		{"Stop", [](Object const &n) { return Item{parseStop(n)}; }},
	};
	return parser.at(node.at("type").asString())(node);
}

[[nodiscard]] Items parseItems(Array const &nodes)
{
	Items items;
	items.reserve(nodes.size());
	for (auto const &node : nodes) {
		items.emplace_back(parseItem(node.asObject()));
	}
	return items;
}

[[nodiscard]] RoutingSettings parseSettings(Object const &node)
{
	return {
		.wait_time = node.at("bus_wait_time").asDouble(),
		.velocity = node.at("bus_velocity").asDouble() * 1000 / 60,
	};
}

} // namespace description::anonymous

Bus parseBus(Object const &node)
{
	return {
		.name = node.at("name").asString(),
		.route = parseRoute(
			node.at("stops").asArray(),
			node.at("is_roundtrip").asBoolean()
		),
	};
}

Stop parseStop(Object const &node)
{
	return {
		.name = node.at("name").asString(),
		.latitude = node.at("latitude").asDouble(),
		.longitude = node.at("longitude").asDouble(),
		.distances = parseDistances(
			node.at("road_distances").asObject()
		),
	};
}

Config parseConfig(Object const &node)
{
	return {
		.items = parseItems(node.at("base_requests").asArray()),
		.settings = parseSettings(node.at("routing_settings").asObject()),
	};
}

} // namespace description
