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

#include "cruise/cruise_main.h"
#include "cruise/cell.h"

namespace Cruise {

unsigned int timer = 0;

void drawSolidBox(int32 x1, int32 y1, int32 x2, int32 y2, uint8 color) {
	int32 i;
	int32 j;

	for (i = x1; i < x2; i++) {
		for (j = y1; j < y2; j++) {
			globalScreen[j * 320 + i] = color;
		}
	}
}

void drawBlackSolidBoxSmall() {
//  gfxModuleData.drawSolidBox(64,100,256,117,0);
	drawSolidBox(64, 100, 256, 117, 0);
}

void resetRaster(uint8 *rasterPtr, int32 rasterSize) {
	memset(rasterPtr, 0, rasterSize);
}

void drawInfoStringSmallBlackBox(uint8 *string) {
	//uint8 buffer[256];

	gfxModuleData_field_90();
	gfxModuleData_gfxWaitVSync();
	drawBlackSolidBoxSmall();

	drawString(10, 100, string, gfxModuleData.pPage10, video4, 300);

	gfxModuleData_flip();

	flipScreen();

	while (1);
}

void loadPakedFileToMem(int fileIdx, uint8 *buffer) {
	changeCursor(CURSOR_DISK);

	currentVolumeFile.seek(volumePtrToFileDescriptor[fileIdx].offset, SEEK_SET);
	currentVolumeFile.read(buffer, volumePtrToFileDescriptor[fileIdx].size);
}

int loadScriptSub1(int scriptIdx, int param) {
	objDataStruct *ptr2;
	int counter;
	int i;

	if (!overlayTable[scriptIdx].ovlData)
		return (0);

	ptr2 = overlayTable[scriptIdx].ovlData->arrayObject;

	if (!ptr2)
		return (0);

	if (overlayTable[scriptIdx].ovlData->numObj == 0)
		return (0);

	counter = 0;

	for (i = 0; i < overlayTable[scriptIdx].ovlData->numObj; i++) {
		if (ptr2[i].var0 == param) {
			counter++;
		}
	}

	return (counter);
}

void saveShort(void *ptr, short int var) {
	*(int16 *) ptr = var;

	flipShort((int16 *) ptr);
}

int16 loadShort(void *ptr) {
	short int temp;

	temp = *(int16 *) ptr;

	flipShort(&temp);

	return (temp);
}

void resetFileEntryRange(int param1, int param2) {
	int i;

	for (i = param1; i < param2; i++) {
		resetFileEntry(i);
	}
}

int getProcParam(int overlayIdx, int param2, uint8 *name) {
	int numSymbGlob;
	int i;
	exportEntryStruct *arraySymbGlob;
	uint8 *exportNamePtr;
	uint8 exportName[80];

	if (!overlayTable[overlayIdx].alreadyLoaded)
		return 0;

	if (!overlayTable[overlayIdx].ovlData)
		return 0;

	numSymbGlob = overlayTable[overlayIdx].ovlData->numSymbGlob;
	arraySymbGlob = overlayTable[overlayIdx].ovlData->arraySymbGlob;
	exportNamePtr = overlayTable[overlayIdx].ovlData->arrayNameSymbGlob;

	if (!exportNamePtr)
		return 0;

	for (i = 0; i < numSymbGlob; i++) {
		if (arraySymbGlob[i].var4 == param2) {
			strcpyuint8(exportName,
			    arraySymbGlob[i].offsetToName + exportNamePtr);

			if (!strcmpuint8(exportName, name)) {
				return (arraySymbGlob[i].idx);
			}
		}
	}

	return 0;
}

void changeScriptParamInList(int param1, int param2, scriptInstanceStruct *pScriptInstance, int newValue, int param3) {
	pScriptInstance = pScriptInstance->nextScriptPtr;
	while (pScriptInstance) {
		if ((pScriptInstance->overlayNumber == param1) || (param1 == -1))
		    if ((pScriptInstance->scriptNumber == param2) || (param2 == -1))
				if ((pScriptInstance->freeze == param3) || (param3 == -1)) {
					pScriptInstance->freeze = newValue;
		}

		pScriptInstance = pScriptInstance->nextScriptPtr;
	}
}

void initBigVar3() {
	int i;

	for (i = 0; i < 257; i++) {
		if (filesDatabase[i].subData.ptr) {
			free(filesDatabase[i].subData.ptr);
		}

		filesDatabase[i].subData.ptr = NULL;
		filesDatabase[i].subData.ptr2 = NULL;

		filesDatabase[i].subData.index = -1;
		filesDatabase[i].subData.resourceType = 0;
	}
}

void resetPtr2(scriptInstanceStruct *ptr) {
	ptr->nextScriptPtr = NULL;
	ptr->scriptNumber = -1;
}

void resetActorPtr(actorStruct *ptr) {
	ptr->next = NULL;
	ptr->prev = NULL;
}

ovlData3Struct *getOvlData3Entry(int32 scriptNumber, int32 param) {
	ovlDataStruct *ovlData = overlayTable[scriptNumber].ovlData;

	if (!ovlData) {
		return NULL;
	}

	if (param < 0) {
		return NULL;
	}

	if (ovlData->numProc <= param) {
		return NULL;
	}

	if (!ovlData->arrayProc) {
		return NULL;
	}

	return (&ovlData->arrayProc[param]);
}

ovlData3Struct *scriptFunc1Sub2(int32 scriptNumber, int32 param) {
	ovlDataStruct *ovlData = overlayTable[scriptNumber].ovlData;

	if (!ovlData) {
		return NULL;
	}

	if (param < 0) {
		return NULL;
	}

	if (ovlData->numRel <= param) {
		return NULL;
	}

	if (!ovlData->ptr1) {
		return NULL;
	}

	return ((ovlData3Struct *) (ovlData->ptr1 + param * 0x1C));
}

void scriptFunc2(int scriptNumber, scriptInstanceStruct * scriptHandle,
	    int param, int param2) {
	if (scriptHandle->nextScriptPtr) {
		if (scriptNumber == scriptHandle->nextScriptPtr->overlayNumber
		    || scriptNumber != -1) {
			if (param2 == scriptHandle->nextScriptPtr->scriptNumber
			    || param2 != -1) {
				scriptHandle->nextScriptPtr->sysKey = param;
			}
		}
	}
}

uint8 *getDataFromData3(ovlData3Struct *ptr, int param) {
	uint8 *dataPtr;

	if (!ptr)
		return (NULL);

	dataPtr = ptr->dataPtr;

	if (!dataPtr)
		return (NULL);

	switch (param) {
	case 0:
		{
			return (dataPtr);
		}
	case 1:
		{
			return (dataPtr + ptr->offsetToSubData3);	// strings
		}
	case 2:
		{
			return (dataPtr + ptr->offsetToSubData2);
		}
	case 3:
		{
			return (dataPtr + ptr->offsetToImportData);	// import data
		}
	case 4:
		{
			return (dataPtr + ptr->offsetToImportName);	// import names
		}
	case 5:
		{
			return (dataPtr + ptr->offsetToSubData5);
		}
	default:
		{
			return (NULL);
		}
	}
}

void printInfoBlackBox(const char *string) {
}

void waitForPlayerInput() {
}

void getFileExtention(const char *name, char *buffer) {
	while (*name != '.' && *name) {
		name++;
	}

	strcpy(buffer, name);
}

void removeExtention(const char *name, char *buffer) {	// not like in original
	char *ptr;

	strcpy(buffer, name);

	ptr = strchr(buffer, '.');

	if (ptr)
		*ptr = 0;
}

int lastFileSize;

int loadFileSub1(uint8 **ptr, uint8 *name, uint8 *ptr2) {
	int i;
	char buffer[256];
	int fileIdx;
	int unpackedSize;
	uint8 *unpackedBuffer;

	for (i = 0; i < 64; i++) {
		if (mediumVar[i].ptr) {
			if (!strcmpuint8(mediumVar[i].name, name)) {
				printf("Unsupported code in loadFIleSub1 !\n");
				exit(1);
			}
		}
	}

	getFileExtention((char *)name, buffer);

	if (!strcmp(buffer, ".SPL")) {
		removeExtention((char *)name, buffer);

		// if (useH32)
		{
			strcatuint8(buffer, ".H32");
		}
		/* else
		 * if (useAdlib)
		 * {
		 * strcatuint8(buffer,".ADL");
		 * }
		 * else
		 * {
		 * strcatuint8(buffer,".HP");
		 * } */
	} else {
		strcpyuint8(buffer, name);
	}

	fileIdx = findFileInDisks((uint8 *) buffer);

	if (fileIdx < 0)
		return (-18);

	unpackedSize = loadFileVar1 =
	    volumePtrToFileDescriptor[fileIdx].extSize + 2;

	// TODO: here, can unpack in gfx module buffer
	unpackedBuffer = (uint8 *) mallocAndZero(unpackedSize);

	if (!unpackedBuffer) {
		return (-2);
	}

	lastFileSize = unpackedSize;

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		uint8 *pakedBuffer =
		    (uint8 *) mallocAndZero(volumePtrToFileDescriptor[fileIdx].
		    size + 2);

		loadPakedFileToMem(fileIdx, pakedBuffer);

		uint32 realUnpackedSize = READ_BE_UINT32(pakedBuffer + volumePtrToFileDescriptor[fileIdx].size - 4);

		lastFileSize = realUnpackedSize;

		delphineUnpack(unpackedBuffer, pakedBuffer, volumePtrToFileDescriptor[fileIdx].size);

		free(pakedBuffer);
	} else {
		loadPakedFileToMem(fileIdx, unpackedBuffer);
	}

	*ptr = unpackedBuffer;

	return (1);
}

void resetFileEntry(int32 entryNumber) {
	if (entryNumber >= 257)
		return;

	if (!filesDatabase[entryNumber].subData.ptr)
		return;

	free(filesDatabase[entryNumber].subData.ptr);

	filesDatabase[entryNumber].subData.ptr = NULL;
	filesDatabase[entryNumber].subData.ptr2 = NULL;
	filesDatabase[entryNumber].widthInColumn = 0;
	filesDatabase[entryNumber].width = 0;
	filesDatabase[entryNumber].resType = 0;
	filesDatabase[entryNumber].height = 0;
	filesDatabase[entryNumber].subData.index = -1;
	filesDatabase[entryNumber].subData.resourceType = 0;
	filesDatabase[entryNumber].subData.field_1C = 0;
	filesDatabase[entryNumber].subData.name[0] = 0;

}

uint8 *mainProc14(uint16 overlay, uint16 idx) {
	ASSERT(0);

	return NULL;
}

int initAllData(void) {
	int i;

	setupFuncArray();
	setupOpcodeTable();
	initOverlayTable();

	setup1 = 0;
	currentActiveBackgroundPlane = 0;

	freeDisk();

	initVar5[0] = -1;
	initVar5[3] = -1;
	initVar5[6] = -1;
	initVar5[9] = -1;

	menuTable[0] = NULL;

	for (i = 0; i < 2000; i++) {
		globalVars[i] = 0;
	}

	for (i = 0; i < 8; i++) {
		backgroundTable[i].name[0] = 0;
	}

	for (i = 0; i < 257; i++) {
		filesDatabase[i].subData.ptr = NULL;
		filesDatabase[i].subData.ptr2 = NULL;
	}

	initBigVar3();

	resetPtr2(&procHead);
	resetPtr2(&relHead);

	resetPtr(&cellHead);

	resetActorPtr(&actorHead);
	resetBackgroundIncrustList(&backgroundIncrustHead);

	bootOverlayNumber = loadOverlay((const uint8 *) "AUTO00");

#ifdef DUMP_SCRIPT
	loadOverlay("TITRE");
	loadOverlay("TOM");
	loadOverlay("XX2");
	loadOverlay("SUPER");
	loadOverlay("BEBE1");
	loadOverlay("BIBLIO");
	loadOverlay("BRACAGE");
	loadOverlay("CONVERS");
	loadOverlay("DAF");
	loadOverlay("DAPHNEE");
	loadOverlay("DESIRE");
	loadOverlay("FAB");
	loadOverlay("FABIANI");
	loadOverlay("FIN");
	loadOverlay("FIN01");
	loadOverlay("FINBRAC");
	loadOverlay("GEN");
	loadOverlay("GENDEB");
	loadOverlay("GIFLE");
	loadOverlay("HECTOR");
	loadOverlay("HECTOR2");
	loadOverlay("I00");
	loadOverlay("I01");
	loadOverlay("I04");
	loadOverlay("I06");
	loadOverlay("I07");
	loadOverlay("INVENT");
	loadOverlay("JULIO");
	loadOverlay("LOGO");
	loadOverlay("MANOIR");
	loadOverlay("MISSEL");
	loadOverlay("POKER");
	loadOverlay("PROJ");
	loadOverlay("REB");
	loadOverlay("REBECCA");
	loadOverlay("ROS");
	loadOverlay("ROSE");
	loadOverlay("S01");
	loadOverlay("S02");
	loadOverlay("S03");
	loadOverlay("S04");
	loadOverlay("S06");
	loadOverlay("S07");
	loadOverlay("S08");
	loadOverlay("S09");
	loadOverlay("S10");
	loadOverlay("S103");
	loadOverlay("S11");
	loadOverlay("S113");
	loadOverlay("S12");
	loadOverlay("S129");
	loadOverlay("S131");
	loadOverlay("S132");
	loadOverlay("S133");
	loadOverlay("int16");
	loadOverlay("S17");
	loadOverlay("S18");
	loadOverlay("S19");
	loadOverlay("S20");
	loadOverlay("S21");
	loadOverlay("S22");
	loadOverlay("S23");
	loadOverlay("S24");
	loadOverlay("S25");
	loadOverlay("S26");
	loadOverlay("S27");
	loadOverlay("S29");
	loadOverlay("S30");
	loadOverlay("S31");
	loadOverlay("int32");
	loadOverlay("S33");
	loadOverlay("S33B");
	loadOverlay("S34");
	loadOverlay("S35");
	loadOverlay("S36");
	loadOverlay("S37");
	loadOverlay("SHIP");
	loadOverlay("SUPER");
	loadOverlay("SUZAN");
	loadOverlay("SUZAN2");
	loadOverlay("TESTA1");
	loadOverlay("TESTA2");
	//exit(1);
#endif

	if (bootOverlayNumber) {
		positionInStack = 0;

		attacheNewScriptToTail(bootOverlayNumber, &procHead, 0, 20, 0, 0, scriptType_PROC);
		scriptFunc2(bootOverlayNumber, &procHead, 1, 0);
	}

	strcpyuint8(systemStrings.bootScriptName, "AUTO00");

	return (bootOverlayNumber);
}

int removeFinishedScripts(scriptInstanceStruct *ptrHandle) {
	scriptInstanceStruct *ptr = ptrHandle->nextScriptPtr;	// can't destruct the head
	scriptInstanceStruct *oldPtr = ptrHandle;

	if (!ptr)
		return (0);

	do {
		if (ptr->scriptNumber == -1) {
			oldPtr->nextScriptPtr = ptr->nextScriptPtr;

			if (ptr->var6 && ptr->varA) {
				//  free(ptr->var6);
			}

			free(ptr);

			ptr = oldPtr->nextScriptPtr;
		} else {
			oldPtr = ptr;
			ptr = ptr->nextScriptPtr;
		}
	} while (ptr);

	return (0);
}

int buttonDown;
int selectDown = 0;
int menuDown = 0;

int findObject(int mouseX, int mouseY, int *outObjOvl, int *outObjIdx)
{
	char objectName[80];

	cellStruct *currentObject = cellHead.prev;

	while (currentObject)
	{
		if (currentObject->overlay >= 0 && overlayTable[currentObject->overlay].alreadyLoaded && (currentObject->type == OBJ_TYPE_SPRITE || currentObject->type == OBJ_TYPE_MASK || currentObject->type == OBJ_TYPE_EXIT || currentObject->type == OBJ_TYPE_VIRTUEL))
		{
			char* pObjectName = getObjectName(currentObject->idx, overlayTable[currentObject->overlay].ovlData->arrayNameObj);
			if(pObjectName)
			{
				strcpy(objectName, pObjectName);

				if (strlen(objectName) && (currentObject->freeze == 0))
				{
					int objIdx = currentObject->idx;
					int objOvl = currentObject->overlay;
					int linkedObjIdx = currentObject->followObjectIdx;
					int linkedObjOvl = currentObject->followObjectOverlayIdx;

					objectParamsQuery params;
					getMultipleObjectParam(objOvl, objIdx, &params);

					int x2 = 0;
					int y2 = 0;
					int j2 = 0;

					if ((objOvl != linkedObjOvl) || (objIdx != linkedObjIdx))
					{
						getMultipleObjectParam(linkedObjOvl, linkedObjIdx, &params);

						x2 = params.X;
						y2 = params.Y;
						j2 = params.fileIdx;
					}

					if (params.var5 >= 0 && params.fileIdx >= 0)
					{
						if (currentObject->type == OBJ_TYPE_SPRITE || currentObject->type == OBJ_TYPE_MASK || currentObject->type == OBJ_TYPE_EXIT)
						{
							int x = params.X + x2;
							int y = params.Y + y2;
							int j = params.fileIdx;

							if (j >= 0) {
								j += j2;
							}

							/*if ((filesDatabase[j].subData.resourceType == OBJ_TYPE_POLY) && (filesDatabase[j].subData.ptr)) {
								ASSERT(0);
							}
							else*/
							{
								int numBitPlanes = filesDatabase[j].resType;

								int nWidth;
								int nHeight;

								if (numBitPlanes == 1)
								{
									nWidth = filesDatabase[j].widthInColumn / 2;
								} else {
									nWidth = filesDatabase[j].width;
								}

								nHeight = filesDatabase[j].height;

								int offsetX = mouseX - x;
								int offsetY = mouseY - y;

								if ((offsetX >= 0) && (offsetX < nWidth * 16) && (offsetY >= 0) && (nWidth <= nHeight) && filesDatabase[j].subData.ptr)
								{
									if (numBitPlanes == 1)
									{
									}
									else
									{
									}

									printf("should compare to mask in findObject...\n");

									*outObjOvl = objOvl;
									*outObjIdx = objIdx;

									printf("Selected: %s\n", objectName);

									return currentObject->type;
								}
							}
						}
						else if (currentObject->type == OBJ_TYPE_VIRTUEL)
						{
							int x = params.X + x2;
							int y = params.Y + y2;
							int width = params.fileIdx;
							int height = params.scale;

							if ((mouseX >= x) && (mouseX <= x+width) && (mouseY >= y) && (mouseY <= y+height))
							{
								*outObjOvl = objOvl;
								*outObjIdx = objIdx;

								return (currentObject->type);
							}
						}
					}
				}
			}
		}

		currentObject = currentObject->prev;
	}

	*outObjOvl = 0;
	*outObjIdx = 0;

	return -1;
}

char keyboardVar = 0;

void freeStuff2(void) {
	printf("implement freeStuff2\n");
}

void *allocAndZero(int size) {
	void *ptr;

	ptr = malloc(size);
	memset(ptr, 0, size);

	return ptr;
}

char *getObjectName(int index, uint8 *string) {
	int i;
	char *ptr = (char *)string;

	if (!string)
		return NULL;

	for (i = 0; i < index; i++) {
		while (*ptr) {
			ptr++;
		}
		ptr++;
	}
	return ptr;
}

int buildInventorySub1(int overlayIdx, int objIdx) {
	objDataStruct *pObjectData =
	    getObjectDataFromOverlay(overlayIdx, objIdx);

	if (pObjectData) {
		return pObjectData->type;
	} else {
		return -11;
	}
}

void buildInventory(int X, int Y) {
	int numObjectInInventory = 0;
	menuStruct *pMenu;

	pMenu = createMenu(X, Y, "Inventaire");
	menuTable[1] = pMenu;

	if (pMenu && numOfLoadedOverlay > 1) {
		for (int i = 1; i < numOfLoadedOverlay; i++) {
			ovlDataStruct *pOvlData = overlayTable[i].ovlData;

			if (pOvlData && pOvlData->arrayObject && pOvlData->numObj) {
				for (int j = 0; j < pOvlData->numObj; j++) {
					if (buildInventorySub1(i, j) != 3) {
						int16 returnVar;

						getSingleObjectParam(i, j, 5, &returnVar);

						if (returnVar < -1) {
							addSelectableMenuEntry(i, j, pMenu, 1, -1, getObjectName(j, pOvlData->arrayNameObj));
							numObjectInInventory++;
						}
					}
				}
			}
		}
	}

	if (numObjectInInventory == 0) {
		freeMenu(menuTable[1]);
		menuTable[1] = NULL;
	}
}

int currentMenuElementX;
int currentMenuElementY;
menuElementStruct *currentMenuElement;

menuElementSubStruct *getSelectedEntryInMenu(menuStruct *pMenu) {
	menuElementStruct *pMenuElement;

	if (pMenu == NULL) {
		return NULL;
	}

	if (pMenu->numElements == 0) {
		return NULL;
	}

	pMenuElement = pMenu->ptrNextElement;

	while (pMenuElement) {
		if (pMenuElement->varC) {
			currentMenuElementX = pMenuElement->x;
			currentMenuElementY = pMenuElement->y;
			currentMenuElement = pMenuElement;

			return pMenuElement->ptrSub;
		}

		pMenuElement = pMenuElement->next;
	}

	return NULL;
}

bool findRelation(int objOvl, int objIdx, int x, int y) {
	bool found = false;
	bool first = true;
	int testState;
	int j;
	int16 objectState;

	testState = -1;

	getSingleObjectParam(objOvl, objIdx, 5, &objectState);

	for (j = 1; j < numOfLoadedOverlay; j++)
	{
		if (overlayTable[j].alreadyLoaded)
		{
			int idHeader = overlayTable[j].ovlData->numMsgRelHeader;

			int i;
			for(i=0; i<idHeader; i++)
			{
				linkDataStruct* ptrHead = &overlayTable[j].ovlData->arrayMsgRelHeader[i];
				int thisOvl = ptrHead->obj1Overlay;

				if (!thisOvl) {
					thisOvl = j;
				}

				objDataStruct* pObject = getObjectDataFromOverlay(thisOvl, ptrHead->obj1Number);

				if ((thisOvl == objOvl) && (objIdx ==ptrHead->obj1Number) && pObject && pObject->type != 3)
				{
					int verbeOvl = ptrHead->verbOverlay;
					int obj1Ovl = ptrHead->obj1Overlay;
					int obj2Ovl = ptrHead->obj2Overlay;

					if (!verbeOvl) verbeOvl=j;
					if (!obj1Ovl)  obj1Ovl=j;
					if (!obj2Ovl)  obj2Ovl=j;
					
					char verbe_name[80];
					char obj1_name[80];
					char obj2_name[80];
					char r_verbe_name[80];
					char r_obj1_name[80];
					char r_obj2_name[80];

					verbe_name[0]	=0;
					obj1_name[0]	=0;
					obj2_name[0]	=0;
					r_verbe_name[0] =0;
					r_obj1_name[0]	=0;
					r_obj2_name[0]	=0;

					ovlDataStruct *ovl2 = NULL;
					ovlDataStruct *ovl3 = NULL;
					ovlDataStruct *ovl4 = NULL;

					if(verbeOvl > 0)
						ovl2 = overlayTable[verbeOvl].ovlData;

					if(obj1Ovl > 0)
						ovl3 = overlayTable[obj1Ovl].ovlData;

					if(obj2Ovl > 0)
						ovl4 = overlayTable[obj2Ovl].ovlData;

					if((ovl3) && (ptrHead->obj1Number >= 0))
					{
						testState = ptrHead->field_1A;

						if((first) && (ovl3->arrayNameObj) && ((testState ==-1) || (testState == objectState)))
						{
							char *ptrName = getObjectName(ptrHead->obj1Number, ovl3->arrayNameObj);

							menuTable[0] = createMenu(x, y, ptrName);
							first = false;
						}
					}
					if((ovl2) && (ptrHead->verbNumber))
					{
						if(ovl2->nameVerbGlob)
						{
							char *ptr = getObjectName(ptrHead->verbNumber, ovl2->nameVerbGlob);
							strcpy(verbe_name, ptr);

							if( (!first) && ((testState==-1) || (testState==objectState)))
							{
								if(!strlen(verbe_name))
									attacheNewScriptToTail(j, &relHead, ptrHead->id, 30, currentScriptPtr->scriptNumber, currentScriptPtr->overlayNumber, scriptType_REL);
								else if(ovl2->nameVerbGlob)
								{
									found = true;
									ptr = getObjectName(ptrHead->verbNumber, ovl2->nameVerbGlob);
									addSelectableMenuEntry(i, j, menuTable[0], 1, -1, ptr);
								}
							}
						}
					}
				}
			}
		}
	}

	return found;
}

int processInventory(void) {
	if (menuTable[1]) {
		menuElementSubStruct *pMenuElementSub =
		    getSelectedEntryInMenu(menuTable[1]);

		if (pMenuElementSub) {
			//int var2;
			//int var4;

			var2 = pMenuElementSub->var2;
			var4 = pMenuElementSub->var4;

			freeMenu(menuTable[1]);
			menuTable[1] = NULL;

			findRelation(var2, var4, currentMenuElementX + 80, currentMenuElementY);

			return 1;
		} else {
			freeMenu(menuTable[1]);
			menuTable[1] = NULL;
		}
	}

	return 0;
}

int processInput(void)
{
	int16 mouseX = 0;
	int16 mouseY = 0;
	int16 button = 0;

	/*if (inputSub1keyboad())
	 * {
	 * return 1;
	 * } */

	button = 0;

	if (sysKey != -1)
	{
		button = sysKey;
		mouseX = sysX;
		mouseY = sysY;
		sysKey = -1;
	}
	else if (automaticMode == 0)
	{
		getMouseStatus(&main10, &mouseX, &button, &mouseY);
	}

	if (!button)
	{
		buttonDown = 0;
	}

	if (userDelay) {
		userDelay--;
		return 0;
	}

	// test both buttons
	if (((button & 3) == 3) || keyboardVar == 0x44 || keyboardVar == 0x53)
	{
		changeCursor(CURSOR_NORMAL);
		keyboardVar = 0;
		return (playerMenu(mouseX, mouseY));
	}

	if (!userEnabled) {
		return 0;
	}

	if ((currentActiveMenu != -1) && menuTable[currentActiveMenu])
	{
		updateMenuMouse(mouseX, mouseY, menuTable[currentActiveMenu]);
	}

	if (dialogueEnabled)
	{
		ASSERT(0);
	}
	else
	{
		// not in dialogue

		// left click
		if ((button & 1) && (buttonDown == 0))
		{
			buttonDown = 1;

			// is there a relation
			if (linkedRelation)
			{
				ASSERT(0);
			}
			else
			{
				// manage click on object menu
				if (menuDown == 0)
				{
					// Handle left click on an object
					if (menuTable[0] == 0)
					{
						int objOvl;
						int objIdx;
						int objType;

						objType = findObject(mouseX, mouseY, &objOvl, &objIdx);

						if (objType != -1)
						{
							int relation = findRelation(objOvl, objIdx, mouseX, mouseY);
							if(menuTable[0])
							{
								if(relation)
								{
									currentActiveMenu = 0;
									selectDown = 1;
								}
								else
								{
									// object has a name but no relation, just move the character
									freeMenu(menuTable[0]);
									menuTable[0] = NULL;

									aniX = mouseX;
									aniY = mouseY;
									animationStart = true;
									buttonDown = 0;
								}
							}
							else
							{
								aniX = mouseX;
								aniY = mouseY;
								animationStart = true;
								buttonDown = 0;
							}
						}else {
							// No object found, we move the character to the cursor
							aniX = mouseX;
							aniY = mouseY;
							animationStart = true;
							buttonDown = 0;
						}
					}
					else
					{
						aniX = mouseX;
						aniY = mouseY;
						animationStart = true;
						buttonDown = 0;
					}
				}
				else
				{
					// Handle left click in inventory
					if (processInventory())
					{
						currentActiveMenu = 0;
						selectDown = 1;
						menuDown = 0;
					} else {
						currentActiveMenu = -1;
						menuDown = 0;
					}
				}
			}
		}
		// test right button
		else if ((button & 2) || (keyboardVar == 0x43) || (keyboardVar == 0x52))
		{
			if (buttonDown == 0)
			{
				keyboardVar = 0;

				// close object menu if there is no linked relation
				if ((linkedRelation == 0) && (menuTable[0])) {
					freeMenu(menuTable[0]);
					menuTable[0] = NULL;
					selectDown = 0;
					menuDown = 0;
					currentActiveMenu = -1;
				}

				if ((!selectDown) && (!menuDown) && (menuTable[1] == NULL))
				{
					buildInventory(mouseX, mouseY);

					if (menuTable[1]) {
						currentActiveMenu = 1;
						menuDown = 1;
					} else {
						menuDown = 1;
					}
				}
				buttonDown = 1;
			}
		}
	}
	return 0;
}

int currentMouseX = 0;
int currentMouseY = 0;
int currentMouseButton = 0;

void getMouseStatus(int16 *pMouseVar, int16 *pMouseX, int16 *pMouseButton, int16 *pMouseY)
{
	*pMouseX = currentMouseX;
	*pMouseY = currentMouseY;
	*pMouseButton = currentMouseButton;
}

bool bFastMode = false;

void manageEvents() {
	Common::Event event;

	Common::EventManager * eventMan = g_system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				currentMouseButton |= 1;
				break;
			case Common::EVENT_LBUTTONUP:
				currentMouseButton &= ~1;
				break;
			case Common::EVENT_RBUTTONDOWN:
				currentMouseButton |= 2;
				break;
			case Common::EVENT_RBUTTONUP:
				currentMouseButton &= ~2;
				break;
			case Common::EVENT_MOUSEMOVE:
				currentMouseX = event.mouse.x;
				currentMouseY = event.mouse.y;
				break;
		case Common::EVENT_QUIT:
			g_system->quit();
			break;
		case Common::EVENT_KEYUP:
			switch(event.kbd.keycode)
			{
				case 27: // ESC
					currentMouseButton &= ~4;
					break;
				default:
					break;
			}
			break;
	    case Common::EVENT_KEYDOWN:
			switch(event.kbd.keycode)
			{
				case 27: // ESC
					currentMouseButton |= 4;
					break;
				default:
					break;
			}

			/*
			 * switch (event.kbd.keycode) {
			 * case '\n':
			 * case '\r':
			 * case 261: // Keypad 5
			 * if (allowPlayerInput) {
			 * mouseLeft = 1;
			 * }
			 * break;
			 * case 27:  // ESC
			 * if (allowPlayerInput) {
			 * mouseRight = 1;
			 * }
			 * break;
			 * case 282: // F1
			 * if (allowPlayerInput) {
			 * playerCommand = 0; // EXAMINE
			 * makeCommandLine();
			 * }
			 * break;
			 * case 283: // F2
			 * if (allowPlayerInput) {
			 * playerCommand = 1; // TAKE
			 * makeCommandLine();
			 * }
			 * break;
			 * case 284: // F3
			 * if (allowPlayerInput) {
			 * playerCommand = 2; // INVENTORY
			 * makeCommandLine();
			 * }
			 * break;
			 * case 285: // F4
			 * if (allowPlayerInput) {
			 * playerCommand = 3; // USE
			 * makeCommandLine();
			 * }
			 * break;
			 * case 286: // F5
			 * if (allowPlayerInput) {
			 * playerCommand = 4; // ACTIVATE
			 * makeCommandLine();
			 * }
			 * break;
			 * case 287: // F6
			 * if (allowPlayerInput) {
			 * playerCommand = 5; // SPEAK
			 * makeCommandLine();
			 * }
			 * break;
			 * case 290: // F9
			 * if (allowPlayerInput && !inMenu) {
			 * makeActionMenu();
			 * makeCommandLine();
			 * }
			 * break;
			 * case 291: // F10
			 * if (!disableSystemMenu && !inMenu) {
			 * g_cine->makeSystemMenu();
			 * }
			 * break;
			 * default:
			 * //lastKeyStroke = event.kbd.keycode;
			 * break;
			 * }
			 * break; */
			if (event.kbd.flags == Common::KBD_CTRL)
			{
				if (event.kbd.keycode == Common::KEYCODE_d)
				{
					// enable debugging stuff ?
				}
				else if (event.kbd.keycode == Common::KEYCODE_f)
				{
					bFastMode = !bFastMode;
				}
			}

		default:
			break;
		}
	}

	/*if (count) {
	 * mouseData.left = mouseLeft;
	 * mouseData.right = mouseRight;
	 * mouseLeft = 0;
	 * mouseRight = 0;
	 * }
	 */
	g_system->updateScreen();

	if(!bFastMode)
	{
		g_system->delayMillis(40);
	}
}

void mainLoop(void) {
	int frames = 0;		/* Number of frames displayed */
	//int32 t_start,t_left;
	//uint32 t_end;
	//int32 q=0;                     /* Dummy */

	int enableUser = 0;

	scriptNameBuffer[0] = 0;
	systemStrings.bootScriptName[0] = 0;
	initVar4[0] = 0;
	currentActiveMenu = -1;
	main14 = -1;
	linkedRelation = 0;
	main21 = 0;
	main22 = 0;
	main7 = 0;
	main8 = 0;
	main15 = 0;

	if (initAllData()) {
		int playerDontAskQuit = 1;
		int quitValue2 = 1;
		int quitValue = 0;

		do {
			frames++;
//      t_start=Osystem_GetTicks();

//      readKeyboard();
			playerDontAskQuit = processInput();

			//if (enableUser)
			{
				userEnabled = 1;
				enableUser = 0;
			}

			manageScripts(&relHead);
			manageScripts(&procHead);

			removeFinishedScripts(&relHead);
			removeFinishedScripts(&procHead);

			processAnimation();

			if (var0) {
				// ASSERT(0);
				/*    main3 = 0;
				 * var24 = 0;
				 * var23 = 0;
				 *
				 * freeStuff2(); */
			}

			if (initVar4[0]) {
				ASSERT(0);
/*        redrawStrings(0,&initVar4,8);

        waitForPlayerInput();

        initVar4 = 0; */
			}

			if (affichePasMenuJoueur)
			{
				if (main5)
					fadeVar = 0;

				/*if (fadeVar)
				 * {
				 * //  TODO!
				 * } */

				mainDraw(0);
				flipScreen();

				if (userEnabled && !main7 && !main15)
				{
					if(currentActiveMenu == -1)
					{
						int16 mouseX;
						int16 mouseY;
						int16 mouseButton;

						static int16 oldMouseX = -1;
						static int16 oldMouseY = -1;

						getMouseStatus(&main10, &mouseX, &mouseButton, &mouseY);
					 
						if (mouseX != oldMouseX && mouseY != oldMouseY)
						{
							int objectType;
							int newCursor1;
							int newCursor2;
							
							oldMouseX = mouseX;
							oldMouseY = mouseY;
							
							objectType = findObject(mouseX, mouseY, &newCursor1, &newCursor2);
							
							if (objectType == 9)
							{
								changeCursor(CURSOR_EXIT);
							}
							else
							if (objectType != -1)
							{
								changeCursor(CURSOR_MAGNIFYING_GLASS);
							}
							else
							{
								changeCursor(CURSOR_WALK);
							}
						}
					}
					else
					{
						changeCursor(CURSOR_NORMAL);
					}
				}
				else
				{
					changeCursor(CURSOR_NORMAL);
				}

				if (main7) {
					ASSERT(0);
				}

				if (main15) {
					ASSERT(0);
				}

				if (main14 != -1) {
					ASSERT(0);
				}
			}
			// t_end = t_start+SPEED;
//      t_left=t_start-Osystem_GetTicks()+SPEED;
#ifndef FASTDEBUG
			/*    if (t_left>0)
			 * if (t_left>SLEEP_MIN)
			 * Osystem_Delay(t_left-SLEEP_GRAN);
			 * while (Osystem_GetTicks()<t_end){q++;}; */
#endif
			manageEvents();

		} while (!playerDontAskQuit && quitValue2 && quitValue != 7);
	}

}

int oldmain(int argc, char *argv[]) {
	printf("Cruise for a corpse recode\n");

//  OSystemInit();
//  osystem = new OSystem;

	printf("Osystem Initialized\n");

	printf("Initializing engine...\n");

//  initBuffer(scaledScreen,640,400);

	fadeVar = 0;

	//lowLevelInit();

	// arg parser stuff

	ptr_something =
	    (ctpVar19Struct *) mallocAndZero(sizeof(ctpVar19Struct) * 0x200);

	/*volVar1 = 0;
	 * fileData1 = 0; */

	/*PAL_fileHandle = -1; */

	// video init stuff

	loadSystemFont();

	// another bit of video init

	if (!readVolCnf()) {
		printf("Fatal: unable to load vol.cnf !\n");
		return (-1);
	}

	printf("Entering main loop...\n");
	mainLoop();

	//freeStuff();

	//freePtr(ptr_something);

	return (0);
}

void *mallocAndZero(int32 size) {
	void *ptr;

	ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

} // End of namespace Cruise
