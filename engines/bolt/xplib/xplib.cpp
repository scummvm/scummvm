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

	memset(g_cursorBuffer, 0, sizeof(g_cursorBuffer));
	memset(g_cursorBackgroundSaveBuffer, 0, sizeof(g_cursorBackgroundSaveBuffer));

	memset(g_paletteBuffer, 0, sizeof(g_paletteBuffer));
	memset(g_shiftedPaletteBuffer, 0, sizeof(g_shiftedPaletteBuffer));
	memset(g_cycleTimerIds, 0, sizeof(g_cycleTimerIds));
	memset(g_cycleTempPalette, 0, sizeof(g_cycleTempPalette));

	_screen = new Graphics::Screen();
}

XpLib::~XpLib() {
	delete _screen;
}

bool XpLib::initialize() {
	if (g_xpInitialized)
		return false;

	g_xpInitialized = true;

	//g_videoSurface = new Graphics::Surface();

	if (_bolt->g_extendedViewport) {
		g_rowDirtyFlags = (byte *)malloc(EXTENDED_SCREEN_HEIGHT);
		assert(g_rowDirtyFlags);
		memset(g_rowDirtyFlags, 0, EXTENDED_SCREEN_HEIGHT);

		g_vgaFramebuffer = (byte *)malloc(EXTENDED_SCREEN_WIDTH * EXTENDED_SCREEN_HEIGHT);
		assert(g_vgaFramebuffer);
		memset(g_vgaFramebuffer, 0, EXTENDED_SCREEN_WIDTH * EXTENDED_SCREEN_HEIGHT);

		//g_videoSurface->init(EXTENDED_SCREEN_WIDTH, EXTENDED_SCREEN_HEIGHT, EXTENDED_SCREEN_WIDTH, g_vgaFramebuffer, Graphics::PixelFormat::createFormatCLUT8());
	} else {
		g_rowDirtyFlags = (byte *)malloc(SCREEN_HEIGHT);
		assert(g_rowDirtyFlags);
		memset(g_rowDirtyFlags, 0, SCREEN_HEIGHT);

		g_vgaFramebuffer = (byte *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT);
		assert(g_vgaFramebuffer);
		memset(g_vgaFramebuffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);

		//g_videoSurface->init(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, g_vgaFramebuffer, Graphics::PixelFormat::createFormatCLUT8());
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

	free(g_vgaFramebuffer);
	g_vgaFramebuffer = nullptr;

	free(g_rowDirtyFlags);
	g_rowDirtyFlags = nullptr;

	//delete g_videoSurface;

	g_xpInitialized = false;
}

} // End of namespace Bolt
