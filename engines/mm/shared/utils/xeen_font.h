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

#ifndef MM_UTILS_XEEN_FONT_H
#define MM_UTILS_XEEN_FONT_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"

namespace MM {

/**
 * Implements a font stored as a grid on a passed surface
 */
class XeenFont : public Graphics::Font {
private:
	Common::Array<uint16> _data;
	Common::Array<byte> _widths;
private:
	static byte _colors[4];
	static byte _colorsSet[40][4];
public:
	static void loadColors(Common::SeekableReadStream *src);

	/**
	 * Set the text colors set index
	 */
	static void setColors(uint index);
public:
	XeenFont() {}

	/**
	 * Loads the font
	 */
	void load(Common::SeekableReadStream *src,
		size_t charsOffset, size_t charWidthsOffset);

	/**
	 * Get the font height
	 */
	int getFontHeight() const override { return 8 + 1; }

	/**
	 * Get the maximum character width
	 */
	int getMaxCharWidth() const override { return 8; }

	/**
	 * Get the width of the given character
	 */
	int getCharWidth(uint32 chr) const override;

	/**
	 * Get the string width
	 */
	int getStringWidth(const Common::String &str) const;

	/**
	 * Draw a character
	 */
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
	void drawChar(Graphics::ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override {
		return Graphics::Font::drawChar(dst, chr, x, y, color);
	}
};

} // namespace MM

#endif
