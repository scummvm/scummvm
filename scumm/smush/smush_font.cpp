/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/file.h"
#include "scumm/scumm.h"
#include "scumm/util.h"

#include "scumm/smush/smush_font.h"

namespace Scumm {

SmushFont::SmushFont(ScummEngine *vm, bool use_original_colors, bool new_colors) :
	NutRenderer(vm),
	_color(-1),
	_new_colors(new_colors),
	_original(use_original_colors) {
}

int SmushFont::getStringWidth(const char *str) {
	assert(str);
	if (!_loaded) {
		warning("SmushFont::getStringWidth() Font is not loaded");
		return 0;
	}

	int width = 0;
	while (*str) {
		if (*str & 0x80 && _vm->_useCJKMode) {
			width += _vm->_2byteWidth + 1;
			str += 2;
		} else
			width += getCharWidth(*str++);
	}
	return width;
}

int SmushFont::getStringHeight(const char *str) {
	assert(str);
	if (!_loaded) {
		warning("SmushFont::getStringHeight() Font is not loaded");
		return 0;
	}

	int height = 0;
	while (*str) {
		int charHeight = getCharHeight(*str++);
		if (height < charHeight)
			height = charHeight;
	}
	return height;
}

int SmushFont::drawChar(byte *buffer, int dst_width, int x, int y, byte chr) {
	int w = _chars[chr].width;
	int h = _chars[chr].height;
	const byte *src = _chars[chr].src;
	byte *dst = buffer + dst_width * y + x;

	assert(dst_width == _vm->_screenWidth);

	if (_original) {
		for (int j = 0; j < h; j++) {
			for (int i = 0; i < w; i++) {
				char value = *src++;
				if (value) dst[i] = value;
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
					} else if (value) {
						dst[i] = value;
					}
				}
				dst += dst_width;
			}
		} else {
			for (int j = 0; j < h; j++) {
				for (int i = 0; i < w; i++) {
					char value = *src++;
					if (value == 1) {
						dst[i] = color;
					} else if (value) {
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

	byte *src = _vm->get2byteCharPtr(idx);
	byte *dst = buffer + dst_width * (y + (_vm->_gameId == GID_CMI ? 7 : (_vm->_gameId == GID_DIG ? 2 : 0))) + x;
	byte bits = 0;

	char color = (_color != -1) ? _color : 1;

	if (_new_colors)
		color = (char)0xff;

	if (_vm->_gameId == GID_FT)
		color = 1;

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			if ((i % 8) == 0)
				bits = *src++;
			if (bits & revBitMask(i % 8)) {
				dst[i + 1] = 0;
				dst[dst_width + i] = 0;
				dst[dst_width + i + 1] = 0;
				dst[i] = color;
			}
		}
		dst += dst_width;
	}
	return w + 1;
}

void SmushFont::drawSubstring(const char *str, byte *buffer, int dst_width, int x, int y) {
	// This happens in the Full Throttle intro. I don't know if our
	// text-drawing functions are buggy, or if this function is supposed
	// to have to check for it.
	if (x < 0)
		x = 0;

	for (int i = 0; str[i] != 0; i++) {
		if ((byte)str[i] >= 0x80 && _vm->_useCJKMode) {
			x += draw2byte(buffer, dst_width, x, y, (byte)str[i] + 256 * (byte)str[i+1]);
			i++;
		} else
			x += drawChar(buffer, dst_width, x, y, str[i]);
	}
}

#define MAX_WORDS	60


void SmushFont::drawString(const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, bool center) {
	debugC(DEBUG_SMUSH, "SmushFont::drawString(%s, %d, %d, %d)", str, x, y, center);

	while (str) {
		char line[256];
		char *pos = (char *)strchr(str, '\n');
		if (pos) {
			memcpy(line, str, pos - str - 1);
			line[pos - str - 1] = 0;
			str = pos + 1;
		} else {
			strcpy(line, str);
			str = 0;
		}
		drawSubstring(line, buffer, dst_width, center ? (x - getStringWidth(line) / 2) : x, y);
		y += getStringHeight(line);
	}
}

void SmushFont::drawStringWrap(const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int left, int right, bool center) {
	debugC(DEBUG_SMUSH, "SmushFont::drawStringWrap(%s, %d, %d, %d, %d, %d)", str, x, y, left, right, center);

	const int width = right - left;
	char *s = strdup(str);
	char *words[MAX_WORDS];
	int word_count = 0;

	char *tmp = s;
	while (tmp) {
		assert(word_count < MAX_WORDS);
		words[word_count++] = tmp;
		tmp = strpbrk(tmp, " \t\r\n");
		if (tmp == 0)
			break;
		*tmp++ = 0;
	}

	int i = 0, max_width = 0, height = 0, line_count = 0;

	char *substrings[MAX_WORDS];
	int substr_widths[MAX_WORDS];
	const int space_width = getCharWidth(' ');

	i = 0;
	while (i < word_count) {
		char *substr = words[i++];
		int substr_width = getStringWidth(substr);

		while (i < word_count) {
			int word_width = getStringWidth(words[i]);
			if ((substr_width + space_width + word_width) >= width)
				break;
			substr_width += word_width + space_width;
			*(words[i]-1) = ' ';	// Convert 0 byte back to space
			i++;
		}

		substrings[line_count] = substr;
		substr_widths[line_count++] = substr_width;
		if (max_width < substr_width)
			max_width = substr_width;
		height += getStringHeight(substr);
	}

	if (y > dst_height - height) {
		y = dst_height - height;
	}
	
	if (center) {
		max_width = (max_width + 1) / 2;
		x = left + width / 2;
	
		if (x < left + max_width)
			x = left + max_width;
		if (x > right - max_width)
			x = right - max_width;
	
		for (i = 0; i < line_count; i++) {
			drawSubstring(substrings[i], buffer, dst_width, x - substr_widths[i] / 2, y);
			y += getStringHeight(substrings[i]);
		}
	} else {
		if (x > dst_width - max_width)
			x = dst_width - max_width;
	
		for (i = 0; i < line_count; i++) {
			drawSubstring(substrings[i], buffer, dst_width, x, y);
			y += getStringHeight(substrings[i]);
		}
	}
	
	free(s);
}

} // End of namespace Scumm
