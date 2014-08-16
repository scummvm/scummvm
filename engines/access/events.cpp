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
	_frameCounter = 10;
	_priorFrameTime = 0;
	_leftButton = false;
	_mouseMove = false;
}

EventsManager::~EventsManager() {
}

void EventsManager::setCursor(CursorType cursorId) {
	if (cursorId == _cursorId)
		return;	
	_cursorId = cursorId;
	
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
	checkForNextFrameCounter();

	_leftButton = false;

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
			}
			return;
		case Common::EVENT_KEYUP:
			return;
		case Common::EVENT_LBUTTONDOWN:
			_leftButton = true;
			return;
		case Common::EVENT_LBUTTONUP:
			return;
		default:
 			break;
		}
	}
}

void EventsManager::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_frameCounter;
		_priorFrameTime = milli;

		nextFrame();
	}
}

void EventsManager::nextFrame() {

	// Give time to the debugger
	_vm->_debugger->onFrame();

	// Update timers
	_vm->_animation->updateTimers();

	// TODO: Refactor for dirty rects
	_vm->_screen->updateScreen();
}

void EventsManager::delay(int time) {
	g_system->delayMillis(time);
}


} // End of namespace Access
