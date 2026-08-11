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

#include "got/gfx/font.h"
#include "common/file.h"
#include "got/gfx/gfx_pics.h"

namespace Got {
namespace Gfx {

const byte DIALOG_COLOR[] = {14, 54, 120, 138, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void Font::load() {
	Common::File f;
	if (!f.open("TEXT"))
		error("Could not open font");

	_font.resize(f.size() / 72 + 32);

	for (uint i = 32; i < _font.size(); ++i) {
		byte buff[8 * 9];
		f.read(buff, 8 * 9);

		_font[i].create(8, 9);
		convertPaneDataToSurface(buff, _font[i]);
	}
}

void Font::drawString(Graphics::ManagedSurface *src, const Common::Point &pos, const Common::String &text, int color) {
	const char *string = text.c_str();
	Common::Point pt = pos;

	while (*string) {
		char ch = *string++;
		if (ch == '~' && Common::isXDigit(*string)) {
			ch = *string++;
			if (Common::isDigit(ch)) {
				ch -= 48;
			} else {
				ch = toupper(ch) - 55;
			}
			color = DIALOG_COLOR[(byte)ch];
			continue;
		}
		if (ch > 31 && ch < 127)
			drawChar(src, ch, pt.x, pt.y, color);

		pt.x += 8;
	}
}

void Font::drawChar(Graphics::Surface *dst, const uint32 chr, const int x, const int y, const uint32 color) const {
	// Character drawing is done twice in the original:
	// first at y + 1 with color 0, then at y with the given color
	rawDrawChar(dst, chr, x, y + 1, 0);
	rawDrawChar(dst, chr, x, y, color);
}

void Font::rawDrawChar(Graphics::Surface *dst, const uint32 chr, const int x, const int y, const uint32 color) const {
	const Graphics::ManagedSurface &glyph = _font[chr];

	for (int yp = 0; yp < glyph.h; ++yp) {
		const int startY = y + yp;
		const byte *srcP = (const byte *)glyph.getBasePtr(0, yp);
		byte *destP = (byte *)dst->getBasePtr(x, startY);

		for (int xp = 0; xp < glyph.w; ++xp, ++srcP, ++destP) {
			if (*srcP)
				*destP = color;
		}
	}
}

} // namespace Gfx
} // namespace Got
