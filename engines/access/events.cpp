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
#include "graphics/cursorman.h"
#include "common/events.h"
#include "common/endian.h"
#include "engines/util.h"
#include "access/access.h"
#include "access/events.h"
#include "access/amazon/amazon_resources.h"

#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16

namespace Access {

EventsManager::EventsManager(AccessEngine *vm): _vm(vm) {
	_cursorId = CURSOR_NONE;
	_normalMouse = CURSOR_CROSSHAIRS;
	_frameCounter = 10;
	_priorFrameTime = 0;
	_leftButton = _rightButton = false;
	_middleButton = false;
	_wheelUp = _wheelDown = false;
	_mouseCol = _mouseRow = 0;
	_cursorExitFlag = false;
	_vbCount = 0;
}

EventsManager::~EventsManager() {
	_invCursor.free();
}

void EventsManager::setCursor(CursorType cursorId) {
	if (cursorId == _cursorId)
		return;	
	_cursorId = cursorId;
	
	if (cursorId == CURSOR_INVENTORY) {
		// Set the cursor
		CursorMan.replaceCursor(_invCursor.getPixels(), _invCursor.w, _invCursor.h,
			_invCursor.w / 2, _invCursor.h / 2, 0);
	} else {
		// Get a pointer to the mouse data to use, and get the cursor hotspot
		const byte *srcP = Amazon::CURSORS[cursorId];
		int hotspotX = (int16)READ_LE_UINT16(srcP);
		int hotspotY = (int16)READ_LE_UINT16(srcP + 2);
		srcP += 4;

		// Create a surface to build up the cursor on
		Graphics::Surface cursorSurface;
		cursorSurface.create(16, 16, Graphics::PixelFormat::createFormatCLUT8());
		byte *destP = (byte *)cursorSurface.getPixels();
		Common::fill(destP, destP + CURSOR_WIDTH * CURSOR_HEIGHT, 0);

		// Loop to build up the cursor
		for (int y = 0; y < CURSOR_HEIGHT; ++y) {
			destP = (byte *)cursorSurface.getBasePtr(0, y);
			int width = CURSOR_WIDTH;
			int skip = *srcP++;
			int plot = *srcP++;
			if (skip >= width)
				break;

			// Skip over pixels
			destP += skip;
			width -= skip;

			// Write out the pixels to plot
			while (plot > 0 && width > 0) {
				*destP++ = *srcP++;
				--plot;
				--width;
			}
		}

		// Set the cursor
		CursorMan.replaceCursor(cursorSurface.getPixels(), CURSOR_WIDTH, CURSOR_HEIGHT,
			hotspotX, hotspotY, 0);

		// Free the cursor surface
		cursorSurface.free();
	}
}

void EventsManager::setCursorData(Graphics::Surface *src, const Common::Rect &r) {
	_invCursor.create(r.width(), r.height(), Graphics::PixelFormat::createFormatCLUT8());
	_invCursor.copyRectToSurface(*src, 0, 0, r);
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
	if (checkForNextFrameCounter()) {
		nextFrame();
	}

	_wheelUp = _wheelDown = false;

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
				_keypresses.push(event.kbd);
			}
			return;
		case Common::EVENT_KEYUP:
			return;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			_mouseCol = _mousePos.x / 8;
			_mouseRow = _mousePos.y / 8;
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
			return;
		case Common::EVENT_MBUTTONDOWN:
			_middleButton = true;
			return;
		case Common::EVENT_MBUTTONUP:
			_middleButton = false;
			return;
		case Common::EVENT_WHEELUP:
			_wheelUp = true;
			return;
		case Common::EVENT_WHEELDOWN:
			_wheelDown = true;
			return;
		default:
 			break;
		}
	}
}

bool EventsManager::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		--_vbCount;
		++_frameCounter;
		_priorFrameTime = milli;

		return true;
	}

	return false;
}

void EventsManager::nextFrame() {
	// Update timers
	_vm->_animation->updateTimers();
	_vm->_timers.updateTimers();

	// Give time to the debugger
	_vm->_debugger->onFrame();

	// TODO: Refactor for dirty rects
	_vm->_screen->updateScreen();
}

void EventsManager::delay(int time) {
	g_system->delayMillis(time);
}

void EventsManager::zeroKeys() {
	_keypresses.clear();
}

bool EventsManager::getKey(Common::KeyState &key) {
	if (_keypresses.empty()) {
		return false;
	} else {
		key = _keypresses.pop();
		return true;
	}
}

void EventsManager::debounceLeft() {
	while (_leftButton && !_vm->shouldQuit()) {
		pollEvents();
		g_system->delayMillis(10);
	}
}

void EventsManager::waitKeyMouse() {
	while (!_vm->shouldQuit() && !_leftButton && _keypresses.size() == 0) {
		pollEvents();
		g_system->delayMillis(10);
	}

	zeroKeys();
	debounceLeft();
}

Common::Point EventsManager::calcRawMouse() {
	Screen &screen = *_vm->_screen;
	Common::Point pt;
	pt.x = _mousePos.x - screen._windowXAdd +
		(screen._scrollCol * TILE_WIDTH) + screen._scrollX;
	pt.y = _mousePos.y - screen._screenYOff - screen._windowYAdd +
		(screen._scrollRow * TILE_HEIGHT) + screen._scrollY;

	return pt;
}

int EventsManager::checkMouseBox1(Common::Array<Common::Rect> &rects) {
	int i = 0;
	for (i = 0;; i++) {
		if (rects[i].left == -1)
			return -1;

		if ((_mousePos.x > rects[i].left) && (_mousePos.x < rects[i].right)
			&& (_mousePos.y > rects[i].top) && (_mousePos.y < rects[i].bottom))
			return i;
	}
}



} // End of namespace Access
