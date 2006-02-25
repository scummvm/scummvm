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

uint16 tempPalette[256];

uint8 colorMode256 = 0;
uint8 palette256[256 * 3];

uint16 palVar;
uint16 palVar0;

Common::File palFileHandle;

palEntryStruct *palPtr;

uint8 paletteBuffer1[16];
uint8 paletteBuffer2[16];

void loadPal(const char *fileName) {
	char buffer[20];

	removeExtention(buffer, fileName);

	strcat(buffer, ".PAL");

	palFileHandle.close();

	if (palPtr) {
		free(palPtr);
		palPtr = NULL;
	}

	palVar = 0;
	palVar0 = 42;

	palFileHandle.open(buffer);

	ASSERT(palFileHandle.isOpen());

	palFileHandle.read(&palVar, 2);	// endian: not fliped !
	palFileHandle.read(&palVar0, 2);

	palPtr = (palEntryStruct *) malloc(palVar * palVar0);

	ASSERT_PTR(palPtr);

	palFileHandle.read(palPtr, palVar * palVar0);
}

int16 findPaletteFromName(const char *fileName) {
	char buffer[10];
	uint16 position = 0;
	uint16 i;

	strcpy(buffer, fileName);

	while (position < strlen(fileName)) {
		if (buffer[position] > 'a' && buffer[position] < 'z') {
			buffer[position] += 0xE0;
		}

		position++;
	}

	for (i = 0; i < palVar; i++) {
		if (!strcmp(buffer, palPtr[i].name)) {
			return i;
		}
	}

	return -1;

}

void loadRelatedPalette(const char *fileName) {
	char localName[16];
	uint8 i;
	int16 paletteIndex;

	removeExtention(localName, fileName);

	paletteIndex = findPaletteFromName(localName);

	if (paletteIndex == -1) {
		for (i = 0; i < 16; i++) {	// generate default palette
			paletteBuffer1[i] = paletteBuffer2[i] = (i << 4) + i;
		}
	} else {
		palEntryStruct *palEntryPtr = &palPtr[paletteIndex];

		ASSERT_PTR(paletteBuffer2);

		for (i = 0; i < 16; i++) {	// convert palette
			paletteBuffer1[i] = palEntryPtr->pal1[i];
			paletteBuffer2[i] = palEntryPtr->pal2[i];
		}
	}
}

} // End of namespace Cine
