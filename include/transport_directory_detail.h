#ifndef DDV_TRANSPORT_DIRECTORY_DETAIL_H_
#define DDV_TRANSPORT_DIRECTORY_DETAIL_H_ 1

#include <cstdint>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include "utils_structures.h"

namespace transport {

using Id = std::uint16_t;

namespace detail {

using StopId = Id;
using BusId = Id;

struct Bus {
	BusId id;
	std::string_view name;
	std::vector<StopId> route;
	bool is_roundtrip;
};

struct Stop {
	StopId id;
	std::string_view name;
	utils::point coords;
	std::unordered_set<StopId> adjacents;
	std::unordered_set<BusId> buses;
};

struct Route {
	struct Span {
		StopId from;
		BusId bus;
		std::uint16_t spans_count;
	};

	struct Transfer {
		StopId from;
		StopId middle;
		StopId to;
	};

	using Item = std::variant<Span, Transfer>;

	double time;
	Item item;
};

} // namespace transport::detail

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_DETAIL_H_ */
