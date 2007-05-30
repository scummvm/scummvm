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

exportEntryStruct *parseExport(int *out1, int *pExportedFuncionIdx,
	    char *buffer) {
	char localBuffer[256];
	uint8 functionName[256];
	uint8 overlayName[256];
	char *dotPtr;
	char *ptr2;
	int idx;
	int numExport;
	exportEntryStruct *currentExportEntry;
	uint8 *entity1Name;
	int i;

	*out1 = 0;
	*pExportedFuncionIdx = 0;

	strcpyuint8(localBuffer, buffer);
	dotPtr = strchr(localBuffer, '.');

	if (dotPtr) {
		strcpyuint8(functionName, dotPtr + 1);
		*dotPtr = 0;

		strcpyuint8(overlayName, localBuffer);
	} else {
		overlayName[0] = 0;

		strcpyuint8(functionName, buffer);
	}

	ptr2 = strchr((char *)functionName, ':');

	if (ptr2) {
		*ptr2 = 0;

		*out1 = 1;
	}

	strToUpper(overlayName);
	strToUpper(functionName);
	if (strlen((char *)overlayName) == 0)
		return NULL;

	idx = findOverlayByName2(overlayName);

	if (idx == -4)
		return (NULL);

	if (overlayTable[idx].alreadyLoaded == 0)
		return (NULL);

	if (!overlayTable[idx].ovlData)
		return (NULL);

	numExport = overlayTable[idx].ovlData->numExport;
	currentExportEntry = overlayTable[idx].ovlData->exportDataPtr;
	entity1Name = overlayTable[idx].ovlData->exportNamesPtr;

	if (!entity1Name)
		return (0);

	for (i = 0; i < numExport; i++) {
		uint8 exportedName[256];
		uint8 *name = entity1Name + currentExportEntry->offsetToName;

		strcpyuint8(exportedName, name);
		strToUpper(exportedName);

		if (!strcmpuint8(functionName, exportedName)) {
			*pExportedFuncionIdx = idx;

			return (currentExportEntry);
		}

		currentExportEntry++;
	}

	return (NULL);
}

int updateScriptImport(int ovlIdx) {
	char buffer[256];
	ovlDataStruct *ovlData;
	int numData3;
	int size5;
	int numImport;
	int param;
	int var_32;
	ovlData3Struct *pScript;
//  char* importDataPtr;
//  char* namePtr;
//  char* linkDataPtr;

	if (!overlayTable[ovlIdx].ovlData)
		return (-4);

	ovlData = overlayTable[ovlIdx].ovlData;

	numData3 = ovlData->numScripts1;
	size5 = ovlData->numScripts2;
	numImport = ovlData->numImport;
	param = 0;

	// do it for the 2 first string types
	do {

		int i = 0;

		if (param == 0) {
			var_32 = numData3;
		} else {
			var_32 = size5;
		}

		if (var_32) {
			do {
				importScriptStruct *ptrImportData;
				uint8 *ptrImportName;
				uint8 *ptrData;

				int var_22 = 0;

				if (param == 0) {
					pScript = getOvlData3Entry(ovlIdx, i);
				} else {
					pScript = scriptFunc1Sub2(ovlIdx, i);
				}

				ptrImportData = (importScriptStruct *) (pScript->dataPtr + pScript->offsetToImportData);	// import data
				ptrImportName = pScript->dataPtr + pScript->offsetToImportName;	// import name
				ptrData = pScript->dataPtr;

				var_22 = 0;

				if (pScript->numImport > 0) {
					int counter = pScript->numImport;

					do {
						int param2 =
						    ptrImportData->type;

						if (param2 != 70) {
							exportEntryStruct
							    * ptrDest2;
							int out1;
							int out2;

							strcpyuint8(buffer,
							    ptrImportName +
							    ptrImportData->
							    offsetToName);
							ptrDest2 =
							    parseExport(&out1,
							    &out2, buffer);

							if (ptrDest2 && out2) {
								int temp =
								    ptrImportData->
								    offset;
								if (out1)	//is sub function... (ie  'invent.livre:s')
								{
									uint8 *ptr = ptrData + temp;

									*(ptr +
									    1)
									    =
									    out2;
									*(int16
									    *)
									    (ptr
									    +
									    2)
									    =
									    ptrDest2->
									    idx;

									flipShort
									    (
									    (int16
										*)
									    (ptr + 2));
								} else {
									if (param2 == 20 || param2 == 30 || param2 == 40 || param2 == 50)	// this patch a double push
									{
										uint8
										    *
										    ptr
										    =
										    ptrData
										    +
										    temp;

										*(ptr + 1) = 0;
										*(ptr + 2) = out2;	// update the overlay number

										*(int16 *) (ptr + 4) = ptrDest2->idx;

										flipShort
										    (
										    (int16
											*)
										    (ptr + 4));
									} else {
										int var_4 = ptrDest2->var4;

										if (var_4 & 1) {
											param2
											    =
											    8;
										} else {
											param2
											    =
											    16;
										}

										if (var_4 >= 0 && var_4 <= 3) {
											param2
											    |=
											    5;
										} else {
											param2
											    |=
											    6;
										}

										*(ptrData + temp) = param2;
										*(ptrData + temp + 1) = out2;

										*(int16 *) (ptrData + temp + 2) = ptrDest2->idx;

										flipShort
										    (
										    (int16
											*)
										    (ptrData
											+
											temp
											+
											2));
									}
								}
							}
						}

						ptrImportData++;
					} while (--counter);
				}

			} while (++i < var_32);

		}

	} while (++param < 2);

	if (ovlData->importDataPtr && ovlData->importNamePtr && numImport) {
		int numImport2 = numImport;
		int i;

		for (i = 0; i < numImport2; i++) {
			int out1;
			int foundExportIdx;
			exportEntryStruct *pFoundExport;
			int linkType;
			int linkEntryIdx;

			strcpyuint8(buffer,
			    ovlData->importNamePtr +
			    ovlData->importDataPtr[i].nameOffset);

			pFoundExport =
			    parseExport(&out1, &foundExportIdx, buffer);

			linkType = ovlData->importDataPtr[i].linkType;
			linkEntryIdx = ovlData->importDataPtr[i].linkIdx;

			if (pFoundExport && foundExportIdx) {
				switch (linkType) {
				case 0:	// var
					{
						ovlData->
						    linkDataPtr[linkEntryIdx].
						    varIdx = foundExportIdx;
						ovlData->
						    linkDataPtr[linkEntryIdx].
						    varNameOffset =
						    pFoundExport->offsetToName;
						break;
					}
				case 1:	// string
					{
						ovlData->
						    linkDataPtr[linkEntryIdx].
						    stringIdx = foundExportIdx;
						ovlData->
						    linkDataPtr[linkEntryIdx].
						    stringNameOffset =
						    pFoundExport->offsetToName;
						break;
					}
				case 2:	// proc
					{
						ovlData->
						    linkDataPtr[linkEntryIdx].
						    procIdx = foundExportIdx;
						ovlData->
						    linkDataPtr[linkEntryIdx].
						    procNameOffset =
						    pFoundExport->offsetToName;
						break;
					}
				}
			}
		}
	}

	return (0);
}

// check that the newly loaded isn't used by the already loaded overlays
void updateAllScriptsImports(void) {
	int i;

	for (i = 0; i < 90; i++) {
		if (overlayTable[i].ovlData && overlayTable[i].alreadyLoaded) {
			updateScriptImport(i);
		}
	}
}

} // End of namespace Cruise
