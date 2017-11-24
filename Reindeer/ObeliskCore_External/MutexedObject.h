#pragma once

#include <functional>

#include "StdThreadSupportWrappers.h"

namespace obelisk
{
	// Allows safe read/write access to an object using user specified functions
	// Has the advantage over std::atomic that multiple functions can be called during a single lock
	template <typename T, bool useRWLocks = false>
	class MutexedObject
	{
		template <bool useRWLocks_>
		struct Impl;

		template <>
		struct Impl<true>
		{
			using MutexT = SharedMutex;
			using ReadLockT = SharedLock;
			using WriteLockT = UniqueLock<MutexT>;
		};

		template <>
		struct Impl<false>
		{
			using MutexT = Mutex;
			using ReadLockT = LockGuard;
			using WriteLockT = LockGuard;
		};

		using MutexT = typename Impl<useRWLocks>::MutexT;
		using ReadLockT = typename Impl<useRWLocks>::ReadLockT;
		using WriteLockT = typename Impl<useRWLocks>::WriteLockT;

	public:

		template <class...Args>
		MutexedObject(Args&&...args) : obj(std::forward<Args>(args)...)
		{
			
		}

		template<class U = T>
		MutexedObject(const MutexedObject<T, useRWLocks>& o, typename std::enable_if<std::is_copy_constructible<U>::value>::type * = nullptr) : obj(o.get())
		{

		}		

		~MutexedObject() = default;

		template <typename RT>
		auto lockedAccess(std::function<RT(const T&)> fn) const -> RT
		{
			ReadLockT const lk(m);
			return fn(obj);
		}

		void lockedAccess(std::function<void(const T&)> fn) const
		{
			ReadLockT const lk(m);
			fn(obj);
		}

		template <typename RT>
		auto lockedModify(std::function<RT(T&)> fn) -> RT
		{
			WriteLockT const lk(m);
			return fn(obj);
		}

		void lockedModify(std::function<void(T&)> fn)
		{
			WriteLockT const lk(m);
			fn(obj);
		}

		template <typename U = T>
		void set(U&& newObj)
		{
			WriteLockT const lk(m);
			obj = std::move(newObj);
		}

		template<class U = T>
		typename std::enable_if<std::is_copy_constructible<U>::value, U>::type
		get() const
		{
			ReadLockT const lk(m);
			return obj;
		}

	private:		

		T obj;
		mutable MutexT m;
	};
}