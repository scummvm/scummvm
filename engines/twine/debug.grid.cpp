/** @file debug.grid.cpp
	@brief
	This file contains grid debug routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.grid.h"
#include "grid.h"
#include "lbaengine.h"
#include "scene.h"
#include "main.h"
#include "redraw.h"

int32 useFreeCamera = 0;
#ifdef _DEBUG
int32 canChangeScenes = 1;
#else
int32 canChangeScenes = 0;
#endif

/** Change scenario camera positions */
void changeGridCamera(int16 pKey) {
	if (useFreeCamera) {
		// Press up - more X positions
		if (pKey == 0x2E) {
			newCameraZ--;
			reqBgRedraw = 1;
		}

		// Press down - less X positions
		if (pKey == 0x2C) {
			newCameraZ++;
			reqBgRedraw = 1;
		}

		// Press left - less Z positions
		if (pKey == 0x1F) {
			newCameraX--;
			reqBgRedraw = 1;
		}

		// Press right - more Z positions
		if (pKey == 0x2D) {
			newCameraX++;
			reqBgRedraw = 1;
		}
	}
}

/** Change grid index */
void changeGrid(int16 pKey) {
	if (canChangeScenes) {
		// Press up - more X positions
		if (pKey == 0x13) {
			currentSceneIdx++;
			if (currentSceneIdx > NUM_SCENES)
				currentSceneIdx = 0;
			needChangeScene = currentSceneIdx;
			reqBgRedraw = 1;
		}

		// Press down - less X positions
		if (pKey == 0x21) {
			currentSceneIdx--;
			if (currentSceneIdx < 0)
				currentSceneIdx = NUM_SCENES;
			needChangeScene = currentSceneIdx;
			reqBgRedraw = 1;
		}

		if (cfgfile.Debug && (pKey == 'f' || pKey == 'r'))
			printf("\nGrid index changed: %d\n", needChangeScene);
	}
}

/** Apply and change disappear celling grid */
void applyCellingGrid(int16 pKey) {
	// Increase celling grid index
	if (pKey == 0x22) {
		cellingGridIdx++;
		if (cellingGridIdx > 133)
			cellingGridIdx = 133;
	}
	// Decrease celling grid index
	if (pKey == 0x30) {
		cellingGridIdx--;
		if (cellingGridIdx < 0)
			cellingGridIdx = 0;
	}

	// Enable/disable celling grid
	if (pKey == 0x14 && useCellingGrid == -1) {
		useCellingGrid = 1;
		//createGridMap();
		initCellingGrid(cellingGridIdx);
		if (cfgfile.Debug && pKey == 0x14)
			printf("\nEnable Celling Grid index: %d\n", cellingGridIdx);
		needChangeScene = -2; // tricky to make the fade
	} else if (pKey == 0x14 && useCellingGrid == 1) {
		useCellingGrid = -1;
		createGridMap();
		reqBgRedraw = 1;
		if (cfgfile.Debug && pKey == 0x14)
			printf("\nDisable Celling Grid index: %d\n", cellingGridIdx);
		needChangeScene = -2; // tricky to make the fade
	}
}

