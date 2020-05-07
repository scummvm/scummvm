/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA4_EVENTS_TIMED_EVENT_MGR_H
#define ULTIMA4_EVENTS_TIMED_EVENT_MGR_H

#include "common/list.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A class for handling timed events.
 */
class TimedEvent {
public:
	/* Typedefs */
	typedef Common::List<TimedEvent *> List;
	typedef void (*Callback)(void *);

	/* Constructors */
	TimedEvent(Callback callback, int interval, void *data = nullptr);

	/* Member functions */
	Callback getCallback() const;
	void *getData();

	/**
	 * Advances the timed event forward a tick.
	 * When (current >= interval), then it executes its callback function.
	 */
	void tick();

	/* Properties */
protected:
	Callback _callback;
	void *_data;
	int _interval;
	int _current;
};

/**
 * A class for managing timed events
 */
class TimedEventMgr {
public:
	/* Typedefs */
	typedef TimedEvent::List List;

	/* Constructors */
	/**
	 * Constructs a timed event manager object.
	 * Adds a timer callback to the SDL subsystem, which
	 * will drive all of the timed events that this object
	 * controls.
	 */
	TimedEventMgr(int baseInterval);

	/**
	 * Destructs a timed event manager object.
	 * It removes the callback timer and un-initializes the
	 * SDL subsystem if there are no other active TimedEventMgr
	 * objects.
	 */
	~TimedEventMgr() {}

	/**
	 * Checks whether the frame time has expired, and if so,
	 * triggers a tick
	 */
	void poll();

	/**
	 * Returns true if the event queue is locked (in use)
	 */
	bool isLocked() const;

	/**
	 * Adds a timed event to the event queue.
	 */
	void add(TimedEvent::Callback theCallback, int interval, void *data = nullptr);

	/**
	 * Removes a timed event from the event queue.
	 */
	List::iterator remove(List::iterator i);
	void remove(TimedEvent *event);
	void remove(TimedEvent::Callback theCallback, void *data = nullptr);

	/**
	 * Runs each of the callback functions of the TimedEvents associated with this manager.
	 */
	void tick();

	/**
	 * Re-initializes the timer manager to a new timer granularity
	 */
	void reset(uint interval);     /**< Re-initializes the event manager to a new base interval */
#if defined(IOS_ULTIMA4)
	bool hasActiveTimer() const;
#endif

private:
	void lock();                /**< Locks the event list */
	void unlock();              /**< Unlocks the event list */

	/* Properties */
protected:
	uint32 _lastTickTime;
	uint32 _baseInterval;
	bool _locked;
	List _events;
	List _deferredRemovals;
#if defined(IOS_ULTIMA4)
	TimedManagerHelper *m_helper;
#endif
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
