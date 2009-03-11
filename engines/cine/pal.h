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

#ifndef CINE_PAL_H
#define CINE_PAL_H

#include "graphics/pixelformat.h"

namespace Cine {

struct PalEntry {
	char name[10];
	byte pal1[16];
	byte pal2[16];
};

extern Common::Array<PalEntry> palArray;

void loadPal(const char *fileName);

void loadRelatedPalette(const char *fileName);

void palRotate(uint16 *pal, byte a, byte b, byte c);
void palRotate(byte *pal, byte a, byte b, byte c);
uint16 transformColor(uint16 baseColor, int r, int g, int b);
void transformPaletteRange(uint16 *srcPal, uint16 *dstPal, int startColor, int stopColor, int r, int g, int b);
void transformPaletteRange(byte *srcPal, byte *dstPal, int startColor, int stopColor, int r, int g, int b);

// This class might be used for handling Cine-engine's palettes in the future. WIP!
// TODO: Document
// TODO: Make use of
// TODO: Test
class Palette {
public:
	Palette &loadCineLowPal(const byte *colors, const uint numColors = 16);
	Palette &loadCineHighPal(const byte *colors, const uint numColors = 256);
	Palette &load(const byte *colors, const Graphics::PixelFormat format, const uint numColors);

	byte *saveCineLowPal(byte *colors, const uint numBytes) const;
	byte *saveCineHighPal(byte *colors, const uint numBytes) const;
	byte *saveOrigFormat(byte *colors, const uint numBytes) const;
	byte *saveSystemFormat(byte *colors, const uint numBytes) const;
	byte *save(byte *colors, const uint numBytes, const Graphics::PixelFormat format) const;

	Palette &rotateRight(byte firstIndex, byte lastIndex);
	Palette &saturatedAddColor(byte firstIndex, byte lastIndex, signed r, signed g, signed b);	
	uint colorCount() const;
	Graphics::PixelFormat colorFormat() const;

private:
	void saturatedAddColor(byte index, signed r, signed g, signed b);

private:
	struct Color {
		uint8 r, g, b;
	};

	Graphics::PixelFormat _format;
	uint _rBits, _gBits, _bBits;
	uint _rMax, _gMax, _bMax;

	Common::Array<Color> _colors;
};

} // End of namespace Cine

#endif
