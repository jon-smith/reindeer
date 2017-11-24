#pragma once

#include <ppl.h>
#include <concurrent_vector.h>
#include <vector>

namespace obelisk
{
	template <bool parallel>
	class OptionalParallel{};
	
	template <>
	class OptionalParallel<true>
	{
	public:
		template <typename IndexT, typename Fn>
		static void forEach(IndexT from, IndexT to, Fn fn)
		{
			concurrency::parallel_for(from, to, fn);
		}

		template <typename T>
		using VectorT = Concurrency::concurrent_vector<T>;
	};

	template <>
	class OptionalParallel<false>
	{
	public:
		template <typename IndexT, typename Fn>
		static void forEach(IndexT from, IndexT to, Fn fn)
		{
			for (IndexT i = from; i != to; ++i)
				fn(i);			
		}

		template <typename T>
		using VectorT = std::vector<T>;
	};	
}
