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
		_events[i].next = _eventFreeList;
		_eventFreeList = &_events[i];
	}

	_inactivityTimerId = 0;
	_inactivityTimeout = 0;
	_mouseButtonState = 0;
	_mouseButtonPrev = 0;
	_eventMouseMoved = 0;
	_eventKeyStates = 0;

	return true;
}

void XpLib::shutdownEvents() {
	_eventFreeList = nullptr;
	_eventQueueTail = nullptr;
	_eventQueueHead = nullptr;

	if (_inactivityTimerId != 0) {
		killTimer(_inactivityTimerId);
		_inactivityTimerId = 0;
	}
}

int16 XpLib::getEvent(int16 filter, uint32 *outData, byte **outPtrData) {
	pumpMessages();

	// Search event queue for matching event type...
	XPEvent *node = _eventQueueHead;
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
	if (outPtrData)
		*outPtrData = node->payloadPtr;

	// Return node to free list...
	node->next = _eventFreeList;
	_eventFreeList = node;

	return node->type;
}

int16 XpLib::peekEvent(int16 filter, uint32 *outData, byte **outPtrData) {
	pumpMessages();

	XPEvent *node = _eventQueueHead;
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
		_eventQueueHead = node->next;
	}

	if (node->next) {
		XPEvent *next = node->next;
		next->prev = node->prev;
	} else {
		_eventQueueTail = node->prev;
	}
}

void XpLib::enqueueEvent(XPEvent *node) {
	// Append node to tail of event queue...
	node->next = nullptr;

	if (_eventQueueTail) {
		XPEvent *tail = _eventQueueTail;
		tail->next = node;
		node->prev = _eventQueueTail;
		_eventQueueTail = node;
	} else {
		// Queue was empty...
		_eventQueueTail = node;
		_eventQueueHead = node;
		node->prev = nullptr;
	}
}

void XpLib::pumpMessages() {
	Common::Event event;
	uint32 dummy = 0;
	byte *soundDataPtr;
	bool mouseMoved = false;

	// Drain any pending sound events...
	while (pollSound(&soundDataPtr) && !_bolt->shouldQuit()) {
		updateTimers();
		postEvent(etSound, dummy, soundDataPtr);
	}

	cycleColors();

	// Check inactivity timeout...
	if (_inactivityDeadline != 0) {
		uint32 now = _bolt->_system->getMillis();
		if (now > _inactivityDeadline) {
			_inactivityDeadline = 0;
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
			_lastRegisteredMousePos.x = event.mouse.x;
			_lastRegisteredMousePos.y = event.mouse.y;
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
	_bolt->_system->updateScreen();
}

void XpLib::handleTimer(uint32 timerId) {
	if (timerId != _inactivityTimerId) {
		postEvent(etTimer, timerId);
		return;
	}

	if (_inactivityCountdown != 0) {
		_inactivityCountdown--;
		if (_inactivityCountdown == 0)
			postEvent(etInactivity, 0);
	}

	if (_screensaverCountdown != 0) {
		_screensaverCountdown--;
		if (_screensaverCountdown == 0)
			activateScreenSaver();
	}

	if (_inactivityCountdown != 0 || _screensaverCountdown != 0) {
		_inactivityTimerId = startTimer(1000);
	} else {
		_inactivityTimerId = 0;
	}
}

void XpLib::handleMouseMove(bool *mouseMoved) {
	resetInactivity();

	if (_eventKeyStates == eksMouseMode) {
		*mouseMoved = true;
		int16 x, y;
		readCursor(nullptr, &x, &y);
		postEvent(etMouseMove, ((uint32)x << 16) | (int16)y);
		_eventMouseMoved = 0;
	} else if (_eventKeyStates == eksJoystickMode) {
		int16 joyX, joyY;
		readJoystick(&joyX, &joyY);

		int16 dx = (joyX < 155) ? -1 : (joyX > 165) ? 1 : 0;
		int16 dy = (joyY < 96) ? -1 :  (joyY > 104) ? 1 : 0;

		if (dx != 0 || dy != 0) {
			setCursorPos(160, 100);
			postJoystickEvent(0, dx, dy);
		}

		// Set inactivity deadline 100ms from now...
		_inactivityDeadline = _bolt->_system->getMillis() + 100;
	}
}

void XpLib::handleMouseButton(int16 down, int16 button) {
	resetInactivity();

	if (_eventKeyStates == eksMouseMode) {
		int16 x, y;
		readCursor(nullptr, &x, &y);
		postEvent(etMouseMove, ((uint32)x << 16) | (int16)y);
	}

	if (down) {
		_mouseButtonPrev |= (1 << button);
		postEvent(etMouseDown, (uint32)button);
	} else {
		_mouseButtonPrev &= ~(1 << button);
		postEvent(etMouseUp, (uint32)button);
	}
}

void XpLib::handleKey(Common::KeyCode vkey, int16 down) {
	resetInactivity();

	if (vkey == Common::KEYCODE_ESCAPE) {
		if (_mouseButtonState != down) {
			if (_eventKeyStates == eksMouseMode) {
				int16 x, y;
				readCursor(nullptr, &x, &y);
				postEvent(etMouseMove, ((uint32)x << 16) | (int16)y);
			}

			_mouseButtonState = down;
			postEvent(down ? etMouseDown : etMouseUp, 0);
		}

		return;
	}

	int8 *dirPtr = nullptr;
	switch (vkey) {
	case Common::KEYCODE_LEFT:
		dirPtr = &_keyStateLeft;
		break; // VK_LEFT
	case Common::KEYCODE_UP:
		dirPtr = &_keyStateUp;
		break; // VK_UP
	case Common::KEYCODE_RIGHT:
		dirPtr = &_keyStateRight;
		break; // VK_RIGHT
	case Common::KEYCODE_DOWN:
		dirPtr = &_keyStateDown;
		break; // VK_DOWN
	}

	if (dirPtr == nullptr)
		return;

	if (*dirPtr == (int8)down)
		return;

	*dirPtr = (int8)down;

	if (_eventKeyStates == eksJoystickMode) {
		int16 dx = _keyStateRight - _keyStateLeft;
		int16 dy = _keyStateDown - _keyStateUp;
		postJoystickEvent(1, dx, dy);
	}
}

void XpLib::postJoystickEvent(int16 source, int16 dx, int16 dy) {
	_eventMouseMoved = source;

	if (dx != _lastJoystickX || dy != _lastJoystickY) {
		postEvent(etJoystick, ((uint32)dx << 16) | (uint16)dy);
		_lastJoystickX = dx;
		_lastJoystickY = dy;
	}
}

void XpLib::postEvent(XPEventTypes type, uint32 data, byte *ptrData) {
	if (type == etMouseMove) {
		if (_lastMouseEventData == data)
			return;

		_lastMouseEventData = data;
		if (_eventQueueTail && _eventQueueTail->type == etMouseMove) {
			_eventQueueTail->payload = data;
			return;
		}
	}

	XPEvent *node;
	if (_eventFreeList) {
		// Allocate from free list...
		node = _eventFreeList;
		_eventFreeList = _eventFreeList->next;
	} else {
		// Queue full: try to evict a droppable event...
		if (!canDropEvent(type))
			return;

		// Walk queue from tail looking for droppable event...
		node = _eventQueueTail;
		while (node) {
			if (canDropEvent(node->type))
				break;

			node = node->prev;
		}

		if (!node)
			node = _eventQueueHead;

		unlinkEvent(node);
	}

	node->type = type;
	node->payload = data;
	node->next = nullptr;

	if (type == etSound) {
		node->payloadPtr = ptrData;
	}

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
	int16 prev = _inactivityTimerValue;

	_inactivityTimerValue = seconds;
	_inactivityCountdown = seconds;

	if (seconds != 0 && _inactivityTimerId == 0)
		_inactivityTimerId = startTimer(1000);

	// Drain any pending inactivity events from the queue...
	uint32 payloadDummy;

	// getEvent calls pumpMessages which calls pollEvent and delayMillis...
	while (getEvent(etInactivity, &payloadDummy) == etInactivity && !_bolt->shouldQuit());

	return prev;
}

int16 XpLib::setScreenSaverTimer(int16 seconds) {
	int16 prev = _screenSaverTimerValue;

	_screenSaverTimerValue = seconds;
	_screensaverCountdown = seconds;

	if (seconds != 0 && _inactivityTimerId == 0)
		_inactivityTimerId = startTimer(1000);

	return prev;
}

void XpLib::activateScreenSaver() {
	_inactivityTimeout = 1;
	setScreenBrightness(25); // Dim to 25%...
}

void XpLib::resetInactivity() {
	if (_inactivityTimeout != 0) {
		setScreenBrightness(100); // Restore full brightness...
		_inactivityTimeout = 0;
	}

	// Reset screensaver countdown
	_screensaverCountdown = _screenSaverTimerValue;

	if (_screenSaverTimerValue != 0 && _inactivityTimerId == 0)
		_inactivityTimerId = startTimer(1000);
}

bool XpLib::enableController() {
	// Don't switch if already in mouse mode...
	if (_eventKeyStates == eksMouseMode)
		return false;

	_eventKeyStates = eksJoystickMode;
	setCursorPos(160, 100);
	return true;
}

void XpLib::disableController() {
	_eventKeyStates = eksInputOff;
	_inactivityDeadline = 0;
}

void XpLib::enableMouse() {
	_eventKeyStates = eksMouseMode;
}

void XpLib::disableMouse() {
	_eventKeyStates = eksInputOff;
}

int16 XpLib::getButtonState() {
	return _mouseButtonPrev | _mouseButtonState;
}

} // End of namespace Bolt
