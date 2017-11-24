#pragma once

#include <functional>

#include "Optional.h"

namespace obelisk
{
	///
	/// \brief Value is calculated on first call to get() and then stored for subsequent calls
	///
	template <typename T>
	class MemoizedValue
	{
	public:

		MemoizedValue() = delete;

		template <typename CreateFunc>
		explicit MemoizedValue(CreateFunc &&fn) : fn(fn)
		{
			static_assert(std::is_same<std::result_of_t<CreateFunc(void)>, T>::value, "Supplied function must return T");
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