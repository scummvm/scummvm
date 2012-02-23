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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "common/system.h"

namespace Common {

List<Event> DefaultEventMapper::mapEvent(const Event &ev, EventSource *source) {
	List<Event> events;
	Event mappedEvent;
	if (ev.type == EVENT_KEYDOWN) {
		if (ev.kbd.hasFlags(KBD_CTRL) && ev.kbd.keycode == KEYCODE_F5) {
			mappedEvent.type = EVENT_MAINMENU;
		}
#ifdef ENABLE_VKEYBD
		else if (ev.kbd.keycode == KEYCODE_F7 && ev.kbd.hasFlags(0)) {
			mappedEvent.type = EVENT_VIRTUAL_KEYBOARD;
		}
#endif
#ifdef ENABLE_KEYMAPPER
		else if (ev.kbd.keycode == KEYCODE_F8 && ev.kbd.hasFlags(0)) {
			mappedEvent.type = EVENT_KEYMAPPER_REMAP;
		}
#endif
	}

	// if it didn't get mapped, just pass it through
	if (mappedEvent.type == EVENT_INVALID)
		mappedEvent = ev;
	events.push_back(mappedEvent);
	return events;
}


void DefaultEventMapper::addDelayedEvent(uint32 millis, Event ev) {
	DelayedEventsEntry entry(millis, ev);
	_delayedEvents.push_back(entry);
}

List<Event> DefaultEventMapper::getDelayedEvents() {
	List<Event> events;
	uint32 now = g_system->getMillis();
	for (List<DelayedEventsEntry>::iterator it = _delayedEvents.begin(); it != _delayedEvents.end(); ++it) {
		DelayedEventsEntry entry = (*it);
		if (entry.millis <= now) {
			events.push_back(entry.event);
			_delayedEvents.erase(it);
		}
	}
	return events;
}

} // namespace Common
