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
	if (!_bolt->g_extendedViewport) {
		g_cursorViewportWidth = SCREEN_WIDTH;
		g_cursorViewportHeight = SCREEN_HEIGHT;
	} else {
		g_cursorViewportWidth = EXTENDED_SCREEN_WIDTH;
		g_cursorViewportHeight = EXTENDED_SCREEN_HEIGHT;
	}

	g_cursorSprite.pixelData = g_cursorBuffer;
	g_cursorSprite.width = 16;
	g_cursorSprite.height = 16;
	g_cursorSprite.flags = 2;

	return true;
}

void XpLib::shutdownCursor() {
}

bool XpLib::readCursor(uint16 *outButtons, int16 *outX, int16 *outY) {
	if (g_cursorHidden != 0)
		return false;

	*outX = g_lastCursorX;
	*outY = g_lastCursorY;

	screenToVirtual(outX, outY);

	if (outButtons != nullptr)
		*outButtons = getButtonState();

	return true;
}

void XpLib::readJoystick(int16 *outX, int16 *outY) {
	g_lastCursorX = (int16)((int32)g_lastRegisteredMousePos.x * (int32)g_virtualWidth / (int32)g_cursorViewportWidth);
	g_lastCursorY = (int16)((int32)g_lastRegisteredMousePos.y * (int32)g_virtualHeight / (int32)g_cursorViewportHeight);

	*outX = g_lastCursorX;
	*outY = g_lastCursorY;

	screenToVirtual(outX, outY);
}

void XpLib::setCursorPos(int16 x, int16 y) {
	virtualToScreen(&x, &y);

	g_lastCursorX = x;
	g_lastCursorY = y;

	if (g_cursorHidden == 0)
		updateDisplay();

	int16 screenX = (int16)((int32)g_lastCursorX * (int32)g_cursorViewportWidth / (int32)g_virtualWidth);
	int16 screenY = (int16)((int32)g_lastCursorY * (int32)g_cursorViewportHeight / (int32)g_virtualHeight);

	_bolt->_system->warpMouse(screenX, screenY);
}

void XpLib::setCursorImage(byte *bitmap, int16 hotspotX, int16 hotspotY) {
	byte *dest = g_cursorSprite.pixelData;

	for (int16 row = 32; row > 0; row--) {
		for (int16 mask = 0x80; mask != 0; mask >>= 1) {
			*dest++ = (*bitmap & mask) ? 0x80 : 0x00;
		}

		bitmap++;
	}

	g_cursorHotspotX = hotspotX;
	g_cursorHotspotY = hotspotY;

	if (g_cursorHidden == 0)
		updateDisplay();
}

void XpLib::setCursorColor(byte r, byte g, byte b) {
	byte rgb[3] = {r, g, b};
	setPalette(1, 128, rgb);
}

bool XpLib::showCursor() {
	if (g_cursorHidden <= 0)
		return true;

	g_cursorHidden--;
	if (g_cursorHidden != 0)
		return false;

	screenToVirtual(&g_lastCursorX, &g_lastCursorY);
	setCursorPos(g_lastCursorX, g_lastCursorY);
	updateDisplay();
	enableMouse();

	return true;
}

void XpLib::hideCursor() {
	g_cursorHidden++;

	if (g_cursorHidden == 1) {
		updateDisplay();
		disableMouse();
	}
}

void XpLib::updateCursorPosition() {
	if (g_cursorHidden != 0)
		return;

	int16 x = (int16)((int32)g_lastRegisteredMousePos.x * (int32)g_virtualWidth / (int32)g_cursorViewportWidth);
	int16 y = (int16)((int32)g_lastRegisteredMousePos.y * (int32)g_virtualHeight / (int32)g_cursorViewportHeight);

	if (x != g_lastCursorX || y != g_lastCursorY) {
		g_lastCursorX = x;
		g_lastCursorY = y;
		updateDisplay();
	}
}

} // End of namespace Bolt
