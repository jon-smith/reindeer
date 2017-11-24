#pragma once

#include <functional>
#include <cassert>

#include "StringFuncs.h"
#include "PlatformSpecific.h"

namespace obelisk
{
	// Strip preceeding namespaces/classes/spaces from type names
	template <typename T>
	std::wstring getSimpleRuntimeTypeName(const T& o)
	{
		// Strip preceeding namespaces/classes/spaces from typenames
		// e.g. class Banana -> Banana; class YellowThings::Banana -> Banana
		std::wstring typeName = strToWStr(typeid(o).name());
		return typeName.substr(typeName.find_last_of(L": ") + 1);
	}

	// Custom comparison function to compare pointer values from a std::reference_wrapper	
	struct ComparePointersFromStdRef
	{
		template <typename T>
		bool operator()(std::reference_wrapper<T> const &a, std::reference_wrapper<T> const &b) const
		{
			return &(a.get()) < &(b.get());
		};
	};	

	// Class which assignes a value to a variable for the duration of the current scope
	template <typename T>
	class ScopedSetVariable
	{
	public:
		explicit ScopedSetVariable(T &var, T set) :
			varReference(var), originalValue(var)
		{
			var = set;
		}

		~ScopedSetVariable()
		{
			varReference = originalValue;
		}

	private:
		T &varReference;
		T originalValue;
	};

	// Class which calls a function on destruction
	// Helpful for cleaning up in a function where exceptions might be thrown or with multiple returns
	class CallOnDestruction
	{
	public:
		template <typename TFunc>
		CallOnDestruction(TFunc func) : func(func){};
		~CallOnDestruction()
		{
			func();
		}

	private:
		std::function<void(void)> func;
	};

	template <typename Fn>
	void tryCatch(Fn &&fn, std::string desc)
	{
		try
		{
			fn();
		}		
		catch (const std::exception &e)
		{
			platform_utilities::outputDebugString(std::string("Exception thrown in ") + desc + std::string(":") + std::string(e.what()));
			assert(false);
		}
		catch (...)
		{
			platform_utilities::outputDebugString(std::string("Unknown exception thrown in ") + desc);
			assert(false);
		}
	}
}
