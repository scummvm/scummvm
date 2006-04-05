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
#include "common/endian.h"

#include "cine/cine.h"
#include "cine/font.h"
#include "cine/main_loop.h"
#include "cine/object.h"
#include "cine/sfx_player.h"
#include "cine/various.h"

namespace Cine {

int16 allowSystemMenu = 0;

int16 commandVar3[4];
int16 commandVar1;
int16 commandVar2;

unk1Struct messageTable[NUM_MAX_MESSAGE];

uint32 var6;
uint32 var8;
byte *var9;

uint16 var2;
uint16 var3;
uint16 var4;
uint16 var5;

void drawString(const char *string, byte param) {
}

void blitRawScreen(byte *frontBuffer) {
	gfxFlipRawPage(frontBuffer);
}

Common::File partFileHandle;

void waitPlayerInput(void) {
}

void freeAnimDataTable(void) {
}

void mainLoopSub1(void) {
}

void setTextWindow(uint16 param1, uint16 param2, uint16 param3, uint16 param4) {
}

uint16 errorVar;
byte menuVar;

void gfxFuncGen1(byte *param1, byte *param2, byte *param3, byte *param4, int16 param5) {
}

byte *page0c;

void ptrGfxFunc13(void) {
}

void gfxFuncGen2(void) {
}

uint16 allowPlayerInput;

uint16 checkForPendingDataLoadSwitch;

uint16 fadeRequired;
uint16 isDrawCommandEnabled;
uint16 waitForPlayerClick;
uint16 menuCommandLen;
uint16 var17;
uint16 var18;
uint16 var19;
uint16 var20;
byte var21;

int16 playerCommand;

char commandBuffer[80];

uint16 palette_[256];

char currentPrcName[20];
char currentRelName[20];
char currentObjectName[20];
char currentMsgName[20];
char newPrcName[20];
char newRelName[20];
char newObjectName[20];
char newMsgName[20];

char currentBgName[8][15];
char currentCtName[15];
char currentPartName[15];
char currentDatName[30];

int16 saveVar2;

byte isInPause = 0;

uint16 defaultMenuBoxColor;

byte inputVar1 = 0;

uint16 inputVar2;
uint16 inputVar3;

commandeType defaultActionCommand[] = {
	"EXAMINE",
	"TAKE",
	"INVENTORY",
	"USE",
	"OPERATE",
	"SPEAK",
	"NOACTION"
// french
/*
	"EXAMINER",
	"PRENDRE",
	"INVENTAIRE",
	"UTILISER",
	"ACTIONNER",
	"PARLER",
	"NOACTION"
*/
};

selectedObjStruct currentSelectedObject;

void mainLoopSub3(void) {
}

int16 stopObjectScript(int16 entryIdx) {
	prcLinkedListStruct *currentHead = &objScriptList;
	prcLinkedListStruct *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead) {
		if (currentHead->scriptIdx == entryIdx) {
			currentHead->scriptIdx = -1;
			return 0;
		}

		currentHead = currentHead->next;
	}

	return (-1);
}

void runObjectScript(int16 entryIdx) {
	uint16 i;
	prcLinkedListStruct *pNewElement;
	prcLinkedListStruct *currentHead = &objScriptList;
	prcLinkedListStruct *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;

		assert(tempHead);

		currentHead = tempHead->next;
	}

	pNewElement = (prcLinkedListStruct *)malloc(sizeof(prcLinkedListStruct));

	assert(pNewElement);

	pNewElement->next = tempHead->next;
	tempHead->next = pNewElement;

	// copy the stack into the script instance
	for (i = 0; i < SCRIPT_STACK_SIZE; i++) {
		pNewElement->stack[i] = 0;
	}

	for (i = 0; i < 50; i++) {
		pNewElement->localVars[i] = 0;
	}

	pNewElement->compareResult = 0;
	pNewElement->scriptPosition = 0;

	pNewElement->scriptPtr = (byte *)relTable[entryIdx].data;
	pNewElement->scriptIdx = entryIdx;

	computeScriptStack(pNewElement->scriptPtr, pNewElement->stack, relTable[entryIdx].size);
}

int16 getRelEntryForObject(uint16 param1, uint16 param2,
    selectedObjStruct *pSelectedObject) {
	int16 i;
	int16 di = -1;

	for (i = 0; i < NUM_MAX_REL; i++) {
		if (relTable[i].data && relTable[i].obj1Param1 == param1 && relTable[i].obj1Param2 == pSelectedObject->idx) {
			if (param2 == 1) {
				di = i;
			} else if (param2 == 2) {
				if (relTable[i].obj2Param == pSelectedObject->param) {
					di = i;
				}
			}
		}

		if (di != -1)
			break;
	}

	return di;
}

int16 getObjectUnderCursor(uint16 x, uint16 y) {
	overlayHeadElement *currentHead = overlayHead.previous;

	while (currentHead) {
		if (currentHead->type < 2) {
			if (objectTable[currentHead->objIdx].name[0]) {
				int16 objX;
				int16 objY;
				int16 frame;
				int16 part;
				int16 treshold;
				int16 height;
				int16 xdif;
				int16 ydif;

				objX = objectTable[currentHead->objIdx].x;
				objY = objectTable[currentHead->objIdx].y;

				frame = abs((int16)(objectTable[currentHead->objIdx].frame));

				part = objectTable[currentHead->objIdx].part;

				if (currentHead->type == 0) {
					treshold = animDataTable[frame].var1;
				} else {
					treshold = animDataTable[frame].width / 2;
				}

				height = animDataTable[frame].height;

				xdif = x - objX;
				ydif = y - objY;

				if ((xdif >= 0) && ((treshold << 4) > xdif) && (ydif > 0) && (ydif < height)) {
					if (animDataTable[frame].ptr1) {
						if (gameType == Cine::GID_OS)
							return currentHead->objIdx;

						if (currentHead->type == 0)	{ // use generated mask
							if (gfxGetBit(x - objX, y - objY, animDataTable[frame].ptr2, animDataTable[frame].width)) {
								return currentHead->objIdx;
							}
						} else if (currentHead->type == 1) { // is mask
							if (gfxGetBit(x - objX, y - objY, animDataTable[frame].ptr1, animDataTable[frame].width * 4)) {
								return currentHead->objIdx;
							}
						}
					}
				}
			}
		}

		currentHead = currentHead->previous;
	}

	return -1;
}

const commandeType systemMenu[] = {
	"Pause",
	"Restart Game",
	"Quit",
	"Backup Drive is A:",
	"Restore game",
	"Save game"
// french
/*
	"Pause",
	"Nouvelle partie",
	"Quitter",
	"Lecteur de Svg. A:",
	"Charger une partie",
	"Sauver la partie"
*/
};

const commandeType confirmMenu[] = {
	"Ok, go ahead ...",
	"Absolutely Not!"
// french
/*
	"Ok , Vas-y ...",
	"Surtout Pas !"
*/
};

commandeType currentSaveName[10];

int16 loadSaveDirectory(void) {
	Common::File fHandle;

	if (gameType == Cine::GID_FW)
		fHandle.open("FW.DIR", Common::File::kFileReadMode, savePath);
	else
		fHandle.open("OS.DIR", Common::File::kFileReadMode, savePath);

	if (!fHandle.isOpen()) {
		return 0;
	}

	fHandle.read(currentSaveName, 10 * 20);
	fHandle.close();

	return 1;
}

int16 currentDisk;

void loadObjectScritpFromSave(Common::File *fHandle) {
	int16 i;

	prcLinkedListStruct *newElement;
	prcLinkedListStruct *currentHead = &globalScriptsHead;
	prcLinkedListStruct *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	newElement =
	    (prcLinkedListStruct *)malloc(sizeof(prcLinkedListStruct));

	newElement->next = tempHead->next;
	tempHead->next = newElement;

	for (i = 0; i < SCRIPT_STACK_SIZE; i++)
		newElement->stack[i] = fHandle->readUint16BE();

	for (i = 0; i < 50; i++)
		newElement->localVars[i] = fHandle->readUint16BE();

	newElement->compareResult = fHandle->readUint16BE();

	newElement->scriptPosition = fHandle->readUint16BE();

	newElement->scriptIdx = fHandle->readUint16BE();

	newElement->scriptPtr = (byte *)relTable[newElement->scriptIdx].data;
}

void loadGlobalScritpFromSave(Common::File *fHandle) {
	int16 i;

	prcLinkedListStruct *newElement;
	prcLinkedListStruct *currentHead = &globalScriptsHead;
	prcLinkedListStruct *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	newElement = (prcLinkedListStruct *)malloc(sizeof(prcLinkedListStruct));

	newElement->next = tempHead->next;
	tempHead->next = newElement;

	for (i = 0; i < SCRIPT_STACK_SIZE; i++)
		newElement->stack[i] = fHandle->readUint16BE();

	for (i = 0; i < 50; i++)
		newElement->localVars[i] = fHandle->readUint16BE();

	newElement->compareResult = fHandle->readUint16BE();

	newElement->scriptPosition = fHandle->readUint16BE();

	newElement->scriptIdx = fHandle->readUint16BE();

	newElement->scriptPtr = scriptTable[newElement->scriptIdx].ptr;
}

void loadOverlayFromSave(Common::File *fHandle) {
	overlayHeadElement *newElement;
	overlayHeadElement *currentHead = &overlayHead;
	overlayHeadElement *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	newElement = (overlayHeadElement *)malloc(sizeof(overlayHeadElement));

	fHandle->readUint32BE();
	fHandle->readUint32BE();

	newElement->objIdx = fHandle->readUint16BE();
	newElement->type = fHandle->readUint16BE();
	newElement->x = fHandle->readSint16BE();
	newElement->y = fHandle->readSint16BE();
	newElement->width = fHandle->readSint16BE();
	newElement->color = fHandle->readSint16BE();

	newElement->next = tempHead->next;
	tempHead->next = newElement;

	if (!currentHead)
		currentHead = &overlayHead;

	newElement->previous = currentHead->previous;

	currentHead->previous = newElement;

}

void setupGlobalScriptList(void) {
	prcLinkedListStruct *currentHead = globalScriptsHead.next;

	while (currentHead) {
		currentHead->scriptPtr = scriptTable[currentHead->scriptIdx].ptr;

		currentHead = currentHead->next;
	}
}

void setupObjectScriptList(void) {
	prcLinkedListStruct *currentHead = objScriptList.next;

	while (currentHead) {
		currentHead->scriptPtr = (byte *)relTable[currentHead->scriptIdx].data;
		currentHead = currentHead->next;
	}
}

int16 makeLoad(char *saveName) {
	int16 i;
	int16 size;
	Common::File fHandle;

	fHandle.open(saveName, Common::File::kFileReadMode, savePath);

	if (!fHandle.isOpen()) {
		drawString("Cette sauvegarde n'existe pas ...", 0);
		waitPlayerInput();
		// restoreScreen();
		checkDataDisk(-1);
		return -1;
	}

	g_sfxPlayer->stop();
	freeAnimDataTable();
	unloadAllMasks();
	freePrcLinkedList();
	releaseObjectScripts();
	closeEngine7();
	closePart();

	for (i = 0; i < NUM_MAX_REL; i++) {
		if (relTable[i].data) {
			free(relTable[i].data);
			relTable[i].data = NULL;
			relTable[i].size = 0;
			relTable[i].obj1Param1 = 0;
			relTable[i].obj1Param2 = 0;
			relTable[i].obj2Param = 0;
		}
	}

	for (i = 0; i < NUM_MAX_SCRIPT; i++) {
		if (scriptTable[i].ptr) {
			free(scriptTable[i].ptr);
			scriptTable[i].ptr = NULL;
			scriptTable[i].size = 0;
		}
	}

	for (i = 0; i < NUM_MAX_MESSAGE; i++) {
		messageTable[i].len = 0;

		if (messageTable[i].ptr) {
			free(messageTable[i].ptr);
			messageTable[i].ptr = NULL;
		}
	}

	for (i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].part = 0;
		objectTable[i].name[0] = 0;
		objectTable[i].frame = 0;
		objectTable[i].mask = 0;
		objectTable[i].costume = 0;
	}

	for (i = 0; i < 255; i++) {
		globalVars[i] = 0;
	}

	var2 = 0;
	var3 = 0;
	var4 = 0;
	var5 = 0;

	strcpy(newPrcName, "");
	strcpy(newRelName, "");
	strcpy(newObjectName, "");
	strcpy(newMsgName, "");
	strcpy(currentBgName[0], "");
	strcpy(currentCtName, "");

	allowPlayerInput = 0;
	waitForPlayerClick = 0;
	playerCommand = -1;
	isDrawCommandEnabled = 0;

	strcpy(commandBuffer, "");

	globalVars[VAR_MOUSE_X_POS] = 0;
	globalVars[VAR_MOUSE_Y_POS] = 0;

	fadeRequired = 0;

	for (i = 0; i < 16; i++) {
		palette_[i] = 0;
	}

	checkForPendingDataLoadSwitch = 0;

	currentDisk = fHandle.readUint16BE();

	fHandle.read(currentPartName, 13);
	fHandle.read(currentDatName, 13);

	saveVar2 = fHandle.readSint16BE();

	fHandle.read(currentPrcName, 13);
	fHandle.read(currentRelName, 13);
	fHandle.read(currentMsgName, 13);
	fHandle.read(currentBgName[0], 13);
	fHandle.read(currentCtName, 13);

	i = fHandle.readUint16BE();
	i = fHandle.readUint16BE();

	fHandle.read(objectTable, i * 255);

	for (i = 0; i < NUM_MAX_OBJECT; i++) {
		objectTable[i].x = TO_BE_16(objectTable[i].x);
		objectTable[i].y = TO_BE_16(objectTable[i].y);
		objectTable[i].mask = TO_BE_16(objectTable[i].mask);
		objectTable[i].frame = TO_BE_16(objectTable[i].frame);
		objectTable[i].costume = TO_BE_16(objectTable[i].costume);
		objectTable[i].part = TO_BE_16(objectTable[i].part);
	}

	for (i = 0; i < 16; i++) {
		palette_[i] = fHandle.readUint16BE();
	}

	for (i = 0; i < 16; i++) {
		tempPalette[i] = fHandle.readUint16BE();
	}

	for (i = 0; i < 255; i++) {
		globalVars[i] = fHandle.readUint16BE();
	}

	for (i = 0; i < 16; i++) {
		zoneData[i] = fHandle.readUint16BE();
	}

	for (i = 0; i < 4; i++) {
		commandVar3[i] = fHandle.readUint16BE();
	}

	fHandle.read(commandBuffer, 0x50);

	defaultMenuBoxColor = fHandle.readUint16BE();
	bgVar0 = fHandle.readUint16BE();
	allowPlayerInput = fHandle.readUint16BE();
	playerCommand = fHandle.readSint16BE();
	commandVar1 = fHandle.readSint16BE();
	isDrawCommandEnabled = fHandle.readUint16BE();
	var5 = fHandle.readUint16BE();
	var4 = fHandle.readUint16BE();
	var3 = fHandle.readUint16BE();
	var2 = fHandle.readUint16BE();
	commandVar2 = fHandle.readSint16BE();
	defaultMenuBoxColor2 = fHandle.readUint16BE();

	i = fHandle.readUint16BE();
	i = fHandle.readSint16BE();
	fHandle.read(animDataTable, i * 255);
	for (i = 0; i < NUM_MAX_ANIMDATA; i++) {
		animDataTable[i].width = TO_BE_16(animDataTable[i].width);
		animDataTable[i].var1 = TO_BE_16(animDataTable[i].var1);
		animDataTable[i].bpp = TO_BE_16(animDataTable[i].bpp);
		animDataTable[i].height = TO_BE_16(animDataTable[i].height);
		animDataTable[i].fileIdx = TO_BE_16(animDataTable[i].fileIdx);
		animDataTable[i].frameIdx = TO_BE_16(animDataTable[i].frameIdx);
	}

	fHandle.seek(12, SEEK_CUR);	// TODO: handle screen params (realy required ?)

	size = fHandle.readSint16BE();
	for (i = 0; i < size; i++) {
		loadGlobalScritpFromSave(&fHandle);
	}

	size = fHandle.readSint16BE();
	for (i = 0; i < size; i++) {
		loadObjectScritpFromSave(&fHandle);
	}

	size = fHandle.readSint16BE();
	for (i = 0; i < size; i++) {
		loadOverlayFromSave(&fHandle);
	}

	size = fHandle.readSint16BE();
	for (i = 0; i < size; i++) {
		// loadBgIncrustFromSave(&fHandle);
	}

	fHandle.close();

	checkDataDisk(currentDisk);

	if (strlen(currentPartName)) {
		loadPart(currentPartName);
	}

	if (strlen(currentPrcName)) {
		loadPrc(currentPrcName);
		setupGlobalScriptList();
	}

	if (strlen(currentRelName)) {
		loadRel(currentRelName);
		setupObjectScriptList();
	}

	if (strlen(currentMsgName)) {
		loadMsg(currentMsgName);
	}

	if (strlen(currentBgName[0])) {
		loadBg(currentBgName[0]);
	}

	if (strlen(currentCtName)) {
		loadCt(currentCtName);
	}

	loadResourcesFromSave();
	//reincrustAllBg();

	setMouseCursor(MOUSE_CURSOR_NORMAL);

	if (strlen(currentDatName)) {
/*		i = saveVar2;
		saveVar2 = 0;
		loadMusic();
		if (i) {
			playMusic();
		}*/
	}

	return (0);
}

void makeSave(char *saveFileName) {
	int16 i;
	Common::File fHandle;

	fHandle.open(saveFileName, Common::File::kFileWriteMode, savePath);

	fHandle.writeUint16BE(currentDisk);
	fHandle.write(currentPartName, 13);
	fHandle.write(currentDatName, 13);
	fHandle.writeUint16BE(saveVar2);
	fHandle.write(currentPrcName, 13);
	fHandle.write(currentRelName, 13);
	fHandle.write(currentMsgName, 13);
	fHandle.write(currentBgName[0], 13);
	fHandle.write(currentCtName, 13);

	fHandle.writeUint16BE(0xFF);
	fHandle.writeUint16BE(0x20);

	for (i = 0; i < 255; i++) {
		fHandle.writeUint16BE(objectTable[i].x);
		fHandle.writeUint16BE(objectTable[i].y);
		fHandle.writeUint16BE(objectTable[i].mask);
		fHandle.writeUint16BE(objectTable[i].frame);
		fHandle.writeUint16BE(objectTable[i].costume);
		fHandle.write(objectTable[i].name, 20);
		fHandle.writeUint16BE(objectTable[i].part);
	}

	for (i = 0; i < 16; i++) {
		fHandle.writeUint16BE(palette_[i]);
	}

	for (i = 0; i < 16; i++) {
		fHandle.writeUint16BE(tempPalette[i]);
	}

	for (i = 0; i < 255; i++) {
		fHandle.writeUint16BE(globalVars[i]);
	}

	for (i = 0; i < 16; i++) {
		fHandle.writeUint16BE(zoneData[i]);
	}

	for (i = 0; i < 4; i++) {
		fHandle.writeUint16BE(commandVar3[i]);
	}

	fHandle.write(commandBuffer, 0x50);

	fHandle.writeUint16BE(defaultMenuBoxColor);
	fHandle.writeUint16BE(bgVar0);
	fHandle.writeUint16BE(allowPlayerInput);
	fHandle.writeUint16BE(playerCommand);
	fHandle.writeUint16BE(commandVar1);
	fHandle.writeUint16BE(isDrawCommandEnabled);
	fHandle.writeUint16BE(var5);
	fHandle.writeUint16BE(var4);
	fHandle.writeUint16BE(var3);
	fHandle.writeUint16BE(var2);
	fHandle.writeUint16BE(commandVar2);
	fHandle.writeUint16BE(defaultMenuBoxColor2);
	fHandle.writeUint16BE(0xFF);
	fHandle.writeUint16BE(0x1E);

	for (i = 0; i < NUM_MAX_ANIMDATA; i++) {
		fHandle.writeUint16BE(animDataTable[i].width);
		fHandle.writeUint16BE(animDataTable[i].var1);
		fHandle.writeUint16BE(animDataTable[i].bpp);
		fHandle.writeUint16BE(animDataTable[i].height);
		fHandle.writeSint16BE(animDataTable[i].fileIdx);
		fHandle.writeSint16BE(animDataTable[i].frameIdx);
		fHandle.write(animDataTable[i].name, 10);
	}

	fHandle.writeUint16BE(0);  // Screen params, unhandled
	fHandle.writeUint16BE(0);
	fHandle.writeUint16BE(0);
	fHandle.writeUint16BE(0);
	fHandle.writeUint16BE(0);
	fHandle.writeUint16BE(0);

	{
		int16 numScript = 0;
		prcLinkedListStruct *currentHead = globalScriptsHead.next;

		while (currentHead) {
			numScript++;
			currentHead = currentHead->next;
		}

		fHandle.writeUint16BE(numScript);

		// actual save
		currentHead = globalScriptsHead.next;

		while (currentHead) {
			for (i = 0; i < SCRIPT_STACK_SIZE; i++) {
				fHandle.writeUint16BE(currentHead->stack[i]);
			}

			for (i = 0; i < 50; i++) {
				fHandle.writeUint16BE(currentHead->localVars[i]);
			}

			fHandle.writeUint16BE(currentHead->compareResult);
			fHandle.writeUint16BE(currentHead->scriptPosition);

			fHandle.writeUint16BE(currentHead->scriptIdx);

			currentHead = currentHead->next;
		}
	}

	{
		int16 numScript = 0;
		prcLinkedListStruct *currentHead = objScriptList.next;

		while (currentHead) {
			numScript++;
			currentHead = currentHead->next;
		}

		fHandle.writeUint16BE(numScript);

		// actual save
		currentHead = objScriptList.next;

		while (currentHead) {
			for (i = 0; i < SCRIPT_STACK_SIZE; i++) {
				fHandle.writeUint16BE(currentHead->stack[i]);
			}

			for (i = 0; i < 50; i++) {
				fHandle.writeUint16BE(currentHead->localVars[i]);
			}

			fHandle.writeUint16BE(currentHead->compareResult);
			fHandle.writeUint16BE(currentHead->scriptPosition);

			fHandle.writeUint16BE(currentHead->scriptIdx);

			currentHead = currentHead->next;
		}
	}

	{
		int16 numScript = 0;
		overlayHeadElement *currentHead = overlayHead.next;

		while (currentHead) {
			numScript++;
			currentHead = currentHead->next;
		}

		fHandle.writeUint16BE(numScript);

		// actual save
		currentHead = overlayHead.next;

		assert(sizeof(overlayHeadElement) == 0x14);
		while (currentHead) {
			fHandle.writeUint32BE(0);
			fHandle.writeUint32BE(0);
			fHandle.writeUint16BE(currentHead->objIdx);
			fHandle.writeUint16BE(currentHead->type);
			fHandle.writeSint16BE(currentHead->x);
			fHandle.writeSint16BE(currentHead->y);
			fHandle.writeSint16BE(currentHead->width);
			fHandle.writeSint16BE(currentHead->color);

			currentHead = currentHead->next;
		}
	}

	fHandle.writeUint16BE(0);

	fHandle.close();

	setMouseCursor(MOUSE_CURSOR_NORMAL);
}

void makeSystemMenu(void) {
	int16 numEntry;
	int16 mouseButton;
	int16 mouseX;
	int16 mouseY;
	int16 systemCommand;

	if (!allowSystemMenu) {
		manageEvents();
		getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton,(uint16 *)&mouseX, (uint16 *)&mouseY);

		while (mouseButton) {
			manageEvents();
			getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
		}

		numEntry = 6;

		if (!allowPlayerInput) {
			numEntry--;
		}

		systemCommand = makeMenuChoice(systemMenu, numEntry, mouseX, mouseY, 140);

		switch (systemCommand) {
		case 0:
			{
				drawString("PAUSE", 0);
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
					exitEngine = 1;
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
					int16 selectedSave;

					getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
					selectedSave = makeMenuChoice(currentSaveName, 10, mouseX, mouseY + 8, 180);

					if (selectedSave >= 0) {
						char saveNameBuffer[256];
						if (gameType == Cine::GID_FW)
							sprintf(saveNameBuffer, "FW.%1d", selectedSave);
						else
							sprintf(saveNameBuffer, "OS.%1d", selectedSave);

						getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
						if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
							char loadString[256];

							sprintf(loadString, "Chargement de | %s", currentSaveName[selectedSave]);
							drawString(loadString, 0);

							makeLoad(saveNameBuffer);
						} else {
							drawString("Chargement Annulé ...", 0);
							waitPlayerInput();
							checkDataDisk(-1);
						}
					} else {
						drawString("Chargement Annulé ...", 0);
						waitPlayerInput();
						checkDataDisk(-1);
					}
				} else {
					drawString("Aucune sauvegarde dans le lecteur ...", 0);
					waitPlayerInput();
					checkDataDisk(-1);
				}
				break;
			}
		case 5:
			{
				int16 selectedSave;

				loadSaveDirectory();
				selectedSave = makeMenuChoice(currentSaveName, 10, mouseX, mouseY + 8, 180);

				if (selectedSave >= 0) {
					char saveFileName[256];
					//makeTextEntryMenu("Veuillez entrer le Nom de la Sauvegarde .", &currentSaveName[selectedSave], 120);
					sprintf(currentSaveName[selectedSave], "temporary save name");

					if (gameType == Cine::GID_FW)
						sprintf(saveFileName, "FW.%1d", selectedSave);
					else
						sprintf(saveFileName, "OS.%1d", selectedSave);

					getMouseData(mouseUpdateStatus, (uint16 *)&mouseButton, (uint16 *)&mouseX, (uint16 *)&mouseY);
					if (!makeMenuChoice(confirmMenu, 2, mouseX, mouseY + 8, 100)) {
						char saveString[256];

						Common::File fHandle;

						if (gameType == Cine::GID_FW)
							fHandle.open("FW.DIR", Common::File::kFileWriteMode, savePath);
						else
							fHandle.open("OS.DIR", Common::File::kFileWriteMode, savePath);

						fHandle.write(currentSaveName, 200);
						fHandle.close();

						sprintf(saveString, "Sauvegarde de |%s", currentSaveName[selectedSave]);
						drawString(saveString, 0);

						makeSave(saveFileName);

						checkDataDisk(-1);
					} else {
						drawString("Sauvegarde Annulée ...", 0);
						waitPlayerInput();
						checkDataDisk(-1);
					}
				}
				break;
			}
		}
	}
}

const int16 choiceResultTable[] = {
	1,
	1,
	1,
	2,
	1,
	1,
	1
};

const int16 subObjectUseTable[] = {
	3,
	3,
	3,
	3,
	3,
	0,
	0
};

const int16 canUseOnItemTable[] = {
	1,
	0,
	0,
	1,
	1,
	0,
	0
};

commandeType objectListCommand[20];
int16 objListTab[20];

int16 processInventory(int16 x, int16 y) {
	return 0;
}

int16 buildObjectListCommand(void) {
	int16 i;
	int16 j;

	assert(gameType == Cine::GID_FW);

	for (i = 0; i < 20; i++) {
		objectListCommand[i][0] = 0;
	}

	j = 0;

	for (i = 0; i < 255; i++) {
		if (objectTable[i].name[0] && objectTable[i].costume == -2) {
			strcpy(objectListCommand[j], objectTable[i].name);
			objListTab[j] = i;
			j++;
		}
	}

	return (j);
}

int16 buildObjectListCommand2(int16 param) {
	int16 i;
	int16 j;

	assert(gameType == Cine::GID_OS);

	for (i = 0; i < 20; i++) {
		objectListCommand[i][0] = 0;
	}

	j = 0;

	for (i = 0; i < 255; i++) {
		if (objectTable[i].name[0] && objectTable[i].costume == param) {
			strcpy(objectListCommand[j], objectTable[i].name);
			objListTab[j] = i;
			j++;
		}
	}

	return (j);
}

int16 selectSubObject(int16 x, int16 y) {
	int16 listSize = buildObjectListCommand();
	int16 selectedObject;

	if (!listSize) {
		return -2;
	}

	selectedObject = makeMenuChoice(objectListCommand, listSize, x, y, 140);

	if (selectedObject == -1)
		return -1;

	return objListTab[selectedObject];
}

int16 selectSubObject2(int16 x, int16 y, int16 param) {
	int16 listSize = buildObjectListCommand2(param);
	int16 selectedObject;

	if (!listSize) {
		return -2;
	}

	selectedObject = makeMenuChoice2(objectListCommand, listSize, x, y, 140);

	if (selectedObject == -1)
		return -1;

	if (selectedObject >= 8000) {
		return objListTab[selectedObject - 8000] + 8000;
	}

	return objListTab[selectedObject];
}

int16 canUseOnObject = 0;

void makeCommandLine(void) {
	uint16 x;
	uint16 y;

	commandVar1 = 0;
	commandVar2 = -10;

	if (playerCommand != -1) {
		strcpy(commandBuffer, defaultActionCommand[playerCommand]);
	} else {
		strcpy(commandBuffer, "");
	}

	if ((playerCommand != -1) && (choiceResultTable[playerCommand] == 2)) {	// need object selection ?
		int16 si;

		getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);

		if (gameType == Cine::GID_FW) {
			si = selectSubObject(x, y + 8);
		} else {
			si = selectSubObject2(x, y + 8, -subObjectUseTable[playerCommand]);
		}

		if (si < 0) {
			playerCommand = -1;
			strcpy(commandBuffer, "");
		} else {
			if (gameType == Cine::GID_OS) {
				if (si >= 8000) {
					si -= 8000;
					canUseOnObject = canUseOnItemTable[playerCommand];
				} else {
					canUseOnObject = 0;
				}
			}

			commandVar3[0] = si;
			commandVar1 = 1;

			strcat(commandBuffer, " ");
			strcat(commandBuffer, objectTable[commandVar3[0]].name);
			strcat(commandBuffer, " sur");
		}
	} else {
		if (playerCommand == 2) {
			getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);
			//processInventory(x, y + 8);
			playerCommand = -1;
			commandVar1 = 0;
			strcpy(commandBuffer, "");
		}
	}

	if (gameType == Cine::GID_OS) {
		if (playerCommand != -1 && canUseOnObject != 0)	{ // call use on sub object
			int16 si;

			getMouseData(mouseUpdateStatus, &dummyU16, &x, &y);

			si = selectSubObject2(x, y + 8, -subObjectUseTable[playerCommand]);

			if (si) {
				if (si >= 8000) {
					si -= 8000;
				}

				commandVar3[commandVar1] = si;

				commandVar1++;

				// TODO: add command message draw
			}

			isDrawCommandEnabled = 1;

			if (playerCommand != -1 && choiceResultTable[playerCommand] == commandVar1) {
				int16 di = getRelEntryForObject(playerCommand, commandVar1, (selectedObjStruct *) & commandVar3);

				if (di != -1) {
					runObjectScript(di);
				}
			}
		}
	}

	if (allowSystemMenu == 0) {
		isDrawCommandEnabled = 1;
	}
}

uint16 needMouseSave = 0;

uint16 menuVar4 = 0;
uint16 menuVar5 = 0;

int16 makeMenuChoice(const commandeType commandList[], uint16 height, uint16 X, uint16 Y,
    uint16 width) {
	byte color = 2;
	byte color2;
	int16 paramY;
	int16 currentX;
	int16 currentY;
	int16 i;
	uint16 button;
	int16 var_A;
	int16 di;
	uint16 j;
	int16 mouseX;
	int16 mouseY;
	int16 var_16;
	int16 var_14;
	int16 currentSelection;
	int16 oldSelection;
	int16 var_4;

	if (allowSystemMenu)
		return -1;

	paramY = (height * 9) + 10;

	if (X + width > 319) {
		X = 319 - width;
	}

	if (Y + paramY > 199) {
		Y = 199 - paramY;
	}

	color2 = defaultMenuBoxColor2;

	hideMouse();
	blitRawScreen(page1Raw);

	gfxDrawPlainBoxRaw(X, Y, X + width, Y + 4, color2, page1Raw);

	currentX = X + 4;
	currentY = Y + 4;

	for (i = 0; i < height; i++) {
		gfxDrawPlainBoxRaw(X, currentY, X + width, currentY + 9, color2, page1Raw);
		currentX = X + 4;

		for (j = 0; j < strlen(commandList[i]); j++) {
			byte currentChar = commandList[i][j];

			if (currentChar == ' ') {
				currentX += 5;
			} else {
				byte characterWidth = fontParamTable[currentChar].characterWidth;

				if (characterWidth) {
					byte characterIdx = fontParamTable[currentChar].characterIdx;
					drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, currentY);
					currentX += characterWidth + 1;
				}
			}
		}

		currentY += 9;
	}

	gfxDrawPlainBoxRaw(X, currentY, X + width, currentY + 4, color2, page1Raw);	// bottom part

	gfxDrawLine(X + 1, Y + 1, X + width - 1, Y + 1, 0, page1Raw);	// top
	gfxDrawLine(X + 1, currentY + 3, X + width - 1, currentY + 3, 0, page1Raw);	// bottom
	gfxDrawLine(X + 1, Y + 1, X + 1, currentY + 3, 0, page1Raw);	// left
	gfxDrawLine(X + width - 1, Y + 1, X + width - 1, currentY + 3, 0, page1Raw);	// left

	gfxDrawLine(X, Y, X + width, Y, color, page1Raw);
	gfxDrawLine(X, currentY + 4, X + width, currentY + 4, color, page1Raw);
	gfxDrawLine(X, Y, X, currentY + 4, color, page1Raw);
	gfxDrawLine(X + width, Y, X + width, currentY + 4, color, page1Raw);

	blitRawScreen(page1Raw);

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	var_A = 0;

	currentSelection = 0;

	di = currentSelection * 9 + Y + 4;
	gfxDrawPlainBoxRaw(X + 2, di - 1, X + width - 2, di + 7, 0, page1Raw);	// draw black box behind selection
	currentX = X + 4;

	for (j = 0; j < strlen(commandList[currentSelection]); j++) {
		byte currentChar = commandList[currentSelection][j];

		if (currentChar == ' ') {
			currentX += 5;
		} else {
			byte characterWidth = fontParamTable[currentChar].characterWidth;

			if (characterWidth) {
				byte characterIdx = fontParamTable[currentChar].characterIdx;
				drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, di);
				currentX += characterWidth + 1;
			}
		}
	}

	blitRawScreen(page1Raw);

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

			di = oldSelection * 9 + Y + 4;

			gfxDrawPlainBoxRaw(X + 2, di - 1, X + width - 2, di + 7, color2, page1Raw);	// restore color

			currentX = X + 4;

			for (j = 0; j < strlen(commandList[oldSelection]); j++) {
				byte currentChar = commandList[oldSelection][j];

				if (currentChar == ' ') {
					currentX += 5;
				} else {
					byte characterWidth = fontParamTable[currentChar].characterWidth;

					if (characterWidth) {
						byte characterIdx = fontParamTable[currentChar].characterIdx;
						drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, di);
						currentX += characterWidth + 1;
					}
				}
			}

			di = currentSelection * 9 + Y + 4;

			gfxDrawPlainBoxRaw(X + 2, di - 1, X + width - 2, di + 7, 0, page1Raw);	// black new

			currentX = X + 4;

			for (j = 0; j < strlen(commandList[currentSelection]);
			    j++) {
				byte currentChar = commandList[currentSelection][j];

				if (currentChar == ' ') {
					currentX += 5;
				} else {
					byte characterWidth = fontParamTable[currentChar].characterWidth;

					if (characterWidth) {
						byte characterIdx = fontParamTable[currentChar].characterIdx;
						drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, di);
						currentX += characterWidth + 1;
					}
				}
			}

			blitRawScreen(page1Raw);

			if (needMouseSave) {
				gfxFuncGen2();
			}
		}

	} while (!var_A);

	assert(!needMouseSave);

	var_4 = button;

	menuVar = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	if (var_4 == 2)	{	// recheck
		return -1;
	}

	return currentSelection;
}

int16 makeMenuChoice2(const commandeType commandList[], uint16 height, uint16 X, uint16 Y,
    uint16 width) {
	byte color = 2;
	byte color2;
	int16 paramY;
	int16 currentX;
	int16 currentY;
	int16 i;
	uint16 button;
	int16 var_A;
	int16 di;
	uint16 j;
	int16 mouseX;
	int16 mouseY;
	int16 var_16;
	int16 var_14;
	int16 currentSelection;
	int16 oldSelection;
	int16 var_4;

	if (allowSystemMenu)
		return -1;

	paramY = (height * 9) + 10;

	if (X + width > 319) {
		X = 319 - width;
	}

	if (Y + paramY > 199) {
		Y = 199 - paramY;
	}

	color2 = defaultMenuBoxColor2;

	hideMouse();
	blitRawScreen(page1Raw);

	gfxDrawPlainBoxRaw(X, Y, X + width, Y + 4, color2, page1Raw);

	currentX = X + 4;
	currentY = Y + 4;

	for (i = 0; i < height; i++) {
		gfxDrawPlainBoxRaw(X, currentY, X + width, currentY + 9, color2, page1Raw);
		currentX = X + 4;

		for (j = 0; j < strlen(commandList[i]); j++) {
			byte currentChar = commandList[i][j];

			if (currentChar == ' ') {
				currentX += 5;
			} else {
				byte characterWidth = fontParamTable[currentChar].characterWidth;

				if (characterWidth) {
					byte characterIdx = fontParamTable[currentChar].characterIdx;
					drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, currentY);
					currentX += characterWidth + 1;
				}
			}
		}

		currentY += 9;
	}

	gfxDrawPlainBoxRaw(X, currentY, X + width, currentY + 4, color2, page1Raw);	// bottom part

	gfxDrawLine(X + 1, Y + 1, X + width - 1, Y + 1, 0, page1Raw);	// top
	gfxDrawLine(X + 1, currentY + 3, X + width - 1, currentY + 3, 0, page1Raw);	// bottom
	gfxDrawLine(X + 1, Y + 1, X + 1, currentY + 3, 0, page1Raw);	// left
	gfxDrawLine(X + width - 1, Y + 1, X + width - 1, currentY + 3, 0, page1Raw);	// left

	gfxDrawLine(X, Y, X + width, Y, color, page1Raw);
	gfxDrawLine(X, currentY + 4, X + width, currentY + 4, color, page1Raw);
	gfxDrawLine(X, Y, X, currentY + 4, color, page1Raw);
	gfxDrawLine(X + width, Y, X + width, currentY + 4, color, page1Raw);

	blitRawScreen(page1Raw);

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	var_A = 0;

	currentSelection = 0;

	di = currentSelection * 9 + Y + 4;
	gfxDrawPlainBoxRaw(X + 2, di - 1, X + width - 2, di + 7, 0, page1Raw);	// draw black box behind selection
	currentX = X + 4;

	for (j = 0; j < strlen(commandList[currentSelection]); j++) {
		byte currentChar = commandList[currentSelection][j];

		if (currentChar == ' ') {
			currentX += 5;
		} else {
			byte characterWidth = fontParamTable[currentChar].characterWidth;

			if (characterWidth) {
				byte characterIdx = fontParamTable[currentChar].characterIdx;
				drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, di);
				currentX += characterWidth + 1;
			}
		}
	}

	blitRawScreen(page1Raw);

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

			di = oldSelection * 9 + Y + 4;

			gfxDrawPlainBoxRaw(X + 2, di - 1, X + width - 2, di + 7, color2, page1Raw);	// restore color

			currentX = X + 4;

			for (j = 0; j < strlen(commandList[oldSelection]); j++) {
				byte currentChar = commandList[oldSelection][j];

				if (currentChar == ' ') {
					currentX += 5;
				} else {
					byte characterWidth = fontParamTable[currentChar].characterWidth;

					if (characterWidth) {
						byte characterIdx = fontParamTable[currentChar].characterIdx;
						drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, di);
						currentX += characterWidth + 1;
					}
				}
			}

			di = currentSelection * 9 + Y + 4;

			gfxDrawPlainBoxRaw(X + 2, di - 1, X + width - 2, di + 7, 0, page1Raw);	// black new

			currentX = X + 4;

			for (j = 0; j < strlen(commandList[currentSelection]);
			    j++) {
				byte currentChar = commandList[currentSelection][j];

				if (currentChar == ' ') {
					currentX += 5;
				} else {
					byte characterWidth = fontParamTable[currentChar].characterWidth;

					if (characterWidth) {
						byte characterIdx = fontParamTable[currentChar].characterIdx;
						drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, currentX, di);
						currentX += characterWidth + 1;
					}
				}
			}

			blitRawScreen(page1Raw);

			if (needMouseSave) {
				gfxFuncGen2();
			}
		}

	} while (!var_A);

	assert(!needMouseSave);

	var_4 = button;

	menuVar = 0;

	do {
		manageEvents();
		getMouseData(mouseUpdateStatus, &button, &dummyU16, &dummyU16);
	} while (button);

	if (var_4 == 2)	{	// recheck
		return currentSelection + 8000;
	}

	return currentSelection;
}

void drawMenuBox(char *command, int16 x, int16 y) {
	byte j;
	byte lColor = 2;

	hideMouse();

	gfxDrawPlainBoxRaw(x, y, x + 300, y + 10, 0, page2Raw);

	gfxDrawLine(x - 1, y - 1, x + 301, y - 1, lColor, page2Raw);	// top
	gfxDrawLine(x - 1, y + 11, x + 301, y + 11, lColor, page2Raw);	// bottom
	gfxDrawLine(x - 1, y - 1, x - 1, y + 11, lColor, page2Raw);	// left
	gfxDrawLine(x + 301, y - 1, x + 301, y + 11, lColor, page2Raw);	// right

	x += 2;
	y += 2;

	for (j = 0; j < strlen(command); j++) {
		byte currentChar = command[j];

		if (currentChar == ' ') {
			x += 5;
		} else {
			byte characterWidth = fontParamTable[currentChar].characterWidth;

			if (characterWidth) {
				byte characterIdx = fontParamTable[currentChar].characterIdx;
				drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page2Raw, x, y);
				x += characterWidth + 1;
			}
		}
	}

	gfxFuncGen2();
}

uint16 executePlayerInput(void) {
	uint16 var_5E;
	uint16 var_2;
	uint16 mouseButton;
	uint16 mouseX;
	uint16 mouseY;

	canUseOnObject = 0;

	if (isInPause) {
		drawString("PAUSE", 0);
		waitPlayerInput();
		isInPause = 0;
	}

	if (allowPlayerInput) {
		uint16 currentEntry = 0;
		uint16 di = 0;

		if (isDrawCommandEnabled) {
			drawMenuBox(commandBuffer, 10, defaultMenuBoxColor);
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
					makeSystemMenu();
				} else {
					int16 si;
					do {
						manageEvents();
						getMouseData(mouseUpdateStatus, &mouseButton, &dummyU16, &dummyU16);
					} while (mouseButton);

					si = getObjectUnderCursor(mouseX,
					    mouseY);

					if (si != -1) {
						commandVar3[commandVar1] = si;
						commandVar1++;

						strcat(commandBuffer, " ");
						strcat(commandBuffer, objectTable[si].name);

						isDrawCommandEnabled = 1;

						if (choiceResultTable[playerCommand] == commandVar1) {
							int16 relEntry;

							drawMenuBox(commandBuffer, 10, defaultMenuBoxColor);

							relEntry = getRelEntryForObject(playerCommand, commandVar1, (selectedObjStruct *)commandVar3);

							if (relEntry != -1) {
								runObjectScript(relEntry);
							} else {
								//addPlayerCommandMessage(playerCommand);
							}

							playerCommand = -1;

							commandVar1 = 0;
							strcpy(commandBuffer, "");
						}
					} else {
						globalVars[VAR_MOUSE_X_POS] = mouseX;
						globalVars[VAR_MOUSE_Y_POS] = mouseY;
					}
				}
			} else {
				if (mouseButton & 2) {
					if (mouseButton & 1) {
						makeSystemMenu();
					}

					if (gameType == Cine::GID_OS) {
						playerCommand = makeMenuChoice2(defaultActionCommand, 6, mouseX, mouseY, 70);

						if (playerCommand >= 8000) {
							playerCommand -= 8000;
							canUseOnObject = 1;
						}
					} else {
						playerCommand =  makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70);
					}

					makeCommandLine();
				} else {
					int16 objIdx;

					objIdx = getObjectUnderCursor(mouseX, mouseY);

					if (commandVar2 != objIdx) {
						if (objIdx != -1) {
							char command[256];

							strcpy(command, commandBuffer);
							strcat(command, " ");
							strcat(command, objectTable[objIdx].name);

							drawMenuBox(command, 10, defaultMenuBoxColor);
						} else {
							isDrawCommandEnabled = 1;
						}
					}

					commandVar2 = objIdx;
				}
			}
		} else {
			if (mouseButton & 2) {
				if (!(mouseButton & 1)) {
					if (gameType == Cine::GID_OS) {
						playerCommand = makeMenuChoice2(defaultActionCommand, 6, mouseX, mouseY, 70);

						if (playerCommand >= 8000) {
							playerCommand -= 8000;
							canUseOnObject = 1;
						}
					} else {
						playerCommand = makeMenuChoice(defaultActionCommand, 6, mouseX, mouseY, 70);
					}

					makeCommandLine();
				} else {
					makeSystemMenu();
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
						makeSystemMenu();
					}
				}
			}
		}
	} else {
		uint16 di = 0;
		getMouseData(mouseUpdateStatus, &mouseButton, &mouseX, &mouseY);

		while (mouseButton) {
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
			makeSystemMenu();
		}
	}

	var_2 = menuVar & 0x7F;
	var_5E = var_2;

	if (menuVar & 0x80) {
		var_5E = 0;
		var_2 = 0;
	}

	if (inputVar1 && allowPlayerInput) {	// use keyboard
		inputVar1 = 0;

		switch (globalVars[VAR_MOUSE_X_MODE]) {
		case 1:
			{
				mouseX = objectTable[1].x + 12;
				break;
			}
		case 2:
			{
				mouseX = objectTable[1].x + 7;
				break;
			}
		default:
			{
				mouseX = globalVars[VAR_MOUSE_X_POS];
				break;
			}
		}

		switch (globalVars[VAR_MOUSE_Y_MODE]) {
		case 1:
			{
				mouseY = objectTable[1].y + 34;
				break;
			}
		case 2:
			{
				mouseY = objectTable[1].y + 28;
				break;
			}
		default:
			{
				mouseY = globalVars[VAR_MOUSE_Y_POS];
				break;
			}
		}

		if (var_5E == bgVar0) {
			var_5E = 0;

			globalVars[VAR_MOUSE_X_POS] = mouseX;
			globalVars[VAR_MOUSE_Y_POS] = mouseY;
		} else {
			if (inputVar2) {
				if (inputVar2 == 2) {
					globalVars[VAR_MOUSE_X_POS] = 1;
				} else {
					globalVars[VAR_MOUSE_X_POS] = 320;
				}
			} else {
				globalVars[VAR_MOUSE_X_POS] = mouseX;
			}

			if (inputVar3) {
				if (inputVar3 == 2) {
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
			{
				if (allowPlayerInput) {
					playerCommand = 0;
					makeCommandLine();
				}
				break;
			}
		case 1:
			{
				if (allowPlayerInput) {
					playerCommand = 1;
					makeCommandLine();
				}
				break;
			}
		case 2:
			{
				if (allowPlayerInput) {
					playerCommand = 2;
					makeCommandLine();
				}
				break;
			}
		case 3:
			{
				if (allowPlayerInput) {
					playerCommand = 3;
					makeCommandLine();
				}
				break;
			}
		case 4:
			{
				if (allowPlayerInput) {
					playerCommand = 4;
					makeCommandLine();
				}
				break;
			}
		case 5:
			{
				if (allowPlayerInput) {
					playerCommand = 5;
					makeCommandLine();
				}
				break;
			}
		case 6:
		case 7:
		case 8:
		case 23:
			{
				break;
			}
		case 9:
		case 24:
			{
				makeSystemMenu();
				break;
			}
		default:
			{
				//  printf("Unhandled case %d in last part of executePLayerInput\n",var2-59);
				break;
			}
		}
	}

	return var_5E;
}

void drawSprite(overlayHeadElement *currentOverlay, byte *spritePtr,
				byte *maskPtr, uint16 width, uint16 height, byte *page, int16 x, int16 y) {
#if 0
	byte *ptr = NULL;
	byte i = 0;
	uint16 si = 0;
	overlayHeadElement *pCurrentOverlay = currentOverlay;

	while(pCurrentOverlay) { // unfinished, probably for mask handling..
		if (pCurrentOverlay->type == 5) {
			int16 maskX;
			int16 maskY;
			int16 maskWidth;
			int16 maskHeight;
			uint16 maskSpriteIdx;
	 
			if (!si) {
				ptr = (byte *)malloc(width * height);
				si = 1;
			}
	 
			maskX = objectTable[pCurrentOverlay->objIdx].x;
			maskY = objectTable[pCurrentOverlay->objIdx].y;
	 
			maskSpriteIdx = objectTable[pCurrentOverlay->objIdx].frame;
	 
			maskWidth = animDataTable[maskSpriteIdx].width / 2;
			maskHeight = animDataTable[maskSpriteIdx].height;
	 
			gfxSpriteFunc2(spritePtr, width, height, animDataTable[maskSpriteIdx].ptr1, maskWidth, maskHeight, ptr, maskX - x,maskY - y, i++);
		}
	 
		pCurrentOverlay = pCurrentOverlay->next;
	} 
	 
	if(si) {
		gfxSpriteFunc1(ptr, width, height, page, x, y);
		free(ptr);
	} else
#endif

	if (gameType == Cine::GID_OS) {
		drawSpriteRaw2(spritePtr, objectTable[currentOverlay->objIdx].part, width, height, page, x, y);
	} else {
		drawSpriteRaw(spritePtr, maskPtr, width, height, page, x, y);
	}

}

int16 additionalBgVScroll = 0;

void backupOverlayPage(void) {
	byte *bgPage;
	byte *scrollBg;

	bgPage = additionalBgTable[currentAdditionalBgIdx];

	if (bgPage) {
		if (!additionalBgVScroll) {
			memcpy(page1Raw, bgPage, 320 * 200);
		} else {
			int16 i;

			scrollBg = additionalBgTable[currentAdditionalBgIdx2];

			for (i = additionalBgVScroll;
			    i < 200 + additionalBgVScroll; i++) {
				if (i > 200) {
					memcpy(page1Raw + (i - additionalBgVScroll) * 320, scrollBg + (i - 200) * 320, 320);
				} else {
					memcpy(page1Raw + (i - additionalBgVScroll) * 320, bgPage + (i) * 320, 320);
				}
			}
		}
	}
}

uint16 computeMessageLength(byte *ptr, uint16 width, uint16 *numWords, uint16 *messageWidth, uint16 *lineResult) {
	byte *localPtr = ptr;

	uint16 var_2 = 0;
	uint16 localLineResult = 0;
	uint16 var_6 = 0;
	uint16 var_8 = 0;
	uint16 localMessageWidth = 0;
	uint16 var_16 = 0;
	uint16 finished = 0;
	uint16 si = 0;
	uint16 di = 0;

	while (!finished) {
		byte character = *(localPtr++);

		if (character == ' ') {
			var_8 = var_16;
			var_6 = localMessageWidth;
			localLineResult = si;
			var_2 = di;

			if (si + 5 < width) {
				var_16++;
				si += 5;
			} else {
				finished = 1;
			}
		} else if (character == 0x7C || character == 0) {
			finished = 1;
			si = 0;
		} else {
			if (fontParamTable[character].characterWidth) {
				uint16 var_C = fontParamTable[character].characterWidth + 1;

				if (si + var_C < width) {
					si += var_C;
					localMessageWidth += var_C;
				} else {
					finished = 1;

					if (localLineResult) {
						var_16 = var_8;
						localMessageWidth = var_6;
						si = localLineResult;
						di = var_2;
					}
				}
			}
		}

		di++;
	}

	*numWords = var_16;
	*messageWidth = localMessageWidth;
	*lineResult = si;

	return (di);
}

void drawDialogueMessage(byte msgIdx, int16 x, int16 y, int16 width, int16 color) {
	byte color2 = 2;
	byte endOfMessageReached = 0;
	int16 localX;
	int16 localY;
	int16 localWidth;

	char *messagePtr = (char *)messageTable[msgIdx].ptr;

	if (!messagePtr) {
		freeOverlay(msgIdx, 2);
	}

	var20 += strlen(messagePtr);

	gfxDrawPlainBoxRaw(x, y, x + width, y + 4, color, page1Raw);

	localX = x + 4;
	localY = y + 4;
	localWidth = width - 8;

	do {
		uint16 messageLength = 0;
		uint16 numWords;
		uint16 messageWidth;
		uint16 lineResult;
		char *endOfMessagePtr;
		uint16 fullLineWidth;
		uint16 interWordSize;
		uint16 interWordSizeRemain;
		byte currentChar;
		byte characterWidth;

		while (messagePtr[messageLength] == ' ') {
			messageLength++;
		}

		messagePtr += messageLength;

		messageLength = computeMessageLength((byte *) messagePtr, localWidth, &numWords, &messageWidth, &lineResult);

		endOfMessagePtr = messagePtr + messageLength;

		if (lineResult) {
			fullLineWidth = localWidth - messageWidth;

			if (numWords) {
				interWordSize = fullLineWidth / numWords;
				interWordSizeRemain = fullLineWidth % numWords;
			} else {
				interWordSize = 5;
				interWordSizeRemain = 0;
			}
		} else {
			interWordSize = 5;
			interWordSizeRemain = 0;
		}

		gfxDrawPlainBoxRaw(x, localY, x + width, localY + 9, color, page1Raw);

		do {
			currentChar = *(messagePtr++);

			if (currentChar == 0) {
				endOfMessageReached = 1;
			} else if (currentChar == ' ') {
				localX += interWordSizeRemain + interWordSize;

				if (interWordSizeRemain)
					interWordSizeRemain = 0;
			} else {
				characterWidth = fontParamTable[currentChar].characterWidth;

				if (characterWidth) {
					byte characterIdx = fontParamTable[currentChar].characterIdx;
					drawSpriteRaw(textTable[characterIdx][0], textTable[characterIdx][1], 2, 8, page1Raw, localX, localY);
					localX += characterWidth + 1;
				}
			}
		} while ((messagePtr < endOfMessagePtr) && !endOfMessageReached);

		localX = x + 4;
		localY += 9;
	} while (!endOfMessageReached);

	gfxDrawPlainBoxRaw(x, localY, x + width, localY + 4, color, page1Raw);

	gfxDrawLine(x + 1, y + 1, x + width - 1, y + 1, 0, page1Raw);	// top
	gfxDrawLine(x + 1, localY + 3, x + width - 1, localY + 3, 0, page1Raw);	// bottom
	gfxDrawLine(x + 1, y + 1, x + 1, localY + 3, 0, page1Raw);	// left
	gfxDrawLine(x + width - 1, y + 1, x + width - 1, localY + 3, 0, page1Raw);	// right

	gfxDrawLine(x, y, x + width, y, color2, page1Raw);
	gfxDrawLine(x, localY + 4, x + width, localY + 4, color2, page1Raw);
	gfxDrawLine(x, y, x, localY + 4, color2, page1Raw);
	gfxDrawLine(x + width, y, x + width, localY + 4, color2, page1Raw);

	freeOverlay(msgIdx, 2);
}

void drawOverlays(void) {
	overlayHeadElement *currentOverlay;

	backupOverlayPage();

	var20 = 0;

	currentOverlay = &overlayHead;

	currentOverlay = currentOverlay->next;

	while (currentOverlay) {
		switch (currentOverlay->type) {
		case 0:	// sprite
			{
				objectStruct *objPtr;
				int16 x;
				int16 y;

				assert(currentOverlay->objIdx <= NUM_MAX_OBJECT);

				objPtr = &objectTable[currentOverlay->objIdx];

				x = objPtr->x;
				y = objPtr->y;

				if (objPtr->frame >= 0) {
					if (gameType == Cine::GID_OS) {
						uint16 partVar1;
						uint16 partVar2;
						AnimData *pPart;
						pPart = &animDataTable[objPtr->frame];

						partVar1 = pPart->var1;
						partVar2 = pPart->height;

						if (pPart->ptr1) {
							drawSprite(currentOverlay, pPart->ptr1, pPart->ptr1, partVar1, partVar2, page1Raw, x, y);
						}
					} else {
						uint16 partVar1;
						uint16 partVar2;
						AnimData *pPart;
						int16 part = objPtr->part;

						assert(part >= 0 && part <= NUM_MAX_ANIMDATA);

						pPart = &animDataTable[objPtr->frame];

						partVar1 = pPart->var1;
						partVar2 = pPart->height;

						if (pPart->ptr1) {
							drawSprite(currentOverlay, pPart->ptr1, pPart->ptr2, partVar1, partVar2, page1Raw, x, y);
						}
					}
				}
				break;
			}
		case 2:	// text
			{
				byte messageIdx;
				int16 x;
				int16 y;
				uint16 partVar1;
				uint16 partVar2;

				// gfxWaitVSync();
				// hideMouse();

				messageIdx = currentOverlay->objIdx;
				x = currentOverlay->x;
				y = currentOverlay->y;
				partVar1 = currentOverlay->width;
				partVar2 = currentOverlay->color;

				blitRawScreen(page1Raw);

				drawDialogueMessage(messageIdx, x, y, partVar1, partVar2);

				//blitScreen(page0, NULL);

				gfxFuncGen2();

				waitForPlayerClick = 1;

				break;
			}
		case 3:
			{
				break;
			}
		case 4:
			{
				objectStruct *objPtr;
				int16 x;
				int16 y;

				assert(currentOverlay->objIdx <= NUM_MAX_OBJECT);

				objPtr = &objectTable[currentOverlay->objIdx];

				x = objPtr->x;
				y = objPtr->y;

				if (objPtr->frame >= 0) {
					uint16 partVar1;
					uint16 partVar2;
					AnimData *pPart;
					int16 part = objPtr->part;

					assert(part >= 0 && part <= NUM_MAX_ANIMDATA);

					pPart = &animDataTable[objPtr->frame];

					partVar1 = pPart->width / 2;
					partVar2 = pPart->height;

					if (pPart->ptr1) {
						gfxFillSprite(pPart->ptr1, partVar1, partVar2, page1Raw, x, y);
					}
				}
				break;
			}
		case 20:
			{
				objectStruct *objPtr;
				int16 x;
				int16 y;

				var5 = currentOverlay->x;

				assert(currentOverlay->objIdx <= NUM_MAX_OBJECT);

				objPtr = &objectTable[currentOverlay->objIdx];

				x = objPtr->x;
				y = objPtr->y;

				if (objPtr->frame >= 0) {
					if (var5 <= 8) {
						if (additionalBgTable[var5]) {
							if (animDataTable[objPtr->frame].bpp == 1) {
								int16 x2;
								int16 y2;

								x2 = animDataTable[objPtr->frame].width / 2;
								y2 = animDataTable[objPtr->frame].height;

								if (animDataTable[objPtr->frame].ptr1) {
									// drawSpriteRaw(animDataTable[objPtr->frame].ptr1, animDataTable[objPtr->frame].ptr1, x2, y2,
									//				additionalBgTable[currentAdditionalBgIdx], x, y);
								}
							}
						}
					}
				}
				break;
			}
		}

		currentOverlay = currentOverlay->next;
	}
}

void flip(void) {
	blitRawScreen(page1Raw);
}

uint16 processKeyboard(uint16 param) {
	return 0;
}

void mainLoopSub6(void) {
}

void checkForPendingDataLoad(void) {
	if (newPrcName[0] != 0) {
		freePrcLinkedList();
		resetglobalScriptsHead();

		loadPrc(newPrcName);

		strcpy(currentPrcName, newPrcName);
		strcpy(newPrcName, "");

		addScriptToList0(1);
	}

	if (newRelName[0] != 0) {
		releaseObjectScripts();
		resetObjectScriptHead();

		loadRel(newRelName);

		strcpy(currentRelName, newRelName);
		strcpy(newRelName, "");
	}

	if (newObjectName[0] != 0) {
		unloadAllMasks();
		resetMessageHead();

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

uint16 exitEngine;

void hideMouse(void) {
}

void closeEngine7(void) {
}

void removeExtention(char *dest, const char *source) {
	byte *ptr;

	strcpy(dest, source);

	ptr = (byte *) strchr(dest, '.');

	if (ptr) {
		*ptr = 0;
	}
}

uint16 var22;

uint16 defaultMenuBoxColor2;

uint16 zoneData[NUM_MAX_ZONE];

void addMessage(byte param1, int16 param2, int16 param3, int16 param4, int16 param5) {
	overlayHeadElement *currentHead = &overlayHead;
	overlayHeadElement *tempHead = currentHead;
	overlayHeadElement *newElement;

	currentHead = tempHead->next;

	while (currentHead) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	newElement = (overlayHeadElement *) malloc(sizeof(overlayHeadElement));

	newElement->next = tempHead->next;
	tempHead->next = newElement;

	newElement->objIdx = param1;
	newElement->type = 2;

	newElement->x = param2;
	newElement->y = param3;
	newElement->width = param4;
	newElement->color = param5;

	if (!currentHead)
		currentHead = &overlayHead;

	newElement->previous = currentHead->previous;

	currentHead->previous = newElement;
}

SeqListElement seqList;

void addSeqListElement(int16 param0, int16 param1, int16 param2, int16 param3, int16 param4, int16 param5, int16 param6, int16 param7, int16 param8) {
	SeqListElement *currentHead = &seqList;
	SeqListElement *tempHead = currentHead;
	SeqListElement *newElement;

	currentHead = tempHead->next;

	while (currentHead && currentHead->varE < param7) {
		tempHead = currentHead;
		currentHead = tempHead->next;
	}

	newElement = (SeqListElement *)malloc(sizeof(SeqListElement));

	newElement->next = tempHead->next;
	tempHead->next = newElement;

	newElement->var6 = param0;
	newElement->var4 = param1;
	newElement->var8 = param2;
	newElement->varA = param3;
	newElement->varC = param4;
	newElement->var14 = 0;
	newElement->var16 = 0;
	newElement->var18 = param5;
	newElement->var1A = param6;
	newElement->varE = param7;
	newElement->var10 = param8;
	newElement->var12 = param8;
	newElement->var1C = 0;
	newElement->var1E = 0;
}

void resetSeqList() {
	seqList.next = NULL;
}

void computeMove1(SeqListElement *element, int16 x, int16 y, int16 param1,
    int16 param2, int16 x2, int16 y2) {
	element->var16 = 0;
	element->var14 = 0;

	if (y2) {
		if (y - param2 > y2) {
			element->var16 = 2;
		}

		if (y + param2 < y2) {
			element->var16 = 1;
		}
	}

	if (x2) {
		if (x - param1 > x2) {
			element->var14 = 2;
		}

		if (x + param1 < x2) {
			element->var14 = 1;
		}
	}
}

uint16 computeMove2(SeqListElement *element) {
	int16 returnVar = 0;

	if (element->var16 == 1) {
		returnVar = 4;
	} else if (element->var16 == 2) {
		returnVar = 3;
	}

	if (element->var14 == 1) {
		returnVar = 1;
	} else if (element->var14 == 2) {
		returnVar = 2;
	}

	return returnVar;
}

// sort all the gfx stuff...

void resetGfxEntityEntry(uint16 objIdx) {
#if 0
	overlayHeadElement* tempHead = &overlayHead;
	byte* var_16 = NULL;
	uint16 var_10 = 0;
	uint16 var_12 = 0;
	overlayHeadElement* currentHead = tempHead->next;
	byte* var_1A = NULL;
	overlayHeadElement* var1E = &overlayHead;

	while(currentHead) {
		tempHead2 = currentHead->next;

		if(currentHead->objIdx == objIdx && currentHead->type!=2 && currentHead->type!=3 && currentHead->type!=0x14) {
			tempHead->next = tempHead2;

			if(tempHead2) {
				tempHead2->previous = currentHead->previous;
			} else {
				seqVar0 = currentHead->previous;
			}

			var_22 = var_16;

			if(!var_22) {
				// todo: goto?
			}

			var_22->previous = currentHead;
		} else {
		}

		if(currentHead->type == 0x14) {
		} else {
		}

		if(currentHead->type == 0x2 || currentHead->type == 0x3) {
			si = 10000;
		} else {
			si = objectTable[currentHead->objIdx];
		}

		if(objectTable[objIdx]>si) {
			var1E = currentHead;
		}

		tempHead = tempHead->next;

	}

	if (var_1A) {
		currentHead = var_16;
		var_22 = var_1E->next;
		var_1E->next = currentHead;
		var_1A->next = var_22;

		if(var_1E != &gfxEntityHead) {
			currentHead->previous = var_1E;
		}

		if(!var_22) {
			seqVar0 = var_1A;
		} else {
			var_22->previous = var_1A;
		}

	}
#endif
}

uint16 addAni(uint16 param1, uint16 param2, byte *ptr, SeqListElement *element, uint16 param3, int16 *param4) {
	byte *currentPtr = ptr;
	byte *ptrData;
	byte *ptr2;
	int16 di;

	assert(ptr);
	assert(element);
	assert(param4);

	dummyU16 = READ_BE_UINT16((currentPtr + param1 * 2) + 8);

	ptrData = ptr + dummyU16;

	assert(*ptrData);

	di = (objectTable[param2].costume + 1) % (*ptrData);
	ptr2 = (ptrData + (di * 8)) + 1;

	if ((checkCollision(param2, ptr2[0], ptr2[1], ptr2[2], ptr[0]) & 1)) {
		return 0;
	}

	objectTable[param2].x += (int8)ptr2[4];
	objectTable[param2].y += (int8)ptr2[5];
	objectTable[param2].mask += (int8)ptr2[6];

	if (objectTable[param2].frame) {
		resetGfxEntityEntry(param2);
	}

	objectTable[param2].frame = ptr2[7] + element->var8;

	if (param3 || !element->var14) {
		objectTable[param2].costume = di;
	} else {
		*param4 = di;
	}

	return 1;
}

void processSeqListElement(SeqListElement *element) {
	int16 x;
	int16 y;
	byte *ptr1;
	int16 var_10;
	int16 var_4;
	int16 var_2;

	if (element->var12 < element->var10) {
		element->var12++;
		return;
	}

	element->var12 = 0;

	x = objectTable[element->var6].x;
	y = objectTable[element->var6].y;
	ptr1 = animDataTable[element->varA].ptr1;

	if (ptr1) {
		uint16 param1;
		uint16 param2;

		param1 = ptr1[1];
		param2 = ptr1[2];

		assert(element->varC == 255);

		if (globalVars[VAR_MOUSE_X_POS] || globalVars[VAR_MOUSE_Y_POS]) {
			computeMove1(element, ptr1[4] + x, ptr1[5] + y, param1, param2, globalVars[VAR_MOUSE_X_POS], globalVars[VAR_MOUSE_Y_POS]);
		} else {
			element->var16 = 0;
			element->var14 = 0;
		}

		var_10 = computeMove2(element);

		if (var_10) {
			element->var1C = var_10;
			element->var1E = var_10;
		}

		var_4 = -1;

		if ((element->var16 == 1
			&& !addAni(3, element->var6, ptr1, element, 0, &var_4)) || (element->var16 == 2	&& !addAni(2, element->var6, ptr1, element, 0,
			    &var_4))) {
			if (element->varC == 255) {
				globalVars[VAR_MOUSE_Y_POS] = 0;
			}
		}

		if ((element->var14 == 1
			&& !addAni(0, element->var6, ptr1, element, 1, &var_2))) {
			if (element->varC == 255) {
				globalVars[VAR_MOUSE_X_POS] = 0;

				if (var_4 != -1) {
					objectTable[element->var6].costume = var_4;
				}
			}
		}

		if ((element->var14 == 2 && !addAni(1, element->var6, ptr1, element, 1, &var_2))) {
			if (element->varC == 255) {
				globalVars[VAR_MOUSE_X_POS] = 0;

				if (var_4 != -1) {
					objectTable[element->var6].costume = var_4;
				}
			}
		}

		if (element->var16 + element->var14) {
			if (element->var1C) {
				if (element->var1E) {
					objectTable[element->var6].costume = 0;
					element->var1E = 0;
				}

				addAni(element->var1C + 3, element->var6, ptr1, element, 1, (int16 *) & var2);

			}
		}

	}
}

void processSeqList(void) {
	SeqListElement *currentHead = &seqList;
	SeqListElement *tempHead = currentHead;

	currentHead = tempHead->next;

	while (currentHead) {
		if (currentHead->var4 != -1) {
			processSeqListElement(currentHead);
		}

		tempHead = currentHead;
		currentHead = tempHead->next;
	}
}

} // End of namespace Cine
