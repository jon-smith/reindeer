#pragma once

#include <tuple>

#include "SimpleAsyncTask.h"
#include "ConcurrentQueueT.h"

namespace obelisk
{
	template <typename...Args>
	class AsyncEventHandler
	{
		// Queue implementation
		// Concurrent queue containing a unique_ptr to a tuple of arguments
		// Unique_ptr is used to allow null for the queue termination argument
		class EventParameterQueue : private MultipleProducerSingleConsumerQueue<std::unique_ptr<std::tuple<Args...>>>
		{
			typedef std::unique_ptr<std::tuple<Args...>> ArgList;

		public:
			using MultipleProducerSingleConsumerQueue<ArgList>::push;
			using MultipleProducerSingleConsumerQueue<ArgList>::waitForPop;
			using MultipleProducerSingleConsumerQueue<ArgList>::empty;
			using MultipleProducerSingleConsumerQueue<ArgList>::clear;
		};

		// Sequence generator helper structures
		// These are used to unpack a tuple into arguments for a function call
		template <std::size_t... Ts>
		struct Index {};

		template <std::size_t N, std::size_t... Ts>
		struct GenerateSequence : GenerateSequence<N - 1, N - 1, Ts...> {};

		template <std::size_t... Ts>
		struct GenerateSequence<0, Ts...> : Index<Ts...>{};

	public:

		template <typename InvokeEventHandlersFunc>
		AsyncEventHandler(InvokeEventHandlersFunc invokeFunc) : 
		invokeFunc(invokeFunc)
		{
			consumer = SimpleAsyncTask([this]()
			{
				// Keep getting arguments until nullptr is passed
				while (auto args = queue.waitForPop())
				{
					invokeHandlers(*args);
				}
			});
		}

		~AsyncEventHandler()
		{
			// Clear and pass nullptr to the queue, to terminate the thread
			queue.clear();
			queue.push(nullptr);
			consumer.wait();
		}

		bool empty() const
		{
			return queue.empty();
		}

		void deferredEventInvoke(Args...args)
		{
			queue.push(std::make_unique<std::tuple<Args...>>(args...));
		}

	private:

		template <std::size_t... Is>
		void invokeHandlers(std::tuple<Args...>& tup, Index<Is...>)
		{
			invokeFunc(std::get<Is>(tup)...);
		}

		void invokeHandlers(std::tuple<Args...>& tup)
		{
			invokeHandlers(tup, GenerateSequence<sizeof...(Args)>{});
		}

		SimpleAsyncTask consumer;
		std::function<void(Args...)> invokeFunc;
		EventParameterQueue queue;
	};
}