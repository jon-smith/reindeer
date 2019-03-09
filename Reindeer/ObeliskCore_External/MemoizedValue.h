#pragma once

#include <functional>

#include "Optional.h"

namespace obelisk
{
	/// Value is calculated on first call to get() and then stored for subsequent calls
	template <typename T>
	class MemoizedValue
	{
	public:

		MemoizedValue() = delete;

		template <typename CreateFunc>
		explicit MemoizedValue(CreateFunc &&fn) : fn(fn)
		{
		}

		T get() const
		{
			LockGuard lk(m);
			if (!value.hasValue())
				value.set(fn());
			
			return value.value();
		}

		void reset()
		{
			LockGuard lk(m);
			value.reset();
		}

	private:

		mutable Mutex m;
		mutable Optional<T> value;
		const std::function<T(void)> fn;
	};
}