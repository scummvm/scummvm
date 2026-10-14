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

#include "graphics/surface.h"
#include "ultima/ultima1/gfx/font.h"
#include "ultima/ultima1/ultima1.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

static const byte ARROW[8][8] = {
	{ 15, 15,  0,  0,  0,  0,  0,  0 },
	{  1,  1, 15, 15,  0,  0,  0,  0 },
	{  1,  1,  1,  1, 15, 15,  0,  0 },
	{  1,  1,  1,  1,  1,  1, 15, 15 },
	{  1,  1,  1,  1,  1, 15, 15,  0 },
	{  1,  1,  1, 15, 15,  0,  0,  0 },
	{  1, 15, 15,  0,  0,  0,  0,  0 },
	{ 15,  0,  0,  0,  0,  0,  0,  0 },
};

void Font::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (chr == '\x10' || chr == '\x11') {
		// Get area for the arrow
		Graphics::Surface sub = dst->getSubArea(Common::Rect(x, y, x + getMaxCharWidth(), y + getFontHeight()));

		// Handle drawing left/right arrow in the border color with an edge color outline
		for (int yp = 0; yp < getFontHeight(); ++yp) {
			const byte *src = &ARROW[yp][0];
			byte *dest = (byte *)sub.getBasePtr(0, yp);

			for (int xp = 0; xp < getMaxCharWidth(); ++xp, ++src) {
				byte pixel = *src;
				if (pixel == 15)
					pixel = _G(colors)._edge;
				else if (pixel == 1)
					pixel = _G(colors)._border;

				dest[chr == '\x10' ? xp : 7 - xp] = pixel;
			}
		}

	} else {
		// Otherwise, dispatch to the standard DosFont drawing
		Graphics::DosFont::drawChar(dst, chr, x, y, color);
	}
}

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima
