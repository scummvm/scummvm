/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "common/file.h"
#include "common/str.h"
#include "scumm/scumm.h"
#include "scumm/util.h"

#include "scumm/smush/smush_font.h"

namespace Scumm {

SmushFont::SmushFont(ScummEngine *vm, const char *filename, bool use_original_colors, bool new_colors) :
	NutRenderer(vm, filename),
	_color(-1),
	_new_colors(new_colors),
	_original(use_original_colors) {
}

int SmushFont::getStringWidth(const char *str, uint numBytesMax) {
	assert(str);
	int maxWidth = 0;
	int width = 0;

	if (!numBytesMax)
		return 0;

	while (*str && numBytesMax) {
		// No treatment of the ^ commands here, since they're handled/removed in handleTextResource().
		if (is2ByteCharacter(_vm->_language, *str)) {
			width += _vm->_2byteWidth + (_vm->_language != Common::JA_JPN ? 1 : 0);
			++str;
			--numBytesMax;
		} else if (*str == '\n') {
			maxWidth = MAX<int>(width, maxWidth);
			width = 0;
		} else if (*str != '\r' && *str != _vm->_newLineCharacter) {
			width += getCharWidth(*str);
		}
		++str;
		--numBytesMax;
	}

	return MAX<int>(width, maxWidth);
}

int SmushFont::getStringHeight(const char *str, uint numBytesMax) {
	assert(str);
	int totalHeight = 0;
	int lineHeight = 0;

	if (!numBytesMax)
		return 0;

	while (*str && numBytesMax) {
		// No treatment of the ^ commands here, since they're handled/removed in handleTextResource().
		if (*str == '\n') {
			totalHeight += (lineHeight ? lineHeight : _fontHeight) + 1;
			lineHeight = 0;
		} else if (*str != '\r' && *str != _vm->_newLineCharacter) {
			lineHeight = MAX<int>(lineHeight, getCharHeight(*str));
			if (is2ByteCharacter(_vm->_language, *str)) {
				++str;
				--numBytesMax;
			}
		}
		++str;
		--numBytesMax;
	}

	return totalHeight + (lineHeight ? lineHeight : _fontHeight) + 1;
}

int SmushFont::drawChar(byte *buffer, int dst_width, int x, int y, byte chr) {
	int w = _chars[chr].width;
	int h = _chars[chr].height;
	const byte *src = unpackChar(chr);
	byte *dst = buffer + dst_width * y + x;

	assert(dst_width == _vm->_screenWidth);

	if (_original) {
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				int8 value = *src++;
				if (value != _chars[chr].transparency)
					dst[i] = value;
			}
			dst += dst_width;
		}
	} else {
		char color = (_color != -1) ? _color : 1;
		if (_new_colors) {
			for (int j = 0; j < h; j++) {
				for (int i = 0; i < w; i++) {
					int8 value = *src++;
					if (value == -color) {
						dst[i] = 0xFF;
					} else if (value == -31) {
						dst[i] = 0;
					} else if (value != _chars[chr].transparency) {
						dst[i] = value;
					}
				}
				dst += dst_width;
			}
		} else {
			for (int j = 0; j < h; j++) {
				for (int i = 0; i < w; i++) {
					int8 value = *src++;
					if (value == 1) {
						dst[i] = color;
					} else if (value != _chars[chr].transparency) {
						dst[i] = 0;
					}
				}
				dst += dst_width;
			}
		}
	}
	return w;
}

int SmushFont::draw2byte(byte *buffer, int dst_width, int x, int y, int idx) {
	int w = _vm->_2byteWidth;
	int h = _vm->_2byteHeight;
	const byte *src = _vm->get2byteCharPtr(idx);
	byte bits = 0;

	char color = (_color != -1) ? _color : 1;

	if (_new_colors)
		color = (char)0xff;

	if (_vm->_game.id == GID_FT)
		color = 1;

	enum ShadowMode {
		kNone,
		kNormalShadowMode,
		kCJKv7ShadowMode,
		kCJKv8ShadowMode
	};

	ShadowMode shadowMode = _vm->_useCJKMode ? (_vm->_game.version == 8 ? kCJKv8ShadowMode : kCJKv7ShadowMode) : kNone;

	int shadowOffsetXTable[4] = { -1, 0, 1, 0 };
	int shadowOffsetYTable[4] = { 0, 1, 0, 0 };
	int shadowOffsetColorTable[4] = { 0, 0, 0, color };

	const byte *origSrc = src;
	for (int shadowIdx = (shadowMode == kCJKv8ShadowMode) ? 0 : (shadowMode == kCJKv7ShadowMode ? 2 : 3); shadowIdx < 4; shadowIdx++) {
		int offX = x + shadowOffsetXTable[shadowIdx];
		int offY = y + shadowOffsetYTable[shadowIdx];
		byte drawColor = shadowOffsetColorTable[shadowIdx];

		src = origSrc;
		byte *dst = buffer + dst_width * offY + offX;

		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				if (offX + i < 0)
					continue;
				if ((i % 8) == 0)
					bits = *src++;
				if (bits & revBitMask(i % 8)) {
					if (shadowMode == kNormalShadowMode) {
						dst[i + 1] = 0;
						dst[dst_width + i] = 0;
						dst[dst_width + i + 1] = 0;
					}
					dst[i] = drawColor;
				}
			}
			dst += dst_width;
		}
	}
	return w + 1;
}

void SmushFont::drawSubstring(const char *str, uint numBytesMax, byte *buffer, int dst_width, int x, int y) {
	if (_vm->_language == Common::HE_ISR) {
		for (int i = strlen(str); i >= 0 && numBytesMax; i--) {
			x += drawChar(buffer, dst_width, x, y, str[i]);
			--numBytesMax;
		}
	} else {
		for (int i = 0; str[i] != 0 && numBytesMax; ++i) {
			if (is2ByteCharacter(_vm->_language, str[i])) {
				x += draw2byte(buffer, dst_width, x, y, (byte)str[i] + 256 * (byte)str[i + 1]);
				++i;
				--numBytesMax;
			} else if (str[i] != '\n' && str[i] != _vm->_newLineCharacter) {
				x += drawChar(buffer, dst_width, x, y, str[i]);
			}
			--numBytesMax;
		}
	}
}

#define MAX_STRINGS		80


void SmushFont::drawString(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, bool center) {
	debugC(DEBUG_SMUSH, "SmushFont::drawString(str: '%s', x: %d, y: %d, clipRect: (%d, %d, %d, %d), center: %d)", str, x, y, clipRect.left, clipRect.top, clipRect.right, clipRect.bottom, center);

	int totalLen = (int)strlen(str);
	int lineStart = 0;

	// COMI always does this for CJK strings (before any other possible yPos fixes).
	if (_vm->_game.id == GID_CMI) {
		if (_vm->_useCJKMode)
			y += 2;
		// No idea whether it is actually used. We currently don't handle this flag.
		/*if (flags & 0x40)
			y -= (getStringHeight(str, totalLen) / 2);*/
	}

	for (int pos = 0; pos <= totalLen; ++pos) {
		if (str[pos] != '\0' && str[pos] != '\n')
			continue;

		int len = pos - lineStart;
		int height = getStringHeight(str + lineStart, len);
		if (y < clipRect.bottom) {
			drawSubstring(str + lineStart, len, buffer, _vm->_screenWidth, center ? (x - getStringWidth(str + lineStart, len) / 2) : x, y);
			y += height;
		}

		lineStart = pos + 1;
	}
}

void SmushFont::drawStringWrap(const char *str, byte *buffer, Common::Rect &clipRect, int x, int y, bool center) {
	debugC(DEBUG_SMUSH, "SmushFont::drawStringWrap(str: '%s', x: %d, y: %d, clipRect: (%d, %d, %d, %d), center: %d)", str, x, y, clipRect.left, clipRect.top, clipRect.right, clipRect.bottom, center);
	// This implementation is from COMI. Things are done a bit differently than in the older implementations.
	// In particular, the older version would insert '\0' chars into the string to cut off the sub strings
	// before calling getStringWidth(), getStringHeight() or drawSubstring() and replace these chars with the
	// original values afterwards. COMI allows a byte length limitation in all the functions so that the sub
	// string length can be passed and no cut off '\0' chars are needed.

	int len = (int)strlen(str);
	Common::String spaceSeparators(Common::String::format(" %c", (char)_vm->_newLineCharacter));
	Common::String breakSeparators(Common::String::format(" \n%c", (char)_vm->_newLineCharacter));

	int16 substrByteLength[MAX_STRINGS];
	memset(substrByteLength, 0, sizeof(substrByteLength));
	int16 substrWidths[MAX_STRINGS];
	memset(substrWidths, 0, sizeof(substrWidths));
	int16 substrStart[MAX_STRINGS];
	memset(substrStart, 0, sizeof(substrStart));

	int16 numSubstrings = 0;
	int height = 0;
	int lastSubstrHeight = 0;
	int maxWidth = 0;
	int curWidth = 0;
	int curPos = -1;

	// COMI does this for CJK strings (before any other possible yPos fixes, see lines 343 - 355).
	if (_vm->_game.id == GID_CMI && _vm->_useCJKMode)
		y += 2;

	while (curPos < len) {
		int textStart = curPos + 1;
		while (str[textStart] && spaceSeparators.contains(str[textStart]))
			++textStart;

		int separatorWidth = curPos > 0 ? getStringWidth(str + curPos, textStart - curPos) : 0;

		int nextSeparatorPos = textStart;
		while (!breakSeparators.contains(str[nextSeparatorPos])) {
			if (++nextSeparatorPos == len)
				break;
		}

		int wordWidth = getStringWidth(str + textStart, nextSeparatorPos - textStart);
		int newWidth = curWidth + separatorWidth + wordWidth;

		if (curWidth && newWidth > clipRect.width()) {
			if (numSubstrings < MAX_STRINGS) {
				substrWidths[numSubstrings] = curWidth;
				substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
				numSubstrings++;
			}
			newWidth = wordWidth;
			substrStart[numSubstrings] = textStart;
		}
		curWidth = newWidth;

		curPos = nextSeparatorPos;
		if (!spaceSeparators.contains(str[curPos])) {
			// This one is only triggered by '\n' (which frequently happens in COMI/English).
			if (numSubstrings < MAX_STRINGS) {
				substrWidths[numSubstrings] = curWidth;
				substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
				numSubstrings++;
				substrStart[numSubstrings] = curPos + 1;
			}
			curWidth = 0;
		}
	}

	if (curWidth && numSubstrings < MAX_STRINGS) {
		substrWidths[numSubstrings] = curWidth;
		substrByteLength[numSubstrings] = curPos - substrStart[numSubstrings];
		numSubstrings++;
	}

	for (int i = 0; i < numSubstrings; ++i) {
		maxWidth = MAX<int>(maxWidth, substrWidths[i]);
		lastSubstrHeight = substrByteLength[i] > 0 ? getStringHeight(str + substrStart[i], substrByteLength[i]) : 0;
		height += lastSubstrHeight;
	}

	// I have verified these y-corrections for DIG (English and Chinese), COMI (English and Chinese) and FT (English).
	// In COMI there seem to be more text flags which we don't support and for which I haven't seen use cases yet. I
	// put some commented-out code in here as a reminder...
	int clipHeight = height + lastSubstrHeight / 2;

	/*if (_vm->_game.id == GID_CMI && (flags & 0x40))
		y -= (lastSubstrHeight / 2);*/

	if (y > clipRect.bottom - clipHeight /*&& !(_vm->_game.id == GID_CMI && (flags & 0x100))*/)
		y = clipRect.bottom - clipHeight;

	if (y < clipRect.top)
		y = clipRect.top;

	if (center) {
		if (x + (maxWidth >> 1) > clipRect.right)
			x = clipRect.right - (maxWidth >> 1);
		if (x - (maxWidth >> 1) < clipRect.left)
			x = clipRect.left + (maxWidth >> 1);
	} else {
		if (x > clipRect.right - maxWidth)
			x = clipRect.right - maxWidth;
		if (x < clipRect.left)
			x = clipRect.left;
	}

	for (int i = 0; i < numSubstrings; i++) {
		int xpos = center ? x - substrWidths[i] / 2 : x;
		len = substrByteLength[i] > 0 ? substrByteLength[i] : 0;
		drawSubstring(str + substrStart[i], len, buffer, _vm->_screenWidth, xpos, y);
		y += getStringHeight(str + substrStart[i], len);
	}
}

} // End of namespace Scumm
