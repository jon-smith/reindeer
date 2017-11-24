#pragma once

#include <map>
#include <set>
#include <functional>

#include "LockUtilsT.h"

namespace obelisk
{
	class EventObserver;
	class SubjectWithEvents;

	class EventBase
	{
	public:
		virtual void unregisterHandler(const EventObserver &observer) = 0;
		virtual void unregisterAllHandlers() = 0;
		std::wstring getName() const { return name; }

	protected:
		explicit EventBase(std::wstring name) : name(name){};
		~EventBase(){}

		const std::wstring name;
	};

	// The EventObserver class can register handlers to Events in a SubjectWithEvent class
	// Subclasses must called unregisterFromAllEvents in their destructors if any class members are used by event callbacks
	class EventObserver
	{
    // Helper for static check of event type (improves error messages)
    template <typename EventType>
    void staticCheckEventType()
    {
      static_assert(std::is_base_of<EventBase, EventType>::value, "EventType must be derived from obelisk::EventBase");
    }

	public:
		EventObserver()
		{
			
		}

		virtual ~EventObserver()
		{
			unregisterFromAllEvents();
		}

		template <typename EventType, typename ClassT, typename...Args>
		typename std::enable_if<std::is_base_of<EventObserver, ClassT>::value, void>::type
		registerToEvent(EventType &e, void(ClassT::*fn)(Args...args) const)
		{   
      staticCheckEventType<EventType>();

			LockGuard lk(m);
			e.registerHandler(*this, [=](Args...args){std::mem_fn(fn)(static_cast<ClassT*>(this), args...); });
			registeredEvents[&e.owner].insert(&e);
		}

    // Non-const function version of above
    template <typename EventType, typename ClassT, typename...Args>
    typename std::enable_if<std::is_base_of<EventObserver, ClassT>::value, void>::type
      registerToEvent(EventType &e, void(ClassT::*fn)(Args...args))
    {
      staticCheckEventType<EventType>();

      LockGuard lk(m);
      e.registerHandler(*this, [=](Args...args) {std::mem_fn(fn)(static_cast<ClassT*>(this), args...); });
      registeredEvents[&e.owner].insert(&e);
    }

		template <typename EventType, typename Lambda>
		void registerToEvent(EventType &e, Lambda&& fn)
		{
      staticCheckEventType<EventType>();

			LockGuard lk(m);
			e.registerHandler(*this, fn);
			registeredEvents[&e.owner].insert(&e);
		}

		template <typename EventType>
		void unregisterFromEvent(EventType &e)
		{
      staticCheckEventType<EventType>();

			LockGuard lk(m);
			e.unregisterHandler(*this);

			for (auto &owner : registeredEvents)
				owner.second.erase(&e);
		}

		template <typename EventType>
		void eraseRegisteredEvents(EventType &e)
		{
      staticCheckEventType<EventType>();

			LockGuard lk(m);
			for (auto &owner : registeredEvents)
				owner.second.erase(&e);
		}

		void unregisterFromAllEvents(const SubjectWithEvents &owner)
		{
			LockGuard lk(m);
			auto o = registeredEvents.find(&owner);
			if (o != registeredEvents.end())
			{
				for (auto const &e : o->second)
					e->unregisterHandler(*this);
				registeredEvents.erase(o);
			}
		}

		void unregisterFromAllEvents()
		{
			LockGuard lk(m);
			for (auto const &owner : registeredEvents)
				for (auto const &e : owner.second)
					e->unregisterHandler(*this);

			registeredEvents.clear();
		}

	private:
		mutable Mutex m;
		std::map<const SubjectWithEvents*, std::set<EventBase*>> registeredEvents;
	};

	// This allows us to temporarily register a lambda to an event
	template<typename EventType, typename Lambda>
	class ScopedEventObserver : public EventObserver {
	public:
		ScopedEventObserver(EventType& e, Lambda&& l)
		{
			registerToEvent(e, l);
		}
		ScopedEventObserver(const ScopedEventObserver && v) : EventObserver(v)
		{}

	private:
		// disable copying
		ScopedEventObserver(const ScopedEventObserver &);
		ScopedEventObserver & operator=(const ScopedEventObserver &);
	};

	template <typename EventType, typename Lambda>
	auto createLambdaObserver( EventType& e, Lambda&& f) -> ScopedEventObserver<EventType, Lambda> {
		return ScopedEventObserver<EventType,Lambda>(e,std::move(f));
	}
}
