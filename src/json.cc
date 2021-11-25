#include <utility>

#include "json.h"

namespace {

[[nodiscard]] json::Element readArray(std::istream &is)
{
	json::Array array;
	for (char c; is >> c && c != ']'; ) {
		if (c != ',') {
			is.putback(c);
		}
		array.push_back(json::readElement(is));
	}
	return array;
}

[[nodiscard]] json::Element readBoolean(std::istream &is)
{
	json::String str;
	while (std::isalpha(is.peek())) {
		str.push_back(static_cast<json::String::value_type>(is.get()));
	}
	return str == "true";
}

[[nodiscard]] json::Element readNumber(std::istream &is)
{
	bool is_negative = false;
	if (is.peek() == '-') {
		is_negative = true;
		is.get();
	}
	json::Integer integer{};
	while (std::isdigit(is.peek())) {
		integer *= 10;
		integer += is.get() - '0';
	}
	if (is.peek() != '.') {
		return integer * (is_negative ? -1 : 1);
	}
	is.get();
	auto number = static_cast<json::Double>(integer);
	json::Double multiplier = 0.1;
	while (std::isdigit(is.peek())) {
		number += multiplier * (is.get() - '0');
		multiplier /= 10;
	}
	return number * (is_negative ? -1 : 1);
}

[[nodiscard]] json::Element readString(std::istream &is)
{
	json::String str;
	std::getline(is, str, '"');
	return str;
}

[[nodiscard]] json::Element readObject(std::istream &is)
{
	json::Object obj;
	for (char c; is >> c && c != '}'; ) {
		if (c == ',') {
			is >> c;
		}
		auto key = readString(is);
		is >> c;
		obj.emplace(key.asString(), json::readElement(is));
	}
	return obj;
}

} // namespace

namespace json {

Element readElement(std::istream &is)
{
	char c;
	is >> c;
	switch (c) {
	case '{':
		return readObject(is);
	case '[':
		return readArray(is);
	case '"':
		return readString(is);
	case 't':
	case 'f':
		is.putback(c);
		return readBoolean(is);
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
	for (Element const &element : array) {
		if (first) {
			first = false;
		} else {
			os << ", ";
		}
		writeElement(element, os);
	}
	os << ']';
}

void writeValue(String const &string, std::ostream &os)
{
	os << '"' << string << '"';
}

void writeValue(Boolean boolean, std::ostream &os)
{
	os << std::boolalpha << boolean;
}

void writeValue(Integer number, std::ostream &os)
{
	os << number;
}

void writeValue(Double number, std::ostream &os)
{
	os << number;
}

} // namespace json
