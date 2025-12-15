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
#include "common/debug.h"
#include "pelrock/fonts/small_font_double.h"

namespace Pelrock {

DoubleSmallFont::DoubleSmallFont() : SmallFont() {
}

DoubleSmallFont::~DoubleSmallFont() {
}

void DoubleSmallFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (!_fontData || chr > kNumChars - 1) {
		// debug("DoubleSmallFont::drawChar: Invalid char %d", chr);
		return;
	}
	int charOffset = chr * 8;

	for (int i = 0; i < 8; i++) {
		byte rowByte = _fontData[charOffset + i];
		for (int bit = 0; bit < 8; bit++) {
			bool pixelOn = (rowByte & (0x80 >> bit)) != 0;
			int yPos = y + (i * 2);
			if (pixelOn) {
				if ((x + bit) < dst->w && (yPos + 1) < dst->h) {
					*((byte *)dst->getBasePtr(x + bit, yPos)) = color;
					*((byte *)dst->getBasePtr(x + bit, yPos + 1)) = color;
				}
			}
		}
	}
}

} // namespace Pelrock
