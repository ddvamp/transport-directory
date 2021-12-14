#ifndef DDV_TRANSPORT_DIRECTORY_CONFIG_H_
#define DDV_TRANSPORT_DIRECTORY_CONFIG_H_ 1

#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "svg_color.h"
#include "util_structures.h"

namespace transport::config {

using Distances = std::vector<std::pair<std::string, double>>;

struct Stop {
	std::string name;
	util::point coords;
	Distances distances;
};

using Route = std::vector<std::string>;

struct Bus {
	std::string name;
	Route route;
	bool is_roundtrip;
};

using Item = std::variant<Stop, Bus>;
using Items = std::vector<Item>;

struct RoutingSettings {
	double wait_time;
	double velocity;
};

using Palette = std::vector<svg::Color>;
using Layers = std::vector<std::string>;

struct RenderSettings {
	double width;
	double height;
	double padding;
	double stop_radius;
	double line_width;
	std::uint32_t bus_label_font_size;
	util::point bus_label_offset;
	std::uint32_t stop_label_font_size;
	util::point stop_label_offset;
	svg::Color underlayer_color;
	double underlayer_width;
	Palette color_palette;
	Layers layers;
};

struct Config {
	Items items;
	RoutingSettings routing_settings;
	RenderSettings render_settings;
};

} // namespace transport::config

#endif /* DDV_TRANSPORT_DIRECTORY_CONFIG_H_ */
