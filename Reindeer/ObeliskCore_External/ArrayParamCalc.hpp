#pragma once

#include <utility>
#include <cassert>
#include <algorithm>

#include <ParallelSort.hpp>

namespace obelisk
{
	template <typename IteratorT, typename T = std::decay_t<typename std::iterator_traits<IteratorT>::value_type>>
	auto updateMinMax(std::pair<T, T> currentMinMax, IteratorT const dataBegin, IteratorT const dataEnd) -> std::pair<T, T>
	{
		for (auto it = dataBegin; it != dataEnd; ++it)
		{
			if (*it < currentMinMax.first)
				currentMinMax.first = *it;
			if (*it > currentMinMax.second)
				currentMinMax.second = *it;
		}

		return currentMinMax;
	}

	template <typename IteratorT, typename T = std::decay_t<typename std::iterator_traits<IteratorT>::value_type>>
	auto findMinAndMax(IteratorT const dataBegin, IteratorT const dataEnd) -> std::pair<T, T>
	{
		return updateMinMax(std::make_pair(std::numeric_limits<T>::max(), std::numeric_limits<T>::lowest()), dataBegin, dataEnd);
	}

	template <typename IteratorT, size_t nPercentiles, bool useParallelSort = true>
	auto calculateApproximatePercentiles(
		const IteratorT dataBegin, const IteratorT dataEnd, const std::array<double, nPercentiles> &percentilesToCalculate)
		->std::array<typename std::iterator_traits<IteratorT>::value_type, nPercentiles>
	{
		// Only allow parallel sorts in non-debug mode, because they are super slow in debug
		constexpr auto allowParallelSort =
#ifdef NDEBUG
			true;
#else
			false;
#endif

		static_assert(nPercentiles > 0, "A number of percentiles must be specified");

		for (auto const &p : percentilesToCalculate)
			assert(p >= 0.0 && p < 100.0);

		if (dataBegin != dataEnd)
		{
			std::vector<typename std::iterator_traits<IteratorT>::value_type> sortedData(dataBegin, dataEnd);

			if (allowParallelSort && useParallelSort)
				concurrentParallelSort(std::begin(sortedData), std::end(sortedData));
			else
				std::sort(std::begin(sortedData), std::end(sortedData));

			auto const nSize = sortedData.size();

			std::array<typename std::iterator_traits<IteratorT>::value_type, nPercentiles> retPercentiles;

			for (size_t i = 0; i < retPercentiles.size(); ++i)
			{
				auto const index = std::min(static_cast<size_t>(nSize * percentilesToCalculate[i] / 100.0), nSize - 1);
				retPercentiles[i] = sortedData[index];
			}

			return retPercentiles;
		}

		return {};
	}
}
