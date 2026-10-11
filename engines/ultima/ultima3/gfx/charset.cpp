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

// Reads a file of a number of glyphs into the pixels given
static void loadGlyphs(const char *filename, byte *dest, int count) {
	Common::File f;

	switch (_G(renderMode)) {
	case Data::RENDER_EGA: {
		byte data[CHARSET_GLYPHS * CHARSET_GLYPH_PIXELS / 2];
		if (!f.open(filename) || f.read(data, count * CHARSET_GLYPH_PIXELS / 2) != (uint)(count * CHARSET_GLYPH_PIXELS / 2))
			error("Could not load %s", filename);

		unpack4bpp(data, dest, count * CHARSET_GLYPH_PIXELS);
		break;
	}

	case Data::RENDER_VGA:
		if (!f.open(filename) || f.read(dest, count * CHARSET_GLYPH_PIXELS) != (uint)(count * CHARSET_GLYPH_PIXELS))
			error("Could not load %s", filename);
		break;

	default: {
		byte data[CHARSET_GLYPHS * CGA_GLYPH_BYTES];
		if (!f.open(filename) || f.read(data, count * CGA_GLYPH_BYTES) != (uint)(count * CGA_GLYPH_BYTES))
			error("Could not load %s", filename);

		// The first 8 bytes of a glyph hold the even scanlines and the next 8 the odd
		// ones, 2 bytes (8 pixels) per scanline
		for (int chr = 0; chr < count; ++chr) {
			const byte *glyph = &data[chr * CGA_GLYPH_BYTES];

			for (int y = 0; y < 8; ++y)
				unpack2bpp(glyph + (y & 1) * 8 + (y >> 1) * 2, &dest[chr * CHARSET_GLYPH_PIXELS + y * 8], 8);
		}
		break;
	}
	}
}

void CharSet::load() {
	const char *charset, *moons;

	switch (_G(renderMode)) {
	case Data::RENDER_EGA:
		charset = "charset.ega";
		moons = "moons.ega";
		break;
	case Data::RENDER_VGA:
		charset = "charset.vga";
		moons = "moons.vga";
		break;
	default:
		charset = "CHARSET.ULT";
		moons = "moons.ult";
		break;
	}

	loadGlyphs(charset, _data, CHARSET_TEXT_GLYPHS);
	loadGlyphs(moons, &_data[CHARSET_TEXT_GLYPHS * CHARSET_GLYPH_PIXELS], CHARSET_MOON_GLYPHS);
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
