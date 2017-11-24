#pragma once

#include <vector>
#include <functional>

namespace obelisk
{
	template <typename T>
	std::vector<T> generateVector(std::function<T(size_t)> generator, size_t const from, size_t const n)
	{
		std::vector<T> vec(n);
		for (auto i = from; i < from + n; ++i)
			vec[i-from] = generator(i);
		return vec;
	}
}