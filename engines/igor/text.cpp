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

#include "igor/igor.h"

namespace Igor {

int IgorEngine::getStringWidth(const char *s) const {
	int w = 0;
	for (; *s; ++s) {
		if (*s == ' ') {
			w += 5;
		} else {
			int chr = _fontCharIndex[(uint8)*s];
			if (chr == 99) {
				continue;
			}
			w += _fontCharWidth[chr];
		}
	}
	return w;
}

void IgorEngine::drawActionSentence(const char *sentence, uint8 color) {
	memset(_screenVGA + 144 * 320, 0, 11 * 320);
	int w = getStringWidth(sentence);
	int x = (320 - w) / 2;
	drawString(_screenVGA, sentence, x, 144, color, 0, 0);
}
void IgorEngine::drawString(uint8 *dst, const char *s, int x, int y, int color1, int color2, int color3) {
	for (; *s; ++s) {
		if (*s == ' ') {
			x += 5;
		} else {
			int chr = _fontCharIndex[(uint8)*s];
			if (chr == 99) {
				continue;
			}
			if (x + _fontCharWidth[chr] > 320) {
				break;
			}
			drawChar(dst, chr, x, y, color1, color2, color3);
			x += _fontCharWidth[chr];
		}
	}
}

void IgorEngine::drawChar(uint8 *dst, int chr, int x, int y, int color1, int color2, int color3) {
	dst += y * 320 + x;
	for (int j = 0; j < 11; ++j, dst += 320) {
		uint32 chrLineMask = _fontData[chr * 11 + j];
		for (int i = 0; i < 9; ++i, chrLineMask >>= 2) {
			switch (chrLineMask & 3) {
			case 1:
				dst[i] = color1;
				break;
			case 2:
				if (color2 != -1) {
					dst[i] = color2;
				}
				break;
			case 3:
				if (color3 != -1) {
					dst[i] = color3;
				}
				break;
			}
		}
	}
}

} // End of namespace Igor
