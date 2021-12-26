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

#ifndef GRAPHICS_FONTS_AMIGAFONT_H
#define GRAPHICS_FONTS_AMIGAFONT_H

#include "graphics/font.h"

namespace Graphics {

class AmigaFont : public Font {

#include "common/pack-start.h"
	struct CharLoc {
		uint16	_offset;
		uint16	_length;
	};

	struct AmigaDiskFont {
		uint16	_ySize;
		byte	_style;
		byte	_flags;
		uint16	_xSize;
		uint16	_baseline;
		uint16	_boldSmear;
		uint16	_accessors;	// unused
		byte	_loChar;
		byte	_hiChar;
		uint32	_charData;
		uint16	_modulo;
		uint32	_charLoc;
		uint32	_charSpace;
		uint32	_charKern;
	};
#include "common/pack-end.h"

	AmigaDiskFont	*_font;
	byte			*_data;
	byte			*_charData;
	CharLoc			*_charLoc;
	uint16			*_charSpace;
	uint16			*_charKern;

	byte			*_cp;
	uint32			_pitch;
	int             _maxCharWidth;

private:
	uint16 getPixels(byte c) const;
	uint16 getOffset(byte c) const;

	uint32 mapChar(uint32 c) const;

public:
	/**
	 * Create font in Amiga format.
	 *
	 * @param stream  Stream with the font data. If NULL, then the built-in
	 *				  Topaz font is used.
	 */
	AmigaFont(Common::SeekableReadStream *stream = NULL);
	virtual ~AmigaFont();

	virtual int getFontHeight() const;
	virtual int getCharWidth(uint32 chr) const;
	virtual int getMaxCharWidth() const;
	virtual int getKerningOffset(uint32 left, uint32 right) const;
	virtual void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const;

	int getLoChar() { return _font->_loChar; }
	int getHiChar() { return _font->_hiChar; }
};

} // End of namespace Graphics

#endif
