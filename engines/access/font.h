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

#ifndef ACCESS_FONT_H
#define ACCESS_FONT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "access/asurface.h"
#include "access/data.h"

namespace Access {

struct FontVal {
public:
	int _lo, _hi;

	FontVal() { _lo = _hi = 0; }
};

class Font {
protected:
	byte _firstCharIndex;
	int _bitWidth;
	int _height;
	Common::Array<Graphics::Surface> _chars;
protected:
	/**
	 * Constructor
	 */
	Font(byte firstCharIndex);
public:
	static byte _fontColors[4];
public:
	/**
	 * Destructor
	 */
	virtual ~Font();

	/**
	 * Get the width of a given character
	 */
	int charWidth(char c);

	/**
	 * Get the width of a given string
	 */
	int stringWidth(const Common::String &msg);

	/**
	 * Get a partial string that will fit in a given width
	 * @param s			Source string. Modified to remove line
	 * @param maxWidth	Maximum width allowed
	 * @param line		Output line
	 * @param width		Calculated width of returned line
	 * @returns			True if last line
	 */
	bool getLine(Common::String &s, int maxWidth, Common::String &line, int &width);

	/**
	 * Draw a string on a given surface
	 */
	void drawString(BaseSurface *s, const Common::String &msg, const Common::Point &pt);

	/**
	 * Draw a character on a given surface
	 */
	int drawChar(BaseSurface *s, char c, Common::Point &pt);

};

class AmazonFont : public Font {
private:
	/**
	 * Load the given font data
	 */
	void load(const int *fontIndex, const byte *fontData);
public:
	/**
	 * Constructor
	 */
	AmazonFont(const int *fontIndex, const byte *fontData) : Font(32) {
		load(fontIndex, fontData);
	}

};

class MartianFont : public Font {
private:
	/**
	 * Load the given font data
	 */
	void load(Common::SeekableReadStream &s);
public:
	/**
	* Constructor
	*/
	MartianFont(int height, Common::SeekableReadStream &s);
};


class FontManager {
public:
	FontVal _charSet;
	FontVal _charFor;
	int _printMaxX;
	Font *_font1;
	Font *_font2;
public:
	/**
	 * Constructor
	 */
	FontManager();

	/**
	 * Set the fonts
	 */
	void load(Font *font1, Font *font2);
};

} // End of namespace Access

#endif /* ACCESS_FONT_H */
