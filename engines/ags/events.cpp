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

#include "ags/events.h"
#include "common/system.h"

namespace AGS {

EventsManager *g_events;

EventsManager::EventsManager() {
	g_events = this;
}

EventsManager::~EventsManager() {
	g_events = nullptr;
}

void EventsManager::pollEvents() {
	Common::Event e;

	while (g_system->getEventManager()->pollEvent(e)) {
		if (e.type == Common::EVENT_KEYDOWN) {
			// Add keypresses to the pending key list
			_pendingKeys.push(e.kbd.keycode);
		} else {
			// Add other event types to the pending events queue. If the event is a
			// mouse move and the prior one was also, then discard the prior one.
			// This'll help prevent too many mouse move events accumulating
			if (e.type == Common::EVENT_MOUSEMOVE && !_pendingEvents.empty() &&
					_pendingEvents.back().type == Common::EVENT_MOUSEMOVE)
				_pendingEvents.back() = e;
			else
				_pendingEvents.push(e);
		}
	}
}

bool EventsManager::keypressed() {
	pollEvents();
	return !_pendingKeys.empty();
}

int EventsManager::readKey() {
	pollEvents();
	return _pendingKeys.empty() ? 0 : _pendingKeys.pop();
}

Common::Event EventsManager::readEvent() {
	pollEvents();
	return _pendingEvents.empty() ? Common::Event() : _pendingEvents.pop();
}

void EventsManager::warpMouse(const Common::Point &newPos) {
	g_system->warpMouse(newPos.x, newPos.y);
}

} // namespace AGS
