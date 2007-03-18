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
#include "common/events.h"
#include "common/system.h"

#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/sfx_player.h"
#include "cine/various.h"
#include "cine/bg_list.h"

namespace Cine {

struct mouseStatusStruct {
	int left;
	int right;
};

mouseStatusStruct mouseData;

uint16 mouseRight = 0;
uint16 mouseLeft = 0;

int lastKeyStroke = 0;

uint16 mouseUpdateStatus;
uint16 dummyU16;

void manageEvents(int count) {
	Common::Event event;

	Common::EventManager *eventMan = g_system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_LBUTTONDOWN:
			mouseLeft = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			mouseRight = 1;
			break;
		case Common::EVENT_MOUSEMOVE:
			break;
		case Common::EVENT_QUIT:
			g_system->quit();
			break;
		case Common::EVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case '\n':
			case '\r':
			case 261: // Keypad 5
				if (allowPlayerInput) {
					mouseLeft = 1;
				}
				break;
			case 27:  // ESC
				if (allowPlayerInput) {
					mouseRight = 1;
				}
				break;
			case 282: // F1
				if (allowPlayerInput) {
					playerCommand = 0; // EXAMINE
					makeCommandLine();
				}
				break;
			case 283: // F2
				if (allowPlayerInput) {
					playerCommand = 1; // TAKE
					makeCommandLine();
				}
				break;
			case 284: // F3
				if (allowPlayerInput) {
					playerCommand = 2; // INVENTORY
					makeCommandLine();
				}
				break;
			case 285: // F4
				if (allowPlayerInput) {
					playerCommand = 3; // USE
					makeCommandLine();
				}
				break;
			case 286: // F5
				if (allowPlayerInput) {
					playerCommand = 4; // ACTIVATE
					makeCommandLine();
				}
				break;
			case 287: // F6
				if (allowPlayerInput) {
					playerCommand = 5; // SPEAK
					makeCommandLine();
				}
				break;
			case 290: // F9
				if (allowPlayerInput && !inMenu) {
					makeActionMenu();
					makeCommandLine();
				}
				break;
			case 291: // F10
				if (!disableSystemMenu && !inMenu) {
					g_cine->makeSystemMenu();
				}
				break;
			default:
				lastKeyStroke = event.kbd.keycode;
				break;
			}
			break;
		default:
			break;
		}
	}

	if (count) {
		mouseData.left = mouseLeft;
		mouseData.right = mouseRight;
		mouseLeft = 0;
		mouseRight = 0;
	}

	int i;

	for (i = 0; i < count; i++) {
		//FIXME(?): Maybe there's a better way to "fix" this?
		//
		//Since not all backends/ports can update the screen 
		//100 times per second, only update the screen every
		//other frame (1000 / 2 * 10 i.e. 50 times per second max.)
		if (i % 2)
			g_system->updateScreen();
		g_system->delayMillis(10);
		manageEvents(0);
	}
}

void getMouseData(uint16 param, uint16 *pButton, uint16 *pX, uint16 *pY) {
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	*pX = mouse.x;
	*pY = mouse.y;

	*pButton = 0;

	if (mouseData.right) {
		(*pButton) |= 2;
	}

	if (mouseData.left) {
		(*pButton) |= 1;
	}
}

int getKeyData() {
	int k = lastKeyStroke;

	lastKeyStroke = -1;

	return k;
}

void CineEngine::mainLoop(int bootScriptIdx) {
	uint16 var_6;
	uint16 quitFlag;
	uint16 i;
	byte di;
	uint16 mouseButton;

	quitFlag = 0;

	if (_preLoad == false) {
		freeAnimDataTable();
		resetMessageHead();
		resetSeqList();
		resetglobalScriptsHead();
		resetObjectScriptHead();
		resetBgIncrustList();

		setTextWindow(0, 0, 20, 200);

		errorVar = 0;

		addScriptToList0(bootScriptIdx);

		menuVar = 0;

		gfxFuncGen1(page0c, page0, page0c, page0, -1);

		ptrGfxFunc13();

		gfxFuncGen2();

		inMenu = false;
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

		g_sfxPlayer->stop();
	}

	do {
		mainLoopSub3();
		di = executePlayerInput();

//		if (g_sfxPlayer->_fadeOutCounter != 0 && g_sfxPlayer->_fadeOutCounter < 100) {
//			g_sfxPlayer->stop();
//		}

		processSeqList();
		executeList1();
		executeList0();

		purgeList1();
		purgeList0();

		if (playerCommand == -1) {
			setMouseCursor(MOUSE_CURSOR_NORMAL);
		} else {
			setMouseCursor(MOUSE_CURSOR_CROSS);
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
	g_sfxPlayer->stop();
	freeAnimDataTable();
	unloadAllMasks();
	freePrcLinkedList();
	releaseObjectScripts();
	// if (g_cine->getGameType() == Cine::GType_OS) {
	// 	freeUnkList();
	// }
	freeBgIncrustList();
	closePart();
}

} // End of namespace Cine
