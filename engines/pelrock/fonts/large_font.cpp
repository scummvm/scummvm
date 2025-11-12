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

#include "pelrock/fonts/large_font.h"

namespace Pelrock {

LargeFont::LargeFont() : _fontData(nullptr) {
}

LargeFont::~LargeFont() {
	delete[] _fontData;
}

bool LargeFont::load(const Common::String &filename) {
	Common::File file;
	if (!file.open(Common::Path(filename))) {
		return false;
	}

	file.seek(0x7DC8, SEEK_SET);
	const int dataSize = 96 * 48; // 96 characters × 48 bytes
	_fontData = new byte[dataSize];
	file.read(_fontData, dataSize);
	debug("LargeFont::load: Loading large font data from %s, size %d bytes", filename.c_str(), dataSize);
	file.close();

	return true;
}

int LargeFont::getCharWidth(uint32 chr) const {
	return CHAR_WIDTH;
}

void LargeFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	chr -= 32; // Adjust for font starting at ASCII 32
	if (!_fontData || chr > 255 || chr < 0) {
		return;
	}
	int charOffset = chr * 0x30;

	for (int i = 0; i < 24; i++) {
		byte rowByte1 = _fontData[charOffset + i * 2];
		byte rowByte2 = _fontData[charOffset + i * 2 + 1];
		for (int bit = 0; bit < 8; bit++) {
			bool pixelOn = (rowByte1 & (0x80 >> bit)) != 0;
			if (pixelOn) {
				if ((x + bit) < dst->w && (y + i) < dst->h) {
					*((byte *)dst->getBasePtr(x + bit, y + i)) = color;
				}
			}
		}
		for (int bit = 0; bit < 4; bit++) {
			bool pixelOn = (rowByte2 & (0x80 >> bit)) != 0;
			if (pixelOn) {
				if ((x + bit + 8) < dst->w && (y + i) < dst->h) {
					*((byte *)dst->getBasePtr(x + bit + 8, y + i)) = color;
				}
			}
		}
	}
}

} // namespace Pelrock
