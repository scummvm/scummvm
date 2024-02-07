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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/detection.h"

#include "graphics/paletteman.h"

namespace Saga2 {

extern uint32 loadingWindowWidth;
extern uint32 loadingWindowHeight;
extern uint8 *loadingWindowData;
extern uint8 *loadingWindowPalette;

static bool inLoadMode = false;

void initLoadMode() {
	inLoadMode = true;
}

void updateLoadMode() {
	if (g_vm->getGameId() == GID_DINO)
		return;	// TODO: Load EXE resources for Dino

	if (inLoadMode) {
		byte normalPalette[768];

		for (int i = 0; i < 256; i++) {
			normalPalette[i * 3 + 0] = loadingWindowPalette[i * 4 + 0];
			normalPalette[i * 3 + 1] = loadingWindowPalette[i * 4 + 1];
			normalPalette[i * 3 + 2] = loadingWindowPalette[i * 4 + 2];
		}

		g_system->getPaletteManager()->setPalette(normalPalette, 0, 256);
		g_system->copyRectToScreen(loadingWindowData, loadingWindowWidth, 0, 0, loadingWindowWidth, loadingWindowHeight);
	}
}

void closeLoadMode() {
	inLoadMode = false;
	//blackOut();
}

void loadingScreen() {
	initLoadMode();
	updateLoadMode();
}

} // end of namespace Saga2
