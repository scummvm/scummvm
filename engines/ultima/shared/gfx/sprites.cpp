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

#include "ultima/shared/gfx/sprites.h"
#include "ultima/shared/core/file.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

Sprite::Sprite() {
}

Sprite::Sprite(const Sprite &src) {
	_surface.copyFrom(src._surface);
	_transSurface.copyFrom(src._transSurface);
}

Sprite::Sprite(const byte *src, uint bpp, uint16 w, uint16 h) {
	_surface.create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	assert((w % bpp) == 0);
	byte v = 0;

	for (int y = 0; y < h; ++y) {
		byte *destP = (byte *)_surface.getBasePtr(0, y);

		for (int x = 0; x < w; ++x, v <<= bpp) {
			if ((x % (8 / bpp)) == 0)
				v = *src++;

			*destP++ = (((uint)v << bpp) & 0xff00) >> 8;
		}
	}
}

void Sprite::draw(Graphics::ManagedSurface &dest, const Common::Point &pt) {
	// Get area to be drawn on
	Graphics::Surface s = dest.getSubArea(Common::Rect(pt.x, pt.y, pt.x + _surface.w, pt.y + _surface.h));

	// Draw the sprite
	for (uint16 y = 0; y < _surface.h; ++y) {
		const byte *srcP = (const byte *)_surface.getBasePtr(0, y);
		const byte *transP = (const byte *)_transSurface.getBasePtr(0, y);
		byte *destP = (byte *)s.getBasePtr(0, y);

		for (uint16 x = 0; x < _surface.w; ++x, ++srcP, ++transP, ++destP) {
			if (_transSurface.empty() || *transP)
				*destP = *srcP;
		}
	}
}

/*-------------------------------------------------------------------*/

void Sprites::load(const Common::String &name, uint bpp, uint16 w, uint16 h) {
	_spriteSize = Point(w, h);

	// Open the tiles for access
	File f(name);
	byte *buffer = new byte[w * h];

	// Figure out how many tiles the file has
	size_t bytesPerTile = (w / (8 / bpp)) * h;
	size_t count = f.size() / bytesPerTile;

	// Ensure there's enough capacity for the tileset
	if (count > size())
		_data.resize(count);

	// Iterate through loading the tile data and creating sprites for them
	for (size_t idx = 0; idx < count; ++idx) {
		f.read(buffer, bytesPerTile);

		_data[idx] = Sprite(buffer, bpp, w, h);
	}

	delete[] buffer;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
