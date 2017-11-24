#pragma once

#include <set>

#include "AsyncEvent.h"

namespace obelisk
{
	class EventBase;

	// Subjects with contain Events as members which observers can register to
	// The subject manages an asynchronous event queue which invokes each event's handlers
	class SubjectWithEvents
	{
		template <class...Args>
		friend class Event;

	public:

		SubjectWithEvents() = default;

		// Helper function: Unregisters all handlers from all events owned by this subject
		void unregisterAllHandlers() const
		{
			for (auto const &e : ownedEvents)
				e->unregisterAllHandlers();
		}

	protected:
		~SubjectWithEvents() = default;

	private:

		// Only to be called by Event constructor
		void addOwnedEvent(EventBase &e)
		{
			ownedEvents.insert(&e);
		}

		std::set<EventBase*> ownedEvents;
	};
}
