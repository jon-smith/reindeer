#pragma once

#include <tuple>

#include "SimpleAsyncTask.h"
#include "ConcurrentQueueT.h"
#include "MutexedObject.h"

namespace obelisk
{
	// useSharedHandling==true uses a single queue and consumer thread for each combination of Args
	// useSharedHandling==false uses a queue/thread per event
	template <bool useSharedHandling, typename...Args>
	class AsyncEventHandler
	{
		using ArgListUniqPtr = std::unique_ptr<std::tuple<Args...>>;

		// Queue implementation
		// Concurrent queue containing a unique_ptr to a tuple of arguments
		// Unique_ptr is used to allow null for the queue termination argument
		class EventParameterQueue : private MultipleProducerSingleConsumerQueue<ArgListUniqPtr>
		{
		public:
			using MultipleProducerSingleConsumerQueue<ArgListUniqPtr>::push;
			using MultipleProducerSingleConsumerQueue<ArgListUniqPtr>::waitForPop;
			using MultipleProducerSingleConsumerQueue<ArgListUniqPtr>::empty;
			using MultipleProducerSingleConsumerQueue<ArgListUniqPtr>::clear;
		};

		// EventParameterQueue shared between all instances of AsyncEventHandler
		class SharedEventParameterQueue: private MultipleProducerSingleConsumerQueue<
			std::pair<AsyncEventHandler*, ArgListUniqPtr>>
		{
		public:
			using ValueT = std::pair<AsyncEventHandler*, ArgListUniqPtr>;
			using MultipleProducerSingleConsumerQueue<ValueT>::push;
			using MultipleProducerSingleConsumerQueue<ValueT>::waitForPop;
			using MultipleProducerSingleConsumerQueue<ValueT>::clear;

			// Count the number of entries for the specified handler
			unsigned count(const AsyncEventHandler &handler) const
			{
				unsigned count = 0;

				LockGuard lock(mutex);
				for (auto const &i : queue)
					if (i.first == &handler)
						++count;

				return count;
			}
		};

		class SharedEventHandlerImpl
		{
			MutexedObject<std::set<AsyncEventHandler*>> activeObjects;
			SimpleAsyncTask sharedConsumer;
			obelisk::Mutex sharedHandlerMutex;
			unsigned objectCount;

		public:

			SharedEventParameterQueue sharedQueue;

			void consumerFunc()
			{
				// Keep getting arguments until nullptr is passed for handler
				bool run = true;
				while (run)
				{
					auto args = sharedQueue.waitForPop();
					if (args.second)
					{
						activeObjects.lockedAccess([&args](const std::set<AsyncEventHandler*> &objects)
						{
							if (auto fnIt = objects.find(args.first);
							fnIt != objects.end())
							{
								(*fnIt)->invokeHandlers(*args.second);
							}
						});
					}
					else
						run = false;
				}
			}

			void onConstruct(AsyncEventHandler &handler)
			{
				lockAndCall(sharedHandlerMutex, [this](){
					if (objectCount == 0)
					{
						// Create consumer
						sharedConsumer = SimpleAsyncTask([this]()
						{
							consumerFunc();
						});
					}
					++objectCount;
				});

				activeObjects.lockedModify([&handler](std::set<AsyncEventHandler*> &objects)
				{
					objects.insert(&handler);
				});
			}

			void onDestruct(AsyncEventHandler &handler)
			{
				activeObjects.lockedModify([&handler](std::set<AsyncEventHandler*> &objects)
				{
					objects.erase(&handler);
				});

				LockGuard lk(sharedHandlerMutex);
				assert(objectCount > 0);
				--objectCount;
				if (objectCount == 0)
				{
					// Destroy consumer
					sharedQueue.clear();
					sharedQueue.push({ nullptr, nullptr });
					sharedConsumer.wait();
				}
			}
		};

		static SharedEventHandlerImpl &getSharedHandler()
		{
			static_assert(useSharedHandling);
			static SharedEventHandlerImpl sharedHandler;
			return sharedHandler;
		}

		template <typename...Args>
		class SharedHandlingImpl
		{
		public:
			SharedHandlingImpl(AsyncEventHandler<true, Args...> &parent) :
				parent(parent)
			{
				getSharedHandler().onConstruct(parent);
			}
			~SharedHandlingImpl()
			{
				getSharedHandler().onDestruct(parent);
			}
			bool empty() const
			{
				return getSharedHandler().sharedQueue.count(parent) == 0;
			}
			void deferredEventInvoke(Args...args)
			{
				getSharedHandler().sharedQueue.push({ &parent, std::make_unique<std::tuple<Args...>>(args...) });
			}
		private:
			AsyncEventHandler<true, Args...> &parent;
		};

		template <typename...Args>
		class UniqueHandlingImpl
		{
		public:
			UniqueHandlingImpl(AsyncEventHandler<false, Args...> &parent) :
				parent(parent)
			{
				consumer = SimpleAsyncTask([this]()
				{
					// Keep getting arguments until nullptr is passed
					while (auto args = queue.waitForPop())
					{
						parent.invokeHandlers(*args);
					}
				});
			}
			~UniqueHandlingImpl()
			{
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
			AsyncEventHandler<false, Args...> &parent;
			SimpleAsyncTask consumer;
			EventParameterQueue queue;
		};

		friend UniqueHandlingImpl<Args...>;

		template<bool shared>
		class ImplT;

		friend ImplT<useSharedHandling>;
		
		template<>
		class ImplT<true>
		{
		public:
			ImplT(AsyncEventHandler<true, Args...> &parent) :
				parent(parent)
			{
				getSharedHandler().onConstruct(parent);
			}
			~ImplT()
			{
				getSharedHandler().onDestruct(parent);
			}
			bool empty() const
			{
				return getSharedHandler().sharedQueue.count(parent) == 0;
			}
			void deferredEventInvoke(Args...args)
			{
				getSharedHandler().sharedQueue.push({ &parent, std::make_unique<std::tuple<Args...>>(args...) });
			}
		private:
			AsyncEventHandler<true, Args...> &parent;
		};

		template<>
		class ImplT<false>
		{
		public:
			ImplT(AsyncEventHandler<false, Args...> &parent) :
				parent(parent)
			{
				consumer = SimpleAsyncTask([this]()
				{
					// Keep getting arguments until nullptr is passed
					while (auto args = queue.waitForPop())
					{
						this->parent.invokeHandlers(*args);
					}
				});
			}
			~ImplT()
			{
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
			AsyncEventHandler<false, Args...> &parent;
			SimpleAsyncTask consumer;
			EventParameterQueue queue;
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
			invokeFunc(invokeFunc),
			impl(*this)
		{
		}

		~AsyncEventHandler() = default;

		bool empty() const
		{
			return impl.empty();
		}

		void deferredEventInvoke(Args...args)
		{
			impl.deferredEventInvoke(args...);
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

		std::function<void(Args...)> invokeFunc;
		ImplT<useSharedHandling> impl;
	};
}