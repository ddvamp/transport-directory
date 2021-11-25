#ifndef UTIL_H_
#define UTIL_H_ 1

#include <cstddef>
#include <string>
#include <utility>

namespace util {

template <typename ...Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <typename ...Ts> overloaded(Ts...) -> overloaded<Ts...>;

inline void hash_combine(std::size_t &seed, std::string const &key) noexcept
{
	std::hash<std::string> hasher{};
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

		return min(seed1, seed2);
	}
};

} // namespace std

#endif /* UTIL_H_ */
