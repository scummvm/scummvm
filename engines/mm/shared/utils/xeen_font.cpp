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
#include "mm/shared/utils/xeen_font.h"

namespace MM {

#define FONT_HEIGHT 8
#define CHARS_COUNT 128

byte XeenFont::_colors[4];
byte XeenFont::_colorsSet[40][4];

void XeenFont::load(Common::SeekableReadStream *src,
		size_t charsOffset, size_t charWidthsOffset) {
	// Read in character data
	src->seek(charsOffset);
	_data.resize(CHARS_COUNT * FONT_HEIGHT);
	for (int i = 0; i < CHARS_COUNT * FONT_HEIGHT; ++i)
		_data[i] = src->readUint16LE();

	// Read in the char widths
	src->seek(charWidthsOffset);
	_widths.resize(128);
	src->read(&_widths[0], 128);
}

void XeenFont::loadColors(Common::SeekableReadStream *src) {
	src->read(_colorsSet, 40 * 4);
}

void XeenFont::setColors(uint index) {
	assert(index < 40);
	Common::copy(&_colorsSet[index][0],
		&_colorsSet[index][4], &_colors[0]);
}

int XeenFont::getCharWidth(uint32 chr) const {
	assert(chr < 128);
	return _widths[chr];
}

void XeenFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	assert(chr < 128);
	if (chr == 'g' || chr == 'p' || chr == 'q' || chr == 'y')
		++y;

	const uint16 *src = &_data[chr * FONT_HEIGHT];
	for (int yCtr = 0; yCtr < FONT_HEIGHT; ++yCtr, ++src) {
		if ((y + yCtr) < 0 || (y + yCtr) > dst->h)
			continue;

		uint16 srcVal = *src;
		byte *dest = (byte *)dst->getBasePtr(x, y + yCtr);

		for (int xCtr = 0; xCtr < _widths[chr];
				++xCtr, ++dest, srcVal >>= 2) {
			if ((srcVal & 3) && (x + xCtr) >= 0 && (x + xCtr) < dst->w)
				*dest = _colors[srcVal & 3];
		}
	}
}

} // namespace MM
