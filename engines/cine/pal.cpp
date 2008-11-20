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

byte& Palette::getComponent(byte colorIndex, byte componentIndex) {
	assert(colorIndex < getColorCount() && componentIndex < COMPONENTS_PER_COLOR);
	return _colors[colorIndex * COMPONENTS_PER_COLOR + componentIndex];
}

void Palette::setComponent(byte colorIndex, byte componentIndex, byte value) {
	getComponent(colorIndex, componentIndex) = value;
}

Palette::PackedColor Palette::getColor(byte colorIndex) {
	return (getComponent(colorIndex, R_INDEX) << (R_INDEX * BITS_PER_COMPONENT)) |
		   (getComponent(colorIndex, G_INDEX) << (G_INDEX * BITS_PER_COMPONENT)) |
		   (getComponent(colorIndex, B_INDEX) << (B_INDEX * BITS_PER_COMPONENT)) |
		   (getComponent(colorIndex, A_INDEX) << (A_INDEX * BITS_PER_COMPONENT));
}

void Palette::setColor(byte colorIndex, PackedColor color) {
	setComponent(colorIndex, R_INDEX, (color >> (R_INDEX * BITS_PER_COMPONENT)) & COMPONENT_MASK);
	setComponent(colorIndex, G_INDEX, (color >> (G_INDEX * BITS_PER_COMPONENT)) & COMPONENT_MASK);
	setComponent(colorIndex, B_INDEX, (color >> (B_INDEX * BITS_PER_COMPONENT)) & COMPONENT_MASK);
	setComponent(colorIndex, A_INDEX, (color >> (A_INDEX * BITS_PER_COMPONENT)) & COMPONENT_MASK);
}

// a.k.a. palRotate
Palette& Palette::rotateRight(byte firstIndex, byte lastIndex) {
	PackedColor lastColor = getColor(lastIndex);

	for (int i = lastIndex; i > firstIndex; i--) {
		setColor(i, getColor(i - 1));
	}

	setColor(firstIndex, lastColor);
	return *this;
}

uint Palette::getColorCount() const {
	return _colors.size() / COMPONENTS_PER_COLOR;
}

// a.k.a. transformPaletteRange
Palette& Palette::saturatedAddColor(byte firstIndex, byte lastIndex, signed r, signed g, signed b) {
	for (uint i = firstIndex; i <= lastIndex; i++) {
		saturatedAddColor(i, r, g, b);
	}
	return *this;
}

// a.k.a. transformColor
// Parameter color components (i.e. r, g and b) are in range [-7, 7]
// e.g. r = 7 sets the resulting color's red component to maximum
// e.g. r = -7 sets the resulting color's red component to minimum (i.e. zero)
void Palette::saturatedAddColor(byte index, signed r, signed g, signed b) {
	byte newR = CLIP<int>(getComponent(index, R_INDEX) + r * COMPONENT_MUL, 0, COMPONENT_MAX);
	byte newG = CLIP<int>(getComponent(index, G_INDEX) + g * COMPONENT_MUL, 0, COMPONENT_MAX);
	byte newB = CLIP<int>(getComponent(index, B_INDEX) + b * COMPONENT_MUL, 0, COMPONENT_MAX);

	setComponent(index, R_INDEX, newR);
	setComponent(index, G_INDEX, newG);
	setComponent(index, B_INDEX, newB);
}

Palette& Palette::load9BitColors(uint16 *colors, uint colorCount) {
	setColorCount(colorCount);
	for (uint i = 0; i < colorCount; i++) {
		setComponent(i, R_INDEX, ((colors[i] >> 8) & 7) * COMPONENT_MUL);
		setComponent(i, G_INDEX, ((colors[i] >> 4) & 7) * COMPONENT_MUL);
		setComponent(i, B_INDEX, ((colors[i] >> 0) & 7) * COMPONENT_MUL);
		setComponent(i, A_INDEX, 0);
	}
	return *this;
}

Palette& Palette::load24BitColors(byte *colors, uint colorCount) {
	setColorCount(colorCount);
	for (uint i = 0; i < colorCount; i++) {
		setComponent(i, R_INDEX, colors[i * 3 + 0]);
		setComponent(i, G_INDEX, colors[i * 3 + 1]);
		setComponent(i, B_INDEX, colors[i * 3 + 2]);
		setComponent(i, A_INDEX, 0);
	}
	return *this;
}

void Palette::setColorCount(uint colorCount) {
	_colors.resize(colorCount * COMPONENTS_PER_COLOR);
}

} // End of namespace Cine
