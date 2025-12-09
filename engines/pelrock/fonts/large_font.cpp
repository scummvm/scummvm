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
	const int numChars = 100;
	const int charWidth = 12;
	const int charHeight = 24;
	const int pad = 1;

	const int paddedWidth = charWidth + 2 * pad;   // 14
	const int paddedHeight = charHeight + 2 * pad; // 26

	const int dataSize = numChars * paddedHeight * paddedWidth; // 96 characters × 14 × 26 bytes
	byte *rawFontData = new byte[numChars * 48];                // original format: 96 × 48 bytes
	file.read(rawFontData, numChars * 48);
	debug("LargeFont::load: Loading large font data from %s, size %d bytes", filename.c_str(), dataSize);
	file.close();

	delete[] _fontData;

	_fontData = new byte[dataSize];
	memset(_fontData, 0, dataSize);
	for (int c = 0; c < numChars; c++) {
		// Temporary bitmap for character + border
		bool mask[paddedHeight][paddedWidth] = {false};
		// Decode character pixels from rawFontData
		int charOffset = c * 0x30;
		for (int i = 0; i < charHeight; i++) {
			byte rowByte1 = rawFontData[charOffset + i * 2];
			byte rowByte2 = rawFontData[charOffset + i * 2 + 1];
			for (int bit = 0; bit < 8; bit++) {
				mask[i + pad][bit + pad] = (rowByte1 & (0x80 >> bit)) != 0;
			}
			for (int bit = 0; bit < 4; bit++) {
				mask[i + pad][bit + 8 + pad] = (rowByte2 & (0x80 >> bit)) != 0;
			}
		}

		bool borderMask[paddedHeight][paddedWidth] = {false};

		for (int y = 0; y < paddedHeight; y++) {
			for (int x = 0; x < paddedWidth; x++) {
				if (mask[y][x]) {
					// Mark 3x3 area around character pixel
					for (int dy = -1; dy <= 1; dy++) {
						for (int dx = -1; dx <= 1; dx++) {
							int ny = y + dy;
							int nx = x + dx;
							if (ny >= 0 && ny < paddedHeight && nx >= 0 && nx < paddedWidth) {
								if (!mask[ny][nx]) {
									borderMask[ny][nx] = true;
								}
							}
						}
					}
				}
			}
		}

		int outOffset = c * paddedHeight * paddedWidth;
		for (int y = 0; y < paddedHeight; y++) {
			for (int x = 0; x < paddedWidth; x++) {
				if (mask[y][x]) {
					_fontData[outOffset + y * paddedWidth + x] = 2;
				} else if (borderMask[y][x]) {
					_fontData[outOffset + y * paddedWidth + x] = 1;
				} else {
					_fontData[outOffset + y * paddedWidth + x] = 0;
				}
			}
		}
	}
	delete[] rawFontData;
	return true;
}

int LargeFont::getCharWidth(uint32 chr) const {
	return CHAR_WIDTH + 1;
}

void LargeFont::drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	chr -= 32; // Adjust for font starting at ASCII 32
	if (!_fontData || chr >= 100 || chr < 0) {
		return;
	}

	const int paddedWidth = 14;
	const int paddedHeight = 26;
	int charOffset = chr * paddedWidth * paddedHeight;

	for (int cy = 0; cy < paddedHeight; cy++) {
		for (int cx = 0; cx < paddedWidth; cx++) {
			byte val = _fontData[charOffset + cy * paddedWidth + cx];
			int px = x + cx;
			int py = y + cy;
			if (px < 0 || px >= dst->w || py < 0 || py >= dst->h)
				continue;
			if (val == 1) {
				*((byte *)dst->getBasePtr(px, py)) = 0;
			} else if (val == 2) {
				*((byte *)dst->getBasePtr(px, py)) = color;
			}
		}
	}
}

} // namespace Pelrock
