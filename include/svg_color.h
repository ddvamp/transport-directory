#ifndef DDV_SVG_COLOR_H_
#define DDV_SVG_COLOR_H_ 1

#include <cstdint>
#include <string>
#include <variant>

namespace svg {

struct Rgb {
	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
};

struct Rgba {
	std::uint8_t red;
	std::uint8_t green;
	std::uint8_t blue;
	double alpha;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

} // namespace svg

#endif /* DDV_SVG_COLOR_H_ */
