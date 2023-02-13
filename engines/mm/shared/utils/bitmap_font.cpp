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
#include "image/bmp.h"
#include "mm/shared/utils/bitmap_font.h"

namespace MM {

bool BitmapFont::load(const Common::String &filename) {
	Image::BitmapDecoder decoder;
	Common::File f;
	_startingChar = ' ';

	if (!f.open(filename))
		error("Could not open font");
	if (!decoder.loadStream(f))
		error("Could not decode font");

	const Graphics::Surface &src = *decoder.getSurface();
	assert(src.format.bytesPerPixel == 1);
	assert((src.w % 8) == 0);
	assert((src.h % 8) == 0);

	// Set up a characters array
	_chars.resize(97 - 32);

	// Iterate through loading characters
	Common::Rect r(8, 8);
	int charsPerRow = src.w / 8;
	for (uint idx = 0; idx < _chars.size(); ++idx) {
		r.moveTo((idx % charsPerRow) * 8, (idx / charsPerRow) * 8);
		Common::Rect charBounds(r.left, r.top, r.left + 8, r.bottom);

		_chars[idx].create(8, 8, Graphics::PixelFormat::createFormatCLUT8());
		_chars[idx].transBlitFrom(src, charBounds, Common::Rect(0, 0, 8, 8));
	}

	return true;
}

void BitmapFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	// Get the fg/bg color. When the character is >= 128,
	// the colors are reversed from normal
	byte fgColor = (chr < 128) ? color : 0;
	byte bgColor = (chr < 128) ? 0 : color;
	chr &= 0x7f;

	const Graphics::ManagedSurface &c = _chars[chr - _startingChar];
	for (int yCtr = 0; yCtr < c.h; ++yCtr) {
		const byte *srcP = (const byte *)c.getBasePtr(0, yCtr);

		for (int xCtr = 0; xCtr < c.w; ++xCtr, ++srcP) {
			dst->hLine(x + xCtr, y + yCtr, x + xCtr,
				*srcP ? bgColor : fgColor);
		}
	}
}

} // namespace MM
