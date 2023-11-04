#pragma once

#include <functional>

namespace lve {

	template <typename T, typename... Rest>
	void hashCombine(std::size_t &seed, const T &v, const Rest &...rest) {
	  seed ^= std::hash<T>{}(v) + 0x9E3779B9 + (seed << 6) + (seed >> 2);
	  (hashCombine(seed, rest), ...);
	};

} // namespace lve
