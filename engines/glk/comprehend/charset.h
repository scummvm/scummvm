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

#ifndef GLK_COMPREHEND_CHARSET_H
#define GLK_COMPREHEND_CHARSET_H

#include "graphics/font.h"

namespace Glk {
namespace Comprehend {

/**
 * Fixed width base font
 */
class FixedFont : public Graphics::Font {
protected:
	byte _data[128 - 32][8];

public:
	~FixedFont() override {}

	/**
	 */
	int getFontHeight() const override {
		return 8;
	}

	/**
	 * Query the maximum width of the font.
	 */
	int getMaxCharWidth() const override {
		return 8;
	}

	/**
	 * Query the width of a specific character.
	 */
	int getCharWidth(uint32 chr) const override {
		return 8;
	}

	/**
	 * Query the kerning offset between two characters.
	 */
	int getKerningOffset(uint32 left, uint32 right) const override {
		return 0;
	}

	/**
	 * Calculate the bounding box of a character. It is assumed that
	 * the character shall be drawn at position (0, 0).
	 */
	Common::Rect getBoundingBox(uint32 chr) const override {
		assert(chr < 127);
		return Common::Rect(0, 0, 8, 8);
	}

	/**
	 * Draw a character at a specific point on a surface.
	 */
	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override;
};

/**
 * Font loaded from charset.gda
 */
class CharSet : public FixedFont {
public:
	CharSet();
	~CharSet() override {}
};

/**
 * Talisman font directly from the executable
 */
class TalismanFont : public FixedFont {
public:
	TalismanFont();
	~TalismanFont() override {}
};

} // End of namespace Comprehend
} // End of namespace Glk

#endif
