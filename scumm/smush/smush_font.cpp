/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#include "smush_font.h"

SmushFont::SmushFont(bool use_original_colors, bool new_colors) :
	NutRenderer(g_scumm),	// FIXME: evil hack
	_color(-1),
	_new_colors(new_colors),
	_original(use_original_colors) {
}

int SmushFont::getStringWidth(const char *str) {
	if (!_loaded) {
		warning("SmushFont::getStringWidth() Font is not loaded");
		return 0;
	}

	int width = 0;
	while (*str) {
		width += getCharWidth(*str++);
	}
	return width;
}

int SmushFont::getStringHeight(const char *str) {
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
					char value = *src++;
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
	byte *dst = buffer + dst_width * (y + (_vm->_gameId == GID_CMI ? 7 : 2)) + x;
	byte bits = 0;

	char color = (_color != -1) ? _color : 1;
	if (_new_colors)
		color = (char)0xff; //FIXME;
	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			if ((i % 8) == 0)
				bits = *src++;
			if (bits & revBitMask[i % 8]) {
				dst[i + 1] = 0;
				dst[i] = color;
			}
		}
		dst += dst_width;
	}
	return w + 1;
}

static char **split(const char *str, char sep) {
	char **ret = new char *[62];
	int n = 0;
	const char *i = str;
	char *j = strchr(i, sep);

	while (j != NULL) {
		assert(n < 60);
		ret[n] = new char[j - i + 1];
		memcpy(ret[n], i, j - i);
		ret[n++][j - i] = 0;
		i = j + 1;
		j = strchr(i, sep);
	}

	int len = strlen(i);
	ret[n] = new char[len + 1];
	memcpy(ret[n], i, len);
	ret[n++][len] = 0;
	ret[n] = 0;

	return ret;
}

void SmushFont::drawSubstring(const char *str, byte *buffer, int dst_width, int x, int y) {
	for (int i = 0; str[i] != 0; i++) {
		if ((byte)str[i] >= 0x80 && _vm->_CJKMode) {
			x += draw2byte(buffer, dst_width, x, y, (byte)str[i] + 256 * (byte)str[i+1]);
			i++;
		} else
			x += drawChar(buffer, dst_width, x, y, str[i]);
	}
}

void SmushFont::drawStringAbsolute(const char *str, byte *buffer, int dst_width, int x, int y) {
	debug(9, "SmushFont::drawStringAbsolute(%s, %d, %d)", str, x, y);

	while (str) {
		char line[256];
		char *pos = strchr(str, '\n');
		if (pos) {
			memcpy(line, str, pos - str - 1);
			line[pos - str - 1] = 0;
			str = pos + 1;
		} else {
			strcpy(line, str);
			str = 0;
		}
		drawSubstring(line, buffer, dst_width, x, y);
		y += getStringHeight(line);
	}
}

void SmushFont::drawStringCentered(const char *str, byte *buffer, int dst_width, int dst_height, int y, int xmin, int width, int offset) {
	debug(9, "SmushFont::drawStringCentered(%s, %d, %d)", str, xmin, y);

	char *z = strchr(str, '\n');
	if (z != 0) {
		// FIXME: this is actually evil, because it silently modifes the
		// string 'str' passed to us, despite it being declared const.
		warning("drawStringCentered: got input string containing \\n");
		*z = 0;
	}
	char **words = split(str, ' ');
	int word_count = 0;

	while (words[word_count])
		word_count++;

	int i = 0, max_width = 0, height = 0, line_count = 0;

	char **substrings = new char *[word_count];
	int *substr_widths = new int[word_count];
	int space_width = getCharWidth(' ');

	i = 0;
	while (i < word_count) {
		int substr_width = getStringWidth(words[i]);
		char *substr = new char[1000];
		strcpy(substr, words[i]);
		int j = i + 1;

		while (j < word_count) {
			int word_width = getStringWidth(words[j]);
			if ((substr_width + space_width + word_width) >= width)
				break;
			substr_width += word_width + space_width;
			j++;
		}

		for (int k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[line_count] = substr;
		substr_widths[line_count++] = substr_width;
		if (substr_width > max_width)
			max_width = substr_width;
		i = j;
		height += getStringHeight(substr);
	}

	for (i = 0; i < word_count; i++) {
		delete[] words[i];
	}
	delete[] words;
	
	if (y > dst_height - height) {
		y = dst_height - height;
	}

	max_width = (max_width + 1) >> 1;
	int x = xmin + width / 2 + offset - dst_width / 2;

	if (x < max_width)
		x = max_width;
	if (x > dst_width - max_width)
		x = dst_width - max_width;

	for (i = 0; i < line_count; i++) {
		drawSubstring(substrings[i], buffer, dst_width, x - substr_widths[i] / 2, y);
		y += getStringHeight(substrings[i]);
		delete[] substrings[i];
	}

	delete[] substr_widths;
	delete[] substrings;
}

void SmushFont::drawStringWrap(const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int width) {
	debug(9, "SmushFont::drawStringWrap(%s, %d, %d)", str, x, y);

	char *z = strchr(str, '\n');
	if (z != 0) {
		// FIXME: this is actually evil, because it silently modifes the
		// string 'str' passed to us, despite it being declared const.
		warning("drawStringWrap: got input string containing \\n");
		*z = 0;
	}
	char **words = split(str, ' ');
	int word_count = 0;

	while (words[word_count])
		word_count++;

	int i = 0, max_width = 0, height = 0, line_count = 0;

	char **substrings = new char *[word_count];
	int *substr_widths = new int[word_count];
	int space_width = getCharWidth(' ');

	i = 0;
	while (i < word_count) {
		int substr_width = getStringWidth(words[i]);
		char *substr = new char[1000];
		strcpy(substr, words[i]);
		int j = i + 1;

		while (j < word_count) {
			int word_width = getStringWidth(words[j]);
			if ((substr_width + space_width + word_width) >= width)
				break;
			substr_width += word_width + space_width;
			j++;
		}

		for (int k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[line_count] = substr;
		substr_widths[line_count++] = substr_width;
		if (max_width < substr_width)
			max_width = substr_width;
		i = j;
		height += getStringHeight(substr);
	}

	for (i = 0; i < word_count; i++) {
		delete[] words[i];
	}
	delete[] words;

	if (y > dst_height - height) {
		y = dst_height - height;
	}

	if (x > dst_width - max_width)
		x = dst_width - max_width;

	for (i = 0; i < line_count; i++) {
		drawSubstring(substrings[i], buffer, dst_width, x, y);
		y += getStringHeight(substrings[i]);
		delete[] substrings[i];
	}

	delete[] substr_widths;
	delete[] substrings;
}

void SmushFont::drawStringWrapCentered(const char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int width) {
	debug(9, "SmushFont::drawStringWrapCentered(%s, %d, %d)", str, x, y);
	
	char *z = strchr(str, '\n');
	if (z != 0) {
		// FIXME: this is actually evil, because it silently modifes the
		// string 'str' passed to us, despite it being declared const.
		warning("drawStringWrapCentered: got input string containing \\n");
		*z = 0;
	}
	char **words = split(str, ' ');
	int word_count = 0;

	while (words[word_count])
		word_count++;

	int i = 0, max_width = 0, height = 0, line_count = 0;

	char **substrings = new char *[word_count];
	int *substr_widths = new int[word_count];
	int space_width = getCharWidth(' ');

	i = 0;
	width = MIN(width, dst_width);
	while (i < word_count) {
		int substr_width = getStringWidth(words[i]);
		char *substr = new char[1000];
		strcpy(substr, words[i]);
		int j = i + 1;

		while (j < word_count) {
			int word_width = getStringWidth(words[j]);
			if ((substr_width + space_width + word_width) >= width)
				break;
			substr_width += word_width + space_width;
			j++;
		}

		for (int k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[line_count] = substr;
		substr_widths[line_count++] = substr_width;
		if (max_width < substr_width)
			max_width = substr_width;
		i = j;
		height += getStringHeight(substr);
	}

	for (i = 0; i < word_count; i++) {
		delete[] words[i];
	}
	delete[] words;

	if (y > dst_height - height) {
		y = dst_height - height;
	}

	max_width = (max_width + 1) >> 1;

	if (x < max_width)
		x = max_width;
	if (x > dst_width - max_width)
		x = dst_width - max_width;

	for (i = 0; i < line_count; i++) {
		drawSubstring(substrings[i], buffer, dst_width, x - substr_widths[i] / 2, y);
		y += getStringHeight(substrings[i]);
		delete[] substrings[i];
	}

	delete[] substr_widths;
	delete[] substrings;
}
