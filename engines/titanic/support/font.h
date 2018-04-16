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

#ifndef TITANIC_FONT_H
#define TITANIC_FONT_H

#include "common/scummsys.h"
#include "common/array.h"
#include "titanic/support/rect.h"
#include "titanic/support/string.h"
#include "titanic/support/text_cursor.h"

namespace Titanic {

enum TextCommand { TEXTCMD_NPC = 26, TEXTCMD_SET_COLOR = 27 };

enum WriteCharacterResult {
	WC_IN_BOUNDS = 0, WC_OUTSIDE_TOP = -1, WC_OUTSIDE_BOTTOM = -2,
	WC_OUTSIDE_LEFT = -3, WC_OUTSIDE_RIGHT = -4
};

class CVideoSurface;

class STFont {
	struct CharEntry {
		uint _width;
		uint _offset;
	};
private:
	/**
	 * Copys a rectangle representing a character in the font data to
	 * a given destination position in the surface
	 */
	void copyRect(CVideoSurface *surface, const Common::Point &destPos,
		Rect &srcRect);

	/**
	 * Write a character
	 */
	WriteCharacterResult writeChar(CVideoSurface *surface, unsigned char c,
		const Common::Point &pt, const Rect &destRect, const Rect *srcRect);

	/**
	 * Extends a passed text area by the space required for
	 * the given character
	 */
	void extendBounds(Point &textSize, byte c, int maxWidth) const;

	/**
	 * Called at spacing between words, checks for line wrapping
	 */
	void checkLineWrap(Point &textSize, int maxWidth, const char *&str) const;
public:
	byte *_dataPtr;
	size_t _dataSize;
	int _fontHeight;
	uint _dataWidth;
	CharEntry _chars[256];
	byte _fontR, _fontG, _fontB;
public:
	STFont();
	~STFont();

	/**
	 * Load a specified font
	 */
	void load(int fontNumber);

	/**
	 * Return the width in pixels of the specified text
	 */
	int stringWidth(const CString &text) const;

	/**
	 * Write a string to the specified surface
	 * @returns		The index of the last character that was visible
	 *				with the drawing area
	 */
	int writeString(CVideoSurface *surface, const Rect &rect1, const Rect &destRect,
		int yOffset, const CString &str, CTextCursor *textCursor);

	/**
	 * Write a string to the specified surface
	 */
	void writeString(CVideoSurface *surface, const Point &destPos, Rect &clipRect,
		const CString &str, int lineWidth = 0);

	/**
	 * Get the text area a string will fit into
	 * @param str		String
	 * @param maxWidth	Maximum width in pixels
	 * @param sizeOut	Optional pointer to output size (width, height)
	 * @returns			Required height
	 */
	int getTextBounds(const CString &str, int maxWidth, Point *sizeOut) const;

	/**
	 * Sets the font color
	 */
	void setColor(byte r, byte g, byte b);

	/**
	 * Gets the font color
	 */
	uint16 getColor() const;
};

} // End of namespace Titanic

#endif /* TITANIC_FONT_H */
