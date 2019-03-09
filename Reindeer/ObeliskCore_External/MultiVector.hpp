#pragma once

#include <vector>
#include <tuple>

#include "ObeliskMeta.h"

namespace obelisk
{
	namespace impl
	{
		template <typename ...Types>
		static void throwOnInconsistentSizes(std::tuple<Types&...> refs)
		{
			const auto size = std::get<0>(refs).size();
			obelisk::forEach(refs, [size](auto &v) {
				if (size != v.size())
					throw std::invalid_argument("MultiVector construct: Inconsistent vector sizes");
			});
		}

		template <typename ...VectorTypes>
		static auto checkSizesAndTie(VectorTypes&&...args) -> decltype(std::tie(args...))
		{
			const auto refs = std::tie(args...);
			throwOnInconsistentSizes(refs);
			return refs;
		}

		template <typename ...VectorTypes>
		static std::tuple<std::decay_t<VectorTypes>...> checkSizesAndMakeTuple(VectorTypes&&...args)
		{
			const auto refs = std::tie(args...);
			throwOnInconsistentSizes(refs);
			return std::make_tuple(std::forward<VectorTypes>(args)...);
		}
	}

	template <typename ...Types>
	class MultiVector
	{
	public:

		MultiVector() = default;

		MultiVector(size_t size) :
			data(std::vector<Types>(size)...)
		{
		}

		MultiVector(const std::vector<Types>&...args) :
			data(impl::checkSizesAndMakeTuple(args...))
		{

		}

		MultiVector(std::vector<Types>&&...args) :
			data(impl::checkSizesAndMakeTuple(std::forward<std::vector<Types>>(args)...))
		{
		}

		MultiVector(MultiVector&& o) :
			data(std::move(o.data))
		{
		}

		MultiVector(const MultiVector& o) :
			data(o.data)
		{
		}

		size_t size() const
		{
			return std::get<0>(data).size();
		}

		bool isEmpty() const
		{
			return std::get<0>(data).empty();
		}

		template <size_t index>
		auto get() const -> decltype(std::get<index>(data))
		{
			return std::get<index>(data);
		}

	private:
		std::tuple<std::vector<Types>...> data;
	};

	template <typename ...Types>
	MultiVector<Types...> MakeMultiVector(const std::vector<Types>&...args)
	{
		return MultiVector<Types...>(args...);
	}

	template <typename ...Types>
	MultiVector<Types...> MakeMultiVector(std::vector<Types>&&...args)
	{
		return MultiVector<Types...>(std::forward<std::vector<Types>>(args)...);
	}

	template <typename ...Types>
	class MultiVectorRef
	{
	public:

		MultiVectorRef(std::vector<Types>&...args) :
			dataRefs(impl::checkSizesAndTie(args...))
		{

		}

		size_t size() const
		{
			return std::get<0>(dataRefs).size();
		}

		bool isEmpty() const
		{
			return std::get<0>(dataRefs).empty();
		}

		template <size_t index>
		auto get() const -> decltype(std::get<index>(dataRefs))
		{
			return std::get<index>(dataRefs);
		}

	private:
		std::tuple<std::vector<Types>&...> dataRefs;
	};

	template <typename ...Types>
	MultiVectorRef<Types...> MakeMultiVectorRef(std::vector<Types>&...args)
	{
		return MultiVectorRef<Types...>(args...);
	}

	template <typename ...Types>
	class MultiVectorCRef
	{
	public:

		MultiVectorCRef(const std::vector<Types>&...args) :
			dataRefs(impl::checkSizesAndTie(args...))
		{

		}

		size_t size() const
		{
			return std::get<0>(dataRefs).size();
		}

		bool isEmpty() const
		{
			return std::get<0>(dataRefs).empty();
		}

		template <size_t index>
		auto get() const -> decltype(std::get<index>(dataRefs))
		{
			return std::get<index>(dataRefs);
		}

	private:
		std::tuple<const std::vector<Types>&...> dataRefs;
	};

	template <typename ...Types>
	MultiVectorCRef<Types...> MakeMultiVectorRef(const std::vector<Types>&...args)
	{
		return MultiVectorCRef<Types...>(args...);
	}

	template <typename ...Types>
	MultiVectorCRef<Types...> MakeMultiVectorCRef(const std::vector<Types>&...args)
	{
		return MultiVectorCRef<Types...>(args...);
	}
}