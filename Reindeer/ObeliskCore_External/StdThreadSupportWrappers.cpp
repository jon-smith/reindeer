#include "StdThreadSupportWrappers.h"

#include <shared_mutex>
#include <cassert>

#include "GeneralHelpers.h"

using namespace obelisk;

Mutex::Mutex() :
	m(std::make_unique<std::mutex>())
{
}

Mutex::~Mutex() = default;

void Mutex::lock()
{
	m->lock();
}

bool Mutex::tryLock()
{
	return m->try_lock();
}

void Mutex::unlock()
{
	m->unlock();
}

SharedMutex::SharedMutex() :
	m(std::make_unique<std::shared_mutex>())
{

}

SharedMutex::~SharedMutex() = default;

void SharedMutex::lock()
{
	m->lock();
}

void SharedMutex::lockShared()
{
	m->lock_shared();
}

void SharedMutex::unlock()
{
	m->unlock();
}

void SharedMutex::unlockShared()
{
	m->unlock_shared();
}

bool SharedMutex::tryLock()
{
	return m->try_lock();
}

bool SharedMutex::tryLockShared()
{
	return m->try_lock_shared();
}

ConditionVariable::ConditionVariable() :
	cv(std::make_unique<std::condition_variable>())
{

}

ConditionVariable::~ConditionVariable()
{

}

void ConditionVariable::notifyOne()
{
	cv->notify_one();
}

ConditionVariable::Status ConditionVariable::waitFor(UniqueLock<Mutex> &lk, std::chrono::milliseconds timeout)
{
	// Transfer lock ownership to unique_lock
	auto &mRef = *lk.m.m;
	std::unique_lock<std::mutex> newLock(mRef, std::adopt_lock_t());
	// Make sure we release from unique_lock on destruction, as UniqueLock retains ownership
	auto const onDestruction = CallOnDestruction([&newLock]()
	{
		newLock.release();
	});

	switch (cv->wait_for(newLock, timeout))
	{
	case std::cv_status::no_timeout:
		return Status::NO_TIMEOUT;
	case std::cv_status::timeout:
		return Status::TIMEOUT;
	default:
		assert(false);
		return Status::NO_TIMEOUT;
	}
}

void ConditionVariable::wait(UniqueLock<Mutex> &lk)
{
	// Shared lock ownership with unique_lock
	auto &mRef = *lk.m.m;
	std::unique_lock<std::mutex> newLock(mRef, std::adopt_lock_t());
	// Make sure we release from unique_lock on destruction, as UniqueLock retains ownership
	auto const onDestruction = CallOnDestruction([&newLock]()
	{
		newLock.release();
	});

	cv->wait(newLock);
}