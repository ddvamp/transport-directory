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

struct Stop {
	std::string name;
	util::point coords;
	std::unordered_set<ID> adjacent;
	std::unordered_set<ID> buses;
};

struct Bus {
	std::string name;
	std::vector<ID> route;
	bool is_roundtrip;
};

struct Route {
	struct Span {
		ID from;
		ID bus;
		std::uint16_t spans_count;
	};

	struct Transfer {
		ID from;
		ID middle;
		ID to;
	};

	using Item = std::variant<Span, Transfer>;

	double time;
	Item item;
};

} // namespace transport::detail

} // namespace transport

#endif /* DDV_TRANSPORT_DIRECTORY_DETAIL_H_ */
