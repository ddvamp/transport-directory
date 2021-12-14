#include <string_view>
#include <unordered_map>

#include "description.h"

using namespace transport::config;

using json::Array;
using json::Object;

namespace description {

namespace {

[[nodiscard]] svg::Color	parseColor(json::Element const &);
[[nodiscard]] Distances		parseDistances(Object const &);
[[nodiscard]] Item			parseItem(Object const &);
[[nodiscard]] Items			parseItems(Array const &);
[[nodiscard]] Layers		parseLayers(Array const &);
[[nodiscard]] Palette		parsePalette(Array const &);
[[nodiscard]] util::point	parsePoint(Array const &);
[[nodiscard]] Route			parseRoute(Array const &, bool is_roundtrip);

} // namespace description::anonymous

Bus parseBus(Object const &node)
{
	return {
		.name = node.at("name").asString(),
		.route = parseRoute(
			node.at("stops").asArray(),
			node.at("is_roundtrip").asBoolean()
		),
		.is_roundtrip = node.at("is_roundtrip").asBoolean(),
	};
}

Stop parseStop(Object const &node)
{
	return {
		.name = node.at("name").asString(),
		.coords = {
			.x = node.at("latitude").asDouble(),
			.y = node.at("longitude").asDouble(),
		},
		.distances = parseDistances(
			node.at("road_distances").asObject()
		),
	};
}

RoutingSettings parseRoutingSettings(Object const &node)
{
	return {
		.wait_time = node.at("bus_wait_time").asDouble(),
		.velocity = node.at("bus_velocity").asDouble() * 1000 / 60,
	};
}

RenderSettings parseRenderSettings(Object const &node)
{
	return {
		.width = node.at("width").asDouble(),
		.height = node.at("height").asDouble(),
		.padding = node.at("padding").asDouble(),
		.stop_radius = node.at("stop_radius").asDouble(),
		.line_width = node.at("line_width").asDouble(),
		.bus_label_font_size = static_cast<std::uint32_t>(
			node.at("bus_label_font_size").asInteger()
		),
		.bus_label_offset = parsePoint(
			node.at("bus_label_offset").asArray()
		),
		.stop_label_font_size = static_cast<std::uint32_t>(
			node.at("stop_label_font_size").asInteger()
		),
		.stop_label_offset = parsePoint(
			node.at("stop_label_offset").asArray()
		),
		.underlayer_color = parseColor(node.at("underlayer_color")),
		.underlayer_width = node.at("underlayer_width").asDouble(),
		.color_palette = parsePalette(node.at("color_palette").asArray()),
		.layers = parseLayers(node.at("layers").asArray()),
	};
}

Config parseConfig(Object const &node)
{
	return {
		.items = parseItems(node.at("base_requests").asArray()),
		.routing_settings = parseRoutingSettings(
			node.at("routing_settings").asObject()
		),
		.render_settings = parseRenderSettings(
			node.at("render_settings").asObject()
		),
	};
}

namespace {

svg::Color parseColor(json::Element const &node)
{
	if (auto str = std::get_if<std::string>(&node.getBase())) {
		return *str;
	}
	auto const &nodes = std::get<Array>(node.getBase());
	if (nodes.size() == 3) {
		return svg::Rgb {
			.red = static_cast<std::uint8_t>(nodes[0].asInteger()),
			.green = static_cast<std::uint8_t>(nodes[1].asInteger()),
			.blue = static_cast<std::uint8_t>(nodes[2].asInteger()),
		};
	}
	return svg::Rgba {
		.red = static_cast<std::uint8_t>(nodes[0].asInteger()),
		.green = static_cast<std::uint8_t>(nodes[1].asInteger()),
		.blue = static_cast<std::uint8_t>(nodes[2].asInteger()),
		.alpha = nodes[3].asDouble(),
	};
}

Distances parseDistances(Object const &nodes)
{
	Distances distances;
	distances.reserve(nodes.size());
	for (auto const &[stop, distance] : nodes) {
		distances.emplace_back(stop, distance.asDouble());
	}
	return distances;
}

Item parseItem(Object const &node)
{
	static std::unordered_map<std::string_view, decltype(&parseItem)> const
	parser = {
		{"Bus",		[](Object const &n) { return Item{parseBus(n)}; }},
		{"Stop",	[](Object const &n) { return Item{parseStop(n)}; }},
	};
	return parser.at(node.at("type").asString())(node);
}

Items parseItems(Array const &nodes)
{
	Items items;
	items.reserve(nodes.size());
	for (auto const &node : nodes) {
		items.emplace_back(parseItem(node.asObject()));
	}
	return items;
}

Layers parseLayers(Array const &nodes)
{
	Layers layers;
	layers.reserve(nodes.size());
	for (auto const &node : nodes) {
		layers.push_back(node.asString());
	}
	return layers;
}

Palette parsePalette(Array const &nodes)
{
	Palette palette;
	palette.reserve(nodes.size());
	for (auto const &node : nodes) {
		palette.push_back(parseColor(node));
	}
	return palette;
}

util::point parsePoint(Array const &nodes)
{
	return {
		.x = nodes[0].asDouble(),
		.y = nodes[1].asDouble(),
	};
}

Route parseRoute(Array const &nodes, bool is_roundtrip)
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

} // namespace description::anonymous

} // namespace description
