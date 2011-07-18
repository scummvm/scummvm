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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/events.h"
#include "common/singleton.h"
#include "graphics/cursorman.h"
#include "common/system.h"

#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/staticres.h"
#include "tsage/tsage.h"
#include "tsage/globals.h"

namespace tSage {

EventsClass::EventsClass() {
	_currentCursor = CURSOR_NONE;
	_lastCursor = CURSOR_NONE;
	_frameNumber = 0;
	_priorFrameTime = 0;
	_prevDelayFrame = 0;
	_saver->addListener(this);
}

bool EventsClass::pollEvent() {
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		_priorFrameTime = milli;
		++_frameNumber;

		// Update screen
		g_system->updateScreen();
	}

	if (!g_system->getEventManager()->pollEvent(_event)) return false;

	// Handle keypress
	switch (_event.type) {
	case Common::EVENT_QUIT:
	case Common::EVENT_RTL:
		break;

	case Common::EVENT_MOUSEMOVE:
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		// Keep a copy of the current mouse position
		_mousePos = _event.mouse;
		break;

	default:
 		break;
	}

	return true;
}

void EventsClass::waitForPress(int eventMask) {
	Event evt;
	while (!_vm->shouldQuit() && !getEvent(evt, eventMask))
		g_system->delayMillis(10);
}

/**
 * Standard event retrieval, which only returns keyboard and mouse clicks
 */
bool EventsClass::getEvent(Event &evt, int eventMask) {
	while (pollEvent() && !_vm->shouldQuit()) {
		evt.handled = false;
		evt.eventType = EVENT_NONE;
		evt.mousePos = _event.mouse;
		evt.kbd = _event.kbd;

		switch (_event.type) {
		case Common::EVENT_MOUSEMOVE:
			evt.eventType = EVENT_MOUSE_MOVE;
			break;
		case Common::EVENT_LBUTTONDOWN:
			evt.eventType = EVENT_BUTTON_DOWN;
			evt.btnState = BTNSHIFT_LEFT;
			break;
		case Common::EVENT_RBUTTONDOWN:
			evt.eventType = EVENT_BUTTON_DOWN;
			evt.btnState = BTNSHIFT_RIGHT;
			break;
		case Common::EVENT_MBUTTONDOWN:
			evt.eventType = EVENT_BUTTON_DOWN;
			evt.btnState = BTNSHIFT_MIDDLE;
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_MBUTTONUP:
			evt.eventType = EVENT_BUTTON_UP;
			evt.btnState = 0;
			break;
		case Common::EVENT_KEYDOWN:
			evt.eventType = EVENT_KEYPRESS;
			evt.kbd = _event.kbd;
			break;
		default:
			break;
		}

		if (evt.eventType & eventMask)
			return true;
	}

	evt.handled = false;
	evt.eventType = EVENT_NONE;

	return false;
}

/**
 * Sets the specified cursor
 *
 * @cursorType Specified cursor number
 */
void EventsClass::setCursor(CursorType cursorType) {
	if (cursorType == _lastCursor)
		return;

	_lastCursor = cursorType;
	_globals->clearFlag(122);
	CursorMan.showMouse(true);

	const byte *cursor;
	bool delFlag = true;
	uint size;

	switch (cursorType) {
	case CURSOR_NONE:
		// No cursor
		_globals->setFlag(122);

		if (_vm->getFeatures() & GF_DEMO) {
			CursorMan.showMouse(false);
			return;
		}
		cursor = _resourceManager->getSubResource(4, 1, 6, &size);
		break;

	case CURSOR_LOOK:
		// Look cursor
		cursor = _resourceManager->getSubResource(4, 1, 5, &size);
		_currentCursor = CURSOR_LOOK;
		break;

	case CURSOR_USE:
		// Use cursor
		cursor = _resourceManager->getSubResource(4, 1, 4, &size);
		_currentCursor = CURSOR_USE;
		break;

	case CURSOR_TALK:
		// Talk cursor
		cursor = _resourceManager->getSubResource(4, 1, 3, &size);
		_currentCursor = CURSOR_TALK;
		break;

	case CURSOR_ARROW:
		// Arrow cursor
		cursor = CURSOR_ARROW_DATA;
		delFlag = false;
		break;

	case CURSOR_WALK:
	default:
		// Walk cursor
		cursor = CURSOR_WALK_DATA;
		_currentCursor = CURSOR_WALK;
		delFlag = false;
		break;
	}

	// Decode the cursor
	GfxSurface s = surfaceFromRes(cursor);

	Graphics::Surface surface = s.lockSurface();
	const byte *cursorData = (const byte *)surface.getBasePtr(0, 0);
	CursorMan.replaceCursor(cursorData, surface.w, surface.h, s._centroid.x, s._centroid.y, s._transColor);
	s.unlockSurface();

	if (delFlag)
		DEALLOCATE(cursor);
}

void EventsClass::pushCursor(CursorType cursorType) {
	const byte *cursor;
	bool delFlag = true;
	uint size;

	switch (cursorType) {
	case CURSOR_NONE:
		// No cursor
		cursor = _resourceManager->getSubResource(4, 1, 6, &size);
		break;

	case CURSOR_LOOK:
		// Look cursor
		cursor = _resourceManager->getSubResource(4, 1, 5, &size);
		break;

	case CURSOR_USE:
		// Use cursor
		cursor = _resourceManager->getSubResource(4, 1, 4, &size);
		break;

	case CURSOR_TALK:
		// Talk cursor
		cursor = _resourceManager->getSubResource(4, 1, 3, &size);
		break;

	case CURSOR_ARROW:
		// Arrow cursor
		cursor = CURSOR_ARROW_DATA;
		delFlag = false;
		break;

	case CURSOR_WALK:
	default:
		// Walk cursor
		cursor = CURSOR_WALK_DATA;
		delFlag = false;
		break;
	}

	// Decode the cursor
	GfxSurface s = surfaceFromRes(cursor);

	Graphics::Surface surface = s.lockSurface();
	const byte *cursorData = (const byte *)surface.getBasePtr(0, 0);
	CursorMan.pushCursor(cursorData, surface.w, surface.h, s._centroid.x, s._centroid.y, s._transColor);
	s.unlockSurface();

	if (delFlag)
		DEALLOCATE(cursor);
}

void EventsClass::popCursor() {
	CursorMan.popCursor();
}

void EventsClass::setCursor(Graphics::Surface &cursor, int transColor, const Common::Point &hotspot, CursorType cursorId) {
	const byte *cursorData = (const byte *)cursor.getBasePtr(0, 0);
	CursorMan.replaceCursor(cursorData, cursor.w, cursor.h, hotspot.x, hotspot.y, transColor);

	_currentCursor = cursorId;
}

void EventsClass::setCursorFromFlag() {
	setCursor(isCursorVisible() ? _currentCursor : CURSOR_NONE);
}

void EventsClass::showCursor() {
	setCursor(_currentCursor);
}

void EventsClass::hideCursor() {
	setCursor(CURSOR_NONE);
}

bool EventsClass::isCursorVisible() const {
	return !_globals->getFlag(122);
}

/**
 * Delays the game for the specified number of frames, if necessary, from the
 * previous time the delay method was called
 */
void EventsClass::delay(int numFrames) {
	while (_frameNumber < (_prevDelayFrame + numFrames)) {
		uint32 delayAmount = CLIP(_priorFrameTime + GAME_FRAME_TIME - g_system->getMillis(),
			(uint32)0, (uint32)GAME_FRAME_TIME);
		if (delayAmount > 0)
			g_system->delayMillis(delayAmount);

		++_frameNumber;
		_priorFrameTime = g_system->getMillis();
	}

	g_system->updateScreen();
	_prevDelayFrame = _frameNumber;
	_priorFrameTime = g_system->getMillis();
}

void EventsClass::listenerSynchronize(Serializer &s) {
	s.syncAsUint32LE(_frameNumber);
	s.syncAsUint32LE(_prevDelayFrame);

	if (s.getVersion() >= 5) {
		s.syncAsSint16LE(_currentCursor);
		s.syncAsSint16LE(_lastCursor);
	}
}

} // end of namespace tSage
