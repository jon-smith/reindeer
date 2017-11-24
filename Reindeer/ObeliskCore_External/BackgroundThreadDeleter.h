#pragma once
#include <memory>
#include <thread>
#include <mutex>

namespace obelisk
{
	// Class which deletes an object held by a unique_ptr in another thread
	template <typename T>
	class DelayedDeleter
	{
	public:
		static void DeleteObject(std::unique_ptr<T> &object)
		{
			// DelayedDelete cleans itself up, just call new!
			new DelayedDeleter(object);
		}
	private:
		DelayedDeleter() = delete;
		DelayedDeleter(std::unique_ptr<T> &object) :
			toDelete(std::move(object))
		{
			// Hold a mutex while we set things up
			std::lock_guard<std::mutex> lock(mutex);

			deleter = std::thread([this]
			{
				// Wait for the mutex to unblock before continuing				
				{std::lock_guard<std::mutex> lock(mutex);}
				this->~DelayedDeleter();
			});

			// Let the thread sort itself out
			deleter.detach();
		}	
		~DelayedDeleter() = default;
		std::thread deleter;
		std::unique_ptr<T> toDelete;
		std::mutex mutex;
	};
}