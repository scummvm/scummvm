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
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "sherlock/sherlock.h"
#include "sherlock/events.h"

namespace Sherlock {

enum ButtonFlag { LEFT_BUTTON = 1, RIGHT_BUTTON = 2 };

Events::Events(SherlockEngine *vm): _vm(vm) {
	_cursorImages = nullptr;
	_cursorId = INVALID_CURSOR;
	_frameCounter = 1;
	_priorFrameTime = 0;
	_mouseButtons = 0;
	_pressed = _released = false;
	_rightPressed = _rightReleased = false;
	_oldButtons = _oldRightButton = false;
	_firstPress = false;

	if (_vm->_interactiveFl)
		loadCursors("rmouse.vgs");
}

Events::~Events() {
	delete _cursorImages;
}

void Events::loadCursors(const Common::String &filename) {
	hideCursor();
	delete _cursorImages;

	if (!IS_3DO) {
		// PC
		_cursorImages = new ImageFile(filename);
	} else {
		// 3DO
		_cursorImages = new ImageFile3DO(filename, kImageFile3DOType_RoomFormat);
	}
	_cursorId = INVALID_CURSOR;
}

void Events::setCursor(CursorId cursorId) {
	if (cursorId == _cursorId)
		return;

	int hotspotX, hotspotY;

	if (cursorId == MAGNIFY) {
		hotspotX = 8;
		hotspotY = 8;
	} else {
		hotspotX = 0;
		hotspotY = 0;
	}

	// Set the cursor data
	Graphics::Surface &s = (*_cursorImages)[cursorId]._frame;

	setCursor(s, hotspotX, hotspotY);

	_cursorId = cursorId;
}

void Events::setCursor(const Graphics::Surface &src, int hotspotX, int hotspotY) {
	_cursorId = INVALID_CURSOR;
	if (!IS_3DO) {
		// PC 8-bit palettized
		CursorMan.replaceCursor(src.getPixels(), src.w, src.h, hotspotX, hotspotY, 0xff);
	} else {
		// 3DO RGB565
		CursorMan.replaceCursor(src.getPixels(), src.w, src.h, hotspotX, hotspotY, 0x0000, false, &src.format);
	}
	showCursor();
}

void Events::animateCursorIfNeeded() {
	if (_cursorId >= WAIT && _cursorId < (WAIT + 3)) {
		CursorId newId = (_cursorId == WAIT + 2) ? WAIT : (CursorId)((int)_cursorId + 1);
		setCursor(newId);
	}
}


void Events::showCursor() {
	CursorMan.showMouse(true);
}

void Events::hideCursor() {
	CursorMan.showMouse(false);
}

CursorId Events::getCursor() const {
	return _cursorId;
}

bool Events::isCursorVisible() const {
	return CursorMan.isVisible();
}

void Events::moveMouse(const Common::Point &pt) {
	g_system->warpMouse(pt.x, pt.y);
}

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
			_mouseButtons |= LEFT_BUTTON;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons |= RIGHT_BUTTON;
			return;
		case Common::EVENT_LBUTTONUP:
			_mouseButtons &= ~LEFT_BUTTON;
			return;
		case Common::EVENT_RBUTTONUP:
			_mouseButtons &= ~RIGHT_BUTTON;
			return;
		default:
 			break;
		}
	}
}

void Events::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);
}

void Events::warpMouse(const Common::Point &pt) {
	g_system->warpMouse(pt.x, pt.y);
}

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

		return true;
	}

	return false;
}

Common::Point Events::mousePos() const {
	return g_system->getEventManager()->getMousePos();
}

Common::KeyState Events::getKey() {
	return _pendingKeys.pop();
}

void Events::clearEvents() {
	_pendingKeys.clear();
	_mouseButtons = 0;
	_pressed = _released = false;
	_rightPressed = _rightReleased = false;
	_oldButtons = _oldRightButton = false;
	_firstPress = false;
}

void Events::clearKeyboard() {
	_pendingKeys.clear();
}

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
		bool result = !(interruptable && (kbHit() || _pressed || _vm->shouldQuit()));

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

		return !_vm->shouldQuit();
	}
}

void Events::setButtonState() {
	_firstPress = ((_mouseButtons & 1) && !_pressed) || ((_mouseButtons & 2) && !_rightPressed);

	_released = _rightReleased = false;
	if (_mouseButtons & LEFT_BUTTON)
		_pressed = _oldButtons = true;

	if ((_mouseButtons & LEFT_BUTTON) == 0 && _oldButtons) {
		_pressed = _oldButtons = false;
		_released = true;
	}

	if (_mouseButtons & RIGHT_BUTTON)
		_rightPressed = _oldRightButton = true;

	if ((_mouseButtons & RIGHT_BUTTON) == 0 && _oldRightButton) {
		_rightPressed = _oldRightButton = false;
		_rightReleased = true;
	}
}

bool Events::checkInput() {
	setButtonState();
	return kbHit() || _pressed || _released || _rightPressed || _rightReleased;
}

} // End of namespace Sherlock
