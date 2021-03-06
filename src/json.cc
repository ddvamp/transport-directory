#include <iomanip>
#include <utility>

#include "json.h"
#include "util.h"

namespace json {

namespace {

[[nodiscard]] Element readArray(std::istream &);
[[nodiscard]] Element readBoolean(std::istream &);
[[nodiscard]] Element readNumber(std::istream &);
[[nodiscard]] Element readObject(std::istream &);
[[nodiscard]] Element readString(std::istream &);

} // namespace json::anonymous

Element readElement(std::istream &is)
{
	char c;
	is >> c;
	switch (c) {
	case '"':
		return readString(is);
	case '[':
		return readArray(is);
	case 'f':
	case 't':
		is.putback(c);
		return readBoolean(is);
	case '{':
		return readObject(is);
	default:
		is.putback(c);
		return readNumber(is);
	}
}

Document readDocument(std::istream &is)
{
	return Document{readElement(is)};
}

void writeDocument(Document const &document, std::ostream &os)
{
	writeElement(document.getRoot(), os);
}

void writeElement(Element const &element, std::ostream &os)
{
	std::visit([&os](auto const &value) {
		writeValue(value, os);
	}, element.getBase());
}

void writeValue(Object const &object, std::ostream &os)
{
	os << '{';
	bool first = true;
	for (auto const &[key, element] : object) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		writeValue(key, os);
		os << ": ";
		writeElement(element, os);
	}
	os << '}';
}

void writeValue(Array const &array, std::ostream &os)
{
	os << '[';
	bool first = true;
	for (auto const &element : array) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		writeElement(element, os);
	}
	os << ']';
}

void writeValue(std::string const &string, std::ostream &os)
{
	os << std::quoted(string);
}

void writeValue(bool boolean, std::ostream &os)
{
	os << std::boolalpha << boolean;
}

void writeValue(Int number, std::ostream &os)
{
	os << number;
}

void writeValue(double number, std::ostream &os)
{
	os << number;
}

namespace {

Element readArray(std::istream &is)
{
	Element element{std::in_place_type<Array>};
	auto &array = element.asArray();
	for (char c; is >> c && c != ']'; ) {
		if (c != ',') {
			is.putback(c);
		}
		array.emplace_back(util::make_builder(
			[&is] { return readElement(is); }
		));
	}
	return element;
}

Element readBoolean(std::istream &is)
{
	std::string str;
	while (std::isalpha(is.peek())) {
		str.push_back(static_cast<char>(is.get()));
	}
	return str == "true";
}

Element readNumber(std::istream &is)
{
	bool is_negative = false;
	if (is.peek() == '-') {
		is_negative = true;
		is.get();
	}
	Int integer{};
	while (std::isdigit(is.peek())) {
		integer *= 10;
		integer += is.get() - '0';
	}
	if (is.peek() != '.') {
		return is_negative ? -integer : integer;
	}
	is.get();
	auto number = static_cast<double>(integer);
	auto multiplier = 0.1;
	while (std::isdigit(is.peek())) {
		number += multiplier * (is.get() - '0');
		multiplier /= 10;
	}
	return is_negative ? -number : number;
}

Element readObject(std::istream &is)
{
	Element element{std::in_place_type<Object>};
	auto &obj = element.asObject();
	for (char c; is >> c && c != '}'; ) {
		if (c == ',') {
			is >> c;
		}
		auto key = readString(is);
		is >> c;
		obj.emplace(std::move(key.asString()), util::make_builder(
			[&is] { return readElement(is); }
		));
	}
	return element;
}

Element readString(std::istream &is)
{
	Element element{std::in_place_type<std::string>};
	std::getline(is, element.asString(), '"');
	return element;
}

} // namespace json::anonymous

} // namespace json
