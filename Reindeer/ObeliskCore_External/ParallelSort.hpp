#pragma once

#include <ppl.h>

namespace obelisk
{
	template <typename IteratorT>
	std::enable_if_t<std::is_integral<typename std::iterator_traits<IteratorT>::value_type>::value, void> concurrentParallelSort(
		IteratorT dataBegin, IteratorT dataEnd)
	{
		// Use a radix sort for integral types (faster)
		concurrency::parallel_radixsort(dataBegin, dataEnd);
	}

	template <typename IteratorT>
	std::enable_if_t<!std::is_integral<typename std::iterator_traits<IteratorT>::value_type>::value, void> concurrentParallelSort(
		IteratorT dataBegin, IteratorT dataEnd)
	{
		concurrency::parallel_buffered_sort(dataBegin, dataEnd);
	}
}