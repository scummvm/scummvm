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

#ifndef GLK_ZCODE_BITMAP_FONT
#define GLK_ZCODE_BITMAP_FONT

#include "common/array.h"
#include "common/rect.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace Glk {
namespace ZCode {

/**
 * Implements a font stored as a grid on a passed surface
 */
class BitmapFont : public Graphics::Font {
private:
	Common::Array<Graphics::ManagedSurface> _chars;
	size_t _startingChar;
	Common::Point _size;
protected:
	/**
	 * Calculate a character width 
	 */
	int getSourceCharacterWidth(uint charIndex, const Graphics::Surface &src,
		const Common::Rect &charBounds);

	/**
	 * Constructor
	 */
	BitmapFont(const Graphics::Surface &src, const Common::Point &size,
		uint srcWidth, uint srcHeight, unsigned char startingChar, bool isFixedWidth);

public:
	/**
	 * Get the font height
	 */
	int getFontHeight() const override { return _size.y; }

	/**
	 * Get the maximum character width
	 */
	int getMaxCharWidth() const override { return _size.x; }

	/**
	 * Get the width of the given character
	 */
	int getCharWidth(uint32 chr) const override { return _chars[chr - _startingChar].w; }

	/**
	 * Draw a character
	 */
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

/**
 * Subclass for fixed width fonts
 */
class FixedWidthBitmapFont : public BitmapFont {
public:
	/**
	 * Constructor
	 */
	FixedWidthBitmapFont(const Graphics::Surface &src, const Common::Point &size,
		uint srcWidth = 8, uint srcHeight = 8, unsigned char startingChar = ' ') :
		BitmapFont(src, size, srcWidth, srcHeight, startingChar, true) {}
};


/**
 * Subclass for fixed width fonts
 */
class VariableWidthBitmapFont : public BitmapFont {
public:
	/**
	 * Constructor
	 */
	VariableWidthBitmapFont(const Graphics::Surface &src, const Common::Point &size,
		uint srcWidth = 8, uint srcHeight = 8, unsigned char startingChar = ' ') :
		BitmapFont(src, size, srcWidth, srcHeight, startingChar, false) {}
};

} // End of namespace ZCode
} // End of namespace Glk

#endif
