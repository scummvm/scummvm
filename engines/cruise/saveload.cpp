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

#include "common/savefile.h"
#include "common/system.h"

namespace Cruise {

struct overlayRestoreTemporary {
	int _sBssSize;
	uint8* _pBss;
	int _sNumObj;
	objectParams* _pObj;
};

overlayRestoreTemporary ovlRestoreData[90];

void resetPreload()
{
	for(unsigned long int i=0; i<64; i++)
	{
		if(strlen(preloadData[i].name))
		{
			if(preloadData[i].ptr)
			{
				free(preloadData[i].ptr);
				preloadData[i].ptr = NULL;
			}
			strcpy(preloadData[i].name, "");
			preloadData[i].nofree = 0;
		}
	}
}

void unloadOverlay(const char*name, int overlayNumber)
{
	releaseOverlay(name);

	strcpy(overlayTable[overlayNumber].overlayName, "");
	overlayTable[overlayNumber].ovlData = NULL;
	overlayTable[overlayNumber].alreadyLoaded = 0;
}

void initVars(void)
{
	closeAllMenu();
	resetFileEntryRange( 0, 257 );

	resetPreload();
	freeCTP();

	freezeCell(&cellHead, -1, -1, -1, -1, -1, 0);
	// TODO: unfreeze anims

	freeObjectList(&cellHead);
	removeAnimation(&actorHead, -1, -1, -1);

	changeScriptParamInList(-1, -1, &procHead, -1, 0);
	removeFinishedScripts(&procHead);

	changeScriptParamInList(-1, -1, &relHead, -1, 0);
	removeFinishedScripts(&relHead);

	for(unsigned long int i=0; i<90; i++)
	{
		if(strlen(overlayTable[i].overlayName) && overlayTable[i].alreadyLoaded)
		{
			unloadOverlay(overlayTable[i].overlayName, i);
		}
	}

	// TODO:
	// stopSound();
	// removeSound();

	closeBase();
	closeCnf();

	initOverlayTable();

	stateID = 0;
	masterScreen = 0;

	freeDisk();

	initVar5[0] = -1;
	initVar5[3] = -1;
	initVar5[6] = -1;
	initVar5[9] = -1;

	for (unsigned long int i = 0; i < 8; i++) {
		menuTable[i] = NULL;
	}

	for (unsigned long int i = 0; i < 2000; i++) {
		globalVars[i] = 0;
	}

	for (unsigned long int i = 0; i < 8; i++) {
		backgroundTable[i].name[0] = 0;
	}

	for (unsigned long int i = 0; i < 257; i++) {
		filesDatabase[i].subData.ptr = NULL;
		filesDatabase[i].subData.ptrMask = NULL;
	}

	initBigVar3();

	resetPtr2(&procHead);
	resetPtr2(&relHead);

	resetPtr(&cellHead);

	resetActorPtr(&actorHead);
	resetBackgroundIncrustList(&backgroundIncrustHead);

	vblLimit = 0;
	remdo = 0;
	songLoaded = 0;
	songPlayed = 0;
	songLoop = 1;
	activeMouse = 0;
	userEnabled = 1;
	dialogueEnabled = 0;
	dialogueOvl = 0;
	dialogueObj = 0;
	userDelay = 0;
	sysKey = -1;
	sysX = 0;
	sysY = 0;
	automoveInc = 0;
	automoveMax = 0;
	displayOn = true;

	// here used to init clip

	isMessage = 0;
	fadeFlag = 0;
	playMusic = 0;
	playMusic2 = 0;
	automaticMode = 0;

	// video param (vga and mcga mode)

	titleColor = 2;
	itemColor = 1;
	selectColor = 3;
	subColor = 5;

	//

	narratorOvl = 0;
	narratorIdx = 0;
	aniX = 0;
	aniY = 0;
	animationStart = false;
	selectDown = 0;
	menuDown = 0;
	buttonDown = 0;
	var41 = 0;
	entrerMenuJoueur = 0;
	PCFadeFlag = 0;
}

void saveOverlay(Common::OutSaveFile& currentSaveFile) {

	for (int i = 1; i < numOfLoadedOverlay; i++) {
		if(overlayTable[i].alreadyLoaded) {

			ovlDataStruct* ovlData = overlayTable[i].ovlData;

			// save BSS
			currentSaveFile.writeSint16LE(ovlData->sizeOfData4);
			if(ovlData->sizeOfData4)
				currentSaveFile.write(ovlData->data4Ptr, ovlData->sizeOfData4);

			// save variables
			currentSaveFile.writeSint16LE(ovlData->size9);
			for(int j=0; j<ovlData->size9; j++) {
					currentSaveFile.writeSint16LE(ovlData->arrayObjVar[j].X);
					currentSaveFile.writeSint16LE(ovlData->arrayObjVar[j].Y);
					currentSaveFile.writeSint16LE(ovlData->arrayObjVar[j].Z);
					currentSaveFile.writeSint16LE(ovlData->arrayObjVar[j].frame);
					currentSaveFile.writeSint16LE(ovlData->arrayObjVar[j].scale);
					currentSaveFile.writeSint16LE(ovlData->arrayObjVar[j].state);
			}
		}
	}
}

void loadSavegameDataSub1(Common::InSaveFile& currentSaveFile) {

	for (int i = 1; i < numOfLoadedOverlay; i++) {
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

void saveScript(Common::OutSaveFile& currentSaveFile, scriptInstanceStruct *entry) {
	int count = 0;

	scriptInstanceStruct* pCurrent = entry->nextScriptPtr;
	while( pCurrent ) {
		count ++;
		pCurrent = pCurrent->nextScriptPtr;
	}

	currentSaveFile.writeSint16LE(count);

	pCurrent = entry->nextScriptPtr;
	while( pCurrent ) {
		char dummy[4] = { 0, 0, 0, 0 };
		currentSaveFile.write(dummy, 2);

		currentSaveFile.writeSint16LE(pCurrent->ccr);
		currentSaveFile.writeSint16LE(pCurrent->var4);
		currentSaveFile.write(dummy, 4);
		currentSaveFile.writeSint16LE(pCurrent->varA);
		currentSaveFile.writeSint16LE(pCurrent->scriptNumber);
		currentSaveFile.writeSint16LE(pCurrent->overlayNumber);
		currentSaveFile.writeSint16LE(pCurrent->sysKey);
		currentSaveFile.writeSint16LE(pCurrent->freeze);
		currentSaveFile.writeSint16LE(pCurrent->type);
		currentSaveFile.writeSint16LE(pCurrent->var16);
		currentSaveFile.writeSint16LE(pCurrent->var18);
		currentSaveFile.writeSint16LE(pCurrent->var1A);

		currentSaveFile.writeSint16LE(pCurrent->varA);

		if (pCurrent->varA) {
			currentSaveFile.write(pCurrent->var6, pCurrent->varA);
		}

		pCurrent = pCurrent->nextScriptPtr;
	}
}

void loadScriptsFromSave(Common::InSaveFile& currentSaveFile, scriptInstanceStruct *entry) {
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

void saveAnim(Common::OutSaveFile& currentSaveFile) {
	int count = 0;

	actorStruct *ptr = actorHead.next;
	while(ptr) {
		count ++;
		ptr = ptr->next;
	}

	currentSaveFile.writeSint16LE(count);

	ptr = actorHead.next;
	while(ptr) {
		char dummy[2] = {0, 0};
		currentSaveFile.write(dummy, 2);
		currentSaveFile.write(dummy, 2);

		currentSaveFile.writeSint16LE(ptr->idx);
		currentSaveFile.writeSint16LE(ptr->type);
		currentSaveFile.writeSint16LE(ptr->overlayNumber);
		currentSaveFile.writeSint16LE(ptr->x_dest);
		currentSaveFile.writeSint16LE(ptr->y_dest);
		currentSaveFile.writeSint16LE(ptr->x);
		currentSaveFile.writeSint16LE(ptr->y);
		currentSaveFile.writeSint16LE(ptr->startDirection);
		currentSaveFile.writeSint16LE(ptr->nextDirection);
		currentSaveFile.writeSint16LE(ptr->endDirection);
		currentSaveFile.writeSint16LE(ptr->stepX);
		currentSaveFile.writeSint16LE(ptr->stepY);
		currentSaveFile.writeSint16LE(ptr->pathId);
		currentSaveFile.writeSint16LE(ptr->phase);
		currentSaveFile.writeSint16LE(ptr->counter);
		currentSaveFile.writeSint16LE(ptr->poly);
		currentSaveFile.writeSint16LE(ptr->flag);
		currentSaveFile.writeSint16LE(ptr->start);
		currentSaveFile.writeSint16LE(ptr->freeze);

		ptr = ptr->next;
	}
}

void loadSavegameActor(Common::InSaveFile& currentSaveFile) {
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

void saveSong(Common::OutSaveFile& currentSaveFile) {
	if (songLoaded) {
		// TODO: implement
		currentSaveFile.writeByte(0);
	} else {
		currentSaveFile.writeByte(0);
	}
}

void loadSavegameDataSub5(Common::InSaveFile& currentSaveFile) {
	if (songLoaded) {
		saveVar1 = currentSaveFile.readByte();

		if (saveVar1) {
			currentSaveFile.read(saveVar2, saveVar1);
		}
	} else {
		saveVar1 = currentSaveFile.readByte();
	}

}

void saveCT(Common::OutSaveFile& currentSaveFile) {
	if(polyStruct) {
		currentSaveFile.writeSint32LE(1);

		currentSaveFile.writeSint16LE(numberOfWalkboxes);

		if(numberOfWalkboxes)
		{
			currentSaveFile.write(walkboxColor, numberOfWalkboxes * 2);
			currentSaveFile.write(walkboxState, numberOfWalkboxes * 2);
		}

		for (unsigned long int i = 0; i < 10; i++) {

			if (persoTable[i]) {
				currentSaveFile.writeSint32LE(1);
				assert(sizeof(persoStruct) == 0x6AA);
				currentSaveFile.write(persoTable[i], 0x6AA);
			} else {
				currentSaveFile.writeSint32LE(0);
			}
		}

	}
	else {
		currentSaveFile.writeSint32LE(0);
	}
}

void loadSavegameDataSub6(Common::InSaveFile& currentSaveFile) {
	int32 var;

	var = currentSaveFile.readUint32BE();

	if (var) {
		int i;

		numberOfWalkboxes = currentSaveFile.readUint16LE();

		if (numberOfWalkboxes) {
			currentSaveFile.read(walkboxColor, numberOfWalkboxes * 2);
			currentSaveFile.read(walkboxState, numberOfWalkboxes * 2);
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

int saveSavegameData(int saveGameIdx) {
	char buffer[256];

	sprintf(buffer, "CR.%d", saveGameIdx);

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::OutSaveFile *currentSaveFile;
	currentSaveFile = saveMan->openForSaving(buffer);

	char saveIdentBuffer[6];
	strcpy(saveIdentBuffer, "SAVPC");

	currentSaveFile->write(saveIdentBuffer, 6);
	currentSaveFile->writeSint16LE(songLoaded);
	currentSaveFile->writeSint16LE(songPlayed);
	currentSaveFile->writeSint16LE(songLoop);
	currentSaveFile->writeSint16LE(activeMouse);
	currentSaveFile->writeSint16LE(userEnabled);
	currentSaveFile->writeSint16LE(dialogueEnabled);
	currentSaveFile->writeSint16LE(dialogueOvl);
	currentSaveFile->writeSint16LE(dialogueObj);
	currentSaveFile->writeSint16LE(userDelay);
	currentSaveFile->writeSint16LE(sysKey);
	currentSaveFile->writeSint16LE(sysX);
	currentSaveFile->writeSint16LE(sysY);
	currentSaveFile->writeSint16LE(automoveInc);
	currentSaveFile->writeSint16LE(automoveMax);
	currentSaveFile->writeSint16LE(displayOn);
	currentSaveFile->writeSint16LE(isMessage);
	currentSaveFile->writeSint16LE(fadeFlag);
	currentSaveFile->writeSint16LE(playMusic);
	currentSaveFile->writeSint16LE(playMusic2);
	currentSaveFile->writeSint16LE(automaticMode);
	currentSaveFile->writeSint16LE(titleColor);
	currentSaveFile->writeSint16LE(itemColor);
	currentSaveFile->writeSint16LE(selectColor);
	currentSaveFile->writeSint16LE(subColor);
	currentSaveFile->writeSint16LE(narratorOvl);
	currentSaveFile->writeSint16LE(narratorIdx);
	currentSaveFile->writeSint16LE(aniX);
	currentSaveFile->writeSint16LE(aniY);

	if(animationStart)
		currentSaveFile->writeSint16LE(1);
	else
		currentSaveFile->writeSint16LE(0);

	currentSaveFile->writeSint16LE(masterScreen);
	currentSaveFile->writeSint16LE(switchPal);
	currentSaveFile->writeSint16LE(scroll);
	currentSaveFile->writeSint16LE(fadeFlag);
	currentSaveFile->writeSint16LE(doFade);
	currentSaveFile->writeSint16LE(numOfLoadedOverlay);
	currentSaveFile->writeSint16LE(stateID);
	currentSaveFile->writeSint16LE(fontFileIndex);
	currentSaveFile->writeSint16LE(currentActiveMenu);
	currentSaveFile->writeSint16LE(userWait);
	currentSaveFile->writeSint16LE(autoOvl);
	currentSaveFile->writeSint16LE(autoMsg);
	currentSaveFile->writeSint16LE(autoTrack);
	currentSaveFile->writeSint16LE(var39);
	currentSaveFile->writeSint16LE(var42);
	currentSaveFile->writeSint16LE(var45);
	currentSaveFile->writeSint16LE(var46);
	currentSaveFile->writeSint16LE(var47);
	currentSaveFile->writeSint16LE(var48);
	currentSaveFile->writeSint16LE(flagCt);
	currentSaveFile->writeSint16LE(var41);
	currentSaveFile->writeSint16LE(entrerMenuJoueur);

	currentSaveFile->write(newPal, sizeof(int16) * NBCOLORS);
	currentSaveFile->write(workpal, sizeof(int16) * NBCOLORS);

	currentSaveFile->write(musicName, 15);

	const char dummy[6] = { 0, 0, 0, 0, 0, 0 };
	currentSaveFile->write(dummy, 6);

	currentSaveFile->write(currentCtpName, 40);

	// restore backgroundTable
	for(int i=0; i<8; i++)
	{
		currentSaveFile->write(backgroundTable[i].name, 9);
		currentSaveFile->write(backgroundTable[i].extention, 6);
	}

	currentSaveFile->write(palScreen, sizeof(int16) * NBCOLORS * NBSCREENS);
	currentSaveFile->write(initVar5, 24);
	currentSaveFile->write(globalVars, stateID * 2); // ok
	for(int i=0; i<257; i++)
	{
		currentSaveFile->writeUint16LE(filesDatabase[i].widthInColumn);
		currentSaveFile->writeUint16LE(filesDatabase[i].width);
		currentSaveFile->writeUint16LE(filesDatabase[i].resType);
		currentSaveFile->writeUint16LE(filesDatabase[i].height);
		if(filesDatabase[i].subData.ptr) {
			currentSaveFile->writeUint32LE(1);
		} else {
			currentSaveFile->writeUint32LE(0);
		}
		currentSaveFile->writeUint16LE(filesDatabase[i].subData.index);
		currentSaveFile->write(filesDatabase[i].subData.name, 13);
		currentSaveFile->write(dummy, 1);
		currentSaveFile->writeUint16LE(filesDatabase[i].subData.transparency);
		if(filesDatabase[i].subData.ptrMask) {
			currentSaveFile->writeUint32LE(1);
		} else {
			currentSaveFile->writeUint32LE(0);
		}
		currentSaveFile->writeByte(filesDatabase[i].subData.resourceType);
		currentSaveFile->write(dummy, 1);
		currentSaveFile->writeUint16LE(filesDatabase[i].subData.compression);
	}

	for(int i=0; i<numOfLoadedOverlay; i++)
	{
		currentSaveFile->write(overlayTable[i].overlayName, 13);
		currentSaveFile->write(dummy, 1);
		currentSaveFile->write(dummy, 4);
		currentSaveFile->writeUint16LE(overlayTable[i].alreadyLoaded);
		currentSaveFile->writeUint16LE(overlayTable[i].state);
		currentSaveFile->write(dummy, 4);
		currentSaveFile->write(dummy, 4);
		currentSaveFile->write(dummy, 4);
		currentSaveFile->write(dummy, 4);
		currentSaveFile->writeUint16LE(overlayTable[i].executeScripts);
	}

	for(int i=0; i<64; i++)
	{
		currentSaveFile->write(preloadData[i].name, 15);
		currentSaveFile->write(dummy, 1);
		currentSaveFile->writeUint32LE(preloadData[i].size);
		currentSaveFile->writeUint32LE(preloadData[i].sourceSize);
		currentSaveFile->write(dummy, 4);
		currentSaveFile->writeUint16LE(preloadData[i].nofree);
		currentSaveFile->writeUint16LE(preloadData[i].protect);
		currentSaveFile->writeUint16LE(preloadData[i].ovl);
	}

	saveOverlay(*currentSaveFile);
	saveScript(*currentSaveFile, &procHead);
	saveScript(*currentSaveFile, &relHead);
	saveCell(*currentSaveFile);
	saveIncrust(*currentSaveFile);
	saveAnim(*currentSaveFile);
	saveSong(*currentSaveFile);
	saveCT(*currentSaveFile);

	currentSaveFile->finalize();
	delete currentSaveFile;
	return 0;
}

int loadSavegameData(int saveGameIdx) {
	char buffer[256];
	char saveIdentBuffer[6];
	int lowMemorySave;
	cellStruct *currentcellHead;

	sprintf(buffer, "CR.%d", saveGameIdx);

	Common::SaveFileManager *saveMan = g_system->getSavefileManager();
	Common::InSaveFile *currentSaveFile;
	currentSaveFile = saveMan->openForLoading(buffer);

	if (currentSaveFile == NULL) {
		printInfoBlackBox("Savegame not found...");
		waitForPlayerInput();
		return (-1);
	}

	printInfoBlackBox("Loading in progress...");

	currentSaveFile->read(saveIdentBuffer, 6);

	if (strcmp(saveIdentBuffer, "SAVPC")) {
		delete currentSaveFile;
		return (-1);
	}
	initVars();

	songLoaded = currentSaveFile->readSint16LE();
	songPlayed = currentSaveFile->readSint16LE();
	songLoop = currentSaveFile->readSint16LE();
	activeMouse = currentSaveFile->readSint16LE();
	userEnabled = currentSaveFile->readSint16LE();
	dialogueEnabled = currentSaveFile->readSint16LE();

	dialogueOvl = currentSaveFile->readSint16LE();
	dialogueObj = currentSaveFile->readSint16LE();
	userDelay = currentSaveFile->readSint16LE();
	sysKey = currentSaveFile->readSint16LE();
	sysX = currentSaveFile->readSint16LE();
	sysY = currentSaveFile->readSint16LE();
	automoveInc = currentSaveFile->readSint16LE();
	automoveMax = currentSaveFile->readSint16LE();
	displayOn = currentSaveFile->readSint16LE();
	isMessage = currentSaveFile->readSint16LE();
	fadeFlag = currentSaveFile->readSint16LE();
	playMusic = currentSaveFile->readSint16LE();
	playMusic2 = currentSaveFile->readSint16LE();
	automaticMode = currentSaveFile->readSint16LE();

	// video param (not loaded in EGA mode)

	titleColor = currentSaveFile->readSint16LE();
	itemColor = currentSaveFile->readSint16LE();
	selectColor = currentSaveFile->readSint16LE();
	subColor = currentSaveFile->readSint16LE();

	//

	narratorOvl = currentSaveFile->readSint16LE();
	narratorIdx = currentSaveFile->readSint16LE();
	aniX = currentSaveFile->readSint16LE();
	aniY = currentSaveFile->readSint16LE();

	if(currentSaveFile->readSint16LE()) // cast to bool
		animationStart = true;
	else
		animationStart = false;

	masterScreen = currentSaveFile->readSint16LE();
	switchPal = currentSaveFile->readSint16LE();
	scroll = currentSaveFile->readSint16LE();
	fadeFlag = currentSaveFile->readSint16LE();
	doFade = currentSaveFile->readSint16LE();
	numOfLoadedOverlay = currentSaveFile->readSint16LE();
	stateID = currentSaveFile->readSint16LE();
	fontFileIndex = currentSaveFile->readSint16LE();
	currentActiveMenu = currentSaveFile->readSint16LE();
	userWait = currentSaveFile->readSint16LE();
	autoOvl = currentSaveFile->readSint16LE();
	autoMsg = currentSaveFile->readSint16LE();
	autoTrack = currentSaveFile->readSint16LE();
	var39 = currentSaveFile->readSint16LE();
	var42 = currentSaveFile->readSint16LE();
	var45 = currentSaveFile->readSint16LE();
	var46 = currentSaveFile->readSint16LE();
	var47 = currentSaveFile->readSint16LE();
	var48 = currentSaveFile->readSint16LE();
	flagCt = currentSaveFile->readSint16LE();
	var41 = currentSaveFile->readSint16LE();
	entrerMenuJoueur = currentSaveFile->readSint16LE();

	currentSaveFile->read(newPal, sizeof(int16) * NBCOLORS);
	currentSaveFile->read(newPal, sizeof(int16) * NBCOLORS);

	// here code seems bogus... this should read music name and it may be a buffer overrun
	currentSaveFile->skip(21);

	currentSaveFile->read(currentCtpName, 40);

	// restore backgroundTable
	for(int i=0; i<8; i++)
	{
		currentSaveFile->read(backgroundTable[i].name, 9);
		currentSaveFile->read(backgroundTable[i].extention, 6);
	}

	currentSaveFile->read(palScreen, sizeof(int16) * NBCOLORS * NBSCREENS);
	currentSaveFile->read(initVar5, 24);
	currentSaveFile->read(globalVars, stateID * 2); // ok
	for(int i=0; i<257; i++)
	{
		filesDatabase[i].widthInColumn = currentSaveFile->readUint16LE();
		filesDatabase[i].width = currentSaveFile->readUint16LE();
		filesDatabase[i].resType = currentSaveFile->readUint16LE();
		filesDatabase[i].height = currentSaveFile->readUint16LE();
		filesDatabase[i].subData.ptr = (uint8*)currentSaveFile->readSint32LE();
		filesDatabase[i].subData.index = currentSaveFile->readSint16LE();
		currentSaveFile->read(filesDatabase[i].subData.name, 13);
		currentSaveFile->skip(1);
		filesDatabase[i].subData.transparency = currentSaveFile->readSint16LE();
		filesDatabase[i].subData.ptrMask = (uint8*)currentSaveFile->readSint32LE();
		filesDatabase[i].subData.resourceType = currentSaveFile->readByte();
		currentSaveFile->skip(1);
		filesDatabase[i].subData.compression = currentSaveFile->readSint16LE();
	}

	for(int i=0; i<numOfLoadedOverlay; i++)
	{
		currentSaveFile->read(overlayTable[i].overlayName, 13);
		currentSaveFile->skip(1);
		currentSaveFile->skip(4);
		overlayTable[i].alreadyLoaded = currentSaveFile->readSint16LE();
		overlayTable[i].state = currentSaveFile->readSint16LE();
		currentSaveFile->skip(4);
		currentSaveFile->skip(4);
		currentSaveFile->skip(4);
		currentSaveFile->skip(4);
		overlayTable[i].executeScripts = currentSaveFile->readSint16LE();
	}

	for(int i=0; i<64; i++)
	{
		currentSaveFile->read(preloadData[i].name, 15);
		currentSaveFile->skip(1);
		preloadData[i].size = currentSaveFile->readSint32LE();
		preloadData[i].sourceSize = currentSaveFile->readSint32LE();
		currentSaveFile->skip(4);
		preloadData[i].nofree = currentSaveFile->readSint16LE();
		preloadData[i].protect = currentSaveFile->readSint16LE();
		preloadData[i].ovl = currentSaveFile->readSint16LE();
	}

	loadSavegameDataSub1(*currentSaveFile);
	loadScriptsFromSave(*currentSaveFile, &procHead);
	loadScriptsFromSave(*currentSaveFile, &relHead);

	loadSavegameDataSub2(*currentSaveFile);
	loadBackgroundIncrustFromSave(*currentSaveFile);
	loadSavegameActor(*currentSaveFile);
	loadSavegameDataSub5(*currentSaveFile);
	loadSavegameDataSub6(*currentSaveFile);

	delete currentSaveFile;

	for (int j = 0; j < 64; j++) {
		preloadData[j].ptr = NULL;
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

	lowMemorySave = lowMemory;

	for (int i = 0; i < 257; i++) {
		if (filesDatabase[i].subData.ptr) {
			int j;
			int k;

			for (j = i + 1; j < 257 && filesDatabase[j].subData.ptr && !strcmp(filesDatabase[i].subData.name, filesDatabase[j].subData.name) && (filesDatabase[j].subData.index == (j-i)); j++);

			for (k = i; k < j; k++) {
				if (filesDatabase[k].subData.ptrMask)
					lowMemory = 0;

				filesDatabase[k].subData.ptr = NULL;
				filesDatabase[k].subData.ptrMask = NULL;
			}

			/*if (j < 2) {
				printf("Unsupported mono file load!\n");
				ASSERT(0);
				//loadFileMode1(filesDatabase[j].subData.name,filesDatabase[j].subData.var4);
			} else */{
				loadFileRange(filesDatabase[i].subData.name, filesDatabase[i].subData.index, i, j - i);
				i = j - 1;
			}

			lowMemory = lowMemorySave;
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
		loadCtFromSave = 1;
		initCt(currentCtpName);
		loadCtFromSave = 0;
	}
	//prepareFadeOut();
	//gfxModuleData.gfxFunction8();

	for (int j = 0; j < 8; j++) {
		if (strlen((char *)backgroundTable[j].name)) {
			loadBackground(backgroundTable[j].name, j);
		}
	}

	//regenerateBackgroundIncrust(&backgroundIncrustHead);

	// to finish

	changeCursor(CURSOR_NORMAL);
	mainDraw(1);
	flipScreen();

	return (0);
}

} // End of namespace Cruise
