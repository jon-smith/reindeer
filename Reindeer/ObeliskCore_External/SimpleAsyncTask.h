#pragma once

#include <memory>
#include <functional>
#include <chrono>

namespace obelisk
{
	// Effectively a wrapper for std::future<void>
	// We can use this in C++/CLI without issues
	class SimpleAsyncTask
	{
	public:
		SimpleAsyncTask();
		explicit SimpleAsyncTask(std::function<void(void)> &&fn);
		SimpleAsyncTask(const SimpleAsyncTask &o);
		SimpleAsyncTask(SimpleAsyncTask &&o);
		const SimpleAsyncTask &operator=(const SimpleAsyncTask &o);
		const SimpleAsyncTask &operator=(SimpleAsyncTask &&o);
		~SimpleAsyncTask();

		enum class Status { INVALID, TIMEOUT, READY };
		void wait() const;
		Status waitFor(std::chrono::duration<double, std::milli> time)  const;

	private:
		class Impl;
		std::unique_ptr<Impl> impl;
	};

	class SimpleAsyncTaskWaiter
	{
	public:
		SimpleAsyncTaskWaiter() = default;
		SimpleAsyncTaskWaiter(const SimpleAsyncTask &task) : task(task) {}
		void wait() const
		{
			task.wait();
		}
		SimpleAsyncTask::Status waitFor(std::chrono::duration<double, std::milli> time) const
		{
			return task.waitFor(time);
		}
	private:
		friend SimpleAsyncTask;
		SimpleAsyncTask task;
	};
}