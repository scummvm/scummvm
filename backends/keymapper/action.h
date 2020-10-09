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

#ifndef COMMON_ACTION_H
#define COMMON_ACTION_H

#include "common/scummsys.h"

#include "common/array.h"
#include "common/events.h"
#include "common/str.h"
#include "common/ustr.h"

namespace Common {

struct KeyActionEntry {
	const char *id;
	const KeyState ks;
	const char *defaultHwId;
	const char *description;
};

struct Action {
	/** unique id used for saving/loading to config */
	const char *id;
	/** Human readable description */
	U32String description;

	/** Event to be sent when mapped key is pressed */
	Event event;

private:
	Array<String> _defaultInputMapping;
	bool _shouldTriggerOnKbdRepeats;

public:
	Action(const char *id, const U32String &description);

	void setEvent(const Event &evt) {
		event = evt;
	}

	void setEvent(const EventType evtType) {
		event = Event();
		event.type = evtType;
	}

	void setCustomBackendActionEvent(const CustomEventType evtType) {
		event = Event();
		event.type = EVENT_CUSTOM_BACKEND_ACTION_START;
		event.customType = evtType;
	}

	void setCustomBackendActionAxisEvent(const CustomEventType evtType) {
		event = Event();
		event.type = EVENT_CUSTOM_BACKEND_ACTION_AXIS;
		event.customType = evtType;
	}

	void setCustomEngineActionEvent(const CustomEventType evtType) {
		event = Event();
		event.type = EVENT_CUSTOM_ENGINE_ACTION_START;
		event.customType = evtType;
	}

	void setKeyEvent(const KeyState &ks) {
		event = Event();
		event.type = EVENT_KEYDOWN;
		event.kbd = ks;
	}

	void setLeftClickEvent() {
		setEvent(EVENT_LBUTTONDOWN);
	}

	void setMiddleClickEvent() {
		setEvent(EVENT_MBUTTONDOWN);
	}

	void setRightClickEvent() {
		setEvent(EVENT_RBUTTONDOWN);
	}

	void setMouseWheelUpEvent() {
		setEvent(EVENT_WHEELUP);
	}

	void setMouseWheelDownEvent() {
		setEvent(EVENT_WHEELDOWN);
	}

	void setX1ClickEvent() {
		setEvent(EVENT_X1BUTTONDOWN);
	}

	void setX2ClickEvent() {
		setEvent(EVENT_X2BUTTONDOWN);
	}

	/**
	 * Allows an action bound to a keyboard event to be repeatedly
	 * triggered by key repeats
	 *
	 * Note that key repeat events should probably not be used for anything
	 * else than text input as they do not trigger when the action is bound
	 * to something else than a keyboard key. Furthermore, the frequency at
	 * which they trigger and whether they trigger at all is operating system
	 * controlled.
	 */
	void allowKbdRepeats() {
		_shouldTriggerOnKbdRepeats = true;
	}

	bool shouldTriggerOnKbdRepeats() const { return _shouldTriggerOnKbdRepeats; }

	/**
	 * Add a default input mapping for the action
	 *
	 * Unknown hardware inputs will be silently ignored.
	 * Having keyboard bindings by default will not cause trouble
	 * on devices without a keyboard.
	 *
	 * @param hwId Hardware input identifier as registered with the keymapper
	 */
	void addDefaultInputMapping(const String &hwId);

	const Array<String> &getDefaultInputMapping() const {
		return _defaultInputMapping;
	}

};

} // End of namespace Common

#endif // #ifndef COMMON_ACTION_H
