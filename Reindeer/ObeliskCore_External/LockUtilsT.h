#pragma once

#include "StdThreadSupportWrappers.h"

namespace obelisk
{
	// Mutex locking helpers
	//
	// Replaces code like the following:
	// {
	//		LockGuard lk(mutex);
	//		funcThatNeedsLocking();
	// }
	// with:
	// lockAndCall(mutex, [&](){funcThatNeedsLocking();};
	//
	// Or:
	// Uniquelock<Mutex> lk(mutex);
	// if (a < b)
	// {
	//		lk.unlock();
	//		externalFunc();
	// }
	// with:
	// if (lockCallAndReturn[&](){return a < b;}))
	//		externalFunc();
	//
	template <typename TFunc, typename MutexT>
	void lockAndCall(MutexT &m, TFunc func, typename std::enable_if<std::is_same<MutexT, Mutex>::value || std::is_same<MutexT, SharedMutex>::value>::type * = nullptr)
	{
		LockGuard lk(m);
		func();
	}

	template <typename TFunc, typename MutexT>
	auto lockCallAndReturn(MutexT &m, TFunc func, typename std::enable_if<std::is_same<MutexT, Mutex>::value || std::is_same<MutexT, SharedMutex>::value>::type * = nullptr) -> typename std::result_of<decltype(func)()>::type
	{
		LockGuard lk(m);
		return func();
	}

	template <typename TFunc>
	void sharedLockAndCall(SharedMutex &m, TFunc func)
	{
		SharedLock lk(m);
		func();
	}

	template <typename TFunc>
	auto sharedLockCallAndReturn(SharedMutex &m, TFunc func) -> typename std::result_of<decltype(func)()>::type
	{
		SharedLock lk(m);
		return func();
	}
}