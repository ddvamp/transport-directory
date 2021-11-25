#include "request.h"

namespace request {

json::Object Bus::process(
	transport::TransportDirectory const &directory) const
{
	json::Object response;
	response.emplace("request_id", static_cast<json::Integer>(id));
	if (auto info = directory.getBus(name)) {
		response.emplace("curvature", info->road_route_length /
			info->geo_route_length);
		response.emplace("route_length", static_cast<json::Integer>(
			info->road_route_length));
		response.emplace("stop_count", static_cast<json::Integer>(
			info->stops_count));
		response.emplace("unique_stop_count", static_cast<json::Integer>(
			info->unique_stops_count));
	} else {
		response.emplace("error_message", static_cast<json::String>(
			"not found"));
	}
	return response;
}

json::Object Stop::process(
	transport::TransportDirectory const &directory) const
{
	json::Object response;
	if (auto info = directory.getStop(name)) {
		json::Array buses;
		for (auto bus : info->buses) {
			buses.emplace_back(std::in_place_type<json::String>, bus);
		}
		response.emplace("buses", std::move(buses));
	} else {
		response.emplace("error_message", static_cast<json::String>(
			"not found"));
	}
	response.emplace("request_id", static_cast<json::Integer>(id));
	return response;
}

json::Object Route::process(
	transport::TransportDirectory const &directory) const
{
	json::Object response;
	if (auto route = directory.getRoute(from, to)) {
		auto &info = *route;
		json::Array items;
		items.reserve(info.items.size() * 2);
		for (json::Object buffer; auto const &item : info.items) {
			buffer.try_emplace("stop_name", std::in_place_type<json::String>,
				item.stop_name);
			buffer.emplace("time", item.wait_time);
			buffer.emplace("type", static_cast<json::String>("Wait"));
			items.push_back(std::move(buffer));
			buffer.clear();
			buffer.try_emplace("bus", std::in_place_type<json::String>,
				item.bus_name);
			buffer.emplace("span_count", static_cast<json::Integer>(
				item.spans_count));
			buffer.emplace("time", item.travel_time);
			buffer.emplace("type", static_cast<json::String>("Bus"));
			items.push_back(std::move(buffer));
			buffer.clear();
		}
		response.emplace("items", std::move(items));
		response.emplace("total_time", info.total_time);
	} else {
		response.emplace("error_message", static_cast<json::String>(
			"not found"));
	}
	response.emplace("request_id", static_cast<json::Integer>(id));
	return response;
}

Request parseFrom(json::Object const &node)
{
	auto const &type = node.at("type").asString();
	if (type == "Bus") {
		return Bus{
			.name = node.at("name").asString(),
			.id = static_cast<std::size_t>(node.at("id").asInteger()),
		};
	} else if (type == "Stop") {
		return Stop{
			.name = node.at("name").asString(),
			.id = static_cast<std::size_t>(node.at("id").asInteger()),
		};
	} else /* if (type == "Route") */ {
		return Route{
			.from = node.at("from").asString(),
			.to = node.at("to").asString(),
			.id = static_cast<std::size_t>(node.at("id").asInteger()),
		};
	} // else (...)
}

json::Array processAll(transport::TransportDirectory const &directory,
	json::Array const &nodes)
{
	json::Array responses;
	responses.reserve(nodes.size());
	for (auto const &node : nodes) {
		responses.emplace_back(std::visit([&directory](auto const &request) {
				return request.process(directory);
			}, parseFrom(node.asObject())));
	}
	return responses;
}

} // namespace request
