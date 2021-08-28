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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/tile.h"
#include "saga2/vpal.h"
#include "saga2/palette.h"
#include "saga2/calender.h"
#include "saga2/modal.h"
#include "saga2/display.h"

namespace Saga2 {

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

bool isModalMode(void) {
	uint16  i;
	bool    modalFlag = false;

	for (i = 0; i < GameMode::modeStackCtr; i++) {
		// go through each stacked mode
		// and if modal mode is one of them,
		// then set the modal flag
		if (GameMode::modeStackPtr[i] == &ModalMode) {
			modalFlag = true;
		}
	}

	return modalFlag;
}

void dayNightUpdate(void) {
	// do nothing while in modal mode
	if (isModalMode()) {
		return;
	}

	audioEnvironmentSetDaytime(isDayTime());

	uint32 lightLevel = g_vm->_calender->lightLevel(MAX_LIGHT);

	//  Code to avoid unneccessary fades.
	if (lightLevel != g_vm->_pal->_prevLightLevel) {
		g_vm->_pal->_prevLightLevel = lightLevel;

		g_vm->_pal->createPalette(
		    &g_vm->_pal->_newPalette,
		    g_vm->_pal->_midnightPalette,
		    g_vm->_pal->_noonPalette,
		    lightLevel,
		    MAX_LIGHT);

		if (g_vm->_currentMapNum == 0)
			g_vm->_pal->beginFade(&g_vm->_pal->_newPalette, 100);
	}

	if (!g_vm->_pal->updatePalette()) {
		gPalettePtr     neededPalette;
		gPalette        currentPalette;

		neededPalette = g_vm->_currentMapNum == 0 ? &g_vm->_pal->_newPalette : g_vm->_pal->_noonPalette;
		g_vm->_pal->getCurrentPalette(&currentPalette);
		if (memcmp(&currentPalette, neededPalette, sizeof(gPalette)) != 0)
			g_vm->_pal->setCurrentPalette(neededPalette);
	}
}

void SystemEventLoop(void);
//-----------------------------------------------------------------------
//	Fade to black

void clearTileAreaPort(void);
void reDrawScreen(void) ;
void updateMainDisplay(void);
void updateActiveRegions();
void drawMainDisplay(void);
void fadeUp();
void fadeDown();
void clearTileAreaPort(void);
void displayUpdate(void);
void disableUserControls(void);
void enableUserControls(void);

void fadeDown(void) {
	if (g_vm->_fadeDepth++ == 0) {
		g_vm->_pal->beginFade(g_vm->_pal->_darkPalette, 20);
		while (g_vm->_pal->updatePalette());
		clearTileAreaPort();
		blackOut();
		disablePaletteChanges();
	}
}

//-----------------------------------------------------------------------
//	Fade to many colors

void fadeUp(void) {
	if (--g_vm->_fadeDepth == 0) {
		enableUserControls();
		updateMainDisplay();
		drawMainDisplay();
		reDrawScreen();
		enablePaletteChanges();
		g_vm->_pal->beginFade(g_vm->_currentMapNum != 0 ? g_vm->_pal->_noonPalette : &g_vm->_pal->_newPalette, 20);
		while (g_vm->_pal->updatePalette()) ;
	}
}

} // end of namespace Saga2
