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

u16 bgVar0;

void loadCtHigh(u8 * currentPtr) {
	currentPtr += 256 * 3;

	memcpy(page3Raw, currentPtr, 320 * 200);
}

u8 loadCt(const char *ctName) {
	u8 *ptr;
	u8 *currentPtr;
	u8 i;
	u16 header[0x20];

	///

	strcpy(currentCtName, ctName);

	currentPtr = ptr = readBundleFile(findFileInBundle(ctName));

	if (gameType == Cine::GID_OS) {
		if (*(u16 *) currentPtr == 0x800)	// detect 256 color background
		{
			loadCtHigh(currentPtr + 2);
			return 0;
		}

		currentPtr += 2;

		currentPtr += 0x20;
		gfxResetRawPage(page3Raw);
		gfxConvertSpriteToRaw(page3Raw, ptr + 0x22, 160, 200);
	} else {
		loadRelatedPalette(ctName);

		ASSERT(strstr(ctName, ".NEO"));

		memcpy(header, currentPtr, 0x20);
		currentPtr += 0x20;

		for (i = 0; i < 0x20; i++) {
			flipU16(&header[i]);
		}

		gfxConvertSpriteToRaw(page3Raw, ptr + 0x80, 160, 200);
	}

	return 0;
}

void loadBgHigh(char *currentPtr) {
	memcpy(palette256, currentPtr, 256 * 3);
	currentPtr += 256 * 3;

	memcpy(page2Raw, currentPtr, 320 * 200);

	colorMode256 = 1;
}

u8 loadBg(const char *bgName) {
	u8 *ptr;
	u8 *currentPtr;
	u8 i;
	u8 fileIdx;

	strcpy(currentBgName[0], bgName);

	fileIdx = findFileInBundle(bgName);

	currentPtr = ptr = readBundleFile(fileIdx);

	if (*(u16 *) currentPtr == 0x800)	// detect 256 color background
	{
		loadBgHigh((char *)currentPtr + 2);
		return 0;
	}

	colorMode256 = 0;

	memcpy(&dummyU16, currentPtr, 2);
	currentPtr += 2;

	memcpy(tempPalette, currentPtr, 32);
	currentPtr += 0x20;

	for (i = 0; i < 16; i++) {
		flipU16(&tempPalette[i]);
	}

	loadRelatedPalette(bgName);

	gfxResetRawPage(page2Raw);
	gfxConvertSpriteToRaw(page2Raw, ptr + 0x22, 160, 200);

	return 0;
}

u8 *additionalBgTable[9] =
    { page2Raw, NULL, NULL, NULL, NULL, NULL, NULL, NULL, page3Raw };
u8 currentAdditionalBgIdx = 0;
u8 currentAdditionalBgIdx2 = 0;

void addBackground(char *bgName, u16 bgIdx) {
	u8 *ptr;
	u8 *currentPtr;
	u8 fileIdx;

	strcpy(currentBgName[bgIdx], bgName);

	fileIdx = findFileInBundle(bgName);

	currentPtr = ptr = readBundleFile(fileIdx);

	additionalBgTable[bgIdx] = (u8 *) malloc(320 * 200);

	if (*(u16 *) currentPtr == 0x800)	// detect 256 color background
	{
		memcpy(additionalBgTable[bgIdx], currentPtr + 2 + 3 * 256,
		    320 * 200);
		return;
	}

	currentPtr += 2;

	currentPtr += 0x20;

	gfxConvertSpriteToRaw(additionalBgTable[bgIdx], ptr + 0x22, 160, 200);
}
