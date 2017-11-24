#pragma once

#include <memory>

#include "StdThreadSupportWrappers.h"

namespace obelisk
{
	/// Temporary classes while std::optional isn't in VS2015 (isn't exactly the same, but will do)
	/// Thread-safe and non-thread-safe versions available (with respect to all calls)

	template <typename T>
	class Optional
	{
	public:

		Optional() = default;

		template <typename U>
		explicit Optional(U &&value) :
			optionalValue(std::make_unique<T>(value))
		{

		}

		Optional(Optional &&o) noexcept :
		optionalValue(std::move(o.optionalValue))
		{

		}

		Optional(const Optional &o)
		{
			if (o.optionalValue.get())
				optionalValue = std::make_unique<T>(*o.optionalValue.get());
		}

		const Optional& operator=(const Optional &o)
		{
			if (o.optionalValue.get())
				optionalValue = std::make_unique<T>(*o.optionalValue.get());
			else
				optionalValue.reset();

			return *this;
		}

		bool hasValue() const
		{
			return hasValue_internal();
		}

		T value() const
		{
			return value_internal();
		}

		template <typename U>
		T valueOr(U &&o) const
		{
			if (hasValue_internal())
				return value_internal();

			return o;
		}

		void reset()
		{
			optionalValue.reset();
		}

		template <typename U>
		void set(U&& o)
		{
			optionalValue = std::make_unique<T>(o);
		}

	private:

		bool hasValue_internal() const
		{
			return optionalValue.get() != nullptr;
		}

		T value_internal() const
		{
			if (!optionalValue.get())
				throw std::exception("Optional contains no value");

			return *optionalValue;
		}

		std::unique_ptr<T> optionalValue;
	};

	template <typename T>
	class Optional_ThreadSafe
	{
	public:

		Optional_ThreadSafe() = default;

		template <typename U>
		explicit Optional_ThreadSafe(U &&value) :
			optionalValue(std::make_unique<T>(value))
		{
			
		}

		Optional_ThreadSafe(Optional_ThreadSafe &&o) noexcept :
			optionalValue(std::move(o.optionalValue))
		{

		}

			Optional_ThreadSafe(const Optional_ThreadSafe &o)
		{
			LockGuard lk(o.m);

			if (o.optionalValue.get())
				optionalValue = std::make_unique<T>(*o.optionalValue.get());
		}

		const Optional_ThreadSafe& operator=(const Optional_ThreadSafe &o)
		{
			LockGuard lk(o.m);
			LockGuard l(m);

			if (o.optionalValue.get())
				optionalValue = std::make_unique<T>(*o.optionalValue.get());
			else
				optionalValue.reset();

			return *this;
			}

		bool hasValue() const
		{
			LockGuard lk(m);
			return hasValue_noLock();
		}

		T value() const
		{
			LockGuard lk(m);
			return value_noLock();
		}

		template <typename U>
		T valueOr(U &&o) const
		{
			LockGuard lk(m);

			if (hasValue_noLock())
				return value_noLock();

			return o;
		}

		void reset()
		{
			LockGuard lk(m);
			optionalValue.reset();
		}

		template <typename U>
		void set(U&& o)
		{
			LockGuard lk(m);
			optionalValue = std::make_unique<T>(o);
		}

	private:

		bool hasValue_noLock() const
		{
			return optionalValue.get() != nullptr;
		}

		T value_noLock() const
		{
			if (!optionalValue.get())
				throw std::exception("Optional contains no value");

			return *optionalValue;
		}

		mutable Mutex m;
		std::unique_ptr<T> optionalValue;
	};
}