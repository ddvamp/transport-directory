#ifndef UTIL_H_
#define UTIL_H_ 1

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace util {

template <typename ...Ts>
struct overloaded : Ts... {
	using Ts::operator()...;
};

template <typename ...Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename Callable, typename ...Args>
requires std::is_invocable_v<Callable, Args...>
class builder {
	using R = std::invoke_result_t<Callable, Args...>;
public:
	explicit inline constexpr builder(Callable &&fn, Args &&...args)
	noexcept(std::is_nothrow_constructible_v<Callable, Callable &&>)
		: fn_{std::forward<Callable>(fn)}
		, args_{std::forward_as_tuple(std::forward<Args>(args)...)}
	{
	}

	explicit(false) inline constexpr operator R ()
	noexcept(std::is_nothrow_invocable_v<Callable, Args...>)
	{
		return std::apply(fn_, args_);
	}

private:
	[[no_unique_address]] Callable fn_;
	[[no_unique_address]] std::tuple<Args &&...> args_;
};

template <typename Callable, typename ...Args>
builder(Callable &&, Args &&...) -> builder<Callable, Args...>;

template <typename T>
void hash_combine(std::size_t &seed, T const &key) noexcept
{
	std::hash<T> hasher{};
	seed ^= hasher(key) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
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
