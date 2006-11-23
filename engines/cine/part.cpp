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

AnimData *animDataTable;
PartBuffer *partBuffer;

void loadPart(const char *partName) {
	uint16 i;

	for (i = 0; i < NUM_MAX_PARTDATA; i++) {
		partBuffer[i].partName[0] = 0;
		partBuffer[i].offset = 0;
		partBuffer[i].packedSize = 0;
		partBuffer[i].unpackedSize = 0;
	}

	numElementInPart = 0;

	partFileHandle.close();

	checkDataDisk(-1);

	partFileHandle.open(partName);

	assert(partFileHandle.isOpen());

	setMouseCursor(MOUSE_CURSOR_DISK);

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

	if (g_cine->getGameType() == Cine::GType_FW && g_cine->getPlatform() == Common::kPlatformPC)
		loadPal(partName);
}

void freePartEntry(byte idx) {
	if (animDataTable[idx].ptr1) {
		//free(animDataTable[idx].ptr1);

		animDataTable[idx].ptr1 = NULL;
		animDataTable[idx].ptr2 = NULL;

		// TODO: finish

		if (frameVar0 > 0)
			frameVar0--;
	}
}

void freePartRange(byte startIdx, byte numIdx) {
	byte i;

	for (i = 0; i < numIdx; i++) {
		freePartEntry(i + startIdx);
	}
}

void closePart(void) {
}

static const char *bundleNamesDOSEN[] = {
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
	"SONS1",
	"SONS2",
	"SONS3",
	"SONS4",
	"SONS5",
	"SONS6",
	"SONS7",
	"SONS8",
	"SONS9",
	NULL
};

static const char *bundleNamesDOSUS[] = {
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
	"PROCS12",
	"PROCS13",
	"PROCS15",
	"PROCS16",
	"RSC00",
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
	"SONS31",
	"SONS32",
	"SONS33",
	"SONS34",
	NULL
};

static const char *bundleNamesDOSFR[] = {
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
	"SONS31",
	"SONS32",
	"SONS33",
	"SONS34",
	NULL
};

static const char *bundleNamesDOSES[] = {
	"EGOUBASE",
	"LABYBASE",
	"PROCS1",
	"PROCS2",
	"PROCS3",
	"PROCS4",
	"PROCS5",
	"PROCS6",
	"SD01A",
	"SD01B",
	"SD01C",
	"SD01D",
	"SD021",
	"SD022",
	"SD03",
	"SDSONS",
	"SDSONS2",
	"SDSONS3",
	"SINTRO2",
	NULL
};

static const char *bundleNamesDOSInt[] = {
	"EGOUBASE",
	"LABYBASE",
	"PROCS1",
	"PROCS2",
	"PROCS3",
	"PROCS4",
	"PROCS5",
	"PROCS6",
	"SD01A",
	"SD01B",
	"SD01C",
	"SD01D",
	"SD021",
	"SD022",
	"SD03",
	"SDS1",
	"SDS2",
	"SDS3",
	"SDS4",
	"SDS5",
	"SDS6",
	"SINTRO2",
	NULL
};

static const char *bundleNamesAmiga[] = {
	"EGOUBASE",
	"LABYBASE",
	"PROCS0",
	"PROCS1",
	"PROCS2",
	"SAMPLES",
	"SAMPLES2",
	"SAMPLES3",
	"SD01A",
	"SD01B",
	"SD01C",
	"SD01D",
	"SD02",
	"SD03",
	"SDSONS",
	"SDSONS2",
	"SDSONS3",
	"SINTRO2",
	NULL
};

static const char *bundleNamesAmigaDemo[] = {
	"DEMO_OS",
	"SDSONS",
	NULL
};

static const char *bundleNamesAtari[] = {
	"EGOUBASE",
	"LABYBASE",
	"PROCS0",
	"PROCS1",
	"PROCS2",
	"SAMPLES",
	"SD01A",
	"SD01B",
	"SD01C",
	"SD01D",
	"SD02",
	"SD03",
	"SDSONS",
	"SDSONS2",
	"SDSONS3",
	"SINTRO2",
	NULL
};

int16 findFileInBundle(const char *fileName) {
	uint16 i;

	if (g_cine->getGameType() == Cine::GType_OS) {
		for (i = 0; i < numElementInPart; i++) {
			if (!strcmp(fileName, partBuffer[i].partName)) {
				return i;
			}
		}

		const char **bPtr = 0;

		if (g_cine->getPlatform() == Common::kPlatformPC) {
			switch (g_cine->getLanguage()) {
			case Common::EN_GRB:
				bPtr = bundleNamesDOSEN;
				break;
			case Common::EN_USA:
				if (g_cine->getFeatures() & GF_CD)
					bPtr = bundleNamesDOSUS;
				else
					bPtr = bundleNamesDOSInt;
				break;
			case Common::DE_DEU:
			case Common::IT_ITA:
				bPtr = bundleNamesDOSInt;
				break;
			case Common::ES_ESP:
				if (g_cine->getFeatures() & GF_CD)
					bPtr = bundleNamesDOSInt;
				else
					bPtr = bundleNamesDOSES;
				break;
			case Common::FR_FRA:
				bPtr = bundleNamesDOSFR;
				break;
			default:
				break;
			}
		} else if (g_cine->getPlatform() == Common::kPlatformAmiga) {
			if (g_cine->getFeatures() & GF_DEMO)
				bPtr = bundleNamesAmigaDemo;
			else
				bPtr = bundleNamesAmiga;
		} else if (g_cine->getPlatform() == Common::kPlatformAtariST) {
				bPtr = bundleNamesAtari;
		}

		while (**bPtr) {
			loadPart(*bPtr);

			for (i = 0; i < numElementInPart; i++) {
				if (!strcmp(fileName, partBuffer[i].partName)) {
					return i;
				}
			}
			bPtr++;
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

void readFromPart(int16 idx, byte *dataPtr) {
	setMouseCursor(MOUSE_CURSOR_DISK);

	partFileHandle.seek(partBuffer[idx].offset, SEEK_SET);
	partFileHandle.read(dataPtr, partBuffer[idx].packedSize);
}

byte *readBundleFile(int16 foundFileIdx) {
	byte *dataPtr;

	dataPtr = (byte *)calloc(partBuffer[foundFileIdx].unpackedSize, 1);

	if (partBuffer[foundFileIdx].unpackedSize != partBuffer[foundFileIdx].packedSize) {
		byte *unpackBuffer;

		unpackBuffer = (byte *)malloc(partBuffer[foundFileIdx].packedSize);
		readFromPart(foundFileIdx, unpackBuffer);
		delphineUnpack(dataPtr, unpackBuffer, partBuffer[foundFileIdx].packedSize);
		free(unpackBuffer);
	} else {
		readFromPart(foundFileIdx, dataPtr);
	}

	return dataPtr;
}

byte *readFile(const char *filename) {
	Common::File in;

	in.open(filename);

	if (!in.isOpen())
		error("readFile(): Cannot open file %s", filename);

	byte *dataPtr;
	uint32 size = in.size();

	dataPtr = (byte *)malloc(size);
	in.read(dataPtr, size);

	return dataPtr;
}

void dumpBundle(const char *fileName) {
	char tmpPart[15];

	strcpy(tmpPart, currentPartName);

	loadPart(fileName);
	for (int i = 0; i < numElementInPart; i++) {
		byte *data = readBundleFile(i);

		Common::File out;

		debug(0, "%s", partBuffer[i].partName);
		out.open(Common::String("dumps/") + partBuffer[i].partName, Common::File::kFileWriteMode);
		out.write(data, partBuffer[i].unpackedSize);
		out.close();
	}

	loadPart(tmpPart);
}

} // End of namespace Cine
