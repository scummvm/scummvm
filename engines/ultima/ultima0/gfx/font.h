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

#ifndef ULTIMA0_GFX_FONT_H
#define ULTIMA0_GFX_FONT_H

#include "common/rect.h"
#include "graphics/managed_surface.h"
#include "ultima/ultima0/data/defines.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

constexpr int GLYPH_HEIGHT = 16;
constexpr int GLYPH_WIDTH = 16;
constexpr int TEXT_WIDTH = DEFAULT_SCX / GLYPH_WIDTH;
constexpr int TEXT_HEIGHT = DEFAULT_SCY / GLYPH_HEIGHT;

class TextRect : public ::Common::Rect {
public:
	TextRect() : ::Common::Rect() {}
	TextRect(int left_, int top_, int right_, int bottom_) :
		::Common::Rect(left_ * GLYPH_WIDTH, top_ * GLYPH_HEIGHT,
			(right_ + 1) * GLYPH_WIDTH, (bottom_ + 1) * GLYPH_HEIGHT) {}
};

class Font {
private:
	/**
	 * Returns true if a pixel is set in the source font data
	 */
	static bool _FONTPixelSet(const byte *Data, int x, int y);

	/**
	 * Draw an angled line - this stops the squared corners on diagonals showing
	 */
	static void _FONTAngleDraw(Graphics::Surface *s, Common::Rect *rc,
		int w, int h, byte colour);

public:
	/**
	 * Draws a character onto the passed surface
	 */
	static void writeChar(Graphics::ManagedSurface *dst, uint32 chr,
		const Common::Point &textPos, byte textColor);
};

} // namespace Gfx
} // namespace Ultima0
} // namespace Ultima

#endif
