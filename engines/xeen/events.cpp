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

#include "common/scummsys.h"
#include "graphics/cursorman.h"
#include "common/events.h"
#include "common/endian.h"
#include "engines/util.h"
#include "xeen/xeen.h"
#include "xeen/events.h"
#include "xeen/screen.h"

namespace Xeen {

EventsManager::EventsManager(XeenEngine *vm) : _vm(vm), _playTime(0), _gameCounter(0),
		_frameCounter(0), _priorFrameCounterTime(0), _priorScreenRefreshTime(0),
		_mousePressed(false), _sprites("mouse.icn") {
	Common::fill(&_gameCounters[0], &_gameCounters[6], 0);
}

EventsManager::~EventsManager() {
}

void EventsManager::setCursor(int cursorId) {
	XSurface cursor;
	_sprites.draw(cursor, cursorId, Common::Point(0, 0), SPRFLAG_RESIZE);

	CursorMan.replaceCursor(cursor.getPixels(), cursor.w, cursor.h, 0, 0, 0);
	showCursor();
}

void EventsManager::showCursor() {
	CursorMan.showMouse(true);
}

void EventsManager::hideCursor() {
	CursorMan.showMouse(false);
}

bool EventsManager::isCursorVisible() {
	return CursorMan.isVisible();
}

void EventsManager::pollEvents() {
	uint32 timer = g_system->getMillis();

	if (timer >= (_priorScreenRefreshTime + SCREEN_UPDATE_TIME)) {
		// Refresh the screen at a higher frame rate than the game's own frame rate
		// to allow for more responsive mouse movement
		_priorScreenRefreshTime = timer;
		g_vm->_screen->update();
	}

	if (timer >= (_priorFrameCounterTime + GAME_FRAME_TIME)) {
		// Time to build up next game frame
		_priorFrameCounterTime = timer;
		nextFrame();
	}

	// Event handling
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;
		case Common::EVENT_KEYDOWN:
			if (!isModifierKey(event.kbd.keycode))
				addEvent(event.kbd);
			break;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mousePressed = true;
			addEvent(true, false);
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mousePressed = true;
			addEvent(false, true);
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mousePressed = false;
			return;
		default:
 			break;
		}
	}
}

void EventsManager::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);
}

void EventsManager::clearEvents() {
	_pendingEvents.clear();
	_mousePressed = false;
}

void EventsManager::debounceMouse() {
	while (_mousePressed && !_vm->shouldExit()) {
		pollEventsAndWait();
	}
}

void EventsManager::addEvent(const Common::KeyState &keyState) {
	if (_pendingEvents.size() < MAX_PENDING_EVENTS)
		_pendingEvents.push(PendingEvent(keyState));
}

void EventsManager::addEvent(bool leftButton, bool rightButton) {
	if (_pendingEvents.size() < MAX_PENDING_EVENTS)
		_pendingEvents.push(PendingEvent(leftButton, rightButton));
}


bool EventsManager::getEvent(PendingEvent &pe) {
	if (_pendingEvents.empty()) {
		return false;
	} else {
		pe = _pendingEvents.pop();
		return true;
	}
}

bool EventsManager::isKeyMousePressed() {
	bool result = isEventPending();
	debounceMouse();
	clearEvents();

	return result;
}

bool EventsManager::wait(uint numFrames, bool interruptable) {
	while (!_vm->shouldExit() && timeElapsed() < numFrames) {
		pollEventsAndWait();
		if (interruptable && isEventPending())
			return true;
	}

	return _vm->shouldExit();
}

void EventsManager::ipause(uint amount) {
	updateGameCounter();
	do {
		_vm->_interface->draw3d(true);
		pollEventsAndWait();
	} while (!_vm->shouldExit() && timeElapsed() < amount);
}

void EventsManager::ipause5(uint amount) {
	do {
		pollEventsAndWait();
	} while (!_vm->shouldExit() && timeElapsed5() < amount);
}

void EventsManager::waitForPressAnimated() {
	clearEvents();

	do {
		updateGameCounter();
		_vm->_interface->draw3d(true);

		while (!_vm->shouldExit() && timeElapsed() == 0)
			pollEventsAndWait();
	} while (!_vm->shouldExit() && !isKeyMousePressed());

	clearEvents();
}

void EventsManager::waitForPress() {
	clearEvents();

	do {
		pollEventsAndWait();
	} while (!_vm->shouldExit() && !isKeyMousePressed());

	clearEvents();
}

void EventsManager::nextFrame() {
	++_frameCounter;
	++_playTime;

	// Update the screen
	_vm->_screen->update();
}

bool EventsManager::isModifierKey(const Common::KeyCode &keycode) const {
	return keycode == Common::KEYCODE_LCTRL || keycode == Common::KEYCODE_LALT
		|| keycode == Common::KEYCODE_RCTRL || keycode == Common::KEYCODE_RALT
		|| keycode == Common::KEYCODE_LSHIFT || keycode == Common::KEYCODE_RSHIFT
		|| keycode == Common::KEYCODE_LSUPER || keycode == Common::KEYCODE_RSUPER
		|| keycode == Common::KEYCODE_CAPSLOCK || keycode == Common::KEYCODE_NUMLOCK
		|| keycode == Common::KEYCODE_SCROLLOCK;
}

} // End of namespace Xeen
