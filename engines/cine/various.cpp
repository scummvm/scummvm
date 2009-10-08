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


#include "common/endian.h"
#include "common/events.h"

#include "cine/cine.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/sound.h"
#include "cine/bg_list.h"
#include "cine/various.h"

namespace Cine {

bool disableSystemMenu = false;
bool inMenu;

int16 commandVar3[4];
int16 commandVar1;
int16 commandVar2;

//Message messageTable[NUM_MAX_MESSAGE];

uint16 var2;
uint16 var3;
uint16 var4;
uint16 var5;

int16 buildObjectListCommand(int16 param);
int16 canUseOnObject = 0;

void drawString(const char *string, byte param) {
}

void waitPlayerInput(void) {
}

void setTextWindow(uint16 param1, uint16 param2, uint16 param3, uint16 param4) {
}

uint16 errorVar;
byte menuVar;

bool fadeRequired;
uint16 allowPlayerInput;
uint16 checkForPendingDataLoadSwitch;
uint16 isDrawCommandEnabled;
uint16 waitForPlayerClick;
uint16 menuCommandLen;
bool _paletteNeedUpdate;
uint16 _messageLen;
byte _danKeysPressed;

int16 playerCommand;

Common::String commandBuffer;
char currentPrcName[20];
char currentRelName[20];
char currentObjectName[20];
char currentMsgName[20];
char newPrcName[20];
char newRelName[20];
char newObjectName[20];
char newMsgName[20];
char currentCtName[15];
char currentPartName[15];
char currentDatName[30];
uint16 musicIsPlaying;

byte isInPause = 0;

/*! \brief Values used by the xMoveKeyb variable */
enum xMoveKeybEnums {
	kKeybMoveCenterX = 0,
	kKeybMoveRight = 1,
	kKeybMoveLeft = 2
};

/*! \brief Values used by the yMoveKeyb variable */
enum yMoveKeybEnums {
	kKeybMoveCenterY = 0,
	kKeybMoveDown = 1,
	kKeybMoveUp = 2
};

uint16 xMoveKeyb = kKeybMoveCenterX;
bool egoMovedWithKeyboard = false;
uint16 yMoveKeyb = kKeybMoveCenterY;

SelectedObjStruct currentSelectedObject;

CommandeType currentSaveName[10];

static const int16 choiceResultTable[] = { 1, 1, 1, 2, 1, 1, 1 };
static const int16 subObjectUseTable[] = { 3, 3, 3, 3, 3, 0, 0 };
static const int16 canUseOnItemTable[] = { 1, 0, 0, 1, 1, 0, 0 };

CommandeType objectListCommand[20];
int16 objListTab[20];

Common::Array<uint16> zoneData;
Common::Array<uint16> zoneQuery; ///< Only exists in Operation Stealth

/*! \brief Move the player character using the keyboard
 * \param x Negative values move left, positive right, zero not at all
 * \param y Negative values move down, positive up, zero not at all
 * NOTE: If both x and y are zero then the character stops
 * FIXME: This seems to only work in Operation Stealth. May need code changes somewhere else...
 */
void moveUsingKeyboard(int x, int y) {
	if (x > 0) {
		xMoveKeyb = kKeybMoveRight;
	} else if (x < 0) {
		xMoveKeyb = kKeybMoveLeft;
	} else {
		xMoveKeyb = kKeybMoveCenterX;
	}

	if (y > 0) {
		yMoveKeyb = kKeybMoveUp;
	} else if (y < 0) {
		yMoveKeyb = kKeybMoveDown;
	} else {
		yMoveKeyb = kKeybMoveCenterY;
	}

	egoMovedWithKeyboard = x || y;
}

void stopMusicAfterFadeOut(void) {
//	if (g_sfxPlayer->_fadeOutCounter != 0 && g_sfxPlayer->_fadeOutCounter < 100) {
//		g_sfxPlayer->stop();
//	}
}

void runObjectScript(int16 entryIdx) {
	ScriptPtr tmp(scriptInfo->create(*relTable[entryIdx], entryIdx));
	assert(tmp);
	objectScripts.push_back(tmp);
}

/*! \brief Add action result message to overlay list
 * \param cmd Message description
 * \todo Why are x, y, width and color left uninitialized?
 */
void addPlayerCommandMessage(int16 cmd) {
	overlay tmp;
	memset(&tmp, 0, sizeof(tmp));
	tmp.objIdx = cmd;
	tmp.type = 3;

	overlayList.push_back(tmp);
}

int16 getRelEntryForObject(uint16 param1, uint16 param2, SelectedObjStruct *pSelectedObject) {
	int16 i;
	int16 found = -1;

	for (i = 0; i < (int16)relTable.size(); i++) {
		if (relTable[i]->_param1 == param1 && relTable[i]->_param2 == pSelectedObject->idx) {
			if (param2 == 1) {
				found = i;
			} else if (param2 == 2) {
				if (relTable[i]->_param3 == pSelectedObject->param) {
					found = i;
				}
			}
		}

		if (found != -1)
			break;
	}

	return found;
}

/*! \brief Find index of the object under cursor
 * \param x Mouse cursor coordinate
 * \param y Mouse cursor coordinate
 * \todo Fix displaced type 1 objects
 */
int16 getObjectUnderCursor(uint16 x, uint16 y) {
	Common::List<overlay>::iterator it;

	int16 objX, objY, frame, part, threshold, height, xdif, ydif;
	int width;

	// reverse_iterator would be nice
	for (it = overlayList.reverse_begin(); it != overlayList.end(); --it) {
		if (it->type >= 2 || !objectTable[it->objIdx].name[0]) {
			continue;
		}

		objX = objectTable[it->objIdx].x;
		objY = objectTable[it->objIdx].y;

		frame = ABS((int16)(objectTable[it->objIdx].frame));
		part = objectTable[it->objIdx].part;

		// Additional case for negative frame values in Operation Stealth
		if (g_cine->getGameType() == Cine::GType_OS && objectTable[it->objIdx].frame < 0) {
			if ((it->type == 1) && (x >= objX) && (objX + frame >= x) && (y >= objY) && (objY + part >= y)) {
				return it->objIdx;
			} else {
				continue;
			}
		}

		if (it->type == 0) {
			threshold = animDataTable[frame]._var1;
		} else {
			threshold = animDataTable[frame]._width / 2;
		}

		height = animDataTable[frame]._height;
		width = animDataTable[frame]._realWidth;

		xdif = x - objX;
		ydif = y - objY;

		if ((xdif < 0) || ((threshold << 4) <= xdif) || (ydif <= 0) || (ydif >= height) || !animDataTable[frame].data()) {
			continue;
		}

		if (g_cine->getGameType() == Cine::GType_OS) {
			// This test isn't present in Operation Stealth's PC version's disassembly
			// but removing it makes things crash sometimes (e.g. when selecting a verb
			// and moving the mouse cursor around the floor in the airport's bathroom).
			if (xdif >= width) {
				continue;
			}

			if (it->type == 0 && animDataTable[frame].getColor(xdif, ydif) != (part & 0x0F)) {
				return it->objIdx;
			} else if (it->type == 1 && gfxGetBit(xdif, ydif, animDataTable[frame].data(), animDataTable[frame]._width * 4)) {
				return it->objIdx;
			}
		} else if (it->type == 0)	{ // use generated mask
			if (gfxGetBit(xdif, ydif, animDataTable[frame].mask(), animDataTable[frame]._width)) {
				return it->objIdx;
			}
		} else if (it->type == 1) { // is mask
			if (gfxGetBit(xdif, ydif, animDataTable[frame].data(), animDataTable[frame]._width * 4)) {
				return it->objIdx;
			}
		}
	}

	return -1;
}

void CineEngine::resetEngine() {
	g_sound->stopMusic();
	freeAnimDataTable();
	overlayList.clear();
	bgIncrustList.clear();
	closePart();

	objectScripts.clear();
	globalScripts.clear();
	relTable.clear();
	scriptTable.clear();
	messageTable.clear();
	resetObjectTable();

	globalVars.reset();

	var2 = var3 = var4 = var5 = 0;

	strcpy(newPrcName, "");
	strcpy(newRelName, "");
	strcpy(newObjectName, "");
	strcpy(newMsgName, "");
	strcpy(currentCtName, "");

	allowPlayerInput = 0;
	waitForPlayerClick = 0;
	playerCommand = -1;
	isDrawCommandEnabled = 0;

	commandBuffer = "";

	globalVars[VAR_MOUSE_X_POS] = 0;
	globalVars[VAR_MOUSE_Y_POS] = 0;

	fadeRequired = false;

	renderer->clear();

	checkForPendingDataLoadSwitch = 0;

	if (g_cine->getGameType() == Cine::GType_OS) {
		seqList.clear();
		currentAdditionalBgIdx = 0;
		currentAdditionalBgIdx2 = 0;
		// TODO: Add resetting of the following variables
		// adBgVar1 = 0;
		// adBgVar0 = 0;
		// gfxFadeOutCompleted = 0;
	}
}

void CineEngine::makeSystemMenu(void) {
	int16 numEntry, systemCommand;
	int16 mouseX, mouseY, mouseButton;
	int16 selectedSave;

	if (!disableSystemMenu) {
		inMenu = true;

		do {
			manageEvents();
			getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
		} while (mouseButton);

		numEntry = 6;

		if (!allowPlayerInput) {
			numEntry--;
		}

		systemCommand = makeMenuChoice(systemMenu, numEntry, mouseX, mouseY, 140);

		switch (systemCommand) {
		case 0:
			{
				drawString(otherMessages[2], 0);
				waitPlayerInput();
				break;
			}
		case 1:
			{
				getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
				if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
					//reinitEngine();
				}
				break;
			}
		case 2:
			{
				getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
				if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
					quitGame();
				}
				break;
			}
		case 3:	// Select save drive... change ?
			{
				break;
			}
		case 4:	// load game
			{
				if (loadSaveDirectory()) {
//					int16 selectedSave;

					getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
					selectedSave = makeMenuChoice(currentSaveName, 10, mouseX, mouseY + 8, 180);

					if (selectedSave >= 0) {
						char saveNameBuffer[256];
						sprintf(saveNameBuffer, "%s.%1d", _targetName.c_str(), selectedSave);

						getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
						if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
							char loadString[256];

							sprintf(loadString, otherMessages[3], currentSaveName[selectedSave]);
							drawString(loadString, 0);

							makeLoad(saveNameBuffer);
						} else {
							drawString(otherMessages[4], 0);
							waitPlayerInput();
							checkDataDisk(-1);
						}
					} else {
						drawString(otherMessages[4], 0);
						waitPlayerInput();
						checkDataDisk(-1);
					}
				} else {
					drawString(otherMessages[5], 0);
					waitPlayerInput();
					checkDataDisk(-1);
				}
				break;
			}
		case 5:
			{
				loadSaveDirectory();
				selectedSave = makeMenuChoice(currentSaveName, 10, mouseX, mouseY + 8, 180);

				if (selectedSave >= 0) {
					char saveFileName[256];
					char saveName[20];
					saveName[0] = 0;

					if (!makeTextEntryMenu(otherMessages[6], saveName, 20, 120))
						break;

					strncpy(currentSaveName[selectedSave], saveName, 20);

					sprintf(saveFileName, "%s.%1d", _targetName.c_str(), selectedSave);

					getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
					if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
						char saveString[256], tmp[80];

						snprintf(tmp, 80, "%s.dir", _targetName.c_str());

						Common::OutSaveFile *fHandle = g_saveFileMan->openForSaving(tmp);
						if (!fHandle) {
							warning("Unable to open file %s for saving", tmp);
							break;
						}

						fHandle->write(currentSaveName, 200);
						delete fHandle;

						sprintf(saveString, otherMessages[3], currentSaveName[selectedSave]);
						drawString(saveString, 0);

						makeSave(saveFileName);

						checkDataDisk(-1);
					} else {
						drawString(otherMessages[4], 0);
						waitPlayerInput();
						checkDataDisk(-1);
					}
				}
				break;
			}
		}

		inMenu = false;
	}
}

void drawMessageBox(int16 x, int16 y, int16 width, int16 currentY, int16 offset, int16 color, byte* page) {
	gfxDrawLine(x + offset, y + offset, x + width - offset, y + offset, color, page);	// top
	gfxDrawLine(x + offset, currentY + 4 - offset, x + width - offset, currentY + 4 - offset, color, page);	// bottom
	gfxDrawLine(x + offset, y + offset, x + offset, currentY + 4 - offset, color, page);	// left
	gfxDrawLine(x + width - offset, y + offset, x + width - offset, currentY + 4 - offset, color, page);	// right
}

void drawDoubleMessageBox(int16 x, int16 y, int16 width, int16 currentY, int16 color, byte* page) {
	drawMessageBox(x, y, width, currentY, 1, 0, page);
	drawMessageBox(x, y, width, currentY, 0, color, page);
}

void processInventory(int16 x, int16 y) {
	uint16 button;
	int menuWidth;
	int listSize;
	int commandParam;

	if (g_cine->getGameType() == Cine::GType_FW) {
		menuWidth = 140;
		commandParam = -2;
	} else { // Operation Stealth
		menuWidth = 160;
		commandParam = -3;
	}

	listSize = buildObjectListCommand(commandParam);

	if (!listSize)
		return;

	Common::StringList list;
	for (int i = 0; i < listSize; ++i)
		list.push_back(objectListCommand[i]);
	SelectionMenu *menu = new SelectionMenu(Common::Point(x, y), menuWidth, list);
	renderer->pushMenu(menu);
	renderer->drawFrame();
	renderer->popMenu();
	delete menu;
	menu = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (!button);

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	// TODO: Both Future Wars and Operation Stealth call showMouse, drawMouse or something similar here.
}

int16 buildObjectListCommand(int16 param) {
	int16 i = 0, j = 0;

	for (i = 0; i < 20; i++) {
		objectListCommand[i][0] = 0;
	}

	for (i = 0; i < 255; i++) {
		if (objectTable[i].name[0] && objectTable[i].costume == param) {
			strcpy(objectListCommand[j], objectTable[i].name);
			objListTab[j] = i;
			j++;
		}
	}

	return j;
}

int16 selectSubObject(int16 x, int16 y, int16 param) {
	int16 listSize = buildObjectListCommand(param);
	int16 selectedObject;
	bool osExtras = g_cine->getGameType() == Cine::GType_OS;

	if (!listSize) {
		return -2;
	}

	selectedObject = makeMenuChoice(objectListCommand, listSize, x, y, 140, osExtras);

	if (selectedObject == -1)
		return -1;

	if (osExtras) {
		if (selectedObject >= 8000) {
			return objListTab[selectedObject - 8000] + 8000;
		}
	}

	return objListTab[selectedObject];
}

// TODO: Make separate functions for Future Wars's and Operation Stealth's version of this function, this is getting too messy
// TODO: Add support for using the different prepositions for different verbs (Doesn't work currently)
void makeCommandLine(void) {
	uint16 x, y;

	commandVar1 = 0;
	commandVar2 = -10;

	if (playerCommand != -1) {
		commandBuffer = defaultActionCommand[playerCommand];
	} else {
		commandBuffer = "";
	}

	if ((playerCommand != -1) && (choiceResultTable[playerCommand] == 2)) {	// need object selection ?
		int16 si;

		getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);

		if (g_cine->getGameType() == Cine::GType_FW) {
			si = selectSubObject(x, y + 8, -2);
		} else {
			si = selectSubObject(x, y + 8, -subObjectUseTable[playerCommand]);
		}

		if (si < 0) {
			if (g_cine->getGameType() == Cine::GType_OS) {
				canUseOnObject = 0;
			} else { // Future Wars
				playerCommand = -1;
				commandBuffer = "";
			}
		} else {
			if (g_cine->getGameType() == Cine::GType_OS) {
				if (si >= 8000) {
					si -= 8000;
					canUseOnObject = canUseOnItemTable[playerCommand];
				} else {
					canUseOnObject = 0;
				}
			}

			commandVar3[0] = si;
			commandVar1 = 1;
			commandBuffer += " ";
			commandBuffer += objectTable[commandVar3[0]].name;
			commandBuffer += " ";
			if (g_cine->getGameType() == Cine::GType_OS) {
				commandBuffer += commandPrepositionTable[playerCommand];
			} else { // Future Wars
				commandBuffer += defaultCommandPreposition;
			}
		}
	}

	if (g_cine->getGameType() == Cine::GType_OS || !(playerCommand != -1 && choiceResultTable[playerCommand] == 2)) {
		if (playerCommand == 2) {
			getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);
			processInventory(x, y + 8);
			playerCommand = -1;
			commandVar1 = 0;
			commandBuffer = "";
		}
	}

	if (g_cine->getGameType() == Cine::GType_OS && playerCommand != 2) {
		if (playerCommand != -1 && canUseOnObject != 0)	{ // call use on sub object
			int16 si;

			getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);

			si = selectSubObject(x, y + 8, -subObjectUseTable[playerCommand]);

			if (si >= 0) {
				if (si >= 8000) {
					si -= 8000;
				}

				commandVar3[commandVar1] = si;
				commandVar1++;
				commandBuffer += " ";
				commandBuffer += objectTable[si].name;
			}
		}

		isDrawCommandEnabled = 1;

		if (playerCommand != -1 && choiceResultTable[playerCommand] == commandVar1) {
			SelectedObjStruct obj;
			obj.idx = commandVar3[0];
			obj.param = commandVar3[1];
			int16 di = getRelEntryForObject(playerCommand, commandVar1, &obj);

			if (di != -1) {
				runObjectScript(di);
			} // TODO: else addFailureMessage(playerCommand)

			playerCommand = -1;
			commandVar1 = 0;
			commandBuffer = "";
		}
	}

	if (g_cine->getGameType() == Cine::GType_OS || !disableSystemMenu) {
		isDrawCommandEnabled = 1;
		renderer->setCommand(commandBuffer);
	}
}

uint16 needMouseSave = 0;

uint16 menuVar4 = 0;
uint16 menuVar5 = 0;

int16 makeMenuChoice(const CommandeType commandList[], uint16 height, uint16 X, uint16 Y, uint16 width, bool recheckValue) {
	int16 paramY;
	uint16 button;
	int16 var_A;
	int16 di;
	uint16 j;
	int16 mouseX, mouseY;
	int16 var_16;
	int16 var_14;
	int16 currentSelection, oldSelection;
	int16 var_4;
	SelectionMenu *menu;

	if (disableSystemMenu)
		return -1;

	paramY = (height * 9) + 10;

	if (X + width > 319) {
		X = 319 - width;
	}

	if (Y + paramY > 199) {
		Y = 199 - paramY;
	}

	Common::StringList list;
	for (uint16 i = 0; i < height; ++i)
		list.push_back(commandList[i]);
	menu = new SelectionMenu(Common::Point(X, Y), width, list);
	renderer->pushMenu(menu);
	renderer->drawFrame();

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button && !g_cine->shouldQuit());

	var_A = 0;

	currentSelection = 0;

	di = currentSelection * 9 + Y + 4;

	menu->setSelection(currentSelection);
	renderer->drawFrame();

	manageEvents();
	getMouseData(mouseUpdateStatus, &button, (uint16 *)&mouseX, (uint16 *)&mouseY);

	var_16 = mouseX;
	var_14 = mouseY;

	menuVar = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, (uint16 *)&mouseX, (uint16 *)&mouseY);

		if (button) {
			var_A = 1;
		}

		oldSelection = currentSelection;

		if (needMouseSave) {
			for (j = 0; j < 3; j++) {
				mainLoopSub6();
			}

			if (menuVar4 && currentSelection > 0) {	// go up
				currentSelection--;
			}

			if (menuVar5) {	// go down
				if (height - 1 > currentSelection) {
					currentSelection++;
				}
			}
		} else {
			if (mouseX > X && mouseX < X + width && mouseY > Y && mouseY < Y + height * 9) {
				currentSelection = (mouseY - (Y + 4)) / 9;

				if (currentSelection < 0)
					currentSelection = 0;

				if (currentSelection >= height)
					currentSelection = height - 1;
			}
		}

		if (currentSelection != oldSelection) {	// old != new
			if (needMouseSave) {
				hideMouse();
			}

			di = currentSelection * 9 + Y + 4;

			menu->setSelection(currentSelection);
			renderer->drawFrame();

//			if (needMouseSave) {
//				gfxRedrawMouseCursor();
//			}
		}

	} while (!var_A && !g_cine->shouldQuit());

	assert(!needMouseSave);

	var_4 = button;

	menuVar = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button && !g_cine->shouldQuit());

	if (var_4 == 2)	{	// recheck
		if (!recheckValue)
			return -1;
		else
			return currentSelection + 8000;
	}

	return currentSelection;
}

void makeActionMenu(void) {
	uint16 mouseButton;
	uint16 mouseX;
	uint16 mouseY;

	inMenu = true;

	getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

	if (g_cine->getGameType() == Cine::GType_OS) {
		playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70, true);

		if (playerCommand >= 8000) {
			playerCommand -= 8000;
			canUseOnObject = 1;
		}
	} else {
		playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70);
	}

	inMenu = false;
}

uint16 executePlayerInput(void) {
	uint16 var_5E;
	uint16 var_2;
	uint16 mouseX, mouseY, mouseButton;
	uint16 currentEntry = 0;
	uint16 di = 0;

	canUseOnObject = 0;

	if (isInPause) {
		drawString(otherMessages[2], 0);
		waitPlayerInput();
		isInPause = 0;
	}

	if (allowPlayerInput) {
		if (isDrawCommandEnabled) {
			renderer->setCommand(commandBuffer);
			isDrawCommandEnabled = 0;
		}

		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		while (mouseButton && currentEntry < 200) {
			if (mouseButton & 1) {
				di |= 1;
			}

			if (mouseButton & 2) {
				di |= 2;
			}

			getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

			currentEntry++;
		}

		if (di) {
			mouseButton = di;
		}

		if (playerCommand != -1) {
			if (mouseButton & 1) {
				if (mouseButton & 2) {
					g_cine->makeSystemMenu();
				} else {
					int16 si;
					do {
						manageEvents();
						getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
					} while (mouseButton && !g_cine->shouldQuit());

					si = getObjectUnderCursor(mouseX,
					    mouseY);

					if (si != -1) {
						commandVar3[commandVar1] = si;
						commandVar1++;

						commandBuffer += " ";
						commandBuffer += objectTable[si].name;


						isDrawCommandEnabled = 1;

						if (choiceResultTable[playerCommand] == commandVar1) {
							int16 relEntry;

							SelectedObjStruct obj;
							obj.idx = commandVar3[0];
							obj.param = commandVar3[1];

							relEntry = getRelEntryForObject(playerCommand, commandVar1, &obj);

							if (relEntry != -1) {
								runObjectScript(relEntry);
							} else {
								addPlayerCommandMessage(playerCommand);
							}

							playerCommand = -1;

							commandVar1 = 0;
							commandBuffer = "";
							renderer->setCommand(commandBuffer);
						}
					} else {
						globalVars[VAR_MOUSE_X_POS] = mouseX;
						globalVars[VAR_MOUSE_Y_POS] = mouseY;
					}
				}
			} else if (mouseButton & 2) {
				if (mouseButton & 1) {
					g_cine->makeSystemMenu();
				}

				makeActionMenu();
				makeCommandLine();
			} else {
				int16 objIdx;

				objIdx = getObjectUnderCursor(mouseX, mouseY);

				if (commandVar2 != objIdx) {
					if (objIdx != -1) {
						renderer->setCommand(commandBuffer + " " + objectTable[objIdx].name);
					} else {
						isDrawCommandEnabled = 1;
					}
				}

				commandVar2 = objIdx;
			}
		} else {
			if (mouseButton & 2) {
				if (!(mouseButton & 1)) {
					if (g_cine->getGameType() == Cine::GType_OS) {
						playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70, true);

						if (playerCommand >= 8000) {
							playerCommand -= 8000;
							canUseOnObject = 1;
						}
					} else {
						playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70);
					}

					makeCommandLine();
				} else {
					g_cine->makeSystemMenu();
				}
			} else {
				if (mouseButton & 1) {
					if (!(mouseButton & 2)) {
						int16 objIdx;
						int16 relEntry;

						globalVars[VAR_MOUSE_X_POS] = mouseX;
						if (!mouseX) {
							globalVars[VAR_MOUSE_X_POS]++;
						}

						globalVars[VAR_MOUSE_Y_POS] = mouseY;

						objIdx = getObjectUnderCursor(mouseX, mouseY);

						if (objIdx != -1) {
							currentSelectedObject.idx = objIdx;
							currentSelectedObject.param = -1;

							relEntry = getRelEntryForObject(6, 1, &currentSelectedObject);

							if (relEntry != -1) {
								runObjectScript(relEntry);
							}
						}
					} else {
						g_cine->makeSystemMenu();
					}
				}
			}
		}
	} else {
		di = 0;
		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		while (mouseButton && !g_cine->shouldQuit()) {
			if (mouseButton & 1) {
				di |= 1;
			}

			if (mouseButton & 2) {
				di |= 2;
			}

			manageEvents();
			getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);
		}

		if (di) {
			mouseButton = di;
		}

		if ((mouseButton & 1) && (mouseButton & 2)) {
			g_cine->makeSystemMenu();
		}
	}

	var_2 = menuVar & 0x7F;
	var_5E = var_2;

	if (menuVar & 0x80) {
		var_5E = 0;
		var_2 = 0;
	}

	if (egoMovedWithKeyboard && allowPlayerInput) {	// use keyboard
		egoMovedWithKeyboard = false;

		switch (globalVars[VAR_MOUSE_X_MODE]) {
		case 1:
			mouseX = objectTable[1].x + 12;
			break;
		case 2:
			mouseX = objectTable[1].x + 7;
			break;
		default:
			mouseX = globalVars[VAR_MOUSE_X_POS];
			break;
		}

		switch (globalVars[VAR_MOUSE_Y_MODE]) {
		case 1:
			mouseY = objectTable[1].y + 34;
			break;
		case 2:
			mouseY = objectTable[1].y + 28;
			break;
		default:
			mouseY = globalVars[VAR_MOUSE_Y_POS];
			break;
		}

		if (var_5E == bgVar0) {
			var_5E = 0;

			globalVars[VAR_MOUSE_X_POS] = mouseX;
			globalVars[VAR_MOUSE_Y_POS] = mouseY;
		} else {
			if (xMoveKeyb) {
				if (xMoveKeyb == kKeybMoveLeft) {
					globalVars[VAR_MOUSE_X_POS] = 1;
				} else {
					globalVars[VAR_MOUSE_X_POS] = 320;
				}
			} else {
				globalVars[VAR_MOUSE_X_POS] = mouseX;
			}

			if (yMoveKeyb) {
				if (yMoveKeyb == kKeybMoveUp) {
					globalVars[VAR_MOUSE_Y_POS] = 1;
				} else {
					globalVars[VAR_MOUSE_Y_POS] = 200;
				}
			} else {
				globalVars[VAR_MOUSE_Y_POS] = mouseY;
			}
		}

		bgVar0 = var_5E;
	} else {		// don't use keyboard for move -> shortcuts to commands
		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		switch (var_2 - 59) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if (allowPlayerInput) {
				playerCommand = var_2 - 59;
				makeCommandLine();
			}
			break;
		case 6:
		case 7:
		case 8:
		case 23:
			break;
		case 9:
		case 24:
			g_cine->makeSystemMenu();
			break;
		default:
			//  printf("Unhandled case %d in last part of executePlayerInput\n",var2-59);
			break;
		}
	}

	renderer->clearMenuStack();

	// Update Operation Stealth specific global variables.
	// This fixes swimming at the bottom of the ocean after
	// having been thrown into it with the girl.
	if (g_cine->getGameType() == Cine::GType_OS) {
		globalVars[VAR_MOUSE_X_POS_2ND] = globalVars[VAR_MOUSE_X_POS];
		globalVars[VAR_MOUSE_Y_POS_2ND] = globalVars[VAR_MOUSE_Y_POS];
	}

	return var_5E;
}

void drawSprite(Common::List<overlay>::iterator it, const byte *spritePtr, const byte *maskPtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
	byte *msk = NULL;
	int16 maskX, maskY, maskWidth, maskHeight;
	uint16 maskSpriteIdx;

	msk = (byte *)malloc(width * height);

	if (g_cine->getGameType() == Cine::GType_OS) {
		generateMask(spritePtr, msk, width * height, objectTable[it->objIdx].part);
	} else {
		memcpy(msk, maskPtr, width * height);
	}

	for (++it; it != overlayList.end(); ++it) {
		if (it->type != 5) {
			continue;
		}

		maskX = objectTable[it->objIdx].x;
		maskY = objectTable[it->objIdx].y;

		maskSpriteIdx = ABS((int16)(objectTable[it->objIdx].frame));

		maskWidth = animDataTable[maskSpriteIdx]._realWidth;
		maskHeight = animDataTable[maskSpriteIdx]._height;
		gfxUpdateSpriteMask(msk, x, y, width, height, animDataTable[maskSpriteIdx].data(), maskX, maskY, maskWidth, maskHeight);

#ifdef DEBUG_SPRITE_MASK
		gfxFillSprite(animDataTable[maskSpriteIdx].data(), maskWidth, maskHeight, page, maskX, maskY, 1);
#endif
	}

	gfxDrawMaskedSprite(spritePtr, msk, width, height, page, x, y);
	free(msk);
}

void removeMessages() {
	Common::List<overlay>::iterator it;
	bool remove;

	for (it = overlayList.begin(); it != overlayList.end(); ) {
		if (g_cine->getGameType() == Cine::GType_OS) {
			// NOTE: These are really removeOverlay calls that have been deferred.
			// In Operation Stealth's disassembly elements are removed from the
			// overlay list right in the drawOverlays function (And actually in
			// some other places too) and that's where incrementing a the overlay's
			// last parameter by one if it's negative and testing it for positivity
			// comes from too.
			remove = it->type == 3 || (it->type == 2 && (it->color >= 0 || ++it->color >= 0));
		} else { // Future Wars
			remove = it->type == 2 || it->type == 3;
		}

		if (remove) {
			it = overlayList.erase(it);
		} else {
			++it;
		}
	}
}

uint16 processKeyboard(uint16 param) {
	return 0;
}

void mainLoopSub6(void) {
}

void checkForPendingDataLoad(void) {
	if (newPrcName[0] != 0) {
		bool loadPrcOk = loadPrc(newPrcName);

		strcpy(currentPrcName, newPrcName);
		strcpy(newPrcName, "");

		// Check that the loading of the script file was successful before
		// trying to add script 1 from it to the global scripts list. This
		// fixes a crash when failing copy protection in Amiga or Atari ST
		// versions of Future Wars.
		if (loadPrcOk) {
			addScriptToGlobalScripts(1);
		} else if (scumm_stricmp(currentPrcName, COPY_PROT_FAIL_PRC_NAME)) {
			// We only show an error here for other files than the file that
			// is loaded if copy protection fails (i.e. L201.ANI).
			warning("checkForPendingDataLoad: loadPrc(%s) failed", currentPrcName);
		}
	}

	if (newRelName[0] != 0) {
		loadRel(newRelName);

		strcpy(currentRelName, newRelName);
		strcpy(newRelName, "");
	}

	if (newObjectName[0] != 0) {
		overlayList.clear();

		loadObject(newObjectName);

		strcpy(currentObjectName, newObjectName);
		strcpy(newObjectName, "");
	}

	if (newMsgName[0] != 0) {
		loadMsg(newMsgName);

		strcpy(currentMsgName, newMsgName);
		strcpy(newMsgName, "");
	}
}

void hideMouse(void) {
}

void removeExtention(char *dest, const char *source) {
	strcpy(dest, source);

	byte *ptr = (byte *) strchr(dest, '.');

	if (ptr) {
		*ptr = 0;
	}
}

void addMessage(byte param1, int16 param2, int16 param3, int16 param4, int16 param5) {
	overlay tmp;

	tmp.objIdx = param1;
	tmp.type = 2;
	tmp.x = param2;
	tmp.y = param3;
	tmp.width = param4;
	tmp.color = param5;

	overlayList.push_back(tmp);
}

Common::List<SeqListElement> seqList;

void removeSeq(uint16 param1, uint16 param2, uint16 param3) {
	Common::List<SeqListElement>::iterator it;

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->objIdx == param1 && it->var4 == param2 && it->varE == param3) {
			it->var4 = -1;
			break;
		}
	}
}

bool isSeqRunning(uint16 param1, uint16 param2, uint16 param3) {
	Common::List<SeqListElement>::iterator it;

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->objIdx == param1 && it->var4 == param2 && it->varE == param3) {
			// Just to be on the safe side there's a restriction of the
			// addition's result to 16-bit arithmetic here like in the
			// original. It's possible that it's not strictly needed.
			return ((it->var14 + it->var16) & 0xFFFF) == 0;
		}
	}

	return true;
}

void addSeqListElement(uint16 objIdx, int16 param1, int16 param2, int16 frame, int16 param4, int16 param5, int16 param6, int16 param7, int16 param8) {
	Common::List<SeqListElement>::iterator it;
	SeqListElement tmp;

	for (it = seqList.begin(); it != seqList.end() && it->varE < param7; ++it) ;

	tmp.objIdx = objIdx;
	tmp.var4 = param1;
	tmp.var8 = param2;
	tmp.frame = frame;
	tmp.varC = param4;
	tmp.var14 = 0;
	tmp.var16 = 0;
	tmp.var18 = param5;
	tmp.var1A = param6;
	tmp.varE = param7;
	tmp.var10 = param8;
	tmp.var12 = param8;
	tmp.var1C = 0;
	tmp.var1E = 0;

	seqList.insert(it, tmp);
}

void modifySeqListElement(uint16 objIdx, int16 var4Test, int16 param1, int16 param2, int16 param3, int16 param4) {
	// Find a suitable list element and modify it
	for (Common::List<SeqListElement>::iterator it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->objIdx == objIdx && it->var4 == var4Test) {
			it->varC  = param1;
			it->var18 = param2;
			it->var1A = param3;
			it->var10 = it->var12 = param4;
			break;
		}
	}
}

void computeMove1(SeqListElement &element, int16 x, int16 y, int16 param1,
    int16 param2, int16 x2, int16 y2) {
	element.var16 = 0;
	element.var14 = 0;

	if (y2) {
		if (y - param2 > y2) {
			element.var16 = 2;
		}

		if (y + param2 < y2) {
			element.var16 = 1;
		}
	}

	if (x2) {
		if (x - param1 > x2) {
			element.var14 = 2;
		}

		if (x + param1 < x2) {
			element.var14 = 1;
		}
	}
}

uint16 computeMove2(SeqListElement &element) {
	int16 returnVar = 0;

	if (element.var16 == 1) {
		returnVar = 4;
	} else if (element.var16 == 2) {
		returnVar = 3;
	}

	if (element.var14 == 1) {
		returnVar = 1;
	} else if (element.var14 == 2) {
		returnVar = 2;
	}

	return returnVar;
}

uint16 addAni(uint16 param1, uint16 objIdx, const int8 *ptr, SeqListElement &element, uint16 param3, int16 *param4) {
	const int8 *ptrData;
	const int8 *ptr2;
	int16 di;

	debug(5, "addAni: param1 = %d, objIdx = %d, ptr = %p, element.var8 = %d, element.var14 = %d param3 = %d",
		param1, objIdx, ptr, element.var8, element.var14, param3);

	// In the original an error string is set and 0 is returned if the following doesn't hold
	assert(ptr);

	// We probably could just use a local variable here instead of the dummyU16 but
	// haven't checked if this has any side-effects so keeping it this way still.
	dummyU16 = READ_BE_UINT16(ptr + param1 * 2 + 8);
	ptrData = ptr + dummyU16;

	// In the original an error string is set and 0 is returned if the following doesn't hold
	assert(*ptrData);

	di = (objectTable[objIdx].costume + 1) % (*ptrData);
	++ptrData; // Jump over the just read byte
	// Here ptr2 seems to be indexing a table of structs (8 bytes per struct):
	//	struct {
	//		int8 x;			// 0 (Used with checkCollision)
	//		int8 y;			// 1 (Used with checkCollision)
	//		int8 numZones;	// 2 (Used with checkCollision)
	//		int8 var3;		// 3 (Not used in this function)
	//		int8 xAdd;		// 4 (Used with an object)
	//		int8 yAdd;		// 5 (Used with an object)
	//		int8 maskAdd;	// 6 (Used with an object)
	//		int8 frameAdd;	// 7 (Used with an object)
	//	};
	ptr2 = ptrData + di * 8;

	// We might probably safely discard the AND by 1 here because
	// at least in the original checkCollision returns always 0 or 1.
	if ((checkCollision(objIdx, ptr2[0], ptr2[1], ptr2[2], ptr[0]) & 1)) {
		return 0;
	}

	objectTable[objIdx].x += ptr2[4];
	objectTable[objIdx].y += ptr2[5];
	objectTable[objIdx].mask += ptr2[6];

	if (ptr2[6]) {
		resetGfxEntityEntry(objIdx);
	}

	objectTable[objIdx].frame = ptr2[7] + element.var8;

	if (param3 || !element.var14) {
		objectTable[objIdx].costume = di;
	} else {
		assert(param4);
		*param4 = di;
	}

	return 1;
}

/*!
 * Permutates the overlay list into a different order according to some logic.
 * \todo Check this function for correctness (Wasn't very easy to reverse engineer so there may be errors)
 */
void resetGfxEntityEntry(uint16 objIdx) {
	Common::List<overlay>::iterator it, bObjsCutPoint;
	Common::List<overlay> aReverseObjs, bObjs;
	bool foundCutPoint = false;

	// Go through the overlay list and partition the whole list into two categories (Type A and type B objects)
	for (it = overlayList.begin(); it != overlayList.end(); ++it) {
		if (it->objIdx == objIdx && it->type != 2 && it->type != 3) { // Type A object
			aReverseObjs.push_front(*it);
		} else { // Type B object
			bObjs.push_back(*it);
			uint16 objectMask;
			if (it->type == 2 || it->type == 3) {
				objectMask = 10000;
			} else {
				objectMask = objectTable[it->objIdx].mask;
			}

			if (objectTable[objIdx].mask > objectMask) { // Check for B objects' cut point
				bObjsCutPoint = bObjs.reverse_begin();
				foundCutPoint = true;
			}
		}
	}

	// Recreate the overlay list in a different order.
	overlayList.clear();
	if (foundCutPoint) {
		// If a cut point was found the order is:
		// B objects before the cut point, the cut point, A objects in reverse order, B objects after cut point.
		++bObjsCutPoint; // Include the cut point in the first list insertion
		overlayList.insert(overlayList.end(), bObjs.begin(), bObjsCutPoint);
		overlayList.insert(overlayList.end(), aReverseObjs.begin(), aReverseObjs.end());
		overlayList.insert(overlayList.end(), bObjsCutPoint, bObjs.end());
	} else {
		// If no cut point was found the order is:
		// A objects in reverse order, B objects.
		overlayList.insert(overlayList.end(), aReverseObjs.begin(), aReverseObjs.end());
		overlayList.insert(overlayList.end(), bObjs.begin(), bObjs.end());
	}
}

void processSeqListElement(SeqListElement &element) {
	int16 x = objectTable[element.objIdx].x;
	int16 y = objectTable[element.objIdx].y;
	const int8 *ptr1 = (const int8 *) animDataTable[element.frame].data();
	int16 var_10;
	int16 var_4;
	int16 var_2;

	// Initial interpretations for variables addressed through ptr1 (8-bit addressing):
	// These may be inaccurate!
	// 0: ?
	// 1: xRadius
	// 2: yRadius
	// 3: ?
	// 4: xAdd
	// 5: yAdd
	// 6: ?
	// 7: ?
	// After this come (At least at positions 0, 1 and 3 in 16-bit addressing)
	// 16-bit big-endian values used for addressing through ptr1.

	if (element.var12 < element.var10) {
		element.var12++;
		return;
	}

	element.var12 = 0;

	if (ptr1) {
		int16 param1 = ptr1[1];
		int16 param2 = ptr1[2];

		if (element.varC != 255) {
			int16 x2 = element.var18;
			int16 y2 = element.var1A;
			if (element.varC) {
				x2 += objectTable[element.varC].x;
				y2 += objectTable[element.varC].y;
			}
			computeMove1(element, ptr1[4] + x, ptr1[5] + y, param1, param2, x2, y2);
		} else {
			if (xMoveKeyb && allowPlayerInput) {
				int16 adder = param1 + 1;
				if (xMoveKeyb != kKeybMoveRight) {
					adder = -adder;
				}
				globalVars[VAR_MOUSE_X_POS] = globalVars[VAR_MOUSE_X_POS_2ND] = ptr1[4] + x + adder;
			}

			if (yMoveKeyb && allowPlayerInput) {
				int16 adder = param2 + 1;
				if (yMoveKeyb != kKeybMoveDown) {
					adder = -adder;
				}
				globalVars[VAR_MOUSE_Y_POS] = globalVars[VAR_MOUSE_Y_POS_2ND] = ptr1[5] + y + adder;
			}

			if (globalVars[VAR_MOUSE_X_POS] || globalVars[VAR_MOUSE_Y_POS]) {
				computeMove1(element, ptr1[4] + x, ptr1[5] + y, param1, param2, globalVars[VAR_MOUSE_X_POS], globalVars[VAR_MOUSE_Y_POS]);
			} else {
				element.var16 = 0;
				element.var14 = 0;
			}
		}

		var_10 = computeMove2(element);

		if (var_10) {
			element.var1C = var_10;
			element.var1E = var_10;
		}

		var_4 = -1;

		if ((element.var16 == 1
			&& !addAni(3, element.objIdx, ptr1, element, 0, &var_4)) || (element.var16 == 2	&& !addAni(2, element.objIdx, ptr1, element, 0,
			    &var_4))) {
			if (element.varC == 255) {
				globalVars[VAR_MOUSE_Y_POS] = 0;
			}
		}

		if ((element.var14 == 1
			&& !addAni(0, element.objIdx, ptr1, element, 1, &var_2))) {
			if (element.varC == 255) {
				globalVars[VAR_MOUSE_X_POS] = 0;

				if (var_4 != -1) {
					objectTable[element.objIdx].costume = var_4;
				}
			}
		}

		if ((element.var14 == 2 && !addAni(1, element.objIdx, ptr1, element, 1, &var_2))) {
			if (element.varC == 255) {
				globalVars[VAR_MOUSE_X_POS] = 0;

				if (var_4 != -1) {
					objectTable[element.objIdx].costume = var_4;
				}
			}
		}

		if (element.var16 + element.var14 == 0) {
			if (element.var1C) {
				if (element.var1E) {
					objectTable[element.objIdx].costume = 0;
					element.var1E = 0;
				}

				addAni(element.var1C + 3, element.objIdx, ptr1, element, 1, &var_2);

			}
		}

	}
}

void processSeqList(void) {
	Common::List<SeqListElement>::iterator it;

	for (it = seqList.begin(); it != seqList.end(); ++it) {
		if (it->var4 == -1) {
			continue;
		}

		processSeqListElement(*it);
	}
}


bool makeTextEntryMenu(const char *messagePtr, char *inputString, int stringMaxLength, int y) {
	int len = strlen(messagePtr);
	int16 width = 6 * len + 20;

	width = CLIP((int)width, 180, 250);

	int16 x = (320 - width) / 2;

	getKeyData(); // clear input key

	int quit = 0;
	bool redraw = true;
	CommandeType tempString;
	int inputLength = strlen(inputString);
	int inputPos = inputLength + 1;

	TextInputMenu *inputBox = new TextInputMenu(Common::Point(x - 16, y), width + 32, messagePtr);
	renderer->pushMenu(inputBox);

	while (!quit) {
		if (redraw) {
			inputBox->setInput(inputString, inputPos);
			renderer->drawFrame();
			redraw = false;
		}

		char ch[2];
		memset(tempString, 0, stringMaxLength);
		ch[1] = 0;

		manageEvents();
		int keycode = getKeyData();
		uint16 mouseButton, mouseX, mouseY;

		getMouseData(0, &mouseButton, &mouseX, &mouseY);

		if ((mouseButton & 2) || g_cine->shouldQuit())
			quit = 2;
		else if (mouseButton & 1)
			quit = 1;

		switch (keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (inputPos <= 1) {
				break;
			}
			inputPos--;
			redraw = true;
		case Common::KEYCODE_DELETE:
			if (inputPos <= inputLength) {
				if (inputPos != 1) {
					strncpy(tempString, inputString, inputPos - 1);
				}
				if (inputPos != inputLength) {
					strncat(tempString, &inputString[inputPos], inputLength - inputPos);
				}
				strcpy(inputString, tempString);
				inputLength = strlen(inputString);
				redraw = true;
			}
			break;
		case Common::KEYCODE_LEFT:
			if (inputPos > 1) {
				inputPos--;
				redraw = true;
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (inputPos <= inputLength) {
				inputPos++;
				redraw = true;
			}
			break;
		default:
			if (((keycode >= 'a') && (keycode <='z')) ||
				((keycode >= '0') && (keycode <='9')) ||
				((keycode >= 'A') && (keycode <='Z')) ||
				(keycode == ' ')) {
				if (inputLength < stringMaxLength - 1) {
					ch[0] = keycode;
					if (inputPos != 1) {
						strncpy(tempString, inputString, inputPos - 1);
						strcat(tempString, ch);
					}
					if ((inputLength == 0) || (inputPos == 1)) {
						strcpy(tempString, ch);
					}
					if ((inputLength != 0) && (inputPos != inputLength)) {
						strncat(tempString, &inputString[inputPos - 1], inputLength - inputPos + 1);
					}

					strcpy(inputString, tempString);
					inputLength = strlen(inputString);
					inputPos++;
					redraw = true;
				}
			}
			break;
		}
	}

	renderer->popMenu();
	delete inputBox;

	if (quit == 2)
		return false;

	return true;
}

} // End of namespace Cine
