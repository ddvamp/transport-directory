#ifndef DDV_SVG_H_
#define DDV_SVG_H_ 1

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include "svg_color.h"
#include "util.h"

namespace svg {

using util::point;

inline void render(Color const &color, std::ostream &out)
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
			[](Rgba rgba, std::ostream &os) {
				os << "rgba(" << +rgba.red << ',' << +rgba.green << ',' <<
					+rgba.blue << ',' << rgba.alpha << ')';
			}
		}(value, out);
	}, color);
}

class PropertiesImpl {
protected:
	void render(std::ostream &os) const
	{
		os << "fill=\"";
		svg::render(fill, os);
		os << "\" stroke=\"";
		svg::render(stroke, os);
		os << "\" stroke-width=\"" << stroke_width << "\" ";
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
protected:
	using Lvalue = T &;
	using Rvalue = T &&;
public:
	Lvalue setFillColor(Color const &color) &
	{
		fill = color;
		return lvalue();
	}
	Rvalue setFillColor(Color const &color) &&
	{
		fill = color;
		return rvalue();
	}

	Lvalue setStrokeColor(Color const &color) &
	{
		stroke = color;
		return lvalue();
	}
	Rvalue setStrokeColor(Color const &color) &&
	{
		stroke = color;
		return rvalue();
	}

	Lvalue setStrokeWidth(double width) & noexcept
	{
		stroke_width = width;
		return lvalue();
	}
	Rvalue setStrokeWidth(double width) && noexcept
	{
		stroke_width = width;
		return rvalue();
	}

	Lvalue setStrokeLineCap(std::string const &linecap) &
	{
		stroke_linecap = linecap;
		return lvalue();
	}
	Rvalue setStrokeLineCap(std::string const &linecap) &&
	{
		stroke_linecap = linecap;
		return rvalue();
	}

	Lvalue setStrokeLineJoin(std::string const &linejoin) &
	{
		stroke_linejoin = linejoin;
		return lvalue();
	}
	Rvalue setStrokeLineJoin(std::string const &linejoin) &&
	{
		stroke_linejoin = linejoin;
		return rvalue();
	}

protected:
	Lvalue lvalue() noexcept
	{
		return static_cast<Lvalue>(*this);
	}
	Rvalue rvalue() noexcept
	{
		return static_cast<Rvalue>(*this);
	}
};

class Circle : public Properties<Circle> {
public:
	Lvalue setCenter(point center) & noexcept
	{
		c = center;
		return lvalue();
	}
	Rvalue setCenter(point center) && noexcept
	{
		c = center;
		return rvalue();
	}

	Lvalue setRadius(double radius) & noexcept
	{
		r = radius;
		return lvalue();
	}
	Rvalue setRadius(double radius) && noexcept
	{
		r = radius;
		return rvalue();
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
	point c = {0.0, 0.0};
	double r = 1.0;
};

class Polyline : public Properties<Polyline> {
public:
	Lvalue addPoint(point point) &
	{
		points.push_back(point);
		return lvalue();
	}
	Rvalue addPoint(point point) &&
	{
		points.push_back(point);
		return rvalue();
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
	std::vector<point> points;
};

class Text : public Properties<Text> {
public:
	Lvalue setPoint(point point) & noexcept
	{
		p = point;
		return lvalue();
	}
	Rvalue setPoint(point point) && noexcept
	{
		p = point;
		return rvalue();
	}

	Lvalue setOffset(point point) & noexcept
	{
		dp = point;
		return lvalue();
	}
	Rvalue setOffset(point point) && noexcept
	{
		dp = point;
		return rvalue();
	}

	Lvalue setFontSize(std::uint32_t size) & noexcept
	{
		font_size = size;
		return lvalue();
	}
	Rvalue setFontSize(std::uint32_t size) && noexcept
	{
		font_size = size;
		return rvalue();
	}

	Lvalue setFontFamily(std::string const &font) &
	{
		font_family = font;
		return lvalue();
	}
	Rvalue setFontFamily(std::string const &font) &&
	{
		font_family = font;
		return rvalue();
	}

	Lvalue setFontWeight(std::string const &weight) &
	{
		font_weight = weight;
		return lvalue();
	}
	Rvalue setFontWeight(std::string const &weight) &&
	{
		font_weight = weight;
		return rvalue();
	}

	Lvalue setData(std::string const &data) &
	{
		text = data;
		return lvalue();
	}
	Rvalue setData(std::string const &data) &&
	{
		text = data;
		return rvalue();
	}
	Lvalue setData(std::string_view data) &
	{
		text = data;
		return lvalue();
	}
	Rvalue setData(std::string_view data) &&
	{
		text = data;
		return rvalue();
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
		if (not font_weight.empty()) {
			os << "font-weight=\"" << font_weight << "\" ";
		}
		Properties::render(os);
		os << '>' << text << "</text>";
	}

private:
	point p = {0.0, 0.0};
	point dp = {0.0, 0.0};
	std::uint32_t font_size = 1;
	std::string font_family;
	std::string font_weight;
	std::string text;
};

using Node = std::variant<Circle, Polyline, Text>;

class Document {
public:
	template <typename ...T>
	void add(T &&...t)
	{
		nodes.emplace_back(std::forward<T>(t)...);
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

#endif /* DDV_SVG_H_ */
