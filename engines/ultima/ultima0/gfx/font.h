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

#include "graphics/managed_surface.h"
#include "ultima/ultima0/defines.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

constexpr int CHAR_HEIGHT = 16;
constexpr int CHAR_WIDTH = 16;
constexpr int TEXT_WIDTH = DEFAULT_SCX / CHAR_WIDTH;
constexpr int TEXT_HEIGHT = DEFAULT_SCY / CHAR_HEIGHT;

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
