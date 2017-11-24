#pragma once

#include "Event.h"
#include "AsyncEventQueue.h"
#include "ObeliskMeta.h"

namespace obelisk
{
	// AsyncEvent adds functionality for invoking event handlers asynchronously (through a queue)
	template <typename...Args>
	class AsyncEvent : public Event<Args...>
	{
		static_assert(ContainsReference<Args...>::value == false, "Async event arguments should not be reference types");

		friend class EventObserver;

	public:

		AsyncEvent() = delete;

		AsyncEvent(SubjectWithEvents &owner, std::wstring name) :
			Event(owner, name),
			eventHandler(std::function<void(Args...)>([this](Args...args)
		{
			invokeHandlers(args...);
		}))
		{
		}

		// Add handlers to a queue for delayed invocation
		// Events already in queue will not be added again (weak guarantee)
		void asyncInvokeHandlers(Args...args) const
		{
			if (eventHandler.empty())
				eventHandler.deferredEventInvoke(args...);
		}

		// Add handlers to a queue for delayed invocation
		// Events will always be added into queue, regardless of whether they are in already
		void asyncInvokeHandlers_AllowDuplicateEvents(Args...args) const
		{
			eventHandler.deferredEventInvoke(args...);
		}

	private:

		mutable AsyncEventHandler<Args...> eventHandler;
	};
}
