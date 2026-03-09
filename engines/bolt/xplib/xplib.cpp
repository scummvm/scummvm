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

namespace Bolt {

XpLib::XpLib(BoltEngine *bolt) {
	_bolt = bolt;

	memset(_cursorBuffer, 0, sizeof(_cursorBuffer));
	memset(_cursorBackgroundSaveBuffer, 0, sizeof(_cursorBackgroundSaveBuffer));

	memset(_paletteBuffer, 0, sizeof(_paletteBuffer));
	memset(_shiftedPaletteBuffer, 0, sizeof(_shiftedPaletteBuffer));
	memset(_cycleTimerIds, 0, sizeof(_cycleTimerIds));
	memset(_cycleTempPalette, 0, sizeof(_cycleTempPalette));
}

XpLib::~XpLib() {

}

bool XpLib::initialize() {
	if (_xpInitialized)
		return false;

	_xpInitialized = true;

	if (_bolt->_extendedViewport) {
		_rowDirtyFlags = (byte *)malloc(EXTENDED_SCREEN_HEIGHT);
		assert(_rowDirtyFlags);
		memset(_rowDirtyFlags, 0, EXTENDED_SCREEN_HEIGHT);

		_vgaFramebuffer = (byte *)malloc(EXTENDED_SCREEN_WIDTH * EXTENDED_SCREEN_HEIGHT);
		assert(_vgaFramebuffer);
		memset(_vgaFramebuffer, 0, EXTENDED_SCREEN_WIDTH * EXTENDED_SCREEN_HEIGHT);
	} else {
		_rowDirtyFlags = (byte *)malloc(SCREEN_HEIGHT);
		assert(_rowDirtyFlags);
		memset(_rowDirtyFlags, 0, SCREEN_HEIGHT);

		_vgaFramebuffer = (byte *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
		assert(_vgaFramebuffer);
		memset(_vgaFramebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
	}

	if (!initTimer()) {
		terminate();
		return false;
	}

	if (!initEvents()) {
		terminate();
		return false;
	}

	if (!initSound()) {
		terminate();
		return false;
	}

	if (!initDisplay()) {
		terminate();
		return false;
	}

	if (!initCursor()) {
		terminate();
		return false;
	}

	return true;
}

void XpLib::terminate() {
	shutdownCursor();
	shutdownDisplay();
	shutdownSound();
	shutdownEvents();
	shutdownTimer();

	free(_vgaFramebuffer);
	_vgaFramebuffer = nullptr;

	free(_rowDirtyFlags);
	_rowDirtyFlags = nullptr;
	_xpInitialized = false;
}

} // End of namespace Bolt
