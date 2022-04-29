#include <algorithm>
#include <cstddef>
#include <limits>
#include <numeric>
#include <sstream>
#include <string_view>
#include <unordered_map>

#include "svg.h"
#include "transport_directory_renderer.h"

namespace transport {

std::string TransportDirectoryRenderer::renderMap() const
{
	static std::unordered_map<
		std::string_view,
		void(TransportDirectoryRenderer::*)(svg::Document &) const
	> const renderer = {
		{"bus_lines",	&TransportDirectoryRenderer::renderBusLines},
		{"bus_labels",	&TransportDirectoryRenderer::renderBusLabels},
		{"stop_points",	&TransportDirectoryRenderer::renderStopPoints},
		{"stop_labels",	&TransportDirectoryRenderer::renderStopLabels},
	};

	svg::Document map;

	for (auto const &layer : settings_.layers) {
		(this->*renderer.at(layer))(map);
	}

	std::ostringstream os;
	map.render(os);
	return std::move(os).str();
}

TransportDirectoryRenderer::TransportDirectoryRenderer(
	std::vector<detail::Bus> const &buses,
	std::vector<detail::Stop> const &stops,
	config::RenderSettings const &settings
)
	: buses_{buses}
	, stops_{stops}
	, settings_{settings}
	, sorted_bus_ids_(buses.size())
	, sorted_stop_ids_(stops.size())
	, scaled_stop_coords_{produceScaledStopCoords()}
{
	std::iota(sorted_bus_ids_.begin(), sorted_bus_ids_.end(), 0);
	std::sort(
		sorted_bus_ids_.begin(),
		sorted_bus_ids_.end(),
		[&buses](auto lhs, auto rhs) {
			return buses[lhs].name < buses[rhs].name;
		}
	);

	std::iota(sorted_stop_ids_.begin(), sorted_stop_ids_.end(), 0);
	std::sort(
		sorted_stop_ids_.begin(),
		sorted_stop_ids_.end(),
		[&stops](auto lhs, auto rhs) {
			return stops[lhs].name < stops[rhs].name;
		}
	);
}

std::vector<util::point> TransportDirectoryRenderer::
	produceScaledStopCoords() const
{
	std::vector<util::point> coords;
	coords.reserve(stops_.size());
	for (auto const &stop : stops_) {
		coords.push_back(stop.coords);
	}
	auto [origin, zoom_coef] = util::calculateScalingFactor(
		coords,
		{.x = settings_.height, .y = settings_.width},
		{.x = settings_.padding, .y = settings_.padding}
	);
	for (auto &p : coords) {
		auto cp = p;
		p.x = (cp.y - origin.y) * zoom_coef + settings_.padding;
		p.y = (origin.x - cp.x) * zoom_coef + settings_.padding;
	}
	return coords;
}

void TransportDirectoryRenderer::renderBusLines(svg::Document &map) const
{
	for (std::size_t iteration{}; auto bus_id : sorted_bus_ids_) {
		svg::Polyline line;
		line.setStrokeColor(settings_.color_palette[
			iteration++ % settings_.color_palette.size()
		]);
		line.setStrokeWidth(settings_.line_width);
		line.setStrokeLineCap("round");
		line.setStrokeLineJoin("round");
		for (auto stop_id : buses_[bus_id].route) {
			line.addPoint(scaled_stop_coords_[stop_id]);
		}
		map.add(std::move(line));
	}
}

void TransportDirectoryRenderer::renderBusLabels(svg::Document &map) const
{
	for (std::size_t iteration{}; auto bus_id : sorted_bus_ids_) {
		std::vector ids{
			buses_[bus_id].route.front(),
			buses_[bus_id].route[buses_[bus_id].route.size() / 2],
		};
		if (buses_[bus_id].is_roundtrip or ids.front() == ids.back()) {
			ids.pop_back();
		}
		for (auto color_index = iteration++ % settings_.color_palette.size();
			auto stop_id : ids) {
			map.add(svg::Text{}.
				setFillColor(settings_.underlayer_color).
				setStrokeColor(settings_.underlayer_color).
				setStrokeWidth(settings_.underlayer_width).
				setStrokeLineCap("round").
				setStrokeLineJoin("round").
				setPoint(scaled_stop_coords_[stop_id]).
				setOffset(settings_.bus_label_offset).
				setFontSize(settings_.bus_label_font_size).
				setFontFamily("Verdana").
				setFontWeight("bold").
				setData(buses_[bus_id].name)
			);
			map.add(svg::Text{}.
				setFillColor(settings_.color_palette[color_index]).
				setPoint(scaled_stop_coords_[stop_id]).
				setOffset(settings_.bus_label_offset).
				setFontSize(settings_.bus_label_font_size).
				setFontFamily("Verdana").
				setFontWeight("bold").
				setData(buses_[bus_id].name)
			);
		}
	}
}

void TransportDirectoryRenderer::renderStopPoints(svg::Document &map) const
{
	for (auto stop_id : sorted_stop_ids_) {
		map.add(svg::Circle{}.
			setCenter(scaled_stop_coords_[stop_id]).
			setRadius(settings_.stop_radius).
			setFillColor("white")
		);
	}
}

void TransportDirectoryRenderer::renderStopLabels(svg::Document &map) const
{
	for (auto stop_id : sorted_stop_ids_) {
		map.add(svg::Text{}.
			setFillColor(settings_.underlayer_color).
			setStrokeColor(settings_.underlayer_color).
			setStrokeWidth(settings_.underlayer_width).
			setStrokeLineCap("round").
			setStrokeLineJoin("round").
			setPoint(scaled_stop_coords_[stop_id]).
			setOffset(settings_.stop_label_offset).
			setFontSize(settings_.stop_label_font_size).
			setFontFamily("Verdana").
			setData(stops_[stop_id].name)
		);
		map.add(svg::Text{}.
			setFillColor("black").
			setPoint(scaled_stop_coords_[stop_id]).
			setOffset(settings_.stop_label_offset).
			setFontSize(settings_.stop_label_font_size).
			setFontFamily("Verdana").
			setData(stops_[stop_id].name)
		);
	}
}

} // namespace transport
