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

#include "pelrock/fonts/small_font.h"

namespace Pelrock {

SmallFont::SmallFont() : _fontData(nullptr) {
}

SmallFont::~SmallFont() {
	delete[] _fontData;
}

bool SmallFont::load(const Common::String &filename) {
	Common::File file;
	if (!file.open(Common::Path(filename))) {
		return false;
	}

	file.seek(0x8F32, SEEK_SET);

	const int dataSize = kNumChars * 8; // 256 characters, 8x8 pixels
	debug("SmallFont::load: Loading font data of size %d from %s", dataSize, filename.c_str());
	_fontData = new byte[dataSize];
	file.read(_fontData, dataSize);
	file.close();

	return true;
}

int SmallFont::getCharWidth(uint32 chr) const {
	return CHAR_WIDTH;
}

void SmallFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	if (!_fontData || chr > kNumChars - 1) {
		return;
	}

	int charOffset = chr * 8;

	for (int i = 0; i < 8; i++) {
		byte rowByte = _fontData[charOffset + i];
		for (int bit = 0; bit < 8; bit++) {
			bool pixelOn = (rowByte & (0x80 >> bit)) != 0;
			if (pixelOn) {
				if ((x + bit) < dst->w && (y + i) < dst->h) {
					*((byte *)dst->getBasePtr(x + bit, y + i)) = color;
				}
			}
		}
	}
}

} // namespace Pelrock
