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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

#include "common/events.h"

namespace Bolt {

bool XpLib::initEvents() {
	for (int16 i = 0; i < 40; i++) {
		g_events[i].next = g_eventFreeList;
		g_eventFreeList = &g_events[i];
	}

	g_inactivityTimerId = 0;
	g_inactivityTimeout = 0;
	g_mouseButtonState = 0;
	g_mouseButtonPrev = 0;
	g_eventMouseMoved = 0;
	g_eventKeyStates = 0;

	return true;
}

void XpLib::shutdownEvents() {
	g_eventFreeList = nullptr;
	g_eventQueueTail = nullptr;
	g_eventQueueHead = nullptr;

	if (g_inactivityTimerId != 0) {
		killTimer(g_inactivityTimerId);
		g_inactivityTimerId = 0;
	}
}

int16 XpLib::getEvent(int16 filter, uint32 *outData) {
	pumpMessages();

	// Search event queue for matching event type...
	XPEvent *node = g_eventQueueHead;
	if (filter != 0) {
		while (node) {
			if (node->type == filter)
				break;

			node = node->next;
		}
	}

	if (!node)
		return etEmpty;

	// Remove from queue...
	unlinkEvent(node);

	// Copy event data to caller...
	*outData = node->payload;

	// Return node to free list...
	node->next = g_eventFreeList;
	g_eventFreeList = node;

	return node->type;
}

int16 XpLib::peekEvent(int16 filter, uint32 *outData) {
	pumpMessages();

	XPEvent *node = g_eventQueueHead;
	if (filter != 0) {
		while (node) {
			if (node->type == filter)
				break;

			node = node->next;
		}
	}

	if (!node)
		return 0;

	// Copy data but don't remove from queue...
	*outData = node->payload;
	return node->type;
}

void XpLib::unlinkEvent(XPEvent *node) {
	// Remove node from doubly-linked event queue...
	if (node->prev) {
		XPEvent *prev = node->prev;
		prev->next = node->next;
	} else {
		g_eventQueueHead = node->next;
	}

	if (node->next) {
		XPEvent *next = node->next;
		next->prev = node->prev;
	} else {
		g_eventQueueTail = node->prev;
	}
}

void XpLib::enqueueEvent(XPEvent *node) {
	// Append node to tail of event queue...
	node->next = nullptr;

	if (g_eventQueueTail) {
		XPEvent *tail = g_eventQueueTail;
		tail->next = node;
		node->prev = g_eventQueueTail;
		g_eventQueueTail = node;
	} else {
		// Queue was empty...
		g_eventQueueTail = node;
		g_eventQueueHead = node;
		node->prev = nullptr;
	}
}

void XpLib::pumpMessages() {
	Common::Event event;
	uint32 timerData;
	bool mouseMoved = false;

	// Drain any pending sound events...
	while (pollSound(&timerData) && !_bolt->shouldQuit()) {
		updateTimers();
		postEvent(etSound, timerData);
	}

	cycleColors();

	// Check inactivity timeout...
	if (g_inactivityDeadline != 0) {
		uint32 now = _bolt->_system->getMillis();
		if (now > g_inactivityDeadline) {
			g_inactivityDeadline = 0;
			setCursorPos(160, 100);
			postJoystickEvent(0, 0, 0);
		}
	}

	// Process all pending events...
	while (_bolt->_eventMan->pollEvent(event) && !_bolt->shouldQuit()) {
		switch (event.type) {
		case Bolt::CustomEventType::EVENT_TIMER:
			handleTimer(event.customType);
			break;

		case Common::EVENT_MOUSEMOVE:
			g_lastRegisteredMousePos.x = event.mouse.x;
			g_lastRegisteredMousePos.y = event.mouse.y;
			handleMouseMove(&mouseMoved);
			break;

		case Common::EVENT_LBUTTONDOWN:
			handleMouseButton(1, 0);
			break;

		case Common::EVENT_LBUTTONUP:
			handleMouseButton(0, 0);
			break;

		case Common::EVENT_RBUTTONDOWN:
			handleMouseButton(1, 1);
			break;

		case Common::EVENT_RBUTTONUP:
			handleMouseButton(0, 1);
			break;

		case Common::EVENT_MBUTTONDOWN:
			handleMouseButton(1, 2);
			break;

		case Common::EVENT_MBUTTONUP:
			handleMouseButton(0, 2);
			break;

		case Common::EVENT_KEYDOWN:
			handleKey(event.kbd.keycode, 1);
			break;

		case Common::EVENT_KEYUP:
			handleKey(event.kbd.keycode, 0);
			break;

		default:
			break;
		}
	}

	if (mouseMoved)
		updateCursorPosition();

	updateTimers();
	_bolt->_system->delayMillis(5);
}

void XpLib::handleTimer(uint32 timerId) {
	if (timerId != g_inactivityTimerId) {
		postEvent(etTimer, timerId);
		return;
	}

	if (g_inactivityCountdown != 0) {
		g_inactivityCountdown--;
		if (g_inactivityCountdown == 0)
			postEvent(etInactivity, 0);
	}

	if (g_cursorBlinkCountdown != 0) {
		g_cursorBlinkCountdown--;
		if (g_cursorBlinkCountdown == 0)
			activateScreenSaver();
	}

	if (g_inactivityCountdown != 0 || g_cursorBlinkCountdown != 0) {
		g_inactivityTimerId = startTimer(1000);
	} else {
		g_inactivityTimerId = 0;
	}
}

void XpLib::handleMouseMove(bool *mouseMoved) {
	resetInactivity();

	if (g_eventKeyStates == eksMouseMode) {
		*mouseMoved = true;
		int16 x, y;
		readCursor(nullptr, &x, &y);
		postEvent(etMouseMove, ((uint32)x << 16) | (int16)y);
		g_eventMouseMoved = 0;
	} else if (g_eventKeyStates == eksJoystickMode) {
		int16 joyX, joyY;
		readJoystick(&joyX, &joyY);

		int16 dx = (joyX < 155) ? -1 : (joyX > 165) ? 1 : 0;
		int16 dy = (joyY < 96) ? -1 :  (joyY > 104) ? 1 : 0;

		if (dx != 0 || dy != 0) {
			setCursorPos(160, 100);
			postJoystickEvent(0, dx, dy);
		}

		// Set inactivity deadline 100ms from now...
		g_inactivityDeadline = _bolt->_system->getMillis() + 100;
	}
}

void XpLib::handleMouseButton(int16 down, int16 button) {
	resetInactivity();

	if (g_eventKeyStates == eksMouseMode) {
		int16 x, y;
		readCursor(nullptr, &x, &y);
		postEvent(etMouseMove, ((uint32)x << 16) | (int16)y);
	}

	if (down) {
		g_mouseButtonPrev |= (1 << button);
		postEvent(etMouseDown, (uint32)button);
	} else {
		g_mouseButtonPrev &= ~(1 << button);
		postEvent(etMouseUp, (uint32)button);
	}
}

void XpLib::handleKey(Common::KeyCode vkey, int16 down) {
	resetInactivity();

	if (vkey == Common::KEYCODE_ESCAPE) {
		if (g_mouseButtonState != down) {
			if (g_eventKeyStates == eksMouseMode) {
				int16 x, y;
				readCursor(nullptr, &x, &y);
				postEvent(etMouseMove, ((uint32)x << 16) | (int16)y);
			}

			g_mouseButtonState = down;
			postEvent(down ? etMouseDown : etMouseUp, 0);
		}

		return;
	}

	int8 *dirPtr = nullptr;
	switch (vkey) {
	case Common::KEYCODE_LEFT:
		dirPtr = &g_keyStateLeft;
		break; // VK_LEFT
	case Common::KEYCODE_UP:
		dirPtr = &g_keyStateUp;
		break; // VK_UP
	case Common::KEYCODE_RIGHT:
		dirPtr = &g_keyStateRight;
		break; // VK_RIGHT
	case Common::KEYCODE_DOWN:
		dirPtr = &g_keyStateDown;
		break; // VK_DOWN
	}

	if (dirPtr == nullptr)
		return;

	if (*dirPtr == (int8)down)
		return;

	*dirPtr = (int8)down;

	if (g_eventKeyStates == eksJoystickMode) {
		int16 dx = g_keyStateRight - g_keyStateLeft;
		int16 dy = g_keyStateDown - g_keyStateUp;
		postJoystickEvent(1, dx, dy);
	}
}

void XpLib::postJoystickEvent(int16 source, int16 dx, int16 dy) {
	g_eventMouseMoved = source;

	if (dx != g_lastJoystickX || dy != g_lastJoystickY) {
		postEvent(etJoystick, ((uint32)dx << 16) | (uint16)dy);
		g_lastJoystickX = dx;
		g_lastJoystickY = dy;
	}
}

void XpLib::postEvent(XPEventTypes type, uint32 data) {
	if (type == etMouseMove) {
		if (g_lastMouseEventData == data)
			return;

		g_lastMouseEventData = data;
		if (g_eventQueueTail && g_eventQueueTail->type == etMouseMove) {
			g_eventQueueTail->payload = data;
			return;
		}
	}

	XPEvent *node;
	if (g_eventFreeList) {
		// Allocate from free list...
		node = g_eventFreeList;
		g_eventFreeList = g_eventFreeList->next;
	} else {
		// Queue full: try to evict a droppable event...
		if (!canDropEvent(type))
			return;

		// Walk queue from tail looking for droppable event...
		node = g_eventQueueTail;
		while (node) {
			if (canDropEvent(node->type))
				break;

			node = node->prev;
		}

		if (!node)
			node = g_eventQueueHead;

		unlinkEvent(node);
	}

	node->type = type;
	node->payload = data;
	node->next = nullptr;
	enqueueEvent(node);
}

bool XpLib::canDropEvent(int16 type) {
	switch (type) {
	case etMouseMove:
	case etMouseDown:
	case etMouseUp:
	case etJoystick:
		return false;
	default:
		return true;
	}
}

int16 XpLib::setInactivityTimer(int16 seconds) {
	int16 prev = g_inactivityTimerValue;

	g_inactivityTimerValue = seconds;
	g_inactivityCountdown = seconds;

	if (seconds != 0 && g_inactivityTimerId == 0)
		g_inactivityTimerId = startTimer(1000);

	// Drain any pending inactivity events from the queue...
	uint32 payloadDummy;

	// getEvent calls pumpMessages which calls pollEvent and delayMillis...
	while (getEvent(etInactivity, &payloadDummy) == etInactivity && !_bolt->shouldQuit());

	return prev;
}

int16 XpLib::setScreenSaverTimer(int16 seconds) {
	int16 prev = g_screenSaverTimerValue;

	g_screenSaverTimerValue = seconds;
	g_cursorBlinkCountdown = seconds;

	if (seconds != 0 && g_inactivityTimerId == 0)
		g_inactivityTimerId = startTimer(1000);

	return prev;
}

void XpLib::activateScreenSaver() {
	g_inactivityTimeout = 1;
	setScreenBrightness(25); // Dim to 25%...
}

void XpLib::resetInactivity() {
	if (g_inactivityTimeout != 0) {
		setScreenBrightness(100); // Restore full brightness...
		g_inactivityTimeout = 0;
	}

	// Reset screensaver countdown
	g_cursorBlinkCountdown = g_screenSaverTimerValue;

	if (g_screenSaverTimerValue != 0 && g_inactivityTimerId == 0)
		g_inactivityTimerId = startTimer(1000);
}

bool XpLib::enableController() {
	// Don't switch if already in mouse mode...
	if (g_eventKeyStates == eksMouseMode)
		return false;

	g_eventKeyStates = eksJoystickMode;
	setCursorPos(160, 100);
	return true;
}

void XpLib::disableController() {
	g_eventKeyStates = eksInputOff;
	g_inactivityDeadline = 0;
}

void XpLib::enableMouse() {
	g_eventKeyStates = eksMouseMode;
}

void XpLib::disableMouse() {
	g_eventKeyStates = eksInputOff;
}

int16 XpLib::getButtonState() {
	return g_mouseButtonPrev | g_mouseButtonState;
}

} // End of namespace Bolt
