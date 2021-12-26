/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_EVENTS_H
#define AGS_EVENTS_H

#include "shared/ac/keycode.h"
#include "common/array.h"
#include "common/queue.h"
#include "common/events.h"

namespace AGS {

class EventsManager {
private:
	Common::Queue<Common::Event> _pendingEvents;
	Common::Queue<Common::Event> _keyEvents;
	Common::Array<bool> _keys;
	Common::Point _mousePos;
	int16 _joystickAxis[32];
	bool _joystickButton[32];
	byte _keyModifierFlags = 0;

	bool isModifierKey(const Common::KeyCode &keycode) const;
	bool isExtendedKey(const Common::KeyCode &keycode) const;

	void updateKeys(const Common::Event &event, bool isDown);
public:
	/**
	 * Converts ags key to ScummVM scancode
	 */
	static bool ags_key_to_scancode(AGS3::eAGSKeyCode key, Common::KeyCode(&kc)[3]);

	/*
	 * Converts a ScummVM event to the ags keycode
	 */
	static AGS3::eAGSKeyCode scummvm_key_to_ags_key(const Common::Event &event);

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
	 * Returns the bitset of currently pressed modifier keys
	 */
	uint getModifierFlags() const {
		return _keyModifierFlags;
	}

	/**
	 * Pushes a keydown event into the keypresses queue,
	 * without updating the key down flag array
	 */
	void pushKeyboardEvent(const Common::Event &evt) {
		_keyEvents.push(evt);
	}

	/**
	 * Returns the next event, if any
	 */
	Common::Event readEvent();

	/**
	 * Sets the mouse position
	 */
	void warpMouse(const Common::Point &newPos);

	/**
	 * Returns true if a given key is pressed
	 */
	bool isKeyPressed(AGS3::eAGSKeyCode key);

	void clearEvents() {
		_pendingEvents.clear();
		_keyEvents.clear();
	}

	/**
	 * Get the current mouse position
	 */
	Common::Point getMousePos() const {
		return _mousePos;
	}

	/**
	 * Gets a joystick axis position
	 */
	int16 getJoystickAxis(byte axis) const {
		assert(axis < 32);
		return _joystickAxis[axis];
	}

	/**
	 * Gets whether a given joystick button is down
	 */
	bool getJoystickButton(byte button) const {
		assert(button < 32);
		return _joystickButton[button];
	}

	/**
	 * Gets whether a given joystick button is down once
	 */
	bool getJoystickButtonOnce(byte button) {
		assert(button < 32);
		bool result = _joystickButton[button];
		_joystickButton[button] = false;
		return result;
	}
};

extern EventsManager *g_events;

} // namespace AGS

#endif
