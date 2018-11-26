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

#ifndef GLK_FROTZ_FONTS
#define GLK_FROTZ_FONTS

#include "graphics/font.h"
#include "graphics/surface.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/rect.h"
#include "glk/screen.h"

namespace Glk {
namespace Frotz {

/**
 * Derived screen class that adds in the Infocom character graphics font
 */
class FrotzScreen : public Glk::Screen {
protected:
	/**
	 * Load the fonts
	 */
	virtual void loadFonts(Common::Archive *archive) override;
public:
	/**
	 * Constructor
	 */
	FrotzScreen();
};

/**
 * Implements a fixed width font stored as a grid on a passed surface
 */
class BitmapFont : public Graphics::Font {
private:
	Common::Array<Graphics::ManagedSurface> _chars;
	size_t _startingChar;
	Common::Point _size;
public:
	/**
	 * Constructor
	 */
	BitmapFont(const Graphics::Surface &src, const Common::Point &size,
		uint srcWidth = 8, uint srcHeight = 8, unsigned char startingChar = ' ');

	/**
	 * Get the font height
	 */
	virtual int getFontHeight() const override { return _size.y; }

	/**
	 * Get the maximum character width
	 */
	virtual int getMaxCharWidth() const override { return _size.x; }

	/**
	 * Get the width of the given character
	 */
	virtual int getCharWidth(uint32 chr) const override { return _size.x; }

	/**
	 * Draw a character
	 */
	virtual void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

} // End of namespace Frotz
} // End of namespace Glk

#endif
