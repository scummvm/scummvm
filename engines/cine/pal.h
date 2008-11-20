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
// All colors are represented internally as 32-bit RGBA, but
// 9-bit color palettes with 16 colors can be loaded and converted on-the-fly.
// TODO: Add palette saving in the peculiar but used 9-bit color format.
// TODO: Make use of this class.
class Palette {
public:
	Palette& load24BitColors(byte *colors, uint colorCount = 256);
	Palette& load9BitColors(uint16 *colors, uint colorCount = 16);
	Palette& rotateRight(byte firstIndex, byte lastIndex);
	Palette& saturatedAddColor(byte firstIndex, byte lastIndex, signed r, signed g, signed b);
	uint getColorCount() const;

private:
	static const byte
		R_INDEX = 0,
		G_INDEX = 1,
		B_INDEX = 2,
		A_INDEX = 3;

	static const uint
		COMPONENTS_PER_COLOR = 4,
		BITS_PER_COMPONENT = 8,
		COMPONENT_MASK = ((1 << BITS_PER_COMPONENT) - 1),
		COMPONENT_MAX = COMPONENT_MASK,
		COMPONENT_MUL = COMPONENT_MAX / 7;

	typedef uint32 PackedColor;

	byte& getComponent(byte colorIndex, byte componentIndex);
	void setComponent(byte colorIndex, byte componentIndex, byte value);
	PackedColor getColor(byte colorIndex);
	void setColor(byte colorIndex, PackedColor color);
	void saturatedAddColor(byte index, signed r, signed g, signed b);

	Common::Array<byte> _colors;
};

} // End of namespace Cine

#endif
