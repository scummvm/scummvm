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

#ifndef CHEWY_EVENTS_H
#define CHEWY_EVENTS_H

#include "common/array.h"
#include "common/queue.h"
#include "common/events.h"

namespace Chewy {

class EventsManager {
private:
	Common::Queue<Common::Event> _pendingEvents;
	Common::Queue<Common::Event> _keyEvents;

public:
	EventsManager();
	~EventsManager();

	/**
	 * Poll any pending events
	 */
	void pollEvents();

	/**
	 * Returns true if any unprocessed keyboard events are pending
	 */
	bool keyEventPending() const {
		return !_keyEvents.empty();
	}

	/**
	 * Returns the next pending unprocessed keyboard event
	 */
	Common::Event getPendingKeyEvent() {
		return _keyEvents.pop();
	}

	/**
	 * Returns the next event, if any
	 */
	Common::Event readEvent();

	/**
	 * Sets the mouse position
	 */
	void warpMouse(const Common::Point &newPos);

	void clearEvents() {
		_pendingEvents.clear();
		_keyEvents.clear();
	}
};

extern EventsManager *g_events;

} // namespace Chewy

#endif
