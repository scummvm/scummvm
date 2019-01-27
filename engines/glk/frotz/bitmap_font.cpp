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

#include "glk/frotz/bitmap_font.h"

namespace Glk {
namespace Frotz {

/*--------------------------------------------------------------------------*/

BitmapFont::BitmapFont(const Graphics::Surface &src, const Common::Point &size,
		uint srcWidth, uint srcHeight, unsigned char startingChar) :
		_startingChar(startingChar), _size(size) {
	assert(src.format.bytesPerPixel == 1);
	assert((src.w % srcWidth) == 0);
	assert((src.h % srcHeight) == 0);

	// Set up a characters array
	_chars.resize((src.w / srcWidth) * (src.h / srcHeight));

	// Iterate through loading characters
	Common::Rect r(srcWidth, srcHeight);
	int charsPerRow = src.w / srcWidth;
	for (uint idx = 0; idx < _chars.size(); ++idx) {
		r.moveTo((idx % charsPerRow) * srcWidth, (idx / charsPerRow) * srcHeight);

		_chars[idx].create(size.x, size.y, src.format);
		_chars[idx].transBlitFrom(src, r, Common::Rect(0, 0, size.x, size.y));
	}
}

void BitmapFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	const Graphics::ManagedSurface &c = _chars[chr - _startingChar];
	for (int yCtr = 0; yCtr < c.h; ++yCtr) {
		const byte *srcP = (const byte *)c.getBasePtr(0, yCtr);

		for (int xCtr = 0; xCtr < c.w; ++xCtr, ++srcP) {
			if (!*srcP)
				dst->hLine(x + xCtr, y + yCtr, x + xCtr, color);
		}
	}
}

} // End of namespace Frotz
} // End of namespace Glk
