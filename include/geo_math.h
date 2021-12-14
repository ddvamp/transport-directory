#ifndef DDV_GEO_MATH_H_
#define DDV_GEO_MATH_H_ 1

#include <cmath>

#include "util_structures.h"

namespace geo {

inline constexpr double kEarthRadius = 6'371'000;
inline constexpr double kPi = 3.141592653589793;

using util::point;

[[nodiscard]] inline double convertDegreesToRadians(double degrees) noexcept
{
	return degrees * (kPi / 180.0);
}

[[nodiscard]] inline point convertGeoCoordinatesToRadians(point p) noexcept
{
	return {
		.x = convertDegreesToRadians(p.x),
		.y = convertDegreesToRadians(p.y),
	};
}

[[nodiscard]] inline double computeGeoDistance(point lhs, point rhs) noexcept
{
	lhs = convertGeoCoordinatesToRadians(lhs);
	rhs = convertGeoCoordinatesToRadians(rhs);
	auto a = std::cos(lhs.x + rhs.x);
	auto b = std::cos(lhs.x - rhs.x);
	auto c = std::cos(lhs.y - rhs.y);
	return std::acos((a + b) * (1 + c) / 2 - a) * kEarthRadius;
}

} // namespace geo

#endif /* DDV_GEO_MATH_H_ */
