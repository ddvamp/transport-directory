#ifndef GEO_MATH_H_
#define GEO_MATH_H_ 1

#include <cmath>

namespace geo {

inline constexpr double kEarthRadius = 6'371'000;
inline constexpr double kPi = 3.141592653589793;

struct Point {
	double latitude;
	double longitude;
};

[[nodiscard]] inline double convertDegreesToRadians(double degrees) noexcept
{
	return degrees * (kPi / 180.0);
}

[[nodiscard]] inline Point convertGeoCoordinatesToRadians(Point p) noexcept
{
	return {
		.latitude = convertDegreesToRadians(p.latitude),
		.longitude = convertDegreesToRadians(p.longitude),
	};
}

[[nodiscard]] inline double computeGeoDistance(Point x, Point y) noexcept
{
	x = convertGeoCoordinatesToRadians(x);
	y = convertGeoCoordinatesToRadians(y);
	auto a = std::cos(x.latitude + y.latitude);
	auto b = std::cos(x.latitude - y.latitude);
	auto c = std::cos(x.longitude - y.longitude);
	return std::acos((a + b) * (1 + c) / 2 - a) * kEarthRadius;
}

} // namespace geo

#endif /* GEO_MATH_H_ */
