#include "SimpleAsyncTask.h"
#include "GeneralHelpers.h"

#include <future>
#include <cassert>

using namespace obelisk;

class SimpleAsyncTask::Impl
{
public:
	// The default constructor assures that the future is always valid
	Impl() : future(std::async(std::launch::async, [] {}).share())
	{
		// We need to wait for the future thread to finish
		// This ensures that "waitFor" always returns READY immediately after construction
		future.wait();
	}

	template<typename FuncT>
	Impl(const SimpleAsyncTask &backRef, FuncT &&func, typename std::enable_if<std::is_convertible<FuncT, std::function<void()>>::value>::type* = nullptr) :
		future(std::async(std::launch::async, [func = std::move(func), backRef = std::ref(backRef)]
	{
		// If a thread throws an exception, abort() gets called, so we catch all rogue exceptions here
		tryCatch(func, wStrToStr(getSimpleRuntimeTypeName(backRef.get())));
	}).share())
	{
	}

	explicit Impl(std::shared_future<void> &&future) : future(future) {}
	explicit Impl(std::shared_future<void> future) : future(future) {}

	Impl(Impl &&) = delete;
	Impl(const Impl &) = delete;

	std::shared_future<void> future;
};

SimpleAsyncTask::SimpleAsyncTask() :
	impl(std::make_unique<Impl>())
{

}

SimpleAsyncTask::SimpleAsyncTask(std::function<void(void)> &&fn) :
	impl(std::make_unique<Impl>(*this, [fn = std::move(fn)]{ fn(); }))
{

}

SimpleAsyncTask::SimpleAsyncTask(const SimpleAsyncTask &o) :
	impl(std::make_unique<Impl>(o.impl->future))
{

}

SimpleAsyncTask::SimpleAsyncTask(SimpleAsyncTask&& o) :
	impl(std::unique_ptr<Impl>(o.impl.release()))
{

}

const SimpleAsyncTask & SimpleAsyncTask::operator=(const SimpleAsyncTask &o)
{
	wait();
	impl->future = o.impl->future;
	return *this;
}

const SimpleAsyncTask & SimpleAsyncTask::operator=(SimpleAsyncTask &&o)
{
	wait();
	swap(impl, o.impl);
	return *this;
}

SimpleAsyncTask::~SimpleAsyncTask()
{
}

enum Status { INVALID, TIMEOUT, READY };

void SimpleAsyncTask::wait() const
{
	// Extra safety...but should never happen
	// Impl should only become invalid before destruction when the move constructor is called
	if (!impl)
	{
		assert(false);
		return;
	}

	impl->future.wait();
}

SimpleAsyncTask::Status SimpleAsyncTask::waitFor(std::chrono::duration<double, std::milli> time) const
{
	switch (impl->future.wait_for(time))
	{
	case std::future_status::ready:
		return Status::READY;
	case std::future_status::timeout:
		return Status::TIMEOUT;
	case std::future_status::deferred:
	default:
		return Status::INVALID;
	}
}