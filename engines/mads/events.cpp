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
#include "engines/util.h"
#include "mads/mads.h"
#include "mads/events.h"
#include "mads/scene.h"

namespace MADS {

EventsManager::EventsManager(MADSEngine *vm) {
	_vm = vm;
	_cursorSprites = nullptr;
	_cursorId = CURSOR_NONE;
	_newCursorId = CURSOR_NONE;
	_frameCounter = 10;
	_priorFrameTime = 0;
	_mouseClicked = false;
	_mouseReleased = false;
	_mouseButtons = 0;
	_mouseStatus = 0;
	_strokeGoing = 0;
	_mouseStatusCopy = 0;
	_mouseMoved = false;
	_rightMousePressed = false;
	_eventTarget = nullptr;
}

EventsManager::~EventsManager() {
	freeCursors();
}

void EventsManager::loadCursors(const Common::String &spritesName) {
	delete _cursorSprites;
	_cursorSprites = new SpriteAsset(_vm, spritesName, 0x4000);
}

void EventsManager::setCursor(CursorType cursorId) {
	_cursorId = cursorId;
	changeCursor();
}

void EventsManager::setCursor2(CursorType cursorId) {
	_cursorId = cursorId;
	_newCursorId = cursorId;
	changeCursor();
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

void EventsManager::waitCursor() {
	CursorType cursorId = (CursorType)MIN(_cursorSprites->getCount(), (int)CURSOR_WAIT);
	_newCursorId = cursorId;
	if (_cursorId != _newCursorId) {
		_cursorId = _newCursorId;
		changeCursor();
	}
}

void EventsManager::changeCursor() {
	if (_cursorSprites) {
		MSprite *cursor = _cursorSprites->getFrame(_cursorId - 1);
		assert(cursor->w == cursor->h);
		byte transIndex = cursor->getTransparencyIndex();

		// Check for hotspot indication pixels along the right-hand and bottom
		// row. Put together, these give the cursor's hotspot x,y
		int hotspotX = 0, hotspotY = 0;
		const byte *cursorData = (const byte *)cursor->getPixels();
		for (int idx = 0; idx < cursor->w; ++idx) {
			if (cursorData[(cursor->h - 1) * cursor->w + idx] != transIndex)
				hotspotX = idx;

			if (cursorData[(idx + 1) * cursor->w - 1] != transIndex)
				hotspotY = idx;
		}

		// Reduce the cursor data to remove the last column from each row, since
		// the cursor routines don't have a pitch option
		byte *destCursor = new byte[(cursor->w - 1) * (cursor->h - 1)];
		const byte *srcP = cursorData;
		byte *destP = destCursor;

		for (int idx = 0; idx < (cursor->h - 1); ++idx) {
			Common::copy(srcP, srcP + cursor->w - 1, destP);
			srcP += cursor->w;
			destP += cursor->w - 1;
		}

		// Set the raw cursor data to use
		CursorMan.replaceCursor(destCursor, cursor->w - 1, cursor->h - 1,
			hotspotX, hotspotY, transIndex);
		showCursor();
		delete[] destCursor;
	}
}

void EventsManager::freeCursors() {
	delete _cursorSprites;
	_cursorSprites = nullptr;
}

void EventsManager::pollEvents() {
	checkForNextFrameCounter();
	_mouseMoved = false;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// If an event target is specified, pass the event to it
		if (_eventTarget) {
			_eventTarget->onEvent(event);
			continue;
		}

		// Handle keypress
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_KEYDOWN:
			// Check for debugger
			_pendingKeys.push(event.kbd);
			return;
		case Common::EVENT_KEYUP:
			return;
		case Common::EVENT_WHEELUP:
			_pendingKeys.push(Common::KeyState(Common::KEYCODE_PAGEUP));
			return;
		case Common::EVENT_WHEELDOWN:
			_pendingKeys.push(Common::KeyState(Common::KEYCODE_PAGEDOWN));
			return;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_mouseClicked = true;
			_mouseButtons = 1;
			_mouseMoved = true;
			if (event.type == Common::EVENT_RBUTTONDOWN) {
				_rightMousePressed = true;
				_mouseStatus |= 2;
			} else {
				_mouseStatus |= 1;
			}
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouseClicked = false;
			_mouseReleased = true;
			_mouseMoved = true;
			_rightMousePressed = false;
			if (event.type == Common::EVENT_RBUTTONUP) {
				_mouseStatus &= ~2;
			} else {
				_mouseStatus &= ~1;
			}
			return;
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			_currentPos = event.mouse;
			_mouseMoved = true;
			break;
		default:
 			break;
		}
	}
}

bool EventsManager::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_frameCounter;
		_priorFrameTime = milli;

		// Do any palette cycling
		_vm->_game->_scene.animatePalette();

		// Display the frame
		_vm->_screen->update();

		return true;
	}

	return false;
}

void EventsManager::delay(int cycles) {
	uint32 totalMilli = cycles * 1000 / GAME_FRAME_RATE;
	uint32 delayEnd = g_system->getMillis() + totalMilli;

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd) {
		g_system->delayMillis(10);

		pollEvents();
	}
}

void EventsManager::waitForNextFrame() {
	_mouseClicked = false;
	_mouseReleased = false;
	_mouseButtons = 0;

	bool mouseClicked = false;
	bool mouseReleased = false;
	int mouseButtons = 0;

	uint32 frameCtr = getFrameCounter();
	while (!_vm->shouldQuit() && frameCtr == _frameCounter) {
		delay(1);

		mouseClicked |= _mouseClicked;
		mouseReleased |= _mouseReleased;
		mouseButtons |= _mouseButtons;
	}

	_mouseClicked = mouseClicked;
	_mouseReleased = mouseReleased;
	_mouseButtons = mouseButtons;
	_mouseMoved |= _mouseClicked || _mouseReleased;
}

void EventsManager::initVars() {
	_mousePos = Common::Point(-1, -1);
	_mouseStatusCopy = _mouseStatus;
	_strokeGoing = 0;
}

void EventsManager::clearEvents() {
	_pendingKeys.clear();
}


} // End of namespace MADS
