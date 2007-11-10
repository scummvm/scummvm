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

#include "cruise/cruise_main.h"

namespace Cruise {

struct overlayRestoreTemporary {
	int _sBssSize;
	uint8* _pBss;
	int _sNumObj;
	objectParams* _pObj;
};

overlayRestoreTemporary ovlRestoreData[90];

void loadSavegameDataSub1(Common::File& currentSaveFile) {
	int i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		ovlRestoreData[i]._sBssSize = ovlRestoreData[i]._sNumObj = 0;
		ovlRestoreData[i]._pBss = NULL;
		ovlRestoreData[i]._pObj = NULL;

		if (overlayTable[i].alreadyLoaded) {
			ovlRestoreData[i]._sBssSize = currentSaveFile.readSint16LE();

			if (ovlRestoreData[i]._sBssSize) {
				ovlRestoreData[i]._pBss = (uint8 *) mallocAndZero(ovlRestoreData[i]._sBssSize);
				ASSERT(ovlRestoreData[i]._pBss);

				currentSaveFile.read(ovlRestoreData[i]._pBss, ovlRestoreData[i]._sBssSize);
			}

			ovlRestoreData[i]._sNumObj = currentSaveFile.readSint16LE();

			if (ovlRestoreData[i]._sNumObj) {
				ovlRestoreData[i]._pObj = (objectParams *) mallocAndZero(ovlRestoreData[i]._sNumObj * sizeof(objectParams));
				ASSERT(ovlRestoreData[i]._pObj);

				for(int j=0; j<ovlRestoreData[i]._sNumObj; j++)
				{
					ovlRestoreData[i]._pObj[j].X = currentSaveFile.readSint16LE();
					ovlRestoreData[i]._pObj[j].Y = currentSaveFile.readSint16LE();
					ovlRestoreData[i]._pObj[j].Z = currentSaveFile.readSint16LE();
					ovlRestoreData[i]._pObj[j].frame = currentSaveFile.readSint16LE();
					ovlRestoreData[i]._pObj[j].scale = currentSaveFile.readSint16LE();
					ovlRestoreData[i]._pObj[j].state = currentSaveFile.readSint16LE();
				}
			}
		}
	}
}

void loadScriptsFromSave(Common::File& currentSaveFile, scriptInstanceStruct *entry) {
	short int numScripts;
	int i;

	numScripts = currentSaveFile.readSint16LE();

	for (i = 0; i < numScripts; i++) {
		scriptInstanceStruct *ptr = (scriptInstanceStruct *)mallocAndZero(sizeof(scriptInstanceStruct));

		currentSaveFile.skip(2);

		ptr->ccr = currentSaveFile.readSint16LE();
		ptr->var4 = currentSaveFile.readSint16LE();
		currentSaveFile.skip(4);
		ptr->varA = currentSaveFile.readSint16LE();
		ptr->scriptNumber = currentSaveFile.readSint16LE();
		ptr->overlayNumber = currentSaveFile.readSint16LE();
		ptr->sysKey = currentSaveFile.readSint16LE();
		ptr->freeze = currentSaveFile.readSint16LE();
		ptr->type = (scriptTypeEnum)currentSaveFile.readSint16LE();
		ptr->var16 = currentSaveFile.readSint16LE();
		ptr->var18 = currentSaveFile.readSint16LE();
		ptr->var1A = currentSaveFile.readSint16LE();

		ptr->varA = currentSaveFile.readUint16LE();

		if (ptr->varA) {
			ptr->var6 = (uint8 *) mallocAndZero(ptr->varA);

			currentSaveFile.read(ptr->var6, ptr->varA);
		}

		ptr->nextScriptPtr = 0;

		entry->nextScriptPtr = ptr;
		entry = ptr;
	}
}

void loadSavegameActor(Common::File& currentSaveFile) {
	short int numEntry;
	actorStruct *ptr;
	int i;

	numEntry = currentSaveFile.readSint16LE();

	ptr = &actorHead;

	for (i = 0; i < numEntry; i++) {
		actorStruct *current = (actorStruct *) mallocAndZero(sizeof(actorStruct));
		currentSaveFile.skip(2);
		currentSaveFile.skip(2);

		current->idx = currentSaveFile.readSint16LE();
		current->type = currentSaveFile.readSint16LE();
		current->overlayNumber = currentSaveFile.readSint16LE();
		current->x_dest = currentSaveFile.readSint16LE();
		current->y_dest = currentSaveFile.readSint16LE();
		current->x = currentSaveFile.readSint16LE();
		current->y = currentSaveFile.readSint16LE();
		current->startDirection = currentSaveFile.readSint16LE();
		current->nextDirection = currentSaveFile.readSint16LE();
		current->endDirection = currentSaveFile.readSint16LE();
		current->stepX = currentSaveFile.readSint16LE();
		current->stepY = currentSaveFile.readSint16LE();
		current->pathId = currentSaveFile.readSint16LE();
		current->phase = (animPhase)currentSaveFile.readSint16LE();
		current->counter = currentSaveFile.readSint16LE();
		current->poly = currentSaveFile.readSint16LE();
		current->flag = currentSaveFile.readSint16LE();
		current->start = currentSaveFile.readSint16LE();
		current->freeze = currentSaveFile.readSint16LE();

		current->next = NULL;
		ptr->next = current;
		current->prev = actorHead.prev;
		actorHead.prev = current;
		ptr = current->next;
	}
}

void loadSavegameDataSub5(Common::File& currentSaveFile) {
	if (var1) {
		saveVar1 = currentSaveFile.readByte();

		if (saveVar1) {
			currentSaveFile.read(saveVar2, saveVar1);
		}
	} else {
		saveVar1 = currentSaveFile.readByte();
	}

}

void loadSavegameDataSub6(Common::File& currentSaveFile) {
	int32 var;

	var = currentSaveFile.readUint32LE();
	flipLong(&var);

	if (var) {
		int i;

		numberOfWalkboxes = currentSaveFile.readUint16LE();

		if (numberOfWalkboxes) {
			currentSaveFile.read(walkboxType, numberOfWalkboxes * 2);
			currentSaveFile.read(walkboxChange, numberOfWalkboxes * 2);
		}

		for (i = 0; i < 10; i++) {
			persoTable[i] = (persoStruct*)currentSaveFile.readSint32LE();

			if (persoTable[i]) {
				assert(sizeof(persoStruct) == 0x6AA);
				persoTable[i] = (persoStruct *)mallocAndZero(sizeof(persoStruct));
				currentSaveFile.read(persoTable[i], 0x6AA);
			}
		}
	}
}

int loadSavegameData(int saveGameIdx) {
	char buffer[256];
	char saveIdentBuffer[6];
	int initVar1Save;
	cellStruct *currentcellHead;

	sprintf(buffer, "CR.%d", saveGameIdx);

	Common::File currentSaveFile;
	currentSaveFile.open(buffer);

	if (!currentSaveFile.isOpen()) {
		printInfoBlackBox("Savegame not found...");
		waitForPlayerInput();
		return (-1);
	}

	printInfoBlackBox("Loading in progress...");

	currentSaveFile.read(saveIdentBuffer, 6);

	if (strcmp(saveIdentBuffer, "SAVPC")) {
		currentSaveFile.close();
		return (-1);
	}
	//initVars();

	var1 = currentSaveFile.readSint16LE();
	var2 = currentSaveFile.readSint16LE();
	var3 = currentSaveFile.readSint16LE();
	var4 = currentSaveFile.readSint16LE();
	userEnabled = currentSaveFile.readSint16LE();
	dialogueEnabled = currentSaveFile.readSint16LE();

	var7 = currentSaveFile.readSint16LE();
	var8 = currentSaveFile.readSint16LE();
	userDelay = currentSaveFile.readSint16LE();
	sysKey = currentSaveFile.readSint16LE();
	sysX = currentSaveFile.readSint16LE();
	sysY = currentSaveFile.readSint16LE();
	var13 = currentSaveFile.readSint16LE();
	var14 = currentSaveFile.readSint16LE();
	affichePasMenuJoueur = currentSaveFile.readSint16LE();
	var20 = currentSaveFile.readSint16LE();
	var22 = currentSaveFile.readSint16LE();
	var23 = currentSaveFile.readSint16LE();
	var24 = currentSaveFile.readSint16LE();
	automaticMode = currentSaveFile.readSint16LE();

	// video param (not loaded in EGA mode)

	video4 = currentSaveFile.readSint16LE();
	video2 = currentSaveFile.readSint16LE();
	video3 = currentSaveFile.readSint16LE();
	colorOfSelectedSaveDrive = currentSaveFile.readSint16LE();

	//

	narratorOvl = currentSaveFile.readSint16LE();
	narratorIdx = currentSaveFile.readSint16LE();
	aniX = currentSaveFile.readSint16LE();
	aniY = currentSaveFile.readSint16LE();

	if(currentSaveFile.readSint16LE()) // cast to bool
		animationStart = true;
	else
		animationStart = false;

	currentActiveBackgroundPlane = currentSaveFile.readSint16LE();
	switchPal = currentSaveFile.readSint16LE();
	initVar2 = currentSaveFile.readSint16LE();
	var22 = currentSaveFile.readSint16LE();
	main5 = currentSaveFile.readSint16LE();
	numOfLoadedOverlay = currentSaveFile.readSint16LE();
	setup1 = currentSaveFile.readSint16LE();
	fontFileIndex = currentSaveFile.readSint16LE();
	currentActiveMenu = currentSaveFile.readSint16LE();
	userWait = currentSaveFile.readSint16LE();
	autoOvl = currentSaveFile.readSint16LE();
	autoMsg = currentSaveFile.readSint16LE();
	autoTrack = currentSaveFile.readSint16LE();
	var39 = currentSaveFile.readSint16LE();
	var42 = currentSaveFile.readSint16LE();
	var45 = currentSaveFile.readSint16LE();
	var46 = currentSaveFile.readSint16LE();
	var47 = currentSaveFile.readSint16LE();
	var48 = currentSaveFile.readSint16LE();
	flagCt = currentSaveFile.readSint16LE();
	var41 = currentSaveFile.readSint16LE();
	entrerMenuJoueur = currentSaveFile.readSint16LE();

	currentSaveFile.read(var50, 64);
	currentSaveFile.read(var50, 64); // Hu ? why 2 times ?

	// here code seems bogus... this should read music name and it may be a buffer overrun
	currentSaveFile.skip(21);

	currentSaveFile.read(currentCtpName, 40);

	// restore backgroundTable
	for(int i=0; i<8; i++)
	{
		currentSaveFile.read(backgroundTable[i].name, 9);
		currentSaveFile.read(backgroundTable[i].extention, 6);
	}

	currentSaveFile.read(palette, 256*2);
	currentSaveFile.read(initVar5, 24);
	currentSaveFile.read(globalVars, setup1 * 2); // ok
	for(int i=0; i<257; i++)
	{
		filesDatabase[i].widthInColumn = currentSaveFile.readUint16LE();
		filesDatabase[i].width = currentSaveFile.readUint16LE();
		filesDatabase[i].resType = currentSaveFile.readUint16LE();
		filesDatabase[i].height = currentSaveFile.readUint16LE();
		filesDatabase[i].subData.ptr = (uint8*)currentSaveFile.readSint32LE();
		filesDatabase[i].subData.index = currentSaveFile.readSint16LE();
		currentSaveFile.read(filesDatabase[i].subData.name, 13);
		currentSaveFile.skip(1);
		filesDatabase[i].subData.transparency = currentSaveFile.readSint16LE();
		filesDatabase[i].subData.ptrMask = (uint8*)currentSaveFile.readSint32LE();
		filesDatabase[i].subData.resourceType = currentSaveFile.readByte();
		currentSaveFile.skip(1);
		filesDatabase[i].subData.compression = currentSaveFile.readSint16LE();
	}

	for(int i=0; i<numOfLoadedOverlay; i++)
	{
		currentSaveFile.read(overlayTable[i].overlayName, 13);
		currentSaveFile.skip(1);
		currentSaveFile.skip(4);
		overlayTable[i].alreadyLoaded = currentSaveFile.readSint16LE();
		overlayTable[i].state = currentSaveFile.readSint16LE();
		currentSaveFile.skip(4);
		currentSaveFile.skip(4);
		currentSaveFile.skip(4);
		currentSaveFile.skip(4);
		overlayTable[i].executeScripts = currentSaveFile.readSint16LE();
	}

	for(int i=0; i<64; i++)
	{
		currentSaveFile.read(mediumVar[i].name, 15);
		currentSaveFile.skip(1);
		mediumVar[i].size = currentSaveFile.readSint32LE();
		mediumVar[i].sourceSize = currentSaveFile.readSint32LE();
		currentSaveFile.skip(4);
		mediumVar[i].nofree = currentSaveFile.readSint16LE();
		mediumVar[i].protect = currentSaveFile.readSint16LE();
		mediumVar[i].ovl = currentSaveFile.readSint16LE();
	}

	loadSavegameDataSub1(currentSaveFile);
	loadScriptsFromSave(currentSaveFile, &procHead);
	loadScriptsFromSave(currentSaveFile, &relHead);

	loadSavegameDataSub2(currentSaveFile);
	loadBackgroundIncrustFromSave(currentSaveFile);
	loadSavegameActor(currentSaveFile);
	loadSavegameDataSub5(currentSaveFile);
	loadSavegameDataSub6(currentSaveFile);

	currentSaveFile.close();

	for (int j = 0; j < 64; j++) {
		mediumVar[j].ptr = NULL;
	}

	for (int j = 1; j < numOfLoadedOverlay; j++) {
		if (overlayTable[j].alreadyLoaded) {
			overlayTable[j].alreadyLoaded = 0;
			loadOverlay(overlayTable[j].overlayName);

			if (overlayTable[j].alreadyLoaded) {
				ovlDataStruct *ovlData = overlayTable[j].ovlData;

				// overlay BSS

				if (ovlRestoreData[j]._sBssSize) {
					if (ovlData->data4Ptr) {
						free(ovlData->data4Ptr);
					}

					ovlData->data4Ptr = ovlRestoreData[j]._pBss;
					ovlData->sizeOfData4 = ovlRestoreData[j]._sBssSize;
				}

				// overlay object data

				if (ovlRestoreData[j]._sNumObj) {
					if (ovlData->arrayObjVar) {
						free(ovlData->arrayObjVar);
					}

					ovlData->arrayObjVar = ovlRestoreData[j]._pObj;
					ovlData->size9 = ovlRestoreData[j]._sNumObj;
				}

			}
		}
	}

	updateAllScriptsImports();

	lastAni[0] = 0;

	initVar1Save = initVar1;

	for (int i = 0; i < 257; i++) {
		if (filesDatabase[i].subData.ptr) {
			int j;
			int k;

			for (j = i + 1; j < 257 && filesDatabase[j].subData.ptr && !strcmp(filesDatabase[i].subData.name, filesDatabase[j].subData.name) && (filesDatabase[j].subData.index == (j-i)); j++);

			for (k = i; k < j; k++) {
				if (filesDatabase[k].subData.ptrMask)
					initVar1 = 0;

				filesDatabase[k].subData.ptr = NULL;
				filesDatabase[k].subData.ptrMask = NULL;
			}

			if (j < 2) {
				printf("Unsupported mono file load!\n");
				ASSERT(0);
				//loadFileMode1(filesDatabase[j].subData.name,filesDatabase[j].subData.var4);
			} else {
				loadFileRange(filesDatabase[i].subData.name, filesDatabase[i].subData.index, i, j - i);
				i = j - 1;
			}

			initVar1 = initVar1Save;
		}
	}

	lastAni[0] = 0;

	currentcellHead = cellHead.next;

	while (currentcellHead) {
		if (currentcellHead->type == 5) {
			uint8 *ptr = mainProc14(currentcellHead->overlay, currentcellHead->idx);

			ASSERT(0);

			if (ptr) {
				ASSERT(0);
				//*(int16*)(currentcellHead->datas+0x2E) = getSprite(ptr,*(int16*)(currentcellHead->datas+0xE));
			} else {
				ASSERT(0);
				//*(int16*)(currentcellHead->datas+0x2E) = 0;
			}
		}

		currentcellHead = currentcellHead->next;
	}

	//TODO: here, restart music

	if (strlen(currentCtpName)) {
		ctpVar1 = 1;
		loadCtp(currentCtpName);
		ctpVar1 = 0;
	}
	//prepareFadeOut();
	//gfxModuleData.gfxFunction8();

	for (int j = 0; j < 8; j++) {
		if (strlen((char *)backgroundTable[j].name)) {
			loadBackground(backgroundTable[j].name, j);
		}
	}

	regenerateBackgroundIncrust(&backgroundIncrustHead);

	// to finish

	changeCursor(CURSOR_NORMAL);
	mainDraw(1);
	flipScreen();

	return (0);
}

} // End of namespace Cruise
