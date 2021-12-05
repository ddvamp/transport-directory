#ifndef UTIL_H_
#define UTIL_H_ 1

#include <concepts>
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>

namespace util {

template <typename ...Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};

template <typename ...Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T>
void hash_combine(std::size_t &seed, T const &key) noexcept
{
	std::hash<T> hasher{};
	seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <auto func>
struct build_tag_t {
	explicit build_tag_t() = default;
};

template <auto func>
inline constexpr build_tag_t<func> build_tag{};

template <auto func_, typename ...Args>
requires std::invocable<decltype(func_), Args...>
class Builder {
public:
	explicit inline constexpr
	Builder(build_tag_t<func_>, Args &&...args) noexcept
		: args_{std::forward_as_tuple(std::forward<Args>(args)...)}
	{
	}

	inline constexpr operator auto () const noexcept(
		noexcept(std::apply(func_, args_)))
	{
		return std::apply(func_, args_);
	}

private:
	std::tuple<Args &&...> args_;
};

template <auto func, typename ...T>
Builder(build_tag_t<func>, T &&...) -> Builder<func, T...>;

template <auto func, typename ...T>
inline constexpr auto make_builder(T &&...t) noexcept
{
	return Builder(build_tag<func>, std::forward<T>(t)...);
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

#endif /* UTIL_H_ */
