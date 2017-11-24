#pragma once

#include "Command.h"
#include <future>

namespace obelisk
{
	// Command object with the option to get a shared_future which allow waiting on execute() to complete
	template <typename CommandType>
	class WaitableCommand : public CommandType
	{
		static_assert(std::is_base_of<Command, CommandType>::value, "WaitableCommand requires the template parameter to be derived from Command");
	public:
		template<class...Args>
		explicit WaitableCommand(Args&...args) :
			CommandType(args...),
			future(std::async(std::launch::async, [this]()
		{
			// waits for cv.notify_one() to be called
			std::unique_lock<std::mutex> lk(m);
			cv.wait(lk);
			return;
		}))
		{
		}

		~WaitableCommand()
		{
			cv.notify_one();
			future.wait();
		}

		// Get a shared future to wait on until execute completes (on object is destroyed)
		std::shared_future<void> getFuture() const
		{
			return future;
		}

		void execute() override
		{
			CommandType::execute();
			cv.notify_one();
		}

	private:
		std::condition_variable cv;
		std::mutex m;
		std::shared_future<void> future;
	};

} // End obelisk namespace
