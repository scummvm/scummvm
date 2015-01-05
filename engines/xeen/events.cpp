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

/**
 * Constructor
 */
EventsManager::EventsManager(XeenEngine *vm) : _vm(vm),
		_frameCounter(0), _priorFrameCounterTime(0), _gameCounter(0),
		_priorGameCounterTime(0), _keyCode(Common::KEYCODE_INVALID),
		_leftButton(false), _rightButton(false),
		_sprites("mouse.icn") {
	Common::fill(&_gameCounters[0], &_gameCounters[6], 0);
}

/**
 * Destructor
 */
EventsManager::~EventsManager() {
}

/*
 * Set the cursor
 */
void EventsManager::setCursor(int cursorId) {
	XSurface cursor;
	_sprites.draw(cursor, cursorId);

	CursorMan.replaceCursor(cursor.getPixels(), cursor.w, cursor.h, 0, 0, 0);
	showCursor();
}

/**
 * Show the mouse cursor
 */
void EventsManager::showCursor() {
	CursorMan.showMouse(true);
}

/**
 * Hide the mouse cursor
 */
void EventsManager::hideCursor() {
	CursorMan.showMouse(false);
}

/**
 * Returns if the mouse cursor is visible
 */
bool EventsManager::isCursorVisible() {
	return CursorMan.isVisible();
}

void EventsManager::pollEvents() {
	uint32 timer = g_system->getMillis();
	if (timer >= (_priorFrameCounterTime + GAME_FRAME_TIME)) {
		_priorFrameCounterTime = timer;
		nextFrame();
	}

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RTL:
			return;
		case Common::EVENT_KEYDOWN:
			_keyCode = event.kbd.keycode;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftButton = true;
			return;
		case Common::EVENT_LBUTTONUP:
			_leftButton = false;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_rightButton = true;
			return;
		case Common::EVENT_RBUTTONUP:
			_rightButton = false;
			break;
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
	_keyCode = Common::KEYCODE_INVALID;
	_leftButton = _rightButton = false;

}

void EventsManager::debounceMouse() {
	while (_leftButton && !_vm->shouldQuit()) {
		pollEventsAndWait();
	}
}
bool EventsManager::getKey(Common::KeyState &key) {
	if (_keyCode == Common::KEYCODE_INVALID) {
		return false;
	} else {
		key = _keyCode;
		_keyCode = Common::KEYCODE_INVALID;
		return true;
	}
}

bool EventsManager::isKeyPending() const {
	return _keyCode != Common::KEYCODE_INVALID;
}

/**
 * Returns true if a key or mouse press is pending
 */
bool EventsManager::isKeyMousePressed() {
	bool result = _leftButton || _rightButton || isKeyPending();
	debounceMouse();
	clearEvents();

	return result;
}

bool EventsManager::wait(uint numFrames, bool interruptable) {
	while (!_vm->shouldQuit() && timeElapsed() < numFrames) {
		pollEventsAndWait();
		if (interruptable && (_leftButton || _rightButton || isKeyPending()))
			return true;
	}

	return false;
}

/**
 * Handles moving to the next game frame
 */
void EventsManager::nextFrame() {
	++_frameCounter;
	_vm->_screen->update();
}

/*------------------------------------------------------------------------*/

GameEvent::GameEvent() {
}

} // End of namespace Xeen
