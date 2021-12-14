#ifndef DDV_JSON_H_
#define DDV_JSON_H_ 1

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace json {

using Object = std::map<std::string, class Element>;
using Array = std::vector<Element>;
using Int = std::int64_t;

class Element : std::variant<Object, Array, std::string,
	Int, double, bool> {
public:
	using variant::variant;
	[[nodiscard]] variant &getBase()
	{
		return *this;
	}
	[[nodiscard]] variant const &getBase() const
	{
		return *this;
	}

	[[nodiscard]] Object &asObject()
	{
		return std::get<Object>(*this);
	}
	[[nodiscard]] Object const &asObject() const
	{
		return std::get<Object>(*this);
	}

	[[nodiscard]] Array &asArray()
	{
		return std::get<Array>(*this);
	}
	[[nodiscard]] Array const &asArray() const
	{
		return std::get<Array>(*this);
	}

	[[nodiscard]] std::string &asString()
	{
		return std::get<std::string>(*this);
	}
	[[nodiscard]] std::string const &asString() const
	{
		return std::get<std::string>(*this);
	}

	[[nodiscard]] Int asInteger() const
	{
		return std::get<Int>(*this);
	}

	[[nodiscard]] double asDouble() const
	{
		return std::holds_alternative<double>(*this) ?
			std::get<double>(*this) :
			static_cast<double>(std::get<Int>(*this));
	}

	[[nodiscard]] bool asBoolean() const
	{
		return std::get<bool>(*this);
	}
};

class Document {
public:
	explicit Document(Element element)
		: root{std::move(element)}
	{
	}

	[[nodiscard]] Element &getRoot()
	{
		return root;
	}
	[[nodiscard]] Element const &getRoot() const
	{
		return root;
	}

private:
	Element root;
};

[[nodiscard]] Element readElement(std::istream &);

[[nodiscard]] Document readDocument(std::istream &);

void writeDocument(Document const &, std::ostream &);

void writeElement(Element const &, std::ostream &);

template <typename Value>
void writeValue(Value const &, std::ostream &) = delete;

void writeValue(Object const &, std::ostream &);

void writeValue(Array const &, std::ostream &);

void writeValue(std::string const &, std::ostream &);

void writeValue(Int, std::ostream &);

void writeValue(double, std::ostream &);

void writeValue(bool, std::ostream &);

} // namespace json

#endif /* DDV_JSON_H_ */
