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


#include "common/endian.h"
#include "common/stream.h"

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

uint16 bgVar0;

void loadCtHigh(byte * currentPtr) {
	memcpy(page3Raw, currentPtr, 320 * 200);
}

byte loadCt(const char *ctName) {
	uint16 header[32];
	byte *ptr, *dataPtr;

	if (currentCtName != ctName)
		strcpy(currentCtName, ctName);

	ptr = dataPtr = readBundleFile(findFileInBundle(ctName));

	if (g_cine->getGameType() == Cine::GType_OS) {
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

		Common::MemoryReadStream readS(ptr, 32);

		for (int i = 0; i < 16; i++) {
			header[i] = readS.readUint16BE();
		}

		gfxConvertSpriteToRaw(page3Raw, ptr + 0x80, 160, 200);
	}

	free(dataPtr);
	return 0;
}

void loadBgHigh(const char *currentPtr) {
	memcpy(palette256, currentPtr, 256 * 3);
	currentPtr += 256 * 3;

	memcpy(page2Raw, currentPtr, 320 * 200);

	colorMode256 = 1;
}

byte loadBg(const char *bgName) {
	byte *ptr, *dataPtr;

	if (currentBgName[0] != bgName)
		strcpy(currentBgName[0], bgName);

	byte fileIdx = findFileInBundle(bgName);
	ptr = dataPtr = readBundleFile(fileIdx);

	uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;
	if (bpp == 8) {
		loadBgHigh((const char *)ptr);
	} else {
		colorMode256 = 0;

		for (int i = 0; i < 16; i++) {
			tempPalette[i] = READ_BE_UINT16(ptr);
			ptr += 2;
		}

		loadRelatedPalette(bgName);

		gfxResetRawPage(page2Raw);
		gfxConvertSpriteToRaw(page2Raw, ptr, 160, 200);
	}
	free(dataPtr);
	return 0;
}

byte *additionalBgTable[9];
byte currentAdditionalBgIdx = 0;
byte currentAdditionalBgIdx2 = 0;

void addBackground(const char *bgName, uint16 bgIdx) {
	byte *ptr, *dataPtr;

	strcpy(currentBgName[bgIdx], bgName);

	byte fileIdx = findFileInBundle(bgName);
	ptr = dataPtr = readBundleFile(fileIdx);

	additionalBgTable[bgIdx] = (byte *) malloc(320 * 200);

	uint16 bpp = READ_BE_UINT16(ptr); ptr += 2;
	if (bpp == 8) {
		ptr += 3 * 256;
		memcpy(additionalBgTable[bgIdx], ptr, 320 * 200);
	} else {
		ptr += 32;
		gfxConvertSpriteToRaw(additionalBgTable[bgIdx], ptr, 160, 200);
	}
	free(dataPtr);
}

} // End of namespace Cine
