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

u8 *textDataPtr;

u8 textTable[256][2][16 * 8];

void generateMask(u8 *sprite, u8 *mask, u16 size, u8 transparency);

void loadTextData(const char *pFileName, u8 *pDestinationBuffer) {
	Common::File pFileHandle;
	u16 entrySize;
	u16 numEntry;
	u16 i;
	u8 *tempBuffer;
	u16 dataSize;

	assert(pFileName);
	assert(pDestinationBuffer);

	pFileHandle.open(pFileName);

	assert(pFileHandle.isOpen());

	pFileHandle.read(&entrySize, 2);
	flipU16(&entrySize);

	pFileHandle.read(&numEntry, 2);
	flipU16(&numEntry);

	dataSize = numEntry * entrySize;
	pFileHandle.read(pDestinationBuffer, numEntry * entrySize);

	tempBuffer = pDestinationBuffer;

	if (gameType == Cine::GID_FW) {
		dataSize = dataSize / 0x4E;

		loadRelatedPalette(pFileName);

		for (i = 0; i < 0x4E; i++) {
			gfxConvertSpriteToRaw(textTable[i][0], tempBuffer, 16,
			    8);
			generateMask(textTable[i][0], textTable[i][1], 16 * 8,
			    0);
			tempBuffer += dataSize;
		}
	} else {
		for (i = 0; i < 90; i++) {
			gfxConvertSpriteToRaw(textTable[i][0], tempBuffer, 8,
			    8);
			generateMask(textTable[i][0], textTable[i][1], 8 * 8,
			    0);
			tempBuffer += 0x40;
		}
	}

	pFileHandle.close();
}
