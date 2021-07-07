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
#include "sherlock/surface.h"
#include "sherlock/tattoo/tattoo.h"

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
	_waitCounter = 0;
	_frameRate = GAME_FRAME_RATE;

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
	if (cursorId == _cursorId || _waitCounter > 0)
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
	_hotspotPos = Common::Point(hotspotX, hotspotY);

	if (!IS_3DO) {
		// PC 8-bit palettized
		CursorMan.replaceCursor(src.getPixels(), src.w, src.h, hotspotX, hotspotY, 0xff);
	} else if (!_vm->_isScreenDoubled) {
		CursorMan.replaceCursor(src.getPixels(), src.w, src.h, hotspotX, hotspotY, 0x0000, false, &src.format);
	} else {
		Graphics::Surface tempSurface;
		tempSurface.create(2 * src.w, 2 * src.h, src.format);

		for (int y = 0; y < src.h; y++) {
			const uint16 *srcP = (const uint16 *)src.getBasePtr(0, y);
			uint16 *destP = (uint16 *)tempSurface.getBasePtr(0, 2 * y);
			for (int x = 0; x < src.w; ++x, ++srcP, destP += 2) {
				*destP = *srcP;
				*(destP + 1) = *srcP;
				*(destP + 2 * src.w) = *srcP;
				*(destP + 2 * src.w + 1) = *srcP;
			}
		}

		// 3DO RGB565
		CursorMan.replaceCursor(tempSurface.getPixels(), tempSurface.w, tempSurface.h, 2 * hotspotX, 2 * hotspotY, 0x0000, false, &src.format);

		tempSurface.free();
	}
	showCursor();
}

void Events::setCursor(CursorId cursorId, const Common::Point &cursorPos, const Graphics::Surface &surface) {
	_cursorId = cursorId;

	// Get the standard cursor frame
	Graphics::Surface &cursorImg = (*_cursorImages)[cursorId]._frame;

	// If the X pos for the cursor image is -100, this is a special value to indicate
	// the cursor should be horizontally centered
	Common::Point cursorPt = cursorPos;
	if (cursorPos.x == -100)
		cursorPt.x = (surface.w - cursorImg.w) / 2;

	// Figure total bounds needed for cursor image and passed image
	Common::Rect bounds(surface.w, surface.h);
	bounds.extend(Common::Rect(cursorPt.x, cursorPt.y, cursorPt.x + cursorImg.w, cursorPt.y + cursorImg.h));
	Common::Rect r = bounds;
	r.moveTo(0, 0);

	// Form a single surface containing both frames
	Surface s(r.width(), r.height());
	s.clear(TRANSPARENCY);

	// Draw the passed image
	Common::Point drawPos;
	if (cursorPt.x < 0)
		drawPos.x = -cursorPt.x;
	if (cursorPt.y < 0)
		drawPos.y = -cursorPt.y;
	s.SHblitFrom(surface, Common::Point(drawPos.x, drawPos.y));

	// Draw the cursor image
	drawPos = Common::Point(MAX(cursorPt.x, (int16)0), MAX(cursorPt.y, (int16)0));
	s.SHtransBlitFrom(cursorImg, Common::Point(drawPos.x, drawPos.y));

	// Set up hotspot position for cursor, adjusting for cursor image's position within the surface
	Common::Point hotspot;
	if (cursorId == MAGNIFY)
		hotspot = Common::Point(8, 8);
	hotspot += drawPos;
	// Set the cursor
	setCursor(s, hotspot.x, hotspot.y);
}

void Events::animateCursorIfNeeded() {
	if (_cursorId >= WAIT && _cursorId < (WAIT + 3)) {
		CursorId newId = (_cursorId == WAIT + 2) ? WAIT : (CursorId)((int)_cursorId + 1);
		setCursor(newId);
	}
}

void Events::showCursor() {
	if (IS_SERRATED_SCALPEL || !static_cast<Tattoo::TattooEngine *>(_vm)->_runningProlog)
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

void Events::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		_mousePos = event.mouse;
		if (_vm->_isScreenDoubled)
			_mousePos = Common::Point(_mousePos.x / 2, _mousePos.y / 2);

		// Handle events
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_KEYDOWN:
			_pendingKeys.push(event.kbd);
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
	Common::Point pos = pt;
	if (_vm->_isScreenDoubled)
		pos = Common::Point(pt.x / 2, pt.y);

	_mousePos = pos - _vm->_screen->_currentScroll;
	g_system->warpMouse(_mousePos.x, _mousePos.y);
}

void Events::warpMouse() {
	Screen &screen = *_vm->_screen;
	warpMouse(Common::Point(screen._currentScroll.x + SHERLOCK_SCREEN_WIDTH / 2,
		screen._currentScroll.y + SHERLOCK_SCREEN_HEIGHT / 2));
}

Common::Point Events::mousePos() const {
	return _vm->_screen->_currentScroll + _mousePos;
}

void Events::setFrameRate(int newRate) {
	_frameRate = newRate;
}

void Events::toggleSpeed() {
	_frameRate = (_frameRate == GAME_FRAME_RATE) ? GAME_FRAME_RATE * 2 : GAME_FRAME_RATE;
}

bool Events::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= (1000 / _frameRate)) {
		++_frameCounter;
		_priorFrameTime = milli;

		// Display the frame
		_vm->_screen->update();

		return true;
	}

	return false;
}

Common::KeyState Events::getKey() {
	Common::KeyState keyState = _pendingKeys.pop();

	switch (keyState.keycode) {
	case Common::KEYCODE_KP1:
		keyState.keycode = Common::KEYCODE_END;
		break;
	case Common::KEYCODE_KP2:
		keyState.keycode = Common::KEYCODE_DOWN;
		break;
	case Common::KEYCODE_KP3:
		keyState.keycode = Common::KEYCODE_PAGEDOWN;
		break;
	case Common::KEYCODE_KP4:
		keyState.keycode = Common::KEYCODE_LEFT;
		break;
	case Common::KEYCODE_KP6:
		keyState.keycode = Common::KEYCODE_RIGHT;
		break;
	case Common::KEYCODE_KP7:
		keyState.keycode = Common::KEYCODE_HOME;
		break;
	case Common::KEYCODE_KP8:
		keyState.keycode = Common::KEYCODE_UP;
		break;
	case Common::KEYCODE_KP9:
		keyState.keycode = Common::KEYCODE_PAGEUP;
		break;
	case Common::KEYCODE_KP_ENTER:
		keyState.keycode = Common::KEYCODE_RETURN;
		break;
	default:
		break;
	}

	return keyState;
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
	uint32 totalMilli = numFrames * 1000 / _frameRate;
	delay(totalMilli);
}

bool Events::delay(uint32 time, bool interruptable) {
	// Different handling for really short versus extended times
	if (time < 10) {
		// For really short periods, simply delay by the desired amount
		pollEvents();
		g_system->delayMillis(time);
		bool result = !(interruptable && (kbHit() || _pressed || _vm->shouldQuit()));

		if (interruptable)
			clearEvents();
		return result;
	} else {
		// For long periods go into a loop where we delay by 10ms at a time and then
		// check for events. This ensures for longer delays that responsiveness is
		// maintained
		uint32 delayEnd = g_system->getMillis() + time;

		while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd) {
			pollEventsAndWait();

			if (interruptable && (kbHit() || _mouseButtons)) {
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

void Events::incWaitCounter() {
	setCursor(WAIT);
	++_waitCounter;
}

void Events::decWaitCounter() {
	assert(_waitCounter > 0);
	--_waitCounter;
}

} // End of namespace Sherlock
