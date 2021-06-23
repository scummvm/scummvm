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

extern  int16               currentMapNum;          // which map is in use

/* ===================================================================== *
   Globals
 * ===================================================================== */

static gPalette     newPalette;

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


	static uint32   prevLightLevel = 0;
	uint32          lightLevel = calender.lightLevel(MAX_LIGHT);

	//  Code to avoid unneccessary fades.
	if (lightLevel != prevLightLevel) {
		prevLightLevel = lightLevel;

		createPalette(
		    &newPalette,
		    midnightPalette,
		    noonPalette,
		    lightLevel,
		    MAX_LIGHT);

		if (currentMapNum == 0)
			beginFade(&newPalette, 100);
	}

	if (!updatePalette()) {
		gPalettePtr     neededPalette;
		gPalette        currentPalette;

		neededPalette = currentMapNum == 0 ? &newPalette : noonPalette;
		getCurrentPalette(&currentPalette);
		if (memcmp(&currentPalette, neededPalette, sizeof(gPalette)) != 0)
			setCurrentPalette(neededPalette);
	}
}

void SystemEventLoop(void);
//-----------------------------------------------------------------------
//	Fade to black

static int fadeDepth = 1;
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
	if (fadeDepth++ == 0) {
		beginFade(darkPalette, 20);
		while (updatePalette());
		clearTileAreaPort();
		blackOut();
		disablePaletteChanges();
	}
}

//-----------------------------------------------------------------------
//	Fade to many colors

void fadeUp(void) {
	if (--fadeDepth == 0) {
		enableUserControls();
		updateMainDisplay();
		drawMainDisplay();
		reDrawScreen();
		enablePaletteChanges();
		beginFade(currentMapNum != 0 ? noonPalette : &newPalette, 20);
		while (updatePalette()) ;
	}
}

} // end of namespace Saga2
