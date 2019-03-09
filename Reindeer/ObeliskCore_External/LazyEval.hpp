#pragma once

#include <functional>
#include <mutex>

#include "Optional.h"

namespace obelisk
{
	template <typename ReturnT>
	class LazyEval
	{
	public:

		LazyEval() = delete;

		template <typename Fn>
		LazyEval(Fn &&fn) : fn(fn)
		{

		}

		ReturnT get() const
		{
			if (!value.hasValue())
				value.set(fn());
			return value.value();
		}

	private:
		mutable Optional<ReturnT> value;
		std::function<ReturnT(void)> fn;
	};

	template <typename ReturnT>
	class LazyEvalThreadSafe
	{
	public:

		LazyEvalThreadSafe() = delete;

		template <typename Fn>
		LazyEvalThreadSafe(Fn &&fn) : fn(fn)
		{

		}

		ReturnT get() const
		{
			std::lock_guard<std::mutex> lk(m);
			if (!value.hasValue())
				value.set(fn());
			return value.value();
		}

	private:
		mutable std::mutex m;
		mutable Optional<ReturnT> value;
		std::function<ReturnT(void)> fn;
	};
}