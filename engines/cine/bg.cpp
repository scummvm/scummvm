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
	memcpy(page3Raw, currentPtr, 320 * 200);
}

uint8 loadCt(const char *ctName) {
	uint16 header[32];

	strcpy(currentCtName, ctName);

	uint8 *ptr = readBundleFile(findFileInBundle(ctName));

	if (gameType == Cine::GID_OS) {
		uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;
		if (bpp == 8) {
			ptr += 3 * 256;
			loadCtHigh(ptr);
		} else {
			ptr += 32;
			gfxResetRawPage(page3Raw);
			gfxConvertSpriteToRaw(page3Raw, ptr, 160, 200);
		}
	} else {
		loadRelatedPalette(ctName);

		assert(strstr(ctName, ".NEO"));

		memcpy(header, ptr, 32); ptr += 32;

		for (int i = 0; i < 16; i++) {
			header[i] = TO_BE_16(header[i]);
		}

		gfxConvertSpriteToRaw(page3Raw, ptr + 0x80 - 0x22, 160, 200);
	}

	return 0;
}

void loadBgHigh(const char *currentPtr) {
	memcpy(palette256, currentPtr, 256 * 3);
	currentPtr += 256 * 3;

	memcpy(page2Raw, currentPtr, 320 * 200);

	colorMode256 = 1;
}

uint8 loadBg(const char *bgName) {
	strcpy(currentBgName[0], bgName);

	uint8 fileIdx = findFileInBundle(bgName);
	uint8 *ptr = readBundleFile(fileIdx);

	uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;
	if (bpp == 8) {
		loadBgHigh((const char *)ptr);
	} else {
		colorMode256 = 0;

		memcpy(tempPalette, ptr, 32); ptr += 32;

		for (int i = 0; i < 16; i++) {
			tempPalette[i] = TO_BE_16(tempPalette[i]);
		}

		loadRelatedPalette(bgName);

		gfxResetRawPage(page2Raw);
		gfxConvertSpriteToRaw(page2Raw, ptr, 160, 200);
	}
	return 0;
}

uint8 *additionalBgTable[9] = { page2Raw, NULL, NULL, NULL, NULL, NULL, NULL, NULL, page3Raw };
uint8 currentAdditionalBgIdx = 0;
uint8 currentAdditionalBgIdx2 = 0;

void addBackground(const char *bgName, uint16 bgIdx) {
	strcpy(currentBgName[bgIdx], bgName);

	uint8 fileIdx = findFileInBundle(bgName);
	uint8 *ptr = readBundleFile(fileIdx);

	additionalBgTable[bgIdx] = (uint8 *) malloc(320 * 200);

	uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;
	if (bpp == 8) {
		ptr += 3 * 256;
		memcpy(additionalBgTable[bgIdx], ptr, 320 * 200);
	} else {
		ptr += 32;
		gfxConvertSpriteToRaw(additionalBgTable[bgIdx], ptr, 160, 200);
	}
}

} // End of namespace Cine
