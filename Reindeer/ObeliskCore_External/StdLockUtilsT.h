#pragma once

#include <mutex>
#include <shared_mutex>

namespace obelisk
{
	// Mutex locking helpers
	//
	// Replaces code like the following:
	// {
	//		std::lock_guard<std::mutex> lk(mutex);
	//		funcThatNeedsLocking();
	// }
	// with:
	// lockAndCall(mutex, [&](){funcThatNeedsLocking();};
	//
	// Or:
	// std::unique_lock<std::mutex> lk(mutex);
	// if (a < b)
	// {
	//		lk.unlock();
	//		externalFunc();
	// }
	// with:
	// if (lockCallAndReturn[&](){return a < b;}))
	//		externalFunc();
	//
	template <typename TFunc, typename Mutex>
	void lockAndCall(Mutex &m, TFunc func)
	{
		std::lock_guard<Mutex> lk(m);
		func();
	}

	template <typename TFunc, typename Mutex>
	auto lockCallAndReturn(Mutex &m, TFunc func) -> typename std::result_of<decltype(func)()>::type
	{
		std::lock_guard<Mutex> lk(m);
		return func();
	}

	template <typename TFunc>
	void sharedLockAndCall(std::shared_mutex &m, TFunc func)
	{
		std::shared_lock<std::shared_mutex> lk(m);
		func();
	}

	template <typename TFunc>
	auto sharedLockCallAndReturn(std::shared_mutex &m, TFunc func) -> typename std::result_of<decltype(func)()>::type
	{
		std::shared_lock<std::shared_mutex> lk(m);
		return func();
	}
}