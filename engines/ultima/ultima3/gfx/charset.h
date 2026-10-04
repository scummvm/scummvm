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

#ifndef ULTIMA3_GFX_CHARSET_H
#define ULTIMA3_GFX_CHARSET_H

#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int CHARSET_GLYPHS = 128;
constexpr int CHARSET_GLYPH_BYTES = 16;

/**
 * The game's CGA font from CHARSET.ULT: 128 glyphs of 8x8 pixels at 2 bits
 * per pixel, so each glyph carries its own colors.
 */
class CharSet : public Graphics::Font {
private:
	static byte _data[CHARSET_GLYPHS * CHARSET_GLYPH_BYTES];

public:
	static void load();

public:
	/**
	 * Return the height of the font.
	 *
	 * @return Font height in pixels.
	 */
	int getFontHeight() const override {
		return 8;
	}

	/**
	 * Return the maximum width of the font.
	 *
	 * @return Maximum font width in pixels.
	 */
	int getMaxCharWidth() const override {
		return 8;
	}

	/**
	 * Return the width of a specific character.
	 *
	 * @param chr  The character to query the width of.
	 *
	 * @return The width of the character in pixels.
	 */
	int getCharWidth(uint32 chr) const override {
		return 8;
	}

	/**
	 * Draw a character at a specific point on the surface.
	 *
	 * @param dst   The surface to draw on.
	 * @param chr   The character to draw.
	 * @param x     The x coordinate where to draw the character.
	 * @param y     The y coordinate where to draw the character.
	 * @param color The color of the character.
	 */
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
