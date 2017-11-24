#pragma once

#include "Command.h"
#include <future>

namespace obelisk
{
	template <typename ReturnType>
	class CommandWithFutureReturn : public Command
	{
	public:

		CommandWithFutureReturn() :
			futureResult(std::async(std::launch::async, [this]()
		{
			// waits for cv.notify_one() to be called
			std::unique_lock<std::mutex> lk(m);
			cv.wait(lk);
			return executeAndReturn();
		}))
		{
		}

		~CommandWithFutureReturn()
		{
			cv.notify_one();
			futureResult.wait();
		}

		// The future can be obtained before execution starts
		// then the result can be accessed after execution has completed
		std::shared_future<ReturnType> getResultFuture() const
		{
			return futureResult;
		}

		// execute asynchronously calls the executeAndReturn function and waits for it to complete
		// declared final - use executeAndReturn to implement command functionality
		void execute() override final
		{
			// Notify the execution thread and wait for it to complete
			cv.notify_one();
			futureResult.wait();
		}

	private:
		// Function to override - nothrow, as it's called from a std::async thread
		virtual ReturnType executeAndReturn() throw() = 0;

		std::condition_variable cv;
		std::mutex m;
		const std::shared_future<ReturnType> futureResult;
	};
} 