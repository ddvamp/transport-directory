#ifndef DDV_TRANSPORT_DIRECTORY_DETAIL_H_
#define DDV_TRANSPORT_DIRECTORY_DETAIL_H_ 1

#include <cstdint>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "util_structures.h"

namespace transport {

using ID = std::uint16_t;

namespace detail {

using StopID = ID;
using BusID = ID;

struct Stop {
	std::string name;
	util::point coords;
	std::unordered_set<StopID> adjacent;
	std::unordered_set<BusID> buses;
};

struct Bus {
	std::string name;
	std::vector<StopID> route;
	bool is_roundtrip;
};

struct Route {
	struct Span {
		StopID from;
		BusID bus;
		std::uint16_t spans_count;
	};

	struct Transfer {
		StopID from;
		StopID middle;
		StopID to;
	};

	using Item = std::variant<Span, Transfer>;

	double time;
	Item item;
};

} // namespace transport::detail

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_DETAIL_H_ */
