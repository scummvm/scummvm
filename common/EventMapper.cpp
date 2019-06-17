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

#include "common/events.h"

#include "common/system.h"
#include "common/textconsole.h"

namespace Common {

List<Event> DefaultEventMapper::mapEvent(const Event &ev, EventSource *source) {
	List<Event> events;
	Event mappedEvent;
#ifdef ENABLE_VKEYBD
	// Trigger virtual keyboard on long press of more than 1 second
	// of middle mouse button.
	const uint32 vkeybdTime = 1000;

	static uint32 vkeybdThen = 0;

	if (ev.type == EVENT_MBUTTONDOWN) {
		vkeybdThen = g_system->getMillis();
	}

	if (ev.type == EVENT_MBUTTONUP) {
		if ((g_system->getMillis() - vkeybdThen) >= vkeybdTime) {
			mappedEvent.type = EVENT_VIRTUAL_KEYBOARD;

			// Avoid blocking event from engine.
			addDelayedEvent(100, ev);
		}
	}
#endif

	if (ev.type == EVENT_KEYDOWN) {
		if (ev.kbd.hasFlags(KBD_CTRL) && ev.kbd.keycode == KEYCODE_F5) {
			mappedEvent.type = EVENT_MAINMENU;
		}
#ifdef ENABLE_VKEYBD
		else if (ev.kbd.hasFlags(KBD_CTRL) && ev.kbd.keycode == KEYCODE_F7) {
			mappedEvent.type = EVENT_VIRTUAL_KEYBOARD;

			// Avoid blocking CTRL-F7 events from engine.
			addDelayedEvent(100, ev);
		}
#endif
#ifdef ENABLE_KEYMAPPER
		else if (ev.kbd.hasFlags(KBD_CTRL) && ev.kbd.keycode == KEYCODE_F8) {
			mappedEvent.type = EVENT_KEYMAPPER_REMAP;

			// Avoid blocking CTRL-F8 events from engine.
			addDelayedEvent(100, ev);
		}
#endif
	}

	if (ev.type == EVENT_JOYBUTTON_DOWN) {
		if (ev.joystick.button == JOYSTICK_BUTTON_START || ev.joystick.button == JOYSTICK_BUTTON_GUIDE) {
			mappedEvent.type = EVENT_MAINMENU;
		}
	}

	// if it didn't get mapped, just pass it through
	if (mappedEvent.type == EVENT_INVALID)
		mappedEvent = ev;

#ifdef ENABLE_KEYMAPPER
	// TODO: this check is not needed post-split
	if (mappedEvent.type == EVENT_CUSTOM_BACKEND_HARDWARE) {
		warning("EVENT_CUSTOM_BACKEND_HARDWARE was not mapped");
		return List<Event>();
	}
#endif

	events.push_back(mappedEvent);
	return events;
}


void DefaultEventMapper::addDelayedEvent(uint32 millis, Event ev) {
	if (_delayedEvents.empty()) {
		_delayedEffectiveTime = g_system->getMillis() + millis;
		millis = 0;
	}
	DelayedEventsEntry entry = DelayedEventsEntry(millis, ev);
	_delayedEvents.push(entry);
}

List<Event> DefaultEventMapper::getDelayedEvents() {
	List<Event> events;

	if (_delayedEvents.empty())
		return events;

	uint32 now = g_system->getMillis();

	while (!_delayedEvents.empty() && now >= _delayedEffectiveTime) {
		DelayedEventsEntry entry = _delayedEvents.pop();
		if (!_delayedEvents.empty())
			_delayedEffectiveTime += _delayedEvents.front().timerOffset;
		events.push_back(entry.event);
	}
	return events;
}

} // namespace Common
