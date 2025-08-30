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
	int charWidth(char c) const;

	/**
	 * Get the width of a given string
	 */
	int stringWidth(const Common::String &msg) const;

	/**
	 * Type of line wrapping - Martian wraps based on chars, Amazon based on px.
	 *
	 * Since the fonts are variable width we need to support both types to
	 * exactly wrap like the originals.
	 */
	enum LINE_WIDTH_TYPE {
		kWidthInPixels,
		kWidthInChars
	};

	/**
	 * Get a partial string that will fit in a given width
	 * @param s			Source string. Modified to remove line
	 * @param maxWidth	Maximum width allowed in px or chars (see widthType)
	 * @param line		Output line
	 * @param width		Actual width of returned line in selected units
	 * @param widthType Select the type of width constraint - px or chars
	 * @returns			True if last line
	 */
	bool getLine(Common::String &s, int maxWidth, Common::String &line, int &width,
				 LINE_WIDTH_TYPE widthType = kWidthInPixels) const;

	/**
	 * Draw a string on a given surface
	 */
	void drawString(BaseSurface *s, const Common::String &msg, const Common::Point &pt) const;

	/**
	 * Draw a character on a given surface
	 */
	int drawChar(BaseSurface *s, char c, Common::Point &pt) const;

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
	void loadFromStream(Common::SeekableReadStream &s);
	void loadFromData(size_t count, const byte *widths, const int *offsets, const byte *data);
public:
	/**
	* Constructor
	*/
	MartianFont(int height, Common::SeekableReadStream &s);
	MartianFont(int height, size_t count, const byte *widths, const int *offsets, const byte *data);
};

class MartianBitFont : public Font {
public:
	/**
	* Constructor
	*/
	MartianBitFont(size_t count, const byte *data);
};


class FontManager {
public:
	FontVal _charSet;
	FontVal _charFor;
	int _printMaxX;
	Font *_font1;
	Font *_font2;
	Font *_bitFont;
public:
	/**
	 * Constructor
	 */
	FontManager();

	/**
	 * Set the fonts
	 */
	void load(Font *font1, Font *font2, Font *bitFont);
};

} // End of namespace Access

#endif /* ACCESS_FONT_H */
