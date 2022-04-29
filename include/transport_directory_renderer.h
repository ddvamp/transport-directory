#ifndef DDV_TRANSPORT_DIRECTORY_RENDERER_H_
#define DDV_TRANSPORT_DIRECTORY_RENDERER_H_ 1

#include <string>
#include <utility>
#include <vector>

#include "transport_directory_detail.h"
#include "transport_directory_config.h"
#include "util_structures.h"

namespace svg {

class Document;

} // namespace svg

namespace transport {

class TransportDirectoryRenderer {
public:
	TransportDirectoryRenderer(std::vector<detail::Bus> const &,
		std::vector<detail::Stop> const &, config::RenderSettings const &);

	[[nodiscard]] std::string renderMap() const;

private:
	[[nodiscard]] std::vector<util::point> produceScaledStopCoords() const;

	void renderBusLines(svg::Document &) const;
	void renderBusLabels(svg::Document &) const;
	void renderStopPoints(svg::Document &) const;
	void renderStopLabels(svg::Document &) const;

private:
	std::vector<detail::Bus> const &buses_;
	std::vector<detail::Stop> const &stops_;
	config::RenderSettings const &settings_;
	std::vector<detail::BusID> sorted_bus_ids_;
	std::vector<detail::StopID> sorted_stop_ids_;
	std::vector<util::point> scaled_stop_coords_;
};

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_RENDERER_H_ */
