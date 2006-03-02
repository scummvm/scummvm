/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * cinE Engine is (C) 2004-2005 by CinE Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/various.h"

namespace Cine {

mouseStatusStruct mouseData;

uint16 mouseRight = 0;
uint16 mouseLeft = 0;

uint16 mouseUpdateStatus;
uint16 dummyU16;

void manageEvents(void) {
	OSystem::Event event;

	while (g_system->pollEvent(event)) {
		switch (event.type) {
		case OSystem::EVENT_LBUTTONDOWN:
			mouseLeft = 1;
			break;
		case OSystem::EVENT_RBUTTONDOWN:
			mouseRight = 1;
			break;
		case OSystem::EVENT_MOUSEMOVE:
			mouseData.X = event.mouse.x;
			mouseData.Y = event.mouse.y;
			break;
		case OSystem::EVENT_QUIT:
			g_system->quit();
			break;
		default:
			break;
		}
	}

	mouseData.left = mouseLeft;
	mouseData.right = mouseRight;

	mouseLeft = 0;
	mouseRight = 0;
}

void getMouseData(uint16 param, uint16 *pButton, uint16 *pX, uint16 *pY) {
	*pX = mouseData.X;
	*pY = mouseData.Y;

	*pButton = 0;

	if (mouseData.right) {
		(*pButton) |= 2;
	}

	if (mouseData.left) {
		(*pButton) |= 1;
	}
}

void mainLoop(int bootScriptIdx) {
	uint16 var_6;
	uint16 quitFlag;
	uint16 i;
	uint8 di;
	uint16 mouseButton;

	closeEngine3();
	resetMessageHead();
	resetSeqList();
	resetglobalScriptsHead();
	resetObjectScriptHead();
	mainLoopSub1();

	mainLoopSub2(0, 0, 20, 200);

	errorVar = 0;

	addScriptToList0(bootScriptIdx);

	menuVar = 0;

	gfxFuncGen1(page0c, page0, page0c, page0, -1);

	ptrGfxFunc13();

	gfxFuncGen2();

	quitFlag = 0;
	allowPlayerInput = 0;
	checkForPendingDataLoadSwitch = 0;

	fadeRequired = 0;
	isDrawCommandEnabled = 0;
	waitForPlayerClick = 0;
	menuCommandLen = 0;

	playerCommand = -1;
	strcpy(commandBuffer, "");

	globalVars[0x1F2] = 0;
	globalVars[0x1F4] = 0;

	for (i = 0; i < 16; i++) {
		c_palette[i] = 0;
	}

	var17 = 1;

	strcpy(newPrcName, "");
	strcpy(newRelName, "");
	strcpy(newObjectName, "");
	strcpy(newMsgName, "");
	strcpy(currentBgName[0], "");
	strcpy(currentCtName, "");
	strcpy(currentPartName, "");

	stopSample();

	do {
		mainLoopSub3();
		di = executePlayerInput();

		if (var18 != 0) {
			if (var18 >= 100 || var19) {
				stopSample();
			}
		}

		processSeqList();
		executeList1();
		executeList0();

		purgeList1();
		purgeList0();

		if (playerCommand == -1) {
			processPendingUpdates(0);
		} else {
			processPendingUpdates(2);
		}

		drawOverlays();
		flip();

		if (waitForPlayerClick) {
			var_6 = 0;

			var20 <<= 3;

			if (var20 < 0x800)
				var20 = 0x800;

			do {
				manageEvents();
				getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
			} while (mouseButton != 0);

			menuVar = 0;

			do {
				manageEvents();
				getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);

				if (mouseButton == 0) {
					if (processKeyboard(menuVar)) {
						var_6 = 1;
					}
				} else {
					var_6 = 1;
				}

				mainLoopSub6();
			} while (!var_6);

			menuVar = 0;

			do {
				manageEvents();
				getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
			} while (mouseButton != 0);

			waitForPlayerClick = 0;
		}

		if (checkForPendingDataLoadSwitch) {
			checkForPendingDataLoad();

			checkForPendingDataLoadSwitch = 0;
		}

		if (di) {
			if ("quit"[menuCommandLen] == (char)di) {
				++menuCommandLen;
				if (menuCommandLen == 4) {
					quitFlag = 1;
				}
			} else {
				menuCommandLen = 0;
			}
		}

		manageEvents();

	} while (!exitEngine && !quitFlag && var21 != 7);

	hideMouse();
	stopSample();
	closeEngine3();
	unloadAllMasks();
	freePrcLinkedList();
	releaseObjectScripts();
	closeEngine7();
	closePart();
}

} // End of namespace Cine
