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

#include "common/file.h"
#include "ultima/ultima3/gfx/charset.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

byte CharSet::_data[CHARSET_GLYPHS * CHARSET_GLYPH_BYTES];

void CharSet::load() {
	Common::File f;
	if (!f.open("CHARSET.ULT") || f.read(_data, sizeof(_data)) != sizeof(_data))
		error("Could not load CHARSET.ULT");
}

void CharSet::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (chr >= CHARSET_GLYPHS)
		return;

	// The first 8 bytes hold the even scanlines and the next 8 the odd ones,
	// 2 bytes (8 pixels) per scanline
	const byte *glyph = &_data[chr * CHARSET_GLYPH_BYTES];

	for (int yp = 0; yp < 8; ++yp) {
		const byte *line = glyph + (yp & 1) * 8 + (yp >> 1) * 2;
		byte *destP = (byte *)dst->getBasePtr(x, y + yp);

		for (int xp = 0; xp < 8; ++xp, ++destP)
			*destP = (line[xp >> 2] >> (6 - (xp & 3) * 2)) & 3;
	}
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
