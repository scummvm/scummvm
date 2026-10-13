/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HOLLYWOOD_RUNAWAY_FONT_H
#define HOLLYWOOD_RUNAWAY_FONT_H

#include "common/array.h"
#include "common/ustr.h"
#include "graphics/font.h"

namespace Hollywood {

// RESOURCE.000 stores 150 antialiased glyphs, including their contour masks.
class RunawayFont : public Graphics::Font {
public:
	bool load();
	int getFontHeight() const override { return 28; }
	int getMaxCharWidth() const override { return 28; }
	int getCharWidth(uint32 chr) const override;
	int getKerningOffset(uint32 left, uint32 right) const override;
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

private:
	struct Glyph {
		uint16 offset;
		byte top, height, width;
	};
	uint glyphIndex(uint32 chr) const;
	Glyph _glyphs[150];
	int8 _kerning[150][150];
	Common::Array<byte> _pixels;
	Common::U32String _characters;
};

} // End of namespace Hollywood

#endif
