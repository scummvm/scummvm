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
#include "cine/various.h"

namespace Cine {

uint16 bgVar0;

void loadCtHigh(uint8 * currentPtr) {
	currentPtr += 256 * 3;

	memcpy(page3Raw, currentPtr, 320 * 200);
}

uint8 loadCt(const char *ctName) {
	uint8 *ptr;
	uint8 *currentPtr;
	uint8 i;
	uint16 header[32];

	///

	strcpy(currentCtName, ctName);

	currentPtr = ptr = readBundleFile(findFileInBundle(ctName));

	if (gameType == Cine::GID_OS) {
		if (*(uint16 *) currentPtr == 0x800) {	// detect 256 color background
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

		memcpy(header, currentPtr, 32);
		currentPtr += 32;

		for (i = 0; i < 16; i++) {
			header[i] = TO_BE_16(header[i]);
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

uint8 loadBg(const char *bgName) {
	uint8 *ptr;
	uint8 *currentPtr;
	uint8 i;
	uint8 fileIdx;

	strcpy(currentBgName[0], bgName);

	fileIdx = findFileInBundle(bgName);

	currentPtr = ptr = readBundleFile(fileIdx);

	if (*(uint16 *) currentPtr == 0x800) {	// detect 256 color background
		loadBgHigh((char *)currentPtr + 2);
		return 0;
	}

	colorMode256 = 0;

	memcpy(&dummyU16, currentPtr, 2);
	currentPtr += 2;

	memcpy(tempPalette, currentPtr, 32);
	currentPtr += 32;

	for (i = 0; i < 16; i++) {
		tempPalette[i] = TO_BE_16(tempPalette[i]);
	}

	loadRelatedPalette(bgName);

	gfxResetRawPage(page2Raw);
	gfxConvertSpriteToRaw(page2Raw, ptr + 0x22, 160, 200);

	return 0;
}

uint8 *additionalBgTable[9] = { page2Raw, NULL, NULL, NULL, NULL, NULL, NULL, NULL, page3Raw };
uint8 currentAdditionalBgIdx = 0;
uint8 currentAdditionalBgIdx2 = 0;

void addBackground(char *bgName, uint16 bgIdx) {
	uint8 *ptr;
	uint8 *currentPtr;
	uint8 fileIdx;

	strcpy(currentBgName[bgIdx], bgName);

	fileIdx = findFileInBundle(bgName);

	currentPtr = ptr = readBundleFile(fileIdx);

	additionalBgTable[bgIdx] = (uint8 *) malloc(320 * 200);

	if (*(uint16 *) currentPtr == 0x800) {	// detect 256 color background
		memcpy(additionalBgTable[bgIdx], currentPtr + 2 + 3 * 256, 320 * 200);
		return;
	}

	currentPtr += 2;

	currentPtr += 0x20;

	gfxConvertSpriteToRaw(additionalBgTable[bgIdx], ptr + 0x22, 160, 200);
}

} // End of namespace Cine
