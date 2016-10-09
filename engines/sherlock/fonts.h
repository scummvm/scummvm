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

#ifndef SHERLOCK_FONTS_H
#define SHERLOCK_FONTS_H

#include "common/rect.h"
#include "common/platform.h"
#include "graphics/surface.h"

namespace Sherlock {

class SherlockEngine;
class ImageFile;
class BaseSurface;

class Fonts {
private:
	static ImageFile *_font;
	static byte _yOffsets[255];
protected:
	static SherlockEngine *_vm;
	static int _fontNumber;
	static int _fontHeight;
	static int _widestChar;
	static uint16 _charCount;

	static void writeString(BaseSurface *surface, const Common::String &str,
		const Common::Point &pt, int overrideColor = 0);

	static inline byte translateChar(byte c);
public:
	/**
	 * Initialise the font manager
	 */
	static void setVm(SherlockEngine *vm);

	/**
	 * Frees the font manager
	 */
	static void freeFont();

	/**
	 * Set the font to use for writing text on the screen
	 */
	void setFont(int fontNum);

	/**
	 * Returns the width of a string in pixels
	 */
	int stringWidth(const Common::String &str);

	/**
	 * Returns the height of a string in pixels (i.e. the tallest displayed character)
	 */
	int stringHeight(const Common::String &str);

	/**
	 * Returns the width of a character in pixels
	 */
	int charWidth(unsigned char c);

	/**
	 * Returns the width of a character in pixels
	 */
	int charHeight(unsigned char c);

	/**
	 * Return the font height
	 */
	int fontHeight() const { return _fontHeight; }

	/**
	 * Return the width of the widest character in the font
	 */
	int widestChar() const { return _widestChar; }

	/**
	 * Return the currently active font number
	 */
	int fontNumber() const { return _fontNumber; }
};

} // End of namespace Sherlock

#endif
