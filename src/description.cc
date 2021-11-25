#include "description.h"

namespace {

[[nodiscard]] auto parseDistances(json::Object const &nodes)
{
	transport::config::Distances distances;
	for (auto const &[stop, distance] : nodes) {
		distances.emplace_back(stop, distance.asDouble());
	}
	return distances;
}

[[nodiscard]] auto parseRoute(json::Array const &nodes, bool is_roundtrip)
{
	transport::config::Route stops;
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

[[nodiscard]] transport::config::Stop parseStop(json::Object const &node)
{
	return {
		.name = node.at("name").asString(),
		.coords = {
			.latitude = node.at("latitude").asDouble(),
			.longitude = node.at("longitude").asDouble(),
		},
		.distances = parseDistances(node.at("road_distances").asObject()),
	};
}

[[nodiscard]] transport::config::Bus parseBus(json::Object const &node)
{
	return {
		.name = node.at("name").asString(),
		.route = parseRoute(node.at("stops").asArray(),
			node.at("is_roundtrip").asBoolean()),
	};
}

[[nodiscard]] auto parseItems(json::Array const &nodes)
{
	transport::config::Items items;
	items.reserve(nodes.size());
	for (auto const &node : nodes) {
		auto const &item = node.asObject();
		if (item.at("type").asString() == "Stop") {
			items.emplace_back(parseStop(item));
		} else { // if (type == "Bus") {
			items.emplace_back(parseBus(item));
		} /* else {...} */
	}
	return items;
}

[[nodiscard]] transport::RoutingSettings parseSettings(
	json::Object const &node)
{
	return {
		.wait_time = node.at("bus_wait_time").asDouble(),
		.velocity = node.at("bus_velocity").asDouble() * 1000 / 60,
	};
}

} // namespace

transport::config::Config description::parseConfig(json::Object const &node)
{
	return {
		.items = parseItems(node.at("base_requests").asArray()),
		.settings = parseSettings(node.at("routing_settings").asObject()),
	};
}
