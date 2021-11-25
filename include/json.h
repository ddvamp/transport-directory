#ifndef JSON_H_
#define JSON_H_ 1

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <variant>

namespace json {

class Element;
using String = std::string;
using Object = std::map<String, Element>;
using Array = std::vector<Element>;
using Integer = std::int64_t;
using Double = double;
using Boolean = bool;

class Element : std::variant<Object, Array, String, Integer, Double, Boolean> {
public:
	using variant::variant;
	[[nodiscard]] variant const &getBase() const
	{
		return *this;
	}

	[[nodiscard]] Object const &asObject() const
	{
		return std::get<Object>(*this);
	}

	[[nodiscard]] Array const &asArray() const
	{
		return std::get<Array>(*this);
	}

	[[nodiscard]] String const &asString() const
	{
		return std::get<String>(*this);
	}

	[[nodiscard]] Integer asInteger() const
	{
		return std::get<Integer>(*this);
	}

	[[nodiscard]] Double asDouble() const
	{
		return std::holds_alternative<Double>(*this) ?
			std::get<Double>(*this) :
			static_cast<Double>(std::get<Integer>(*this));
	}

	[[nodiscard]] Boolean asBoolean() const
	{
		return std::get<Boolean>(*this);
	}
};

class Document {
public:
	explicit Document(Element element) :
		root{std::move(element)}
	{
	}

	[[nodiscard]] Element const &getRoot() const
	{
		return root;
	}

private:
	Element root;
};

[[nodiscard]] Element readElement(std::istream &in);

[[nodiscard]] Document readDocument(std::istream &in);

void writeDocument(Document const &document, std::ostream &os);

void writeElement(Element const &element, std::ostream &os);

template <typename Value>
void writeValue(Value const &value, std::ostream &os) = delete;

void writeValue(Object const &object, std::ostream &os);

void writeValue(Array const &array, std::ostream &os);

void writeValue(String const &string, std::ostream &os);

void writeValue(Integer number, std::ostream &os);

void writeValue(Double number, std::ostream &os);

void writeValue(Boolean boolean, std::ostream &os);

} // namespace json

#endif /* JSON_H_ */
