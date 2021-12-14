#ifndef DDV_UTIL_H_
#define DDV_UTIL_H_ 1

#include <algorithm>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "util_structures.h"

namespace util {

/**
 *	@brief		Convert a value to an lvalue.
 *	@param arg	A thing of arbitrary type.
 *	@return		The parameter implicitly converted to an lvalue-reference.
 *
 *	This function can be used to convert an prvalue to an lvalue.
 *	In this case temporary materialization occurs.
 */
[[nodiscard]] inline constexpr auto &copy(auto &&arg) noexcept { return arg; }

template <typename ...Ts>
struct overloaded : Ts... { using Ts::operator()...; };

template <typename ResultType = void, typename Callable, typename ...Args>
requires std::is_invocable_r_v<ResultType, Callable, Args...> and
    (not std::is_same_v<std::invoke_result_t<Callable, Args...>, void>)
[[nodiscard]] inline constexpr auto make_builder(
    Callable &&fn, Args &&...args) noexcept
{
	using R = std::conditional_t<
		std::is_same_v<ResultType, void>,
		std::invoke_result_t<Callable, Args...>,
		ResultType
	>;

    struct builder {
        [[nodiscard]] explicit(false) inline constexpr operator R ()
        noexcept(std::is_nothrow_invocable_r_v<R, Callable, Args...>)
        {
            return std::apply(fn_, std::move(args_));
        }

        [[no_unique_address]] Callable &&fn_;
        [[no_unique_address]] std::tuple<Args &&...> args_;
    };

    return builder{
        .fn_ = std::forward<Callable>(fn),
        .args_ = std::forward_as_tuple(std::forward<Args>(args)...),
    };
}

template <typename T>
inline constexpr void hash_combine(std::size_t &seed, T const &key) noexcept
{
	std::hash<T> hasher{};
	seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

[[nodiscard]] inline std::pair<point, point>
	getCoverageBox(std::vector<point> const &points)
{
	auto [bottom, top] = std::minmax_element(
		points.begin(),
		points.end(),
		[](point lhs, point rhs) noexcept {
			return lhs.x < rhs.x;
		}
	);
	auto [left, right] = std::minmax_element(
		points.begin(),
		points.end(),
		[](point lhs, point rhs) noexcept {
			return lhs.y < rhs.y;
		}
	);
	return {{top->x, left->y}, {bottom->x, right->y}};
}

[[nodiscard]] inline std::pair<point, double> calculateScalingFactor(
	std::vector<point> const &points, point box_size, point box_offset)
{
	auto [top_left, bottom_right] = getCoverageBox(points);

	double coverage_height = top_left.x - bottom_right.x;
	double coverage_width = bottom_right.y - top_left.y;

	double height_zoom_coef =
		coverage_height > 0.0 ?
		(box_size.x - 2 * box_offset.x) / coverage_height :
		0.0;
	double width_zoom_coef =
		coverage_width > 0.0 ?
		(box_size.y - 2 * box_offset.y) / coverage_width :
		0.0;
	double zoom_coef =
		not (height_zoom_coef > 0.0) ?
		width_zoom_coef :
		not (width_zoom_coef > 0.0) ?
		height_zoom_coef :
		std::min(height_zoom_coef, width_zoom_coef);

	return {top_left, zoom_coef};
}

} // namespace util

namespace std {

template <typename T> struct hash;

template <>
struct hash<pair<string, string>> {
	size_t operator()(pair<string, string> const &p) const noexcept
	{
		size_t seed1{};
		util::hash_combine(seed1, p.first);
		util::hash_combine(seed1, p.second);

		size_t seed2{};
		util::hash_combine(seed2, p.second);
		util::hash_combine(seed2, p.first);

		return seed2 < seed1 ? seed2 : seed1;
	}
};

} // namespace std

#endif /* DDV_UTIL_H_ */
