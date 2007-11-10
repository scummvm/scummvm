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

overlayStruct overlayTable[90];
int numOfLoadedOverlay;

void initOverlayTable(void) {
	int i;

	for (i = 0; i < 90; i++) {
		overlayTable[i].overlayName[0] = 0;
		overlayTable[i].ovlData = NULL;
		overlayTable[i].alreadyLoaded = 0;
		overlayTable[i].executeScripts = 0;
	}

	numOfLoadedOverlay = 1;
}

int loadOverlay(const char *scriptName) {
	int newNumberOfScript;
	bool scriptNotLoadedBefore;
	int scriptIdx;
	char fileName[50];
	int fileIdx;
	int unpackedSize;
	char *unpackedBuffer;
	char *scriptPtr;
	ovlDataStruct *ovlData;

	printf("Load overlay: %s\n", scriptName);

	newNumberOfScript = numOfLoadedOverlay;

	scriptNotLoadedBefore = false;

	scriptIdx = findOverlayByName((const char *)scriptName);

	if (scriptIdx == -4) {
		scriptIdx = numOfLoadedOverlay;

		newNumberOfScript++;

		scriptNotLoadedBefore = true;
	}

	if (overlayTable[scriptIdx].alreadyLoaded) {
		return (scriptIdx);
	}

	overlayTable[scriptIdx].ovlData =
	    (ovlDataStruct *) mallocAndZero(sizeof(ovlDataStruct));

	if (!overlayTable[scriptIdx].ovlData)
		return (-2);

	strcpy(overlayTable[scriptIdx].overlayName, scriptName);

	overlayTable[scriptIdx].alreadyLoaded = 1;

	numOfLoadedOverlay = newNumberOfScript;

	overlayTable[scriptIdx].ovlData->scriptNumber = scriptIdx;

	strcpy(fileName, scriptName);

	strcat(fileName, ".OVL");

	printf("Attempting to load overlay file %s...\n", fileName);

	fileIdx = findFileInDisks(fileName);

	if (fileIdx < 0) {
		printf("Unable to load overlay %s !\n", scriptName);
		//releaseScript(scriptName);
		return (-18);
	}

	unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;

	// TODO: here, can unpack in gfx module buffer
	unpackedBuffer = (char *)mallocAndZero(unpackedSize);

	if (!unpackedBuffer) {
		return (-2);
	}

	if (volumePtrToFileDescriptor[fileIdx].size + 2 != unpackedSize) {
		char *pakedBuffer =
		    (char *)mallocAndZero(volumePtrToFileDescriptor[fileIdx].
		    size + 2);

		loadPakedFileToMem(fileIdx, (uint8 *) pakedBuffer);

		delphineUnpack((uint8 *)unpackedBuffer, (const uint8 *)pakedBuffer, volumePtrToFileDescriptor[fileIdx].size);

		free(pakedBuffer);
	} else {
		loadPakedFileToMem(fileIdx, (uint8 *) unpackedBuffer);
	}

	printf("OVL loading done...\n");

	scriptPtr = unpackedBuffer;

	ovlData = overlayTable[scriptIdx].ovlData;

	memcpy(ovlData, scriptPtr, sizeof(ovlDataStruct));

	ovlData->arrayProc = NULL;
	ovlData->ptr1 = NULL;
	ovlData->arrayObject = NULL;
	ovlData->arrayStates = NULL;
	ovlData->arrayObjVar = NULL;
	ovlData->stringTable = NULL;
	ovlData->arraySymbGlob = NULL;
	ovlData->arrayRelocGlob = NULL;
	ovlData->arrayMsgRelHeader = NULL;
	ovlData->nameVerbGlob = NULL;
	ovlData->arrayNameObj = NULL;
	ovlData->arrayNameRelocGlob = NULL;
	ovlData->arrayNameSymbGlob = NULL;
	ovlData->data4Ptr = NULL;
	ovlData->ptr8 = NULL;
	ovlData->numProc = readB16(scriptPtr + 60);
	ovlData->numRel = readB16(scriptPtr + 62);
	ovlData->numSymbGlob = readB16(scriptPtr + 64);
	ovlData->numRelocGlob = readB16(scriptPtr + 66);
	ovlData->numMsgRelHeader = readB16(scriptPtr + 68);
	ovlData->numObj = readB16(scriptPtr + 70);
	ovlData->numStrings = readB16(scriptPtr + 72);
	ovlData->size8 = readB16(scriptPtr + 74);
	ovlData->size9 = readB16(scriptPtr + 76);
	ovlData->nameExportSize = readB16(scriptPtr + 78);
	ovlData->exportNamesSize = readB16(scriptPtr + 80);
	ovlData->specialString2Length = readB16(scriptPtr + 82);
	ovlData->sizeOfData4 = readB16(scriptPtr + 84);
	ovlData->size12 = readB16(scriptPtr + 86);
	ovlData->specialString1Length = readB16(scriptPtr + 88);
	ovlData->scriptNumber = readB16(scriptPtr + 90);

	scriptPtr += 92;

	if (ovlData->numSymbGlob)	{ // export data
		int i;
		ovlData->arraySymbGlob =
		    (exportEntryStruct *) mallocAndZero(ovlData->numSymbGlob *
		    sizeof(exportEntryStruct));

		if (!ovlData->arraySymbGlob) {
			return (-2);
		}

		for (i = 0; i < ovlData->numSymbGlob; i++) {
			ovlData->arraySymbGlob[i].var0 = readB16(scriptPtr);
			ovlData->arraySymbGlob[i].var2 =
			    readB16(scriptPtr + 2);
			ovlData->arraySymbGlob[i].var4 =
			    readB16(scriptPtr + 4);
			ovlData->arraySymbGlob[i].idx = readB16(scriptPtr + 6);
			ovlData->arraySymbGlob[i].offsetToName =
			    readB16(scriptPtr + 8);

			scriptPtr += 10;
		}
	}

	if (ovlData->exportNamesSize) {	// export names
		ovlData->arrayNameSymbGlob = (char *) mallocAndZero(ovlData->exportNamesSize);

		if (!ovlData->arrayNameSymbGlob) {
			return (-2);
		}

		memcpy(ovlData->arrayNameSymbGlob, scriptPtr, ovlData->exportNamesSize);
		scriptPtr += ovlData->exportNamesSize;
	}

	if (ovlData->numRelocGlob) {	// import data
		int i;

		ovlData->arrayRelocGlob =
		    (importDataStruct *) mallocAndZero(ovlData->numRelocGlob *
		    sizeof(importDataStruct));

		if (!ovlData->arrayRelocGlob) {
			return (-2);
		}

		for (i = 0; i < ovlData->numRelocGlob; i++) {
			ovlData->arrayRelocGlob[i].var0 = readB16(scriptPtr);
			ovlData->arrayRelocGlob[i].var1 =
			    readB16(scriptPtr + 2);
			ovlData->arrayRelocGlob[i].linkType =
			    readB16(scriptPtr + 4);
			ovlData->arrayRelocGlob[i].linkIdx =
			    readB16(scriptPtr + 6);
			ovlData->arrayRelocGlob[i].nameOffset =
			    readB16(scriptPtr + 8);

			scriptPtr += 10;
		}
	}

	if (ovlData->nameExportSize) {	// import name
		ovlData->arrayNameRelocGlob = (char *) mallocAndZero(ovlData->nameExportSize);

		if (!ovlData->arrayNameRelocGlob) {
			return (-2);
		}

		memcpy(ovlData->arrayNameRelocGlob, scriptPtr,
		    ovlData->nameExportSize);
		scriptPtr += ovlData->nameExportSize;
	}

	if (ovlData->numMsgRelHeader) {	// link data
		ASSERT(sizeof(linkDataStruct) == 0x22);

		ovlData->arrayMsgRelHeader =
		    (linkDataStruct *) mallocAndZero(ovlData->numMsgRelHeader *
		    sizeof(linkDataStruct));

		if (!ovlData->arrayMsgRelHeader) {
			return (-2);
		}

		memcpy(ovlData->arrayMsgRelHeader, scriptPtr,
		    ovlData->numMsgRelHeader * sizeof(linkDataStruct));
		scriptPtr += ovlData->numMsgRelHeader * sizeof(linkDataStruct);
		flipGen(ovlData->arrayMsgRelHeader,
		    ovlData->numMsgRelHeader * sizeof(linkDataStruct));
	}

	if (ovlData->numProc) {	// script
		ovlData3Struct *tempPtr;
		int i;

		ovlData->arrayProc =
		    (ovlData3Struct *) mallocAndZero(ovlData->numProc *
		    sizeof(ovlData3Struct));

		if (!ovlData->arrayProc) {
/*      releaseScript(scriptIdx,scriptName);

      if (freeIsNeeded) {
        freePtr(unpackedBuffer);
      } */

			return (-2);
		}

		memcpy(ovlData->arrayProc, scriptPtr,
		    ovlData->numProc * sizeof(ovlData3Struct));
		scriptPtr += ovlData->numProc * 0x1C;

		flipGen(ovlData->arrayProc,
		    ovlData->numProc * sizeof(ovlData3Struct));

		tempPtr = ovlData->arrayProc;

		for (i = 0; i < ovlData->numProc; i++) {
			uint8 *ptr = tempPtr->dataPtr =
			    (uint8 *) mallocAndZero(tempPtr->sizeOfData);

			if (!ptr) {
				/*      releaseScript(scriptIdx,scriptName);
				 * 
				 * if (freeIsNeeded)
				 * {
				 * freePtr(unpackedBuffer);
				 * } */

				return (-2);
			}

			memcpy(ptr, scriptPtr, tempPtr->sizeOfData);
			scriptPtr += tempPtr->sizeOfData;

			if (tempPtr->offsetToImportData) {
				flipGen(ptr + tempPtr->offsetToImportData,
				    tempPtr->numRelocGlob * 10);
			}

			if (tempPtr->offsetToSubData2) {
				flipGen(ptr + tempPtr->offsetToImportData,
				    tempPtr->subData2Size * 10);
			}

			tempPtr++;
		}
	}

	if (ovlData->numRel) {
		ovlData3Struct *tempPtr;
		int i;

		ovlData->ptr1 =
		    (uint8 *) mallocAndZero(ovlData->numRel * 0x1C);

		if (!ovlData->ptr1) {
			return (-2);
		}

		memcpy(ovlData->ptr1, scriptPtr, ovlData->numRel * 0x1C);
		scriptPtr += ovlData->numRel * 0x1C;
		flipGen(ovlData->ptr1, ovlData->numRel * 0x1C);

		tempPtr = (ovlData3Struct *) ovlData->ptr1;

		for (i = 0; i < ovlData->numRel; i++) {
			uint8 *ptr = tempPtr->dataPtr =
			    (uint8 *) mallocAndZero(tempPtr->sizeOfData);

			if (!ptr) {
				/*      releaseScript(scriptIdx,scriptName);
				 * 
				 * if (freeIsNeeded)
				 * {
				 * freePtr(unpackedBuffer);
				 * } */

				return (-2);
			}

			memcpy(ptr, scriptPtr, tempPtr->sizeOfData);
			scriptPtr += tempPtr->sizeOfData;

			if (tempPtr->offsetToImportData) {
				flipGen(ptr + tempPtr->offsetToImportData,
				    tempPtr->numRelocGlob * 10);
			}

			if (tempPtr->offsetToSubData2) {
				flipGen(ptr + tempPtr->offsetToImportData,
				    tempPtr->subData2Size * 10);
			}

			tempPtr++;
		}
	}

	if (ovlData->size12) {
		ovlData->ptr8 = (uint8 *) mallocAndZero(ovlData->size12);

		if (!ovlData->ptr8) {
/*      releaseScript(scriptIdx,scriptName);

      if (freeIsNeeded) {
        freePtr(unpackedBuffer);
      } */

			return (-2);
		}

		memcpy(ovlData->ptr8, scriptPtr, ovlData->size12);
		scriptPtr += ovlData->size12;
	}

	if (ovlData->numObj) {
		int i;
		ovlData->arrayObject =
		    (objDataStruct *) mallocAndZero(ovlData->numObj *
		    sizeof(objDataStruct));

		if (!ovlData->arrayObject) {
/*      releaseScript(scriptIdx,scriptName);

      if (freeIsNeeded) {
        freePtr(unpackedBuffer);
      } */

			return (-2);
		}

		for (i = 0; i < ovlData->numObj; i++) {
			ovlData->arrayObject[i].var0 = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].var0);

			ovlData->arrayObject[i].type = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].type);

			ovlData->arrayObject[i].var2 = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].var2);

			ovlData->arrayObject[i].var3 = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].var3);

			ovlData->arrayObject[i].var4 = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].var4);

			ovlData->arrayObject[i].var5 = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].var5);

			ovlData->arrayObject[i].stateTableIdx = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->arrayObject[i].stateTableIdx);
		}

		if (scriptNotLoadedBefore) {
			//int var1;
			//int var2;

			overlayTable[scriptIdx].state = (char)setup1;

			var1 = loadScriptSub1(scriptIdx, 3);
			var2 = loadScriptSub1(scriptIdx, 0);

			setup1 = var1 + var2;
		}
	}

	if (ovlData->size9) {
		ovlData->arrayObjVar =
		    (objectParams *) mallocAndZero(ovlData->size9 *
		    sizeof(objectParams));
		memset(ovlData->arrayObjVar, 0,
		    ovlData->size9 * sizeof(objectParams));

		if (!ovlData->arrayObjVar) {
/*      releaseScript(scriptIdx,scriptName);

      if (freeIsNeeded) {
        freePtr(unpackedBuffer);
      } */

			return (-2);
		}
	}

	if (ovlData->size8) {
		ovlData->arrayStates =
		    (objectParams *) mallocAndZero(ovlData->size8 *
		    sizeof(objectParams));

		if (!ovlData->arrayStates) {
/*      releaseScript(scriptIdx,scriptName);

      if (freeIsNeeded) {
        freePtr(unpackedBuffer);
      } */

			return (-2);
		}

		memcpy(ovlData->arrayStates, scriptPtr, ovlData->size8 * 12);	// TODO: made read item by item
		scriptPtr += ovlData->size8 * 12;
		flipGen(ovlData->arrayStates, ovlData->size8 * 12);
	}

	if (ovlData->numStrings) {
		int i;

		ovlData->stringTable =
		    (stringEntryStruct *) mallocAndZero(ovlData->numStrings *
		    sizeof(stringEntryStruct));

		for (i = 0; i < ovlData->numStrings; i++) {
			ovlData->stringTable[i].idx = *(int16 *) scriptPtr;
			flipShort(&ovlData->stringTable[i].idx);
			scriptPtr += 2;
		}
	}

/*  if (freeIsNeeded) {
    freePtr(unpackedBuffer);
  } */

	if (ovlData->sizeOfData4) {
		ovlData->data4Ptr =
		    (uint8 *) mallocAndZero(ovlData->sizeOfData4);
		memset(ovlData->data4Ptr, 0, ovlData->sizeOfData4);

		if (!ovlData->data4Ptr) {
			//releaseScript(scriptIdx,scriptName);
			return (-2);
		}
	}

	if (ovlData->
	    specialString1Length /*|| ovlData->specialString2Length */ 
	    || ovlData->stringTable) {
		int i;
		//int unpackedSize;
		//int fileIdx;
		//uint8 fileName[50];
		//char* unpackedBuffer;

		strcpy(fileName, scriptName);

		strcat(fileName, ".FR");

		fileIdx = findFileInDisks(fileName);

		if (fileIdx < 0) {
			//releaseScript(scriptName);
			return (-18);
		}

		unpackedSize = volumePtrToFileDescriptor[fileIdx].extSize + 2;

		// TODO: here, can unpack in gfx module buffer
		unpackedBuffer = (char *)mallocAndZero(unpackedSize);

		if (!unpackedBuffer) {
			return (-2);
		}

		if (volumePtrToFileDescriptor[fileIdx].size + 2 !=
		    unpackedSize) {
			char *pakedBuffer =
			    (char *)
			    mallocAndZero(volumePtrToFileDescriptor[fileIdx].
			    size + 2);

			loadPakedFileToMem(fileIdx, (uint8 *) pakedBuffer);

			delphineUnpack((uint8 *) unpackedBuffer, (const uint8 *)pakedBuffer, volumePtrToFileDescriptor[fileIdx].size);

			free(pakedBuffer);
		} else {
			loadPakedFileToMem(fileIdx, (uint8 *) unpackedBuffer);
		}

		scriptPtr = unpackedBuffer;

		memcpy(&ovlData->specialString1Length, scriptPtr, 2);
		scriptPtr += 2;
		flipShort(&ovlData->specialString1Length);	// recheck if needed

		if (ovlData->specialString1Length) {
			ovlData->nameVerbGlob = (char *) mallocAndZero(ovlData->specialString1Length);

			if (!ovlData->nameVerbGlob) {
				/*      releaseScript(scriptIdx,scriptName);
				 * 
				 * if (freeIsNeeded)
				 * {
				 * freePtr(unpackedBuffer);
				 * } */

				return (-2);
			}

			memcpy(ovlData->nameVerbGlob, scriptPtr,
			    ovlData->specialString1Length);
			scriptPtr += ovlData->specialString1Length;
		}

		memcpy(&ovlData->specialString2Length, scriptPtr, 2);
		scriptPtr += 2;
		flipShort(&ovlData->specialString2Length);	// recheck if needed

		if (ovlData->specialString2Length) {
			ovlData->arrayNameObj = (char *) mallocAndZero(ovlData->specialString2Length);

			if (!ovlData->arrayNameObj) {
				/*      releaseScript(scriptIdx,scriptName);
				 * 
				 * if (freeIsNeeded)
				 * {
				 * freePtr(unpackedBuffer);
				 * } */

				return (-2);
			}

			memcpy(ovlData->arrayNameObj, scriptPtr,
			    ovlData->specialString2Length);
			scriptPtr += ovlData->specialString2Length;
		}

		for (i = 0; i < ovlData->numStrings; i++) {
			ovlData->stringTable[i].length = *(int16 *) scriptPtr;
			scriptPtr += 2;
			flipShort(&ovlData->stringTable[i].length);

			if (ovlData->stringTable[i].length) {
				ovlData->stringTable[i].string =
				    (char *)mallocAndZero(ovlData->
				    stringTable[i].length);

				if (!ovlData->stringTable[i].string) {
					/*      releaseScript(scriptIdx,scriptName);
					 * 
					 * if (freeIsNeeded)
					 * {
					 * freePtr(unpackedBuffer);
					 * } */

					return (-2);
				}

				memcpy(ovlData->stringTable[i].string,
				    scriptPtr, ovlData->stringTable[i].length);
				scriptPtr += ovlData->stringTable[i].length;
			}
		}
	}
#ifdef DUMP_SCRIPT
	{
		int i;
		for (i = 0; i < ovlData->numProc; i++) {
			dumpScript(scriptName, ovlData, i);
		}
	}
#endif
#ifdef DUMP_OBJECT
	{
		int i;
		FILE *fHandle;
		char nameBundle[100];
		sprintf(nameBundle, "%s-objs.txt", scriptName);

		fHandle = fopen(nameBundle, "w+");
		ASSERT(fHandle);

		for (i = 0; i < ovlData->numMsgRelHeader; i++) {
			linkDataStruct *var_34;
			var_34 = &ovlData->arrayMsgRelHeader[i];

			if (ovlData->arrayNameObj) {
				fprintf(fHandle, "----- object %02d -----\n",
				    i);
				if (var_34->stringNameOffset != 0xFFFF) {
					fprintf(fHandle, "name: %s\n",
					    getObjectName(var_34->
						stringNameOffset,
						ovlData->arrayNameObj));
				}
			}
		}

		fclose(fHandle);
	}
#endif

	return (scriptIdx);
}

int releaseOverlay(const char *name) {
	int overlayIdx;
	ovlDataStruct *ovlDataPtr;

	overlayIdx = findOverlayByName(name);

	if (overlayIdx == -4)
		return -4;

	if (overlayTable[overlayIdx].alreadyLoaded == 0)
		return -4;

	overlayTable[overlayIdx].alreadyLoaded = 0;

	ovlDataPtr = overlayTable[overlayIdx].ovlData;

	if (!ovlDataPtr)
		return -4;
/*
  if (overlayTable[overlayIdx].var1E) {
    free(overlayTable[overlayIdx].var1E);
    overlayTable[overlayIdx].var1E = NULL;
  }

  if (overlayTable[overlayIdx].var16) {
    free(overlayTable[overlayIdx].var16);
    overlayTable[overlayIdx].var16 = NULL;
  } */

	removeScript(overlayIdx, -1, &procHead);
	removeScript(overlayIdx, -1, &procHead);

	removeScript(overlayIdx, -1, &relHead);
	removeScript(overlayIdx, -1, &relHead);

	printf("releaseOverlay: finish !\n");

	return 0;
}

int32 findOverlayByName2(const char *name) {
	int i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmp(overlayTable[i].overlayName, name))
			return (i);
	}

	return (-4);
}

int findOverlayByName(const char *overlayName) {
	int i;

	for (i = 1; i < numOfLoadedOverlay; i++) {
		if (!strcmp(overlayTable[i].overlayName, overlayName)) {
			return (i);
		}
	}

	return (-4);
}

} // End of namespace Cruise
