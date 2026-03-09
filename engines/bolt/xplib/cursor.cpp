/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

#include "graphics/cursorman.h"

namespace Bolt {

bool XpLib::initCursor() {
	if (!_bolt->_extendedViewport) {
		_cursorViewportWidth = SCREEN_WIDTH;
		_cursorViewportHeight = SCREEN_HEIGHT;
	} else {
		_cursorViewportWidth = EXTENDED_SCREEN_WIDTH;
		_cursorViewportHeight = EXTENDED_SCREEN_HEIGHT;
	}

	_cursorSprite.pixelData = _cursorBuffer;
	_cursorSprite.width = 16;
	_cursorSprite.height = 16;
	_cursorSprite.flags = 2;

	return true;
}

void XpLib::shutdownCursor() {
}

bool XpLib::readCursor(uint16 *outButtons, int16 *outX, int16 *outY) {
	if (_cursorHidden != 0)
		return false;

	*outX = _lastCursorX;
	*outY = _lastCursorY;

	screenToVirtual(outX, outY);

	if (outButtons != nullptr)
		*outButtons = getButtonState();

	return true;
}

void XpLib::readJoystick(int16 *outX, int16 *outY) {
	_lastCursorX = (int16)((int32)_lastRegisteredMousePos.x * (int32)_virtualWidth / (int32)_cursorViewportWidth);
	_lastCursorY = (int16)((int32)_lastRegisteredMousePos.y * (int32)_virtualHeight / (int32)_cursorViewportHeight);

	*outX = _lastCursorX;
	*outY = _lastCursorY;

	screenToVirtual(outX, outY);
}

void XpLib::setCursorPos(int16 x, int16 y) {
	virtualToScreen(&x, &y);

	_lastCursorX = x;
	_lastCursorY = y;

	if (_cursorHidden == 0)
		updateDisplay();

	int16 screenX = (int16)((int32)_lastCursorX * (int32)_cursorViewportWidth / (int32)_virtualWidth);
	int16 screenY = (int16)((int32)_lastCursorY * (int32)_cursorViewportHeight / (int32)_virtualHeight);

	_bolt->_system->warpMouse(screenX, screenY);
}

void XpLib::setCursorImage(byte *bitmap, int16 hotspotX, int16 hotspotY) {
	byte *dest = _cursorSprite.pixelData;

	for (int16 row = 32; row > 0; row--) {
		for (int16 mask = 0x80; mask != 0; mask >>= 1) {
			*dest++ = (*bitmap & mask) ? 0x80 : 0x00;
		}

		bitmap++;
	}

	_cursorHotspotX = hotspotX;
	_cursorHotspotY = hotspotY;

	if (_cursorHidden == 0)
		updateDisplay();
}

void XpLib::setCursorColor(byte r, byte g, byte b) {
	byte rgb[3] = {r, g, b};
	setPalette(1, 128, rgb);
}

bool XpLib::showCursor() {
	if (_cursorHidden <= 0)
		return true;

	_cursorHidden--;
	if (_cursorHidden != 0)
		return false;

	screenToVirtual(&_lastCursorX, &_lastCursorY);
	setCursorPos(_lastCursorX, _lastCursorY);
	updateDisplay();
	enableMouse();

	return true;
}

void XpLib::hideCursor() {
	_cursorHidden++;

	if (_cursorHidden == 1) {
		updateDisplay();
		disableMouse();
	}
}

void XpLib::updateCursorPosition() {
	if (_cursorHidden != 0)
		return;

	int16 x = (int16)((int32)_lastRegisteredMousePos.x * (int32)_virtualWidth / (int32)_cursorViewportWidth);
	int16 y = (int16)((int32)_lastRegisteredMousePos.y * (int32)_virtualHeight / (int32)_cursorViewportHeight);

	if (x != _lastCursorX || y != _lastCursorY) {
		_lastCursorX = x;
		_lastCursorY = y;
		updateDisplay();
	}
}

} // End of namespace Bolt
