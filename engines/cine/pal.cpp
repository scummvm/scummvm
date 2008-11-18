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

#include "cine/cine.h"
#include "cine/various.h"

namespace Cine {

Common::Array<PalEntry> palArray;
static byte paletteBuffer1[16];
static byte paletteBuffer2[16];

void loadPal(const char *fileName) {
	char buffer[20];

	removeExtention(buffer, fileName);

	strcat(buffer, ".PAL");
	palArray.clear();

	Common::File palFileHandle;
	if (!palFileHandle.open(buffer))
		error("loadPal(): Cannot open file %s", fileName);

	uint16 palEntriesCount = palFileHandle.readUint16LE();	
	palFileHandle.readUint16LE(); // entry size

	palArray.resize(palEntriesCount);
	for (uint i = 0; i < palArray.size(); ++i) {
		palFileHandle.read(palArray[i].name, 10);
		palFileHandle.read(palArray[i].pal1, 16);
		palFileHandle.read(palArray[i].pal2, 16);
	}
	palFileHandle.close();
}

int16 findPaletteFromName(const char *fileName) {
	char buffer[10];
	uint16 position = 0;
	uint16 i;

	strcpy(buffer, fileName);

	while (position < strlen(fileName)) {
		if (buffer[position] > 'a' && buffer[position] < 'z') {
			buffer[position] += 'A' - 'a';
		}

		position++;
	}

	for (i = 0; i < palArray.size(); i++) {
		if (!strcmp(buffer, palArray[i].name)) {
			return i;
		}
	}

	return -1;

}

void loadRelatedPalette(const char *fileName) {
	char localName[16];
	byte i;
	int16 paletteIndex;

	removeExtention(localName, fileName);

	paletteIndex = findPaletteFromName(localName);

	if (paletteIndex == -1) {
		for (i = 0; i < 16; i++) {	// generate default palette
			paletteBuffer1[i] = paletteBuffer2[i] = (i << 4) + i;
		}
	} else {
		assert(paletteIndex < (int32)palArray.size());
		memcpy(paletteBuffer1, palArray[paletteIndex].pal1, 16);
		memcpy(paletteBuffer2, palArray[paletteIndex].pal2, 16);
	}
}

void palRotate(uint16 *pal, byte a, byte b, byte c) {
	assert(pal);

	if (c == 1) {
		uint16 currentColor = pal[b];

		for (int i = b; i > a; i--) {
			pal[i] = pal[i - 1];
		}

		pal[a] = currentColor;
	}
}

void palRotate(byte *pal, byte a, byte b, byte c) {
	assert(pal);

	if (c == 1) {
		byte currentR = pal[3 * b + 0];
		byte currentG = pal[3 * b + 1];
		byte currentB = pal[3 * b + 2];

		for (int i = b; i > a; i--) {
			pal[3 * i + 0] = pal[3 * (i - 1) + 0];
			pal[3 * i + 1] = pal[3 * (i - 1) + 1];
			pal[3 * i + 2] = pal[3 * (i - 1) + 2];
		}

		pal[3 * a + 0] = currentR;
		pal[3 * a + 1] = currentG;
		pal[3 * a + 2] = currentB;
	}
}

uint16 transformColor(uint16 baseColor, int r, int g, int b) {
	int8 oriR = CLIP( (baseColor & 0x007)       + b, 0, 7);
	int8 oriG = CLIP(((baseColor & 0x070) >> 4) + g, 0, 7);
	int8 oriB = CLIP(((baseColor & 0x700) >> 8) + r, 0, 7);

	return oriR | (oriG << 4) | (oriB << 8);
}

void transformPaletteRange(uint16 *dstPal, uint16 *srcPal, int startColor, int stopColor, int r, int g, int b) {
	assert(srcPal && dstPal);

	for (int i = startColor; i <= stopColor; i++) {
		dstPal[i] = transformColor(srcPal[i], r, g, b);
	}
}

void transformPaletteRange(byte *dstPal, byte *srcPal, int startColor, int stopColor, int r, int g, int b) {
	assert(srcPal && dstPal);

	for (int i = startColor; i <= stopColor; i++) {
		dstPal[3 * i + 0] = CLIP(srcPal[3 * i + 0] + r * 36, 0, 252);
		dstPal[3 * i + 1] = CLIP(srcPal[3 * i + 1] + g * 36, 0, 252);
		dstPal[3 * i + 2] = CLIP(srcPal[3 * i + 2] + b * 36, 0, 252);
	}
}

} // End of namespace Cine
