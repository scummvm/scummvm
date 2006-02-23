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

struct animHeader2Struct {
	uint32 field_0;
	uint16 width;
	uint16 height;
	uint16 type;
	uint16 field_A;
	uint16 field_C;
	uint16 field_E;
};

typedef struct animHeader2Struct animHeader2Struct;

uint16 frameVar0 = 0;

animHeaderStruct animHeader;

animDataEntry animData[] = {
	{"ALPHA", 0xF},
	{"TITRE2", 0xF},
	{"ET", 0xC},
	{"L311", 0x3},
	{"L405", 0x1},
	{"L515", 0xC},
	{"L009", 0xE},
	{"L010", 0xE},
	{"FUTUR", 0x6},
	{"PAYSAN3", 0xB},
	{"L801", 0xC},
	{"L802", 0xC},
	{"L803", 0xC},
	{"L901", 0xD},
	{"L902", 0x8},
	{"L903", 0xD},
	{"L904", 0xD},
	{"L905", 0xD},
	{"L906", 0xD},
	{"L907", 0xD},
	{"LA03", 0x4},
	{"MOINE", 0xB},
	{"L908", 0x8},
	{"L909", 0x8},
	{"L807", 0xC},
	{"L808", 0xC},
	{"LA01", 0xB},
	{"L1201", 0xC},
	{"L1202", 0xC},
	{"L1203", 0xC},
	{"L1210", 0x5},
	{"L1211", 0xC},
	{"L1214", 0xC},
	{"L1215", 0xC},
	{"L1216", 0xC},
	{"L1217", 0xC},
	{"L1218", 0xC},
	{"L1219", 0xC},
	{"L1220", 0xC},
	{"SEIGNEUR", 0x6},
	{"PERE0", 0xD},
	{"L1302", 0x4},
	{"L1303", 0x4},
	{"L1304", 0x4},
	{"L1401", 0xF},
	{"L1402", 0xF},
	{"L1501", 0x8},
	{"L1503", 0x8},
	{"L1504", 0x4},
	{"L1505", 0x8},
	{"L1506", 0x8},
	{"L1601", 0xB},
	{"L1602", 0xB},
	{"L1603", 0xB},
	{"L1604", 0x4},
	{"L1605", 0x4},
	{"L1701", 0x4},
	{"L1702", 0x4},
	{"L1801", 0x6},
	{"L1904", 0x8},
	{"L2002", 0x8},
	{"L2003", 0x8},
	{"L2101", 0x4},
	{"L2102", 0x4},
	{"L2201", 0x7},
	{"L2202", 0x7},
	{"L2203", 0xE},
	{"L2305", 0x9},
	{"L2306", 0x9},
	{"GARDE1", 0x7},
	{"L2402", 0x7},
	{"L2407", 0x7},
	{"L2408", 0x7},
	{"GARDE2", 0x6},
	{"L2601", 0x6},
	{"L2602", 0x6},
	{"L2603", 0x6},
	{"L2604", 0x6},
	{"L2605", 0x8},
	{"L2606", 0x8},
	{"L2607", 0x8},
	{"L2610", 0x6},
	{"L2611", 0x6},
	{"L2612", 0x6},
	{"L2613", 0x8},
	{"L2614", 0x6},
	{"VOYAGEUR", 0x6},
	{"L2701", 0xD},
	{"L2702", 0xD},
	{"L2703", 0x6},
	{"L2801", 0xD},
	{"L2802", 0xD},
	{"L2803", 0xD},
	{"L2804", 0xD},
	{"L2807", 0xD},
	{"L2902", 0x8},
	{"L2903", 0x8},
	{"L3101", 0xA},
	{"L3102", 0xA},
	{"L3103", 0xA},
	{"L3203", 0xF},
	{"L3204", 0xF},
	{"L3001", 0x7},
	{"L3002", 0x7},
	{"L3416", 0xC},
	{"L3601", 0x5},
	{"L3602", 0x5},
	{"L3603", 0x5},
	{"L3607", 0x5},
	{"L3701", 0x8},
	{"L3702", 0x8},
	{"L3703", 0x8},
	{"L4001", 0xD},
	{"L4002", 0xD},
	{"L4103", 0xF},
	{"L4106", 0xF},
	{"CRUGHON1", 0xC},
	{"L4203", 0xC},
	{"L4301", 0xC},
	{"L4302", 0xC},
	{"L4303", 0xC},
	{"FUTUR2", 0x6},
	{"L4601", 0xE},
	{"L4603", 0x1},
	{"L4106", 0xF},
	{"L4801", 0xD},
	{"L4802", 0xD},
	{"FIN01", 0xB},
	{"FIN02", 0xB},
	{"FIN03", 0xB},
	{"FIN", 0x9},
};

#define NUM_ANIM_DATA (sizeof(animData)/sizeof(animDataEntry))

uint8 findAnimInHardcodedData(char *animName) {
	char name[15];
	uint16 i;

	removeExtention(name, animName);

	for (i = 0; i < NUM_ANIM_DATA; i++) {
		if (!strcmp(name, animData[i].name)) {
			return (animData[i].param);
		}
	}

	return (0);
}

int16 allocFrame(uint16 width, uint16 height, int8 isMask) {
	uint16 i;
	uint32 frameSize;

	for (i = 0; i < NUM_MAX_PARTDATA; i++) {
		if (!animDataTable[i].ptr1)
			break;
	}

	if (i == NUM_MAX_PARTDATA)
		return -1;

	if (!isMask) {		// sprite + generated mask
		frameSize = width * height;

		animDataTable[i].ptr1 = (uint8 *)malloc(frameSize);
		animDataTable[i].ptr2 = (uint8 *)malloc(frameSize);

		animDataTable[i].width = width;
		animDataTable[i].var1 = width >> 3;
		animDataTable[i].field_4 = 4;
		animDataTable[i].var2 = height;

		animDataTable[i].fileIdx = -1;
		animDataTable[i].frameIdx = -1;
	} else {
		// mask
		frameSize = width * height * 8;

		animDataTable[i].ptr1 = (uint8 *)malloc(frameSize);
		animDataTable[i].ptr2 = NULL;

		animDataTable[i].width = width;
		animDataTable[i].var1 = width >> 3;
		animDataTable[i].field_4 = 4;
		animDataTable[i].var2 = height;

		animDataTable[i].fileIdx = -1;
		animDataTable[i].frameIdx = -1;
	}

	frameVar0++;

	return (i);
}

int16 allocFrame2(uint16 width, uint16 height, uint16 type) {
	uint16 i;
	uint32 frameSize;

	for (i = 0; i < NUM_MAX_PARTDATA; i++) {
		if (!animDataTable[i].ptr1)
			break;
	}

	if (i == NUM_MAX_PARTDATA)
		return -1;

	frameSize = width * height;

	if (type == 4) {		// 16 color sprites to 256
		frameSize *= 2;
		type = 8;
		width *= 2;
	}

	if (type == 5) {
		frameSize += 16;
	}

	frameSize *= 2;

	animDataTable[i].ptr1 = (uint8 *)malloc(frameSize);

	ASSERT_PTR(animDataTable[i].ptr1);

	animDataTable[i].width = width;

	if (type == 5) {
		animDataTable[i].var1 = width / 8;
	} else {
		animDataTable[i].var1 = width / 16;
	}

	animDataTable[i].field_4 = type;	// bbp ?

	animDataTable[i].var2 = height;

	animDataTable[i].fileIdx = -1;
	animDataTable[i].frameIdx = -1;

	frameVar0++;

	return (i);
}

int16 reserveFrame(uint16 width, uint16 height, uint16 type, uint16 idx) {
	uint16 i;
	uint32 frameSize;

	i = idx;

	frameSize = width * height;

	if (type == 4) {		// 256 color sprites
		frameSize *= 2;
		type = 8;
		width *= 2;
	}

	if (type == 5) {
		frameSize += 16;
	}

	frameSize *= 2;

	animDataTable[i].ptr1 = (uint8 *)malloc(frameSize);

	ASSERT_PTR(animDataTable[i].ptr1);

	animDataTable[i].width = width;

	if (type == 5) {
		animDataTable[i].var1 = width / 8;
	} else {
		animDataTable[i].var1 = width / 16;
	}

	animDataTable[i].field_4 = type;	// bbp ?

	animDataTable[i].var2 = height;

	animDataTable[i].fileIdx = -1;
	animDataTable[i].frameIdx = -1;

	frameVar0++;

	return (i);
}

void generateMask(uint8 * sprite, uint8 * mask, uint16 size, uint8 transparency) {
	uint16 i;

	for (i = 0; i < size; i++) {
		if (*(sprite++) != transparency) {
			*(mask++) = 0;
		} else {
			*(mask++) = 1;
		}
	}
}

void convertMask(uint8 * dest, uint8 * source, int16 width, int16 height) {
	int16 i;
	int16 j;

	for (i = 0; i < width * height; i++) {
		uint8 maskEntry = *(source++);
		for (j = 0; j < 8; j++) {
			if (maskEntry & 0x80) {
				*(dest++) = 0;
			} else {
				*(dest++) = 1;
			}

			maskEntry <<= 1;
		}
	}
}

void convert4BBP(uint8 * dest, uint8 * source, int16 width, int16 height) {
	int16 i;

	for (i = 0; i < width * height; i++) {
		uint8 maskEntry = *(source++);
		*(dest++) = (maskEntry & 0xF0) >> 4;
		*(dest++) = (maskEntry & 0xF);
	}
}

void loadSpl(char *resourceName) {
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	entry = allocFrame((uint16) partBuffer[foundFileIdx].unpacked_size, 1, -1);

	ASSERT(entry != -1);

	memcpy(animDataTable[entry].ptr1, dataPtr, (uint16) partBuffer[foundFileIdx].unpacked_size);

	animDataTable[entry].fileIdx = foundFileIdx;
	animDataTable[entry].frameIdx = 0;
	strcpy(animDataTable[entry].name, currentPartName);
}

void loadMsk(char *resourceName) {
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;
	uint8 *ptr;
	int16 i;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	ptr = dataPtr;

	memcpy(&animHeader, ptr, sizeof(animHeaderStruct));
	ptr += sizeof(animHeaderStruct);

	flipU16(&animHeader.frameWidth);
	flipU16(&animHeader.frameHeight);
	flipU16(&animHeader.numFrames);

	for (i = 0; i < animHeader.numFrames; i++) {
		entry = allocFrame(animHeader.frameWidth * 2, animHeader.frameHeight, 1);

		ASSERT(entry != -1);

		convertMask(animDataTable[entry].ptr1, ptr, animHeader.frameWidth, animHeader.frameHeight);
		ptr += animHeader.frameWidth * animHeader.frameHeight;

		animDataTable[entry].fileIdx = foundFileIdx;
		animDataTable[entry].frameIdx = i;
		strcpy(animDataTable[entry].name, currentPartName);
	}
}

void loadAni(char *resourceName) {
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;
	uint8 *ptr;
	int16 i;
	uint8 transparentColor;
	uint32 fullSize;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	ptr = dataPtr;

	memcpy(&animHeader, ptr, sizeof(animHeaderStruct));
	ptr += sizeof(animHeaderStruct);

	flipU16(&animHeader.frameWidth);
	flipU16(&animHeader.frameHeight);
	flipU16(&animHeader.numFrames);

	transparentColor = findAnimInHardcodedData(resourceName);

	fullSize = animHeader.frameWidth * animHeader.frameHeight;

	for (i = 0; i < animHeader.numFrames; i++) {
		uint8 *animPtr;

		entry = allocFrame(animHeader.frameWidth * 2, animHeader.frameHeight, 0);

		ASSERT(entry != -1);

		// special case transparency handling
		if (!strcmp(resourceName, "L2202.ANI")) {
			if (i < 2) {
				transparentColor = 0;
			} else {
				transparentColor = 7;
			}
		}

		if (!strcmp(resourceName, "L4601.ANI")) {
			if (i < 1) {
				transparentColor = 0xE;
			} else {
				transparentColor = 0;
			}
		}

		animPtr = (uint8 *)malloc(fullSize);

		memcpy(animPtr, ptr, fullSize);
		ptr += fullSize;

		gfxConvertSpriteToRaw(animDataTable[entry].ptr1, animPtr, animHeader.frameWidth, animHeader.frameHeight);

		generateMask(animDataTable[entry].ptr1, animDataTable[entry].ptr2, animHeader.frameWidth * 2 * animHeader.frameHeight, transparentColor);

		free(animPtr);

		animDataTable[entry].fileIdx = foundFileIdx;
		animDataTable[entry].frameIdx = i;
		strcpy(animDataTable[entry].name, currentPartName);
	}
}

void convert8BBP(uint8 * dest, uint8 * source, int16 width, int16 height) {
	uint16 i;
	uint8 table[16];

	memcpy(table, source, 16);
	source += 16;

	for (i = 0; i < width * height; i++) {
		uint8 color = *(source++);

		*(dest++) = table[color >> 4];
		*(dest++) = table[color & 0xF];
	}
}

void convert8BBP2(uint8 * dest, uint8 * source, int16 width, int16 height) {
	uint16 i;
	uint16 j;

	uint8 al;
	uint8 ah;
	uint8 bl;
	uint8 bh;
	uint8 cl;
	uint8 ch;
	uint8 dl;
	uint8 dh;
	uint8 color;

	for (j = 0; j < (width * height) / 16; j++) {
		al = *(source);
		ah = *(source + 2);
		bl = *(source + 4);
		bh = *(source + 6);
		cl = *(source + 8);
		ch = *(source + 0xA);
		dl = *(source + 0xC);
		dh = *(source + 0xE);

		for (i = 0; i < 8; i++) {
			color = 0;

			color |= ((dh & 0x080) >> 7);
			dh <<= 1;
			color <<= 1;
			color |= ((dl & 0x080) >> 7);
			dl <<= 1;
			color <<= 1;
			color |= ((ch & 0x080) >> 7);
			ch <<= 1;
			color <<= 1;
			color |= ((cl & 0x080) >> 7);
			cl <<= 1;
			color <<= 1;
			color |= ((bh & 0x080) >> 7);
			bh <<= 1;
			color <<= 1;
			color |= ((bl & 0x080) >> 7);
			bl <<= 1;
			color <<= 1;
			color |= ((ah & 0x080) >> 7);
			ah <<= 1;
			color <<= 1;
			color |= ((al & 0x080) >> 7);
			al <<= 1;

			*(dest++) = color;
		}

		al = *(source + 1);
		ah = *(source + 3);
		bl = *(source + 5);
		bh = *(source + 7);
		cl = *(source + 9);
		ch = *(source + 0xB);
		dl = *(source + 0xD);
		dh = *(source + 0xF);

		for (i = 0; i < 8; i++) {
			color = 0;

			color |= ((dh & 0x080) >> 7);
			dh <<= 1;
			color <<= 1;
			color |= ((dl & 0x080) >> 7);
			dl <<= 1;
			color <<= 1;
			color |= ((ch & 0x080) >> 7);
			ch <<= 1;
			color <<= 1;
			color |= ((cl & 0x080) >> 7);
			cl <<= 1;
			color <<= 1;
			color |= ((bh & 0x080) >> 7);
			bh <<= 1;
			color <<= 1;
			color |= ((bl & 0x080) >> 7);
			bl <<= 1;
			color <<= 1;
			color |= ((ah & 0x080) >> 7);
			ah <<= 1;
			color <<= 1;
			color |= ((al & 0x080) >> 7);
			al <<= 1;

			*(dest++) = color;
		}

		source += 0x10;
	}
}

void loadSet(char *resourceName) {
	animHeader2Struct header2;
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;
	uint8 *ptr;
	int16 i;
	uint32 fullSize;
	uint16 numSpriteInAnim;
	uint8 *startOfDataPtr;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	ASSERT(!memcmp(dataPtr, "SET", 3));

	ptr = dataPtr + 4;

	numSpriteInAnim = *(uint16 *)ptr;
	flipU16(&numSpriteInAnim);
	ptr += 2;

	startOfDataPtr = ptr + numSpriteInAnim * 0x10;

	for (i = 0; i < numSpriteInAnim; i++) {
		int16 typeParam;
		uint8 table[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

		memcpy(&header2, ptr, 0x10);
		ptr += 0x10;

		flipU32(&header2.field_0);
		flipU16(&header2.width);
		flipU16(&header2.height);
		flipU16(&header2.type);

		fullSize = header2.width * header2.height;

		typeParam = 0;

		if (header2.type == 5) {
			fullSize += 16;
		}

		if (header2.type == 4) {
			header2.type = 5;
			typeParam = 1;
		}

		if (typeParam) {
			entry = allocFrame2(header2.width * 2, header2.height, header2.type);
		} else {
			if (header2.type == 1) {
				entry = allocFrame2(header2.width * 2, header2.height, header2.type);
			} else {
				entry = allocFrame2(header2.width * 2, header2.height, header2.type);
			}
		}

		ASSERT(entry != -1);

		dataPtr = startOfDataPtr + header2.field_0;

		if (typeParam) {
			memcpy(animDataTable[entry].ptr1, table, 0x10);
			gfxConvertSpriteToRaw(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			//memcpy(animDataTable[entry].ptr1 + 0x10, dataPtr, fullSize);
		} else {
			if (header2.type == 1) {
				convert4BBP(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			} else if (header2.type == 5) {
				convert8BBP(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			} else if (header2.type == 4) {
				ASSERT(0);
			} else {
				convert8BBP2(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			}
		}

		animDataTable[entry].fileIdx = foundFileIdx;
		animDataTable[entry].frameIdx = i;
		strcpy(animDataTable[entry].name, currentPartName);
	}
}

void loadSetAbs(char *resourceName, uint16 idx) {
	animHeader2Struct header2;
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;
	uint8 *ptr;
	int16 i;
	uint32 fullSize;
	uint16 numSpriteInAnim;
	uint8 *startOfDataPtr;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	ASSERT(!memcmp(dataPtr, "SET", 3));

	ptr = dataPtr + 4;

	numSpriteInAnim = *(uint16 *) ptr;
	flipU16(&numSpriteInAnim);
	ptr += 2;

	startOfDataPtr = ptr + numSpriteInAnim * 0x10;

	for (i = 0; i < numSpriteInAnim; i++) {
		int16 typeParam;
		uint8 table[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

		memcpy(&header2, ptr, 0x10);
		ptr += 0x10;

		flipU32(&header2.field_0);
		flipU16(&header2.width);
		flipU16(&header2.height);
		flipU16(&header2.type);

		fullSize = header2.width * header2.height;

		typeParam = 0;

		if (header2.type == 5) {
			fullSize += 16;
		}

		if (header2.type == 4) {
			header2.type = 5;
			typeParam = 1;
		}

		if (typeParam) {
			entry = reserveFrame(header2.width * 2, header2.height, header2.type, idx + i);
		} else {
			if (header2.type == 1) {
				entry = reserveFrame(header2.width * 2, header2.height, header2.type, idx + i);
			} else {
				entry = reserveFrame(header2.width * 2, header2.height, header2.type, idx + i);
			}
		}

		ASSERT(entry != -1);

		dataPtr = startOfDataPtr + header2.field_0;

		if (typeParam) {
			memcpy(animDataTable[entry].ptr1, table, 0x10);
			gfxConvertSpriteToRaw(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			//memcpy(animDataTable[entry].ptr1 + 0x10, dataPtr, fullSize);
		} else {
			if (header2.type == 1) {
				convert4BBP(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			} else if (header2.type == 5) {
				convert8BBP(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			} else if (header2.type == 4) {
				ASSERT(0);
			} else {
				convert8BBP2(animDataTable[entry].ptr1, dataPtr, header2.width, header2.height);
			}
		}

		animDataTable[entry].fileIdx = foundFileIdx;
		animDataTable[entry].frameIdx = i;
		strcpy(animDataTable[entry].name, currentPartName);
	}
}

void loadSeq(char *resourceName) {
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	entry = allocFrame2((uint16) partBuffer[foundFileIdx].unpacked_size, 1, 0);

	memcpy(animDataTable[entry].ptr1, dataPtr + 0x16, (uint16) partBuffer[foundFileIdx].unpacked_size - 0x16);
}

void loadSeqAbs(char *resourceName, uint16 idx) {
	int16 foundFileIdx;
	uint8 *dataPtr;
	int16 entry;

	foundFileIdx = findFileInBundle(resourceName);
	dataPtr = readBundleFile(foundFileIdx);

	entry = reserveFrame((uint16) partBuffer[foundFileIdx].unpacked_size, 1, 0, idx);

	memcpy(animDataTable[entry].ptr1, dataPtr + 0x16, (uint16) partBuffer[foundFileIdx].unpacked_size - 0x16);
}

void loadResource(char *resourceName) {
	/* uint8 isMask = 0; */
	/* uint8 isSpl = 0; */

	if (strstr(resourceName, ".SPL")) {
		loadSpl(resourceName);
		return;
	} else if (strstr(resourceName, ".MSK")) {
		loadMsk(resourceName);
		return;
	} else if (strstr(resourceName, ".ANI")) {
		loadAni(resourceName);
		return;
	} else if (strstr(resourceName, ".ANM")) {
		loadAni(resourceName);
		return;
	} else if (strstr(resourceName, ".SET")) {
		loadSet(resourceName);
		return;
	} else if (strstr(resourceName, ".SEQ")) {
		loadSeq(resourceName);
		return;
	}

	ASSERT(0);
}

void loadAbs(char *resourceName, uint16 idx) {
	/* uint8 isMask = 0; */
	/* uint8 isSpl = 0; */

	if (strstr(resourceName, ".SET")) {
		loadSetAbs(resourceName, idx);
		return;
	} else if (strstr(resourceName, ".H32")) {
		return;
	} else if (strstr(resourceName, ".SEQ")) {
		loadSeqAbs(resourceName, idx);
		return;
	} else if (strstr(resourceName, ".SPL")) {
		return;
	} else if (strstr(resourceName, ".AMI")) {
		return;
	} else if (strstr(resourceName, ".ANI")) {
		return;
	}

	ASSERT(0);
}

void loadResourcesFromSave() {
	char part[256];
	int16 currentAnim;

	strcpy(part, currentPartName);

	for (currentAnim = 0; currentAnim < NUM_MAX_ANIMDATA; currentAnim++) {
		animDataStruct *currentPtr = &animDataTable[currentAnim];
		if (currentPtr->ptr1 && currentPtr->fileIdx != -1) {
			int8 isMask = 0;
			int8 isSpl = 0;
			int16 foundFileIdx;
			uint8 *dataPtr;
			uint8 *ptr;
			char animName[256];

			if (strcmp(currentPartName, currentPtr->name)) {
				closePart();
				loadPart(currentPtr->name);
			}

			foundFileIdx = currentPtr->fileIdx;

			strcpy(animName, partBuffer[foundFileIdx].part_name);

			if (strstr(animName, ".SPL")) {
				isSpl = 1;
			} else {
				isSpl = 0;
			}

			dataPtr = readBundleFile(foundFileIdx);

			ptr = dataPtr;

			if (strstr(animName, ".MSK")) {
				isMask = 1;
			}

			if (isSpl) {
				animHeader.frameWidth = (uint16) partBuffer[foundFileIdx].unpacked_size;
				animHeader.frameHeight = 1;
				animHeader.numFrames = 1;
				isMask = -1;
			} else {
				memcpy(&animHeader, ptr, sizeof(animHeaderStruct));
				ptr += sizeof(animHeaderStruct);

				flipU16(&animHeader.frameWidth);
				flipU16(&animHeader.frameHeight);
				flipU16(&animHeader.numFrames);
			}

			{
				uint16 fullSize;
				uint16 i;
				uint8 transparentColor;

				fullSize = animHeader.frameWidth * animHeader.frameHeight;

				loadRelatedPalette(animName);

				transparentColor = findAnimInHardcodedData(animName);

				for (i = 0; i < animHeader.numFrames; i++) { // load all the frames
					int16 entry;
					uint8 *animPtr;

					// special case transparency handling
					if (!strcmp(animName, "L2202.ANI")) {
						if (i < 2) {
							transparentColor = 0;
						} else {
							transparentColor = 7;
						}
					}

					if (!strcmp(animName, "L4601.ANI")) {
						if (i < 1) {
							transparentColor = 0xE;
						} else {
							transparentColor = 0;
						}
					}

					currentPtr[i].ptr1 = NULL;
					entry = allocFrame(animHeader.frameWidth * 2, animHeader.frameHeight, isMask);

					currentPtr->fileIdx = foundFileIdx;

					ASSERT(entry != -1);

					if (isSpl) {
						memcpy(animDataTable[entry].ptr1, ptr, fullSize);
						ptr += fullSize;
					} else {
						if (!isMask) {
							animPtr = (uint8 *)malloc(fullSize);
							memcpy(animPtr, ptr, fullSize);
							ptr += fullSize;

							gfxConvertSpriteToRaw(animDataTable[entry].ptr1, animPtr,
										animHeader.frameWidth, animHeader.frameHeight);
							generateMask(animDataTable[entry].ptr1, animDataTable[entry].ptr2,
										animHeader.frameWidth * 2 *animHeader.frameHeight, transparentColor);

							free(animPtr);
						} else {
							convertMask(animDataTable[entry].ptr1, ptr, animHeader.frameWidth,
										animHeader.frameHeight);
							ptr += fullSize;
						}
					}

					//animDataTable[entry].fileIdx = foundFileIdx; // Only when reading from bundles

					animDataTable[entry].frameIdx = i;
					strcpy(animDataTable[entry].name, currentPartName);
				}
			}

		}
	}

	loadPart(part);
}
