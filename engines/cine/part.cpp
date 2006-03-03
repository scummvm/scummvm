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
#include "cine/unpack.h"
#include "cine/various.h"

namespace Cine {

uint16 numElementInPart;

AnimData animDataTable[NUM_MAX_PARTDATA];
PartBuffer *partBuffer;

void loadPart(const char *partName) {
	uint16 i;

	for (i = 0; i < 255; i++) {
		partBuffer[i].partName[0] = 0;
		partBuffer[i].offset = 0;
		partBuffer[i].packedSize = 0;
		partBuffer[i].unpackedSize = 0;
	}

	numElementInPart = 0;

	partFileHandle.close();

	checkDataDisk(-1);

	partFileHandle.open(partName);

	ASSERT(partFileHandle.isOpen());

	processPendingUpdates(-1);

	numElementInPart = partFileHandle.readUint16BE();
	partFileHandle.readUint16BE(); // entry size

	strcpy(currentPartName, partName);

	for (i = 0; i < numElementInPart; i++) {
		partFileHandle.read(partBuffer[i].partName, 14);
		partBuffer[i].offset = partFileHandle.readUint32BE();
		partBuffer[i].packedSize = partFileHandle.readUint32BE();
		partBuffer[i].unpackedSize = partFileHandle.readUint32BE();
		partFileHandle.readUint32BE(); // unused
	}

	if (gameType == Cine::GID_FW)
		loadPal(partName);
}

void freePartEntry(uint8 idx) {
	if (animDataTable[idx].ptr1) {
		//free(animDataTable[idx].ptr1);

		animDataTable[idx].ptr1 = NULL;
		animDataTable[idx].ptr2 = NULL;

		// TODO: finish

		if (frameVar0 > 0)
			frameVar0--;
	}
}

void freePartRange(uint8 startIdx, uint8 numIdx) {
	uint8 i;

	for (i = 0; i < numIdx; i++) {
		freePartEntry(i + startIdx);
	}
}

void closePart(void) {
}

static const char *bundleNames[] = {
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

int16 findFileInBundle(const char *fileName) {
	uint16 i;

	if (gameType == Cine::GID_OS) {
		uint16 j;

		for (i = 0; i < numElementInPart; i++) {
			if (!strcmp(fileName, partBuffer[i].partName)) {
				return i;
			}
		}

		for (j = 0; j < 39; j++) {
			loadPart(bundleNames[j]);

			for (i = 0; i < numElementInPart; i++) {
				if (!strcmp(fileName, partBuffer[i].partName)) {
					return i;
				}
			}
		}
	} else {
		for (i = 0; i < numElementInPart; i++) {
			if (!strcmp(fileName, partBuffer[i].partName)) {
				return i;
			}
		}
	}
	return -1;
}

void readFromPart(int16 idx, uint8 *dataPtr) {
	processPendingUpdates(1);

	partFileHandle.seek(partBuffer[idx].offset, SEEK_SET);
	partFileHandle.read(dataPtr, partBuffer[idx].packedSize);
}

uint8 *readBundleFile(int16 foundFileIdx) {
	uint8 *dataPtr;

	dataPtr = (uint8 *) malloc(partBuffer[foundFileIdx].unpackedSize);
	memset(dataPtr, 0, partBuffer[foundFileIdx].unpackedSize);

	if (partBuffer[foundFileIdx].unpackedSize != partBuffer[foundFileIdx].packedSize) {
		uint8 *unpackBuffer;

		unpackBuffer = (uint8 *)malloc(partBuffer[foundFileIdx].packedSize);
		readFromPart(foundFileIdx, unpackBuffer);
		delphineUnpack(dataPtr, unpackBuffer, partBuffer[foundFileIdx].packedSize);
		free(unpackBuffer);
	} else {
		readFromPart(foundFileIdx, dataPtr);
	}

	return dataPtr;
}

} // End of namespace Cine
