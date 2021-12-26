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

#include <nds.h>

#include "backends/events/ds/ds-events.h"
#include "backends/platform/ds/osystem_ds.h"

bool DSEventSource::pollEvent(Common::Event &event) {
	// Ensure the mixer and timers are updated frequently
	g_system->delayMillis(0);

	if (_eventQueue.empty()) {
		if (!_firstPoll) {
			_firstPoll = true;
			return false;
		}


		addEventsToQueue();
		if (_eventQueue.empty())
			return false;

		_firstPoll = false;
	}

	event = _eventQueue.pop();

	if (Common::isMouseEvent(event)) {
		g_system->warpMouse(event.mouse.x, event.mouse.y);
	}

	return true;
}

void DSEventSource::addJoyButtonEvent(u32 keysPressed, u32 keysReleased, u32 ndsKey, uint8 svmButton) {
	if (keysPressed & ndsKey || keysReleased & ndsKey) {
		Common::Event event;
		event.type = (keysPressed & ndsKey) ? Common::EVENT_JOYBUTTON_DOWN : Common::EVENT_JOYBUTTON_UP;
		event.joystick.button = svmButton;

		_eventQueue.push(event);
	}
}

void DSEventSource::addEventsToQueue() {
	Common::Event event;

	scanKeys();
	uint32 held = keysHeld(), keysPressed = keysDown(), keysReleased = keysUp();

	// Touch screen events
	if (_handleTouch) {
		if (held & KEY_TOUCH) {
			touchPosition touchPos;
			touchRead(&touchPos);
			event.mouse = dynamic_cast<OSystem_DS *>(g_system)->transformPoint(touchPos.px, touchPos.py);

			if (event.mouse.x != _lastTouch.x || event.mouse.y != _lastTouch.y) {
				event.type = Common::EVENT_MOUSEMOVE;
				_eventQueue.push(event);
			}
			if (keysPressed & KEY_TOUCH) {
				event.type = Common::EVENT_LBUTTONDOWN;
				_eventQueue.push(event);
			}

			_lastTouch = event.mouse;
		} else if (keysReleased & KEY_TOUCH) {
			event.mouse = _lastTouch;
			event.type = Common::EVENT_LBUTTONUP;
			_eventQueue.push(event);
		}
	}

	// Button events
	addJoyButtonEvent(keysPressed, keysReleased, KEY_L,      Common::JOYSTICK_BUTTON_LEFT_SHOULDER);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_R,      Common::JOYSTICK_BUTTON_RIGHT_SHOULDER);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_A,      Common::JOYSTICK_BUTTON_A);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_B,      Common::JOYSTICK_BUTTON_B);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_X,      Common::JOYSTICK_BUTTON_X);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_Y,      Common::JOYSTICK_BUTTON_Y);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_UP,     Common::JOYSTICK_BUTTON_DPAD_UP);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_DOWN,   Common::JOYSTICK_BUTTON_DPAD_DOWN);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_LEFT,   Common::JOYSTICK_BUTTON_DPAD_LEFT);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_RIGHT,  Common::JOYSTICK_BUTTON_DPAD_RIGHT);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_START,  Common::JOYSTICK_BUTTON_START);
	addJoyButtonEvent(keysPressed, keysReleased, KEY_SELECT, Common::JOYSTICK_BUTTON_BACK);
}
