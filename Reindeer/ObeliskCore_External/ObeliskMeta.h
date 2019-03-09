#pragma once

///===============================================
/// Template metaprogramming stuff
///===============================================

#include <type_traits>
#include <map>

namespace obelisk
{
	//=================================================
	// Size of parameter pack
	//=================================================
	template<typename... Args> struct CountArgs;

	template<>
	struct CountArgs<> {
		static const size_t value = 0;
	};

	template<typename T, typename... Args>
	struct CountArgs<T, Args...> {
		static const size_t value = 1 + CountArgs<Args...>::value;
	};

	//=================================================
	// Check if a parameter pack contains references
	//=================================================
	template <typename... Types>
	struct ContainsReference : std::true_type {};

	template <typename Head, typename... Rest>
	struct ContainsReference<Head, Rest...>
		: std::conditional<std::is_reference<Head>::value,
		std::true_type, ContainsReference<Rest... >> ::type{};

	template <>
	struct ContainsReference<> : std::false_type{};

	//=================================================
	// Check if a parameter pack contains a certain type
	//=================================================
	template <typename ... >
	struct ContainsAnyOf : std::false_type {};

	template <typename TypeToCheck, typename Head, typename... Rest>
	struct ContainsAnyOf<TypeToCheck, Head, Rest...>
		: std::conditional<std::is_same<TypeToCheck, Head>::value,
		std::true_type, ContainsAnyOf<TypeToCheck, Rest... >> ::type{};

	//=================================================
	// Check if type is a map
	//=================================================
	template <class T>
	struct isMap : std::false_type{};

	template<class Key, class Value>
	struct isMap<const std::map<Key, Value>> : std::true_type{};

	template<class Key, class Value>
	struct isMap<std::map<Key, Value>> : std::true_type{};

	//=================================================
	// Tuple for each
	// from: https://codereview.stackexchange.com/questions/51407/stdtuple-foreach-implementation
	//=================================================
	namespace impl
	{
		template <typename Tuple, typename F, std::size_t ...Indices>
		void forEach(Tuple&& tuple, F&& f, std::index_sequence<Indices...>) {
			using swallow = int[];
			(void)swallow {
				1,
					(f(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int{})...
			};
		}
	}

	template <typename Tuple, typename F>
	void forEach(Tuple&& tuple, F&& f) {
		constexpr std::size_t N = std::tuple_size<std::remove_reference_t<Tuple>>::value;
		impl::forEach(std::forward<Tuple>(tuple), std::forward<F>(f),
			std::make_index_sequence<N>{});
	}
}