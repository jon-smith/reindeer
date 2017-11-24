#pragma once

#include <vector>

#include "FormatString.hpp"

namespace obelisk
{
	template <typename ContainerT, typename Comparator = std::less<typename ContainerT::value_type>>
	auto restrictToIncreasingRange(const size_t fromIndex, const ContainerT &container, Comparator&& comparator = Comparator()) ->
		std::vector<typename ContainerT::value_type>
	{
		if (fromIndex >= container.size())
			throw std::invalid_argument(formatString("Index out of range during restrict increase/decrease (idx: %d, size: %d)", fromIndex, container.size()).c_str());

		std::vector<typename ContainerT::value_type> restricted;

		if (fromIndex != 0)
		{
			// Reduce until we get to a value that isn't more than the one below it
			auto firstIdx = fromIndex;
			do
			{
				if (!comparator(container[firstIdx - 1], container[firstIdx]))
					break;

				--firstIdx;
			} while (firstIdx != 0);

			// Increase back up to "from", adding to the restricted vector
			for (; firstIdx != fromIndex; ++firstIdx)
				restricted.push_back(container[firstIdx]);
		}

		if (fromIndex < container.size())
			restricted.push_back(container[fromIndex]);

		if (fromIndex < container.size())
		{
			for (auto idx = fromIndex; (idx + 1) != container.size(); ++idx)
			{
				auto const next = idx + 1;
				if (!comparator(container[idx], container[next]))
					break;

				restricted.push_back(container[next]);
			}
		}

		return restricted;
	}

	template <typename ContainerT, typename Comparator = std::less<typename ContainerT::value_type>>
	auto restrictToDecreasingRange(const size_t fromIndex, const ContainerT &container, Comparator&& comparator = Comparator()) ->
		std::vector<typename ContainerT::value_type>
	{
		// Use restrictToIncreasingRange and invert the comparator order
		return restrictToIncreasingRange(fromIndex, container, [&comparator](typename ContainerT::value_type lhs, typename ContainerT::value_type rhs)
		{
			return comparator(rhs, lhs);
		});
	}
}