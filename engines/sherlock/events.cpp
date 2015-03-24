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

#include "common/scummsys.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "sherlock/sherlock.h"
#include "sherlock/events.h"

namespace Sherlock {

Events::Events(SherlockEngine *vm) {
	_vm = vm;
	_cursorImages = nullptr;
	_cursorId = INVALID_CURSOR;
	_frameCounter = 1;
	_priorFrameTime = 0;
	_mouseButtons = 0;
	_pressed = _released = false;
	_rightPressed = _rightReleased = false;
	_oldButtons = _oldRightButton = false;
}

Events::~Events() {
	delete _cursorImages;
}

/**
 * Load a set of cursors from the specified file
 */
void Events::loadCursors(const Common::String &filename) {
	hideCursor();
	delete _cursorImages;

	_cursorImages = new ImageFile(filename);
}

/**
 * Set the cursor to show
 */
void Events::setCursor(CursorId cursorId) {
	if (cursorId == _cursorId)
		return;

	_cursorId = cursorId;

	// Set the cursor data
	Graphics::Surface &s = (*_cursorImages)[cursorId];
	CursorMan.replaceCursor(s.getPixels(), s.w, s.h, s.w / 2, s.h / 2, 0xff);

	showCursor();
}

/**
 * Show the mouse cursor
 */
void Events::showCursor() {
	CursorMan.showMouse(true);
}

/**
 * Hide the mouse cursor
 */
void Events::hideCursor() {
	CursorMan.showMouse(false);
}

/**
 * Returns the cursor 
 */
CursorId Events::getCursor() const {
	return _cursorId;
}

/**
 * Returns true if the mouse cursor is visible
 */
bool Events::isCursorVisible() const {
	return CursorMan.isVisible();
}

/**
 * Check for any pending events
 */
void Events::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// Handle keypress
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
				_pendingKeys.push(event.kbd);
			}
			return;
		case Common::EVENT_KEYUP:
			return;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons |= 1;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons |= 2;
			return;
		case Common::EVENT_LBUTTONUP:
			_mouseButtons &= ~1;
			return;
		case Common::EVENT_RBUTTONUP:
			_mouseButtons &= ~2;
			return;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;
		default:
 			break;
		}
	}
}

/**
 * Poll for events and introduce a small delay, to allow the system to
 * yield to other running programs
 */
void Events::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);
}

/**
 * Check whether it's time to display the next screen frame
 */
bool Events::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_frameCounter;
		_priorFrameTime = milli;

		// Give time to the debugger
		_vm->_debugger->onFrame();

		// Display the frame
		_vm->_screen->update();

		// Signal the ScummVM debugger
		_vm->_debugger->onFrame();

		return true;
	}

	return false;
}

/**
 * Clear any current keypress or mouse click
 */
void Events::clearEvents() {
	_pendingKeys.clear();
	_pressed = _rightPressed = false;
	_rightPressed = _rightReleased = false;
	_oldButtons = _oldRightButton = false;
}

/**
 * Delay for a given number of game frames, where each frame is 1/60th of a second
 */
void Events::wait(int numFrames) {
	uint32 totalMilli = numFrames * 1000 / GAME_FRAME_RATE;
	delay(totalMilli);
}

bool Events::delay(uint32 time, bool interruptable) {
	// Different handling for really short versus extended times
	if (time < 10) {
		// For really short periods, simply delay by the desired amount
		pollEvents();
		g_system->delayMillis(time);
		bool result = !(interruptable && (kbHit() || _pressed));

		clearEvents();
		return result;
	} else {
		// For long periods go into a loop where we delay by 10ms at a time and then
		// check for events. This ensures for longer delays that responsiveness is
		// maintained
		uint32 delayEnd = g_system->getMillis() + time;

		while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd) {
			pollEventsAndWait();

			if (interruptable && (kbHit() || _pressed)) {
				clearEvents();
				return false;
			}
		}

		return true;
	}
}

/**
 * Sets the pressed and released button flags depending on the value passed
 */
void Events::setButtonState() {
	_released = _rightReleased = false;
	if (_mouseButtons & 1)
		_pressed = _oldButtons = true;

	if ((_mouseButtons & 1) == 0 && _oldButtons) {
		_pressed = _oldButtons = false;
		_released = true;
	}

	if (_mouseButtons & 2)
		_rightPressed = _oldRightButton = true;

	if ((_mouseButtons & 2) == 0 && _oldRightButton) {
		_rightPressed = _oldRightButton = false;
		_rightReleased = true;
	}
}

/**
 * Checks to see to see if a key or a mouse button is pressed.
 */
bool Events::checkInput() {
	setButtonState();
	return kbHit() || _pressed || _released || _rightPressed || _rightReleased;
}

} // End of namespace MADS
