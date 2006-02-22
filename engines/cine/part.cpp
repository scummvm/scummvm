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

#include "cine/cine.h"

u16 numElementInPart;
u16 partVar1;

animDataStruct animDataTable[NUM_MAX_PARTDATA];
partBufferStruct *partBuffer;

void loadPart(const char *partName) {
	u16 i;

	ASSERT(sizeof(partBufferStruct) == 0x1E);

	for (i = 0; i < 255; i++) {
		partBuffer[i].part_name[0] = 0;

		partBuffer[i].offset = 0;
		partBuffer[i].packed_size = 0;
		partBuffer[i].unpacked_size = 0;
		partBuffer[i].var1A = 0;
	}

	numElementInPart = 0;
	partVar1 = 30;

	partFileHandle.close();

	checkDataDisk(-1);

	partFileHandle.open(partName);

	ASSERT(partFileHandle.isOpen());

	processPendingUpdates(-1);

	partFileHandle.read(&numElementInPart, 2);
	partFileHandle.read(&partVar1, 2);

	flipU16(&numElementInPart);
	flipU16(&partVar1);

	partFileHandle.read(partBuffer, numElementInPart * partVar1);

	strcpy(currentPartName, partName);

	for (i = 0; i < numElementInPart; i++) {
		flipU32(&partBuffer[i].offset);
		flipU32(&partBuffer[i].packed_size);
		flipU32(&partBuffer[i].unpacked_size);
	}

	if (gameType == Cine::GID_FW)
		loadPal(partName);
}

void freePartEntry(u8 idx) {
	if (animDataTable[idx].ptr1) {
		//free(animDataTable[idx].ptr1);

		animDataTable[idx].ptr1 = NULL;
		animDataTable[idx].ptr2 = NULL;

		// TODO: finish

		if (frameVar0 > 0)
			frameVar0--;
	}
}

void freePartRange(u8 startIdx, u8 numIdx) {
	u8 i;

	for (i = 0; i < numIdx; i++) {
		freePartEntry(i + startIdx);
	}
}

void closePart(void) {
}

const char *bundleNames[] = {
	"EGOUBASE",
	"LABYBASE",
	"PROCEGOU",
	"PROCLABY",
	"PROCS00",
	"PROCS01",
	"PROCS02",
	"PROCS03",
	"PROCS04",
	"PROCS06",
	"PROCS07",
	"PROCS08",
	"PROCS10",
	"PROCS12",
	"PROCS13",
	"PROCS15",
	"PROCS16",
	"RSC00",
	"RSC01",
	"RSC02",
	"RSC03",
	"RSC04",
	"RSC05",
	"RSC06",
	"RSC07",
	"RSC08",
	"RSC09",
	"RSC10",
	"RSC11",
	"RSC12",
	"RSC13",
	"RSC14",
	"RSC15",
	"RSC16",
	"RSC17",
// english version
	"SONS1",
	"SONS2",
	"SONS3",
	"SONS4",
	"SONS5",
	"SONS6",
	"SONS7",
	"SONS8",
	"SONS9",
/*
"SONS31", // french version
"SONS32",
"SONS33",
"SONS34"
*/
};

s16 findFileInBundle(const char *fileName) {
	u16 i;

	if (gameType == Cine::GID_OS) {
		u16 j;

		for (i = 0; i < numElementInPart; i++) {
			if (!strcmp(fileName, partBuffer[i].part_name)) {
				return i;
			}
		}

		for (j = 0; j < 39; j++) {
			loadPart(bundleNames[j]);

			for (i = 0; i < numElementInPart; i++) {
				if (!strcmp(fileName, partBuffer[i].part_name)) {
					return i;
				}
			}
		}
	} else {
		for (i = 0; i < numElementInPart; i++) {
			if (!strcmp(fileName, partBuffer[i].part_name)) {
				return i;
			}
		}
	}
	return -1;
}

void readFromPart(s16 idx, u8 *dataPtr) {
	processPendingUpdates(1);

	partFileHandle.seek(partBuffer[idx].offset, SEEK_SET);

	partFileHandle.read(dataPtr, partBuffer[idx].packed_size);
}

u8 *readBundleFile(s16 foundFileIdx) {
	u8 *dataPtr;

	dataPtr = (u8 *) malloc(partBuffer[foundFileIdx].unpacked_size + 2);
	memset(dataPtr, 0, partBuffer[foundFileIdx].unpacked_size + 2);

	if (partBuffer[foundFileIdx].unpacked_size !=
	    partBuffer[foundFileIdx].packed_size) {
		u8 *unpackBuffer;
		u16 realSize;

		unpackBuffer =
		    (u8 *) malloc(partBuffer[foundFileIdx].packed_size + 500);
		readFromPart(foundFileIdx, unpackBuffer);

		realSize =
		    *(u16 *) (unpackBuffer +
		    partBuffer[foundFileIdx].packed_size - 2);
		flipU16(&realSize);

		decomp(unpackBuffer + partBuffer[foundFileIdx].packed_size - 4,
		    dataPtr + realSize, realSize);
		free(unpackBuffer);
	} else {
		readFromPart(foundFileIdx, dataPtr);
	}

	return dataPtr;
}
