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
#include "ultima/ultima3/gfx/pixels.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

constexpr int CGA_GLYPH_BYTES = 16;

byte CharSet::_data[CHARSET_GLYPHS * CHARSET_GLYPH_PIXELS];

void CharSet::load() {
	Common::File f;

	switch (_G(renderMode)) {
	case Data::RENDER_EGA: {
		byte data[CHARSET_GLYPHS * CHARSET_GLYPH_PIXELS / 2];
		if (!f.open("charset.ega") || f.read(data, sizeof(data)) != sizeof(data))
			error("Could not load charset.ega");

		unpack4bpp(data, _data, sizeof(_data));
		break;
	}

	case Data::RENDER_VGA:
		if (!f.open("charset.vga") || f.read(_data, sizeof(_data)) != sizeof(_data))
			error("Could not load charset.vga");
		break;

	default: {
		byte data[CHARSET_GLYPHS * CGA_GLYPH_BYTES];
		if (!f.open("CHARSET.ULT") || f.read(data, sizeof(data)) != sizeof(data))
			error("Could not load CHARSET.ULT");

		// The first 8 bytes of a glyph hold the even scanlines and the next 8 the odd
		// ones, 2 bytes (8 pixels) per scanline
		for (int chr = 0; chr < CHARSET_GLYPHS; ++chr) {
			const byte *glyph = &data[chr * CGA_GLYPH_BYTES];

			for (int y = 0; y < 8; ++y)
				unpack2bpp(glyph + (y & 1) * 8 + (y >> 1) * 2, &_data[chr * CHARSET_GLYPH_PIXELS + y * 8], 8);
		}
		break;
	}
	}
}

void CharSet::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (chr >= CHARSET_GLYPHS)
		return;

	const byte *glyph = &_data[chr * CHARSET_GLYPH_PIXELS];

	for (int yp = 0; yp < 8; ++yp)
		memcpy(dst->getBasePtr(x, y + yp), glyph + yp * 8, 8);
}

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima
