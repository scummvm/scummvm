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

#ifndef XEEN_FONT_H
#define XEEN_FONT_H

#include "xeen/xsurface.h"

namespace Xeen {

#define FONT_WIDTH 8
#define FONT_HEIGHT 8
#define DEFAULT_BG_COLOR 0x99

enum Justify { JUSTIFY_NONE = 0, JUSTIFY_CENTER = 1, JUSTIFY_RIGHT = 2 };

struct FontData {
	static const byte *_fontData;
	static Common::Point *_fontWritePos;
	static byte _textColors[4];
	static byte _bgColor;
	static bool _fontReduced;
	static Justify _fontJustify;
};

class FontSurface: public XSurface, public FontData {
private:
	const char *_displayString;
	bool _msgWraps;

	/**
	 * Return the next pending character to display
	 */
	char getNextChar();

	/**
	 * Return the width of a given character
	 */
	bool getNextCharWidth(int &total);

	/**
	 * Handles moving to the next line of the given bounded area
	 */
	bool newLine(const Common::Rect &bounds);

	/**
	 * Extract a number of a given maximum length from the string
	 */
	int fontAtoi(int len = 3);

	/**
	 * Set the text colors based on the specified index in the master text colors list
	 */
	void setTextColor(int idx);

	/**
	 * Wrie a character to the surface
	 */
	void writeChar(char c, const Common::Rect &clipRect);
public:
	Common::Point &_writePos;
public:
	FontSurface();
	FontSurface(int wv, int hv);
	~FontSurface() override {}

	/**
	 * Draws a symbol to the surface.
	 * @param symbolId	Symbol number from 0 to 19
	 */
	void writeSymbol(int symbolId);

	/**
	 * Write a string to the surface
	 * @param s			String to display
	 * @param clipRect	Window bounds to display string within
	 * @returns			Any string remainder that couldn't be displayed
	 * @remarks		Note that bounds is just used for wrapping purposes. Unless
	 *		justification is set, the message will be written at _writePos
	 */
	const char *writeString(const Common::String &s, const Common::Rect &clipRect);

	/**
	 * Write a charcter to the window
	 * @param c			Character
	 * @param clipRect	Window bounds to display string within
	 */
	void writeCharacter(char c, const Common::Rect &clipRect);
};

} // End of namespace Xeen

#endif /* XEEN_FONT_H */
