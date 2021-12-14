#ifndef DDV_TRANSPORT_DIRECTORY_INFO_H_
#define DDV_TRANSPORT_DIRECTORY_INFO_H_ 1

#include <cstddef>
#include <string_view>
#include <vector>

namespace transport::info {

struct Stop {
	std::vector<std::string_view> buses;
};

struct Bus {
	std::size_t stops_count;
	std::size_t unique_stops_count;
	double road_route_length;
	double geo_route_length;
};

struct Route {
	struct Span {
		std::string_view stop_name;
		double wait_time;

		std::string_view bus_name;
		double travel_time;
		std::size_t spans_count;
	};

	using Items = std::vector<Span>;

	Items items;
	double total_time{};
};

struct Map {
	std::string_view data;
};

} // namespace transport::info

#endif /* DDV_TRANSPORT_DIRECTORY_INFO_H_ */
