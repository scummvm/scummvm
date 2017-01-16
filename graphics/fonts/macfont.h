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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GRAPHICS_FONTS_MACFONT_H
#define GRAPHICS_FONTS_MACFONT_H

#include "common/array.h"
#include "graphics/font.h"

namespace Graphics {

/**
 * Processing of Mac FONT/NFNT rResources
 */
class MacFont : public Font {
public:
	MacFont();
	virtual ~MacFont();

	virtual int getFontHeight() const;
	virtual int getMaxCharWidth() const;
	virtual int getCharWidth(uint32 chr) const;
	virtual void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const;

	bool loadFont(Common::SeekableReadStream &stream);
	bool loadFOND(Common::SeekableReadStream &stream);

private:
	// FONT/NFNT
	uint16 _fontType;
	uint16 _firstChar;
	uint16 _lastChar;
	uint16 _maxWidth;
	int16  _kernMax;
	int16  _nDescent;
	uint16 _fRectWidth;
	uint16 _fRectHeight;
	uint32 _owTLoc;
	uint16 _ascent;
	uint16 _descent;
	uint16 _leading;
	uint16 _rowWords;

	// FOND
	uint16 _ffFlags;
	uint16 _ffFamID;
	uint16 _ffFirstChar;
	uint16 _ffLastChar;
	uint16 _ffAscent;
	uint16 _ffDescent;
	uint16 _ffLeading;
	uint16 _ffWidMax;
	uint32 _ffWTabOff;
	uint32 _ffKernOff;
	uint32 _ffStylOff;
	uint16 _ffProperty[9];
	uint16 _ffIntl[2];
	uint16 _ffVersion;

	byte *_bitImage;

	struct Glyph {
		void clear() {
			bitmapOffset = 0;
			width = 0;
			bitmapWidth = 0;
			kerningOffset = 0;
		}

		uint16 bitmapOffset;
		byte width;
		uint16 bitmapWidth;
		int kerningOffset;
	};

	Common::Array<Glyph> _glyphs;
	Glyph _defaultChar;
	const Glyph *findGlyph(uint32 c) const;
};

} // End of namespace Graphics

#endif
