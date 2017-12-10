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

EventsManager::EventsManager(XeenEngine *vm) : _vm(vm),
		_frameCounter(0), _priorFrameCounterTime(0), _gameCounter(0),
		_leftButton(false), _rightButton(false), _sprites("mouse.icn") {
	Common::fill(&_gameCounters[0], &_gameCounters[6], 0);
	_key.keycode = Common::KEYCODE_INVALID;
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
			// Check for debugger
			if (event.kbd.keycode == Common::KEYCODE_d && (event.kbd.flags & Common::KBD_CTRL)) {
				// Attach to the debugger
				_vm->_debugger->attach();
				_vm->_debugger->onFrame();
			} else {
				_key = event.kbd;
			}
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
	_key.keycode = Common::KEYCODE_INVALID;
	_leftButton = _rightButton = false;

}

void EventsManager::debounceMouse() {
	while (_leftButton && !_vm->shouldQuit()) {
		pollEventsAndWait();
	}
}
bool EventsManager::getKey(Common::KeyState &key) {
	if (_key.keycode == Common::KEYCODE_INVALID) {
		return false;
	} else {
		key = _key;
		_key.keycode = Common::KEYCODE_INVALID;
		return true;
	}
}

bool EventsManager::isKeyPending() const {
	return _key.keycode != Common::KEYCODE_INVALID;
}

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

	return _vm->shouldQuit();
}

void EventsManager::ipause(uint amount) {
	updateGameCounter();
	do {
		_vm->_interface->draw3d(true);
		pollEventsAndWait();
	} while (!_vm->shouldQuit() && timeElapsed() < amount);
}

void EventsManager::ipause5(uint amount) {
	do {
		pollEventsAndWait();
	} while (!_vm->shouldQuit() && timeElapsed5() < amount);
}

void EventsManager::waitForPressAnimated() {
	clearEvents();

	do {
		updateGameCounter();
		_vm->_interface->draw3d(true);

		while (!_vm->shouldQuit() && timeElapsed() == 0)
			pollEventsAndWait();
	} while (!_vm->shouldQuit() && !isKeyMousePressed());

	clearEvents();
}

void EventsManager::nextFrame() {
	++_frameCounter;

	// Allow debugger to update
	_vm->_debugger->update();

	// Update the screen
	_vm->_screen->update();
}

/*------------------------------------------------------------------------*/

GameEvent::GameEvent() {
}

} // End of namespace Xeen
