#ifndef SVG_H_
#define SVG_H_ 1

#include <cstdint>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

#include "util.h"

namespace svg {

using util::Point;

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

std::ostream &operator<< (std::ostream &, Color const &);

inline std::ostream &operator<< (std::ostream &out, Color const &color)
{
	std::visit([&out](auto const &value) {
		util::overloaded{
			[](std::monostate, std::ostream &os) {
				os << "none";
			},
			[](std::string const &str, std::ostream &os) {
				os << str;
			},
			[](Rgb rgb, std::ostream &os) {
				os << "rgb(" << +rgb.red << ',' << +rgb.green << ',' <<
					+rgb.blue << ')';
			},
			[](Rgba rgba, astd::ostream &os) {
				os << "rgba(" << +rgba.red << ',' << +rgba.green << ',' <<
					+rgba.blue << ',' << rgba.alpha << ')';
			}
		}(value, out);
	}, color);
	return out;
}

class PropertiesImpl {
protected:
	void render(std::ostream &os) const
	{
		os << "fill=\"" << fill << "\" "
			"stroke=\"" << stroke << "\" "
			"stroke-width=\"" << stroke_width << "\" ";
		if (not stroke_linecap.empty()) {
			os << "stroke-linecap=\"" << stroke_linecap << "\" ";
		}
		if (not stroke_linejoin.empty()) {
			os << "stroke-linejoin=\"" << stroke_linejoin << "\" ";
		}
	}

protected:
	Color fill;
	Color stroke;
	double stroke_width = 1.0;
	std::string stroke_linecap;
	std::string stroke_linejoin;
};

template <typename T>
class Properties : protected PropertiesImpl {
public:
	T &setFillColor(Color const &color)
	{
		fill = color;
		return self();
	}

	T &setStrokeColor(Color const &color)
	{
		stroke = color;
		return self();
	}

	T &setStrokeWidth(double width) noexcept
	{
		stroke_width = width;
		return self();
	}

	T &setStrokeLineCap(std::string const &linecap)
	{
		stroke_linecap = linecap;
		return self();
	}

	T &setStrokeLineJoin(std::string const &linejoin)
	{
		stroke_linejoin = linejoin;
		return self();
	}

private:
	T &self() noexcept
	{
		return static_cast<T &>(*this);
	}
};

class Circle : public Properties<Circle> {
public:
	Circle &setCenter(Point center) noexcept
	{
		c = center;
		return *this;
	}

	Circle &setRadius(double radius) noexcept
	{
		r = radius;
		return *this;
	}

	void render(std::ostream &os) const
	{
		os << "<circle "
			"cx=\"" << c.x << "\" "
			"cy=\"" << c.y << "\" "
			"r=\"" << r << "\" ";
		Properties::render(os);
		os << "/>";
	}

private:
	Point c = {0.0, 0.0};
	double r = 1.0;
};

class Polyline : public Properties<Polyline> {
public:
	Polyline &addPoint(Point point)
	{
		points.push_back(point);
		return *this;
	}

	void render(std::ostream &os) const
	{
		os << "<polyline "
			"points=\"";
		for (auto p : points) {
			os << p.x << ',' << p.y << ' ';
		}
		os << "\" ";
		Properties::render(os);
		os << "/>";
	}

private:
	std::vector<Point> points;
};

class Text : public Properties<Text> {
public:
	Text &setPoint(Point point) noexcept
	{
		p = point;
		return *this;
	}

	Text &setOffset(Point point) noexcept
	{
		dp = point;
		return *this;
	}

	Text &setFontSize(std::uint32_t size) noexcept
	{
		font_size = size;
		return *this;
	}

	Text &setFontFamily(std::string const &font)
	{
		font_family = font;
		return *this;
	}

	Text &setData(std::string const &data)
	{
		text = data;
		return *this;
	}

	void render(std::ostream &os) const
	{
		os << "<text x=\"" << p.x << "\" "
			"y=\"" << p.y << "\" "
			"dx=\"" << dp.x << "\" "
			"dy=\"" << dp.y << "\" "
			"font-size=\"" << font_size << "\" ";
		if (not font_family.empty()) {
			os << "font-family=\"" << font_family << "\" ";
		}
		Properties::render(os);
		os << '>' << text << "</text>";
	}

private:
	Point p = {0.0, 0.0};
	Point dp = {0.0, 0.0};
	std::uint32_t font_size = 1;
	std::string font_family;
	std::string text;
};

using Node = std::variant<Circle, Polyline, Text>;

class Document {
public:
	void add(Node node)
	{
		nodes.push_back(std::move(node));
	}

	void render(std::ostream &os) const
	{
		os << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
			"<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">";
		for (auto const &node : nodes) {
			std::visit([&os](auto const &value) {
				value.render(os);
			}, node);
		}
		os << "</svg>";
	}
	
private:
	std::vector<Node> nodes;
};

} // namespace svg

#endif /* SVG_H_ */
