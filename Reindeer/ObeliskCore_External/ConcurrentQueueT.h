#pragma once

#include <queue>
#include <typeinfo>

#include "StdThreadSupportWrappers.h"

namespace obelisk
{
	template <typename T>
	class SimpleConcurrentQueue
	{
	public:
		SimpleConcurrentQueue() = default;
		~SimpleConcurrentQueue() = default;

		T pop()
		{
			static_assert(std::is_default_constructible<T>::value,
				"Type must be default constructible to use SimpleConcurrentQueue derived classes queue. Tip: wrap in a std::unique_ptr");

			T item;

			UniqueLock<Mutex> lock(mutex);
			if (!queue.empty())
			{
				// Use swap to allow use with move-only types (e.g. unique_ptr)
				std::swap(queue.front(), item);
				queue.pop_front();
			}
			lock.unlock();

			return item;
		}

		void push(T&& item)
		{
			UniqueLock<Mutex> lock(mutex);
			queue.push_back(std::move(item));
		}

		void clear()
		{
			UniqueLock<Mutex> lock(mutex);
			queue.clear();
		}

		size_t size() const
		{
			UniqueLock<Mutex> lock(mutex);
			return queue.size();
		}

		bool empty() const
		{
			UniqueLock<Mutex> lock(mutex);
			return queue.empty();
		}

	protected:
		std::deque<T> queue;
		mutable Mutex mutex;
	};

	template <typename T>
	// Privately inherits to prevent polymorphic use and unintended base class function use
	class MultipleProducerSingleConsumerQueue : private SimpleConcurrentQueue<T>
	{
	public:

		MultipleProducerSingleConsumerQueue() = default;
		~MultipleProducerSingleConsumerQueue() = default;

		using SimpleConcurrentQueue<T>::clear;
		using SimpleConcurrentQueue<T>::size;
		using SimpleConcurrentQueue<T>::empty;

		T waitForPop()
		{
			T item;

			// Scope for lock
			{
				UniqueLock<Mutex> lock(mutex);
				while (queue.empty())
				{
					condition.wait(lock);
				}

				// Use swap to allow use with move-only types (e.g. unique_ptr)
				std::swap(queue.front(), item);
				queue.pop_front();
			}

			return item;
		}

		enum class WaitResult {SUCCESS,TIMEOUT};
		std::pair<T, WaitResult> waitForPopTimed(std::chrono::milliseconds timeout)
		{
			T item;

			// Scope for lock
			{
				UniqueLock<Mutex> lock(mutex);
				while (queue.empty())
				{
					if (condition.waitFor(lock, timeout) == ConditionVariable::Status::TIMEOUT)
						return{ item, WaitResult::TIMEOUT };
				}

				// Use swap to allow use with move-only types (e.g. unique_ptr)
				std::swap(queue.front(), item);
				queue.pop_front();
			}

			return{ item, WaitResult::SUCCESS };
		}

		void push(const T& item)
		{
			UniqueLock<Mutex> lock(mutex);
			queue.push_back(item);
			lock.unlock();
			condition.notifyOne();
		}

		void push(T&& item)
		{
			UniqueLock<Mutex> lock(mutex);
			queue.push_back(std::move(item));
			lock.unlock();
			condition.notifyOne();
		}

		template <typename IteratorT>
		void push(IteratorT itFirst, IteratorT itLast)
		{
			UniqueLock<Mutex> lock(mutex);
			for (auto it = itFirst; it != itLast; ++it)
				queue.push_back(std::move(*it));
			lock.unlock();
			condition.notifyOne();
		}

	protected:
		using SimpleConcurrentQueue<T>::queue;
		using SimpleConcurrentQueue<T>::mutex;
		mutable ConditionVariable condition;
	};	
}