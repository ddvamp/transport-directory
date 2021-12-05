#include <cstddef>
#include <string_view>
#include <unordered_map>

#include "request.h"

using json::Array;
using json::Int;
using json::Object;

namespace request {

namespace {

Object processBus(Object const &node,
	transport::TransportDirectory const &directory)
{
	Object response;
	response.emplace("request_id", node.at("id"));
	if (auto info = directory.getBus(node.at("name").asString())) {
		response.emplace_hint(
			response.begin(),
			"curvature",
			info->road_route_length / info->geo_route_length
		);
		response.emplace_hint(
			response.end(),
			"route_length",
			static_cast<Int>(info->road_route_length)
		);
		response.emplace_hint(
			response.end(),
			"stop_count",
			static_cast<Int>(info->stops_count)
		);
		response.emplace_hint(
			response.end(),
			"unique_stop_count",
			static_cast<Int>(info->unique_stops_count)
		);
	} else {
		response.emplace_hint(
			response.begin(),
			"error_message",
			std::string{"not found"}
		);
	}
	return response;
}

Object processStop(Object const &node,
	transport::TransportDirectory const &directory)
{
	Object response;
	response.emplace("request_id", node.at("id"));
	if (auto info = directory.getStop(node.at("name").asString())) {
		auto &buses = response.emplace_hint(
			response.begin(),
			"buses",
			std::in_place_type<Array>
		)->second.asArray();

		buses.reserve(info->buses.size());
		for (auto bus : info->buses) {
			buses.emplace_back(std::in_place_type<std::string>, bus);
		}
	} else {
		response.emplace_hint(
			response.begin(),
			"error_message",
			std::string{"not found"}
		);
	}
	return response;
}

Object processRoute(Object const &node,
	transport::TransportDirectory const &directory)
{
	Object response;
	response.emplace("request_id", node.at("id"));
	if (auto route = directory.getRoute(node.at("from").asString(),
			node.at("to").asString())) {
		response.emplace_hint(response.end(), "total_time", route->total_time);
		auto &items =
			response.emplace_hint(
				response.begin(),
				"items",
				std::in_place_type<Array>
			)->second.asArray();

		items.reserve(2 * route->items.size());
		for (auto const &item : route->items) {
			auto &wait =
				items.emplace_back(std::in_place_type<Object>).asObject();

			wait.emplace("stop_name", std::string{item.stop_name});
			wait.emplace_hint(wait.end(), "time", item.wait_time);
			wait.emplace_hint(wait.end(), "type", std::string{"Wait"});

			auto &bus =
				items.emplace_back(std::in_place_type<Object>).asObject();

			bus.emplace("bus", std::string{item.bus_name});
			bus.emplace_hint(
				bus.end(),
				"span_count",
				static_cast<Int>(item.spans_count)
			);
			bus.emplace_hint(bus.end(), "time", item.travel_time);
			bus.emplace_hint(bus.end(), "type", std::string{"Bus"});
		}
	} else {
		response.emplace_hint(
			response.begin(),
			"error_message",
			std::string{"not found"}
		);
	}
	return response;
}

} // namespace request::anonymous

Object process(Object const &node,
	transport::TransportDirectory const &directory)
	
{
	static std::unordered_map<std::string_view, decltype(&process)> const
	processor = {
		{"Bus", processBus},
		{"Stop", processStop},
		{"Route", processRoute},
	};
	return processor.at(node.at("type").asString())(node, directory);
}

Array processAll(Array const &nodes,
	transport::TransportDirectory const &directory)
	
{
	Array responses;
	responses.reserve(nodes.size());
	for (auto const &node : nodes) {
		responses.emplace_back(process(node.asObject(), directory));
	}
	return responses;
}

} // namespace request
