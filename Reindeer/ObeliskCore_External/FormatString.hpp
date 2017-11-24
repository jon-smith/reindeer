#pragma once

#include <string>
#include <memory>

#include "ObeliskMeta.h"

namespace obelisk
{
	namespace impl
	{
		template <typename ... Args>
		struct ContainsAnyStdStringTypes
		{
			static constexpr bool value =
				ContainsAnyOf<std::wstring, Args...>::value ||
				ContainsAnyOf<const std::wstring, Args...>::value ||
				ContainsAnyOf<std::wstring&, Args...>::value ||
				ContainsAnyOf<const std::wstring&, Args...>::value ||
				ContainsAnyOf<std::string, Args...>::value ||
				ContainsAnyOf<const std::string, Args...>::value ||
				ContainsAnyOf<std::string&, Args...>::value ||
				ContainsAnyOf<const std::string&, Args...>::value;
		};
	}

	// From https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
	template<typename ... Args>
	std::string formatString(const std::string& format, Args ...args)
	{
		// Assert on any std strings or wchar_t types
		static_assert(impl::ContainsAnyStdStringTypes<Args...>::value == false, "");
		static_assert(ContainsAnyOf<wchar_t*, Args...>::value == false, "");
		static_assert(ContainsAnyOf<const wchar_t*, Args...>::value == false, "");

		auto const size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
		if (size < 1)
			return{};

		std::unique_ptr<char[]> buf(new char[size]);
		snprintf(buf.get(), static_cast<size_t>(size), format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + static_cast<size_t>(size) - 1);
	}

	template<typename ... Args>
	std::wstring formatString(const std::wstring& format, Args ...args)
	{
		// Assert on any std strings or char types
		static_assert(impl::ContainsAnyStdStringTypes<Args...>::value == false, "");
		static_assert(ContainsAnyOf<char*, Args...>::value == false, "");
		static_assert(ContainsAnyOf<const char*, Args...>::value == false, "");
		
		auto const size = std::swprintf(nullptr, 0, format.c_str(), args ...) + 1;
		if (size < 1)
			return{};

		std::unique_ptr<wchar_t[]> buf(new wchar_t[size]);
		std::swprintf(buf.get(), static_cast<size_t>(size), format.c_str(), args ...);
		return std::wstring(buf.get(), buf.get() + static_cast<size_t>(size) - 1);
	}
}
