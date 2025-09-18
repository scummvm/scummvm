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
#ifndef GRAPHICS_FONTS_BGIFONT_H
#define GRAPHICS_FONTS_BGIFONT_H

#include "common/file.h"
#include "common/array.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Graphics {

const int OPCODE_END = 0;
const int OPCODE_DOSCAN = 1;
const int OPCODE_MOVE = 2;
const int OPCODE_DRAW = 3;

class BgiFont : public Font {
public:
	BgiFont();
	~BgiFont();

	bool loadChr(const Common::Path &fileName);
	bool loadChr(Common::SeekableReadStream &stream);

	void close();

	int getFontHeight() const { return _totalHeight; }
	int getCharWidth(uint32 chr) const;
	int getMaxCharWidth() const { return _maxWidth; }
	void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const;

private:
	struct DrawingInstruction {
		int opCode;
		int8 xCoord;
		int8 yCoord;
	};

	struct GlyphEntry {
		GlyphEntry() {
			charWidth = 0;
			offset = 0;
		}
		~GlyphEntry() {}

		uint16 charWidth;
		uint32 offset;
		Common::Array<DrawingInstruction *> insts;
	} *_glyphs;

	struct CachedFont {
		int widths[256];
		int offsets[256];
		Graphics::Surface *surface;
		~CachedFont() {
			delete surface;
		}
	};

	uint16 _charCount;
	byte _firstChar;
	// uint16 _pixHeight;
	uint16 _maxWidth = 10;
	uint32 _totalWidth = 0;
	int16 _totalHeight = 0;
	int8 _originToAscender = 0;
	int8 _originToDescender = 0;
	Common::Array<CachedFont *> _fontCache;

	uint16 characterToIndex(uint32 character) const;
	byte fixSign(byte original);
	CachedFont *drawCachedFont(int size);
};

} // namespace Graphics

#endif
