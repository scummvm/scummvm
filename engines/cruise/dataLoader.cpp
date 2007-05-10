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

#include "cruise/cruise_main.h"

namespace Cruise {

void loadSetEntry(uint8 * name, uint8 * ptr, int currentEntryIdx,
    int currentDestEntry);
void loadFNTSub(uint8 * ptr, int destIdx);

enum fileTypeEnum {
	type_UNK,
	type_SPL,
	type_SET,
	type_FNT
};

int loadSingleFile;

// TODO: Unify decodeGfxFormat1, decodeGfxFormat4 and decodeGfxFormat5

void decodeGfxFormat1(dataFileEntry *pCurrentFileEntry) {
	uint8 *buffer;
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;

	int spriteSize =
	    pCurrentFileEntry->height * pCurrentFileEntry->widthInColumn * 8;
	int x = 0;

	buffer = (uint8 *) malloc(spriteSize);

	while (x < spriteSize) {
		uint8 c;
		uint16 p0;

		p0 = (dataPtr[0] << 8) | dataPtr[1];

		/* decode planes */
		for (c = 0; c < 16; c++) {
			buffer[x + c] = ((p0 >> 15) & 1);

			p0 <<= 1;
		}

		x += 16;

		dataPtr += 2;
	}

	pCurrentFileEntry->subData.ptr = buffer;
}

void decodeGfxFormat4(dataFileEntry *pCurrentFileEntry) {
	uint8 *buffer;
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;

	int spriteSize =
	    pCurrentFileEntry->height * pCurrentFileEntry->widthInColumn * 2;
	int x = 0;

	buffer = (uint8 *) malloc(spriteSize);

	while (x < spriteSize) {
		uint8 c;
		uint16 p0;
		uint16 p1;
		uint16 p2;
		uint16 p3;

		p0 = (dataPtr[0] << 8) | dataPtr[1];
		p1 = (dataPtr[2] << 8) | dataPtr[3];
		p2 = (dataPtr[4] << 8) | dataPtr[5];
		p3 = (dataPtr[6] << 8) | dataPtr[7];

		/* decode planes */
		for (c = 0; c < 16; c++) {
			buffer[x + c] =
			    ((p0 >> 15) & 1) | ((p1 >> 14) & 2) | ((p2 >> 13) &
			    4) | ((p3 >> 12) & 8);

			p0 <<= 1;
			p1 <<= 1;
			p2 <<= 1;
			p3 <<= 1;
		}

		x += 16;

		dataPtr += 8;
	}

	pCurrentFileEntry->subData.ptr = buffer;
}

void decodeGfxFormat5(dataFileEntry *pCurrentFileEntry) {
	uint8 *buffer;
	uint8 *dataPtr = pCurrentFileEntry->subData.ptr;

	int spriteSize =
	    pCurrentFileEntry->height * pCurrentFileEntry->widthInColumn;
	int x = 0;
	int range = pCurrentFileEntry->height * pCurrentFileEntry->width;

	buffer = (uint8 *) malloc(spriteSize);

	while (x < spriteSize) {
		uint8 c;
		uint16 p0;
		uint16 p1;
		uint16 p2;
		uint16 p3;
		uint16 p4;

		p0 = (dataPtr[0 + range * 0] << 8) | dataPtr[1 + range * 0];
		p1 = (dataPtr[0 + range * 1] << 8) | dataPtr[1 + range * 1];
		p2 = (dataPtr[0 + range * 2] << 8) | dataPtr[1 + range * 2];
		p3 = (dataPtr[0 + range * 3] << 8) | dataPtr[1 + range * 3];
		p4 = (dataPtr[0 + range * 4] << 8) | dataPtr[1 + range * 4];

		/* decode planes */
		for (c = 0; c < 16; c++) {
			buffer[x + c] =
			    ((p0 >> 15) & 1) | ((p1 >> 14) & 2) | ((p2 >> 13) &
			    4) | ((p3 >> 12) & 8) | ((p4 >> 11) & 16);

			p0 <<= 1;
			p1 <<= 1;
			p2 <<= 1;
			p3 <<= 1;
			p4 <<= 1;
		}

		x += 16;

		dataPtr += 2;
	}

	pCurrentFileEntry->subData.ptr = buffer;
}

int updateResFileEntry(int height, int width, int entryNumber, int resType) {
	int div = 0;
	int size;

	resetFileEntry(entryNumber);

	filesDatabase[entryNumber].subData.field_1C = 0;

	size = height * width;	// for sprites: width * height

	if (resType == 4) {
		div = size / 4;
	} else if (resType == 5) {
		width = (width * 8) / 5;
	}

	filesDatabase[entryNumber].subData.ptr =
	    (uint8 *) mallocAndZero(size + div);

	if (!filesDatabase[entryNumber].subData.ptr)
		return (-2);

	filesDatabase[entryNumber].widthInColumn = width;
	filesDatabase[entryNumber].subData.ptr2 = filesDatabase[entryNumber].subData.ptr + size;
	filesDatabase[entryNumber].width = width / 8;
	filesDatabase[entryNumber].resType = resType;
	filesDatabase[entryNumber].height = height;
	filesDatabase[entryNumber].subData.index = -1;

	return entryNumber;
}

int createResFileEntry(int width, int height, int resType) {
	int i;
	int entryNumber;
	int div = 0;
	int size;

	printf("Executing untested createResFileEntry!\n");
	exit(1);

	for (i = 0; i < 257; i++) {
		if (!filesDatabase[i].subData.ptr)
			break;
	}

	if (i >= 257) {
		return (-19);
	}

	entryNumber = i;

	filesDatabase[entryNumber].subData.field_1C = 0;

	size = width * height;	// for sprites: width * height

	if (resType == 4) {
		div = size / 4;
	} else if (resType == 5) {
		width = (width * 8) / 5;
	}

	filesDatabase[entryNumber].subData.ptr = (uint8 *) mallocAndZero(size + div);

	if (filesDatabase[entryNumber].subData.ptr) {
		return (-2);
	}

	filesDatabase[entryNumber].widthInColumn = width;
	filesDatabase[entryNumber].subData.ptr2 = filesDatabase[entryNumber].subData.ptr + size;
	filesDatabase[entryNumber].width = width / 8;
	filesDatabase[entryNumber].resType = resType;
	filesDatabase[entryNumber].height = height;
	filesDatabase[entryNumber].subData.index = -1;

	return entryNumber;
}

fileTypeEnum getFileType(uint8 *name) {
	char extentionBuffer[16];

	fileTypeEnum newFileType = type_UNK;

	getFileExtention((char *)name, extentionBuffer);

	if (!strcmp(extentionBuffer, ".SPL")) {
		newFileType = type_SPL;
	} else if (!strcmp(extentionBuffer, ".SET")) {
		newFileType = type_SET;
	} else if (!strcmp(extentionBuffer, ".FNT")) {
		newFileType = type_FNT;
	}

	ASSERT(newFileType != type_UNK);

	return newFileType;
}

int getNumMaxEntiresInSet(uint8 *ptr) {
	uint16 numEntries = *(uint16 *) (ptr + 4);
	flipShort(&numEntries);

	return numEntries;
}

int loadFileMode2(uint8 *name, int startIdx, int currentEntryIdx, int numIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	switch (fileType) {
	case type_SET:
		{
			int i;
			int numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);

			for (i = 0; i < numIdx; i++) {
				if ((currentEntryIdx + i) > numMaxEntriesInSet) {
					return 0;	// exit if limit is reached 
				}
				loadSetEntry(name, ptr, currentEntryIdx + i,
				    startIdx + i);
			}

			break;
		}
	case type_FNT:
		{
			loadFNTSub(ptr, startIdx);
			break;
		}
	case type_UNK:
		{
			break;
		}
	case type_SPL:
		{
			break;
		}
	}
	return 0;
}

int loadFullBundle(uint8 *name, int startIdx) {
	uint8 *ptr = NULL;
	fileTypeEnum fileType;

	fileType = getFileType(name);

	loadFileSub1(&ptr, name, NULL);

	switch (fileType) {
	case type_SET:
		{
			int i;
			int numMaxEntriesInSet;

			numMaxEntriesInSet = getNumMaxEntiresInSet(ptr);	// get maximum number of sprites/animations in SET file

			for (i = 0; i < numMaxEntriesInSet; i++) {
				loadSetEntry(name, ptr, i, startIdx + i);
			}

			break;
		}
	case type_FNT:
		{
			loadFNTSub(ptr, startIdx);
			break;
		}
	case type_UNK:
		{
			break;
		}
	case type_SPL:
		{
			break;
		}
	}

	return 0;
}

void loadFNTSub(uint8 *ptr, int destIdx) {
	uint8 *ptr2 = ptr;
	uint8 *destPtr;
	int fileIndex;
	uint32 fontSize;

	ptr2 += 4;
	memcpy(&loadFileVar1, ptr2, 4);

	flipLong(&loadFileVar1);

	if (destIdx == -1) {
		fileIndex = createResFileEntry(loadFileVar1, 1, 1);
	} else {
		fileIndex = updateResFileEntry(loadFileVar1, 1, destIdx, 1);
	}

	destPtr = filesDatabase[fileIndex].subData.ptr;

	memcpy(destPtr, ptr2, loadFileVar1);

	memcpy(&fontSize, ptr2, 4);
	flipLong(&fontSize);

	if (destPtr != NULL) {
		int32 i;
		uint8 *currentPtr;

		destPtr = filesDatabase[fileIndex].subData.ptr;

		flipLong((int32 *) destPtr);
		flipLong((int32 *) (destPtr + 4));
		flipGen(destPtr + 8, 6);

		currentPtr = destPtr + 14;

		for (i = 0; i < *(int16 *) (destPtr + 8); i++) {
			flipLong((int32 *) currentPtr);
			currentPtr += 4;

			flipGen(currentPtr, 8);
			currentPtr += 8;
		}
	}
}

void loadSetEntry(uint8 *name, uint8 *ptr, int currentEntryIdx,
	    int currentDestEntry) {
	uint8 *ptr2;
	uint8 *ptr3;
	int offset;
	int sec = 0;
	uint16 numIdx;

	if (!strcmpuint8(ptr, "SEC")) {
		sec = 1;
	}

	ptr2 = ptr + 4;

	memcpy(&numIdx, ptr2, 2);
	flipShort(&numIdx);

	ptr3 = ptr + 6;

	offset = currentEntryIdx * 16;

	{
		uint8 *ptr4;
		int resourceSize;
		int fileIndex;
		setHeaderEntry localBuffer;
		uint8 *ptr5;

		ptr4 = ptr + offset + 6;

		memcpy(&localBuffer, ptr4, sizeof(setHeaderEntry));

		flipLong((int32 *) & localBuffer.field_0);
		flipGen(&localBuffer.width, 12);

		if ((sec == 1) || (localBuffer.type == 5)) {
			localBuffer.width = localBuffer.width - (localBuffer.type * 2);	// Type 1: Width - (1*2) , Type 5: Width - (5*2)
		}

		resourceSize = localBuffer.width * localBuffer.height;

		if (currentDestEntry == -1) {
			fileIndex =
			    createResFileEntry(localBuffer.width,
			    localBuffer.height, localBuffer.type);
		} else {
			fileIndex =
			    updateResFileEntry(localBuffer.height,
			    localBuffer.width, currentDestEntry,
			    localBuffer.type);
		}

		if (fileIndex < 0) {
			return;	// TODO: buffer is not freed
		}

		ptr5 = ptr3 + localBuffer.field_0 + numIdx * 16;

		memcpy(filesDatabase[fileIndex].subData.ptr, ptr5,
		    resourceSize);
		ptr5 += resourceSize;

		switch (localBuffer.type) {
		case 0:
			{
				filesDatabase[fileIndex].subData.resourceType =
				    8;
				break;
			}
		case 1:
			{
				filesDatabase[fileIndex].subData.resourceType =
				    2;
				decodeGfxFormat1(&filesDatabase[fileIndex]);
				break;
			}
		case 4:
			{
				filesDatabase[fileIndex].width *= 2;
				filesDatabase[fileIndex].subData.resourceType =
				    4;
				decodeGfxFormat4(&filesDatabase[fileIndex]);
				break;
			}
		case 5:
			{
				if (sec == 0) {
					// TODO sec type 5 needs special conversion. cut out 2 bytes at every width/5 position.
					return;
				}
				filesDatabase[fileIndex].subData.resourceType =
				    4;
				decodeGfxFormat5(&filesDatabase[fileIndex]);
				break;
			}
		case 8:
			{
				filesDatabase[fileIndex].subData.resourceType = 4;	// dummy !
				break;
			}
		default:
			{
				printf("Unsuported gfx loading type: %d\n",
				    localBuffer.type);
				break;
			}
		}

		filesDatabase[fileIndex].subData.index = currentDestEntry;
		filesDatabase[fileIndex].subData.transparency =
		    localBuffer.transparency; /*% 0x10 */ ;

		strcpyuint8(filesDatabase[fileIndex].subData.name, name);
	}

	// TODO: free

	return;
}

} // End of namespace Cruise
