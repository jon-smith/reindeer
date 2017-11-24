#pragma once

#include <map>
#include <set>
#include <sstream>

#include "ObeliskMeta.h"
#include "SubjectWithEvents.h"
#include "EventObserver.h"
#include "PlatformSpecific.h"
#include "GeneralHelpers.h"

namespace obelisk
{
	class EventObserver;
	class SubjectWithEvents;
	
	// Event class allows registering a std::function<void(Args...)> with an observer
	// The association with the observer is only there to allow easy unregistering of handlers
	template <typename...Args>
	class Event : public EventBase
	{
		friend class EventObserver;
		const bool outputEventObserverDebugInfo = false;

	public:
		Event() = delete;

		Event(SubjectWithEvents &owner, std::wstring name) :
			EventBase(name),
			owner(owner)
		{
			owner.addOwnedEvent(*this);
		}

		virtual ~Event()
		{
			for (auto &h : handlers)
				h.first->eraseRegisteredEvents(*this);
		}

		void unregisterAllHandlers() override final
		{
			LockGuard lk(m);
			
			for (auto &h : handlers)
				h.first->eraseRegisteredEvents(*this);

			handlers.clear();
		}

		void invokeHandlers(Args&...args) const
		{
			LockGuard lk(m);
			for (const auto &h : handlers)
				h.second(args...);

			if (outputEventObserverDebugInfo)
				outputDebugInfo();
		}

		// Only enable invokeHandlers r-value overload if we have arguments
		// Otherwise it resolves to the same as the above function
		template <typename DummyT = typename std::enable_if<CountArgs<Args...>::value != 0, void>::type>
		void invokeHandlers(Args&&...args) const
		{
			// Call the l-value reference invoke handlers
			invokeHandlers(std::add_lvalue_reference<Args>(args)...);
		}

	private:

		// Should only be accessed using EventObserver helper functions
		void registerHandler(EventObserver &observer, std::function<void(Args&...)> handler)
		{
			LockGuard lk(m);
			handlers[&observer] = handler;
		}

		void unregisterHandler(const EventObserver &observer) override final
		{
			LockGuard lk(m);
			handlers.erase(const_cast<EventObserver*>(&observer));
		}

		void outputDebugInfo() const
		{
			// Output debug info about event and observers
			std::wstringstream debugString;
			debugString << L"Event " << obelisk::getSimpleRuntimeTypeName(*this) << " " << name << " called handlers for observers:\n";
		
			for (const auto &h : handlers)
				if (h.first)
					debugString << obelisk::getSimpleRuntimeTypeName(*h.first) << "\n";

			debugString << (handlers.empty() ? std::wstring(L"No registered observers\n") : std::wstring());
			platform_utilities::outputDebugString(debugString.str());
		}

	protected:
		mutable Mutex m;
		SubjectWithEvents &owner;
		std::map<EventObserver*, std::function<void(Args&...)>> handlers;
	};
}
