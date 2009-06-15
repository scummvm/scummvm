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

#ifndef FONT_H
#define FONT_H

#include "graphics/font.h"

namespace Draci {

extern const Common::String kFontSmall;
extern const Common::String kFontBig;

/**
 *  Represents the game's fonts. See docs for setFont() for font format details.
 */

class Font {
	
public: 
	
	Font();
	Font(const Common::String &filename);
	~Font();

	bool setFont(const Common::String &filename);
	uint8 getFontHeight() const { return _fontHeight; };
	uint8 getMaxCharWidth() const { return _maxCharWidth; };
	uint8 getCharWidth(byte chr) const;
	void drawChar(Graphics::Surface *dst, uint8 chr, int tx, int ty) const;
	void drawString(Graphics::Surface *dst, Common::String &str, 
					int x, int y, int spacing = 0) const;
	int getStringWidth(Common::String &str, int spacing = 0) const;
	void setColour(uint8 colour);

private:
	uint8 _fontHeight;
	uint8 _maxCharWidth;
	
	/** Pointer to an array of individual char widths */	
	uint8 *_charWidths;
	
	/** Pointer to a raw byte array representing font pixels stored row-wise */
	byte *_charData;
	
	/** Number of glyphs in the font */
	static const unsigned int kCharNum = 138;

	/** 
	 *	Chars are indexed from the space character so this should be subtracted
	 *  to get the index of a glyph
	 */
	static const unsigned int kCharIndexOffset = 32;

	/** The varying font colour; initially set to kFontColour1 */
	uint8 _currentFontColour;

	/** Internal function for freeing fonts when destructing/loading another */
	void freeFont();
};

} // End of namespace Draci

#endif // FONT_H
