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
#include "common/util.h"
#include "common/engine.h"
#include "common/file.h"
#include "scumm/scumm.h"

#include "smush_font.h"

SmushFont::SmushFont(bool use_original_colors, bool new_colors) :
	_nbChars(0),
	_color(-1),
	_new_colors(new_colors),
	_original(use_original_colors) {
	for(int i = 0; i < 256; i++)
		_chars[i].chr = NULL;

	_dataSrc = NULL;
}

SmushFont::~SmushFont() {
	for(int i = 0; i < _nbChars; i++) {
		if(_chars[i].chr)
			delete []_chars[i].chr;
	}
}

bool SmushFont::loadFont(const char *filename, const char *directory) {
	debug(2, "SmushFont::loadFont() called");

	File file;
	file.open(filename, directory);
	if (file.isOpen() == false) {
		warning("SmushFont::loadFont() Can't open font file: %s/%s", directory, filename);
		return false;
	}

	uint32 tag = file.readUint32BE();
	if (tag != 'ANIM') {
		debug(2, "SmushFont::loadFont() there is no ANIM chunk in font header");
		return false;
	}

	if (_dataSrc != NULL) {
		free(_dataSrc);
		_dataSrc = NULL;
	}

	uint32 length = file.readUint32BE();
	_dataSrc = (byte *)malloc(length);
	file.read(_dataSrc, length);
	file.close();

	if (READ_BE_UINT32(_dataSrc) != 'AHDR') {
		debug(2, "SmushFont::loadFont() there is no AHDR chunk in font header");
		free(_dataSrc);
		_dataSrc = NULL;
		return false;
	}
	
	_nbChars = READ_LE_UINT16(_dataSrc + 10);
	int32 offset = READ_BE_UINT32(_dataSrc + 4) + 8;
	for (int l = 0; l < _nbChars; l++) {
		if (READ_BE_UINT32(_dataSrc + offset) == 'FRME') {
			offset += 8;
			if (READ_BE_UINT32(_dataSrc + offset) == 'FOBJ') {
				_chars[l].width = READ_LE_UINT16(_dataSrc + offset + 14);
				_chars[l].height = READ_LE_UINT16(_dataSrc + offset + 16);
				_chars[l].chr = new byte[_chars[l].width * _chars[l].height + 1000];
				decodeCodec(_chars[l].chr, _dataSrc + offset + 22, READ_BE_UINT32(_dataSrc + offset + 4) - 14);
				offset += READ_BE_UINT32(_dataSrc + offset + 4) + 8;
			} else {
				debug(2, "SmushFont::loadFont(%s, %s) there is no FOBJ chunk in FRME chunk %d (offset %x)", filename, directory, l, offset);
				break;
			}
		} else {
			debug(2, "SmushFont::loadFont(%s, %s) there is no FRME chunk %d (offset %x)", filename, directory, l, offset);
			break;
		}
	}

	free(_dataSrc);
	_dataSrc = NULL;
	return true;
}

int SmushFont::getCharWidth(byte v) {
	if(v >= _nbChars)
		error("invalid character in SmushFont::charWidth : %d (%d)", v, _nbChars);

	return _chars[v].width;
}

int SmushFont::getCharHeight(byte v) {
	if(v >= _nbChars)
		error("invalid character in SmushFont::charHeight : %d (%d)", v, _nbChars);

	return _chars[v].height;
}

int SmushFont::getStringWidth(char *str) {
	int ret = 0;

	while(*str) {
		ret += getCharWidth(*str++);
	}

	return ret;
}

int SmushFont::getStringHeight(char *str) {
	int ret = 0;

	for(int i = 0; str[i] != 0; i++) {
		int h = getCharHeight(str[i]);
		ret = MAX(ret, h);
	}

	return ret;
}

void SmushFont::decodeCodec(byte *dst, byte *src, int length) {
	int size_line, num;
	byte *src2 = src;
	byte *dst2 = dst;
	byte val;

	do {
		size_line = READ_LE_UINT16(src2);
		src2 += 2;
		length -= 2;

		while (size_line != 0) {
			num = *src2++;
			val = *src2++;
			memset(dst2, val, num);
			dst2 += num;
			length -= 2;
			size_line -= 2;
			if (size_line != 0) {
				num = READ_LE_UINT16(src2) + 1;
				src2 += 2;
				memcpy(dst2, src2, num);
				dst2 += num;
				src2 += num;
				length -= num + 2;
				size_line -= num + 2;
			}
		}
		dst2--;

	} while (length > 1);
}

int SmushFont::drawChar(byte *buffer, int dst_width, int x, int y, byte chr) {
	int w = _chars[chr].width;
	int h = _chars[chr].height;
	byte *src = _chars[chr].chr;
	byte *dst = buffer + dst_width * y + x;

	if(_original) {
		for(int32 j = 0; j < h; j++) {
			for(int32 i = 0; i < w; i++) {
				char value = *src++;
				if(value) dst[i] = value;
			}
			dst += dst_width;
		}
	} else {
		char color = (_color != -1) ? _color : 1;
		if (_new_colors == true) {
			for(int j = 0; j < h; j++) {
				for(int i = 0; i < w; i++) {
					char value = *src++;
					if(value == -color) {
						dst[i] = 0xFF;
					} else if(value == -31) {
						dst[i] = 0;
					} else if(value) {
						dst[i] = value;
					}
				}
				dst += dst_width;
			}
		} else {
			for(int j = 0; j < h; j++) {
				for(int i = 0; i < w; i++) {
					char value = *src++;
					if(value == 1) {
						dst[i] = color;
					} else if(value) {
						dst[i] = 0;
					}
				}
				dst += dst_width;
			}
		}
	}
	return w;
}

static char **split(char *str, char sep) {
	char **ret = new char *[62];
	int n = 0;
	const char *i = str;
	char *j = strchr(i, sep);

	while(j != NULL) {
		assert(n < 60);
		ret[n] = new char[j - i + 1];
		memcpy(ret[n], i, j - i);
		ret[n++][j - i] = 0;
		i = j + 1;
		j = strchr(i, sep);
	}

	ret[n] = new char[strlen(i) + 1];
	memcpy(ret[n], i, strlen(i));
	ret[n++][strlen(i)] = 0;
	ret[n] = 0;

	return ret;
}

void SmushFont::drawSubstring(char *str, byte *buffer, int dst_width, int x, int y) {
	for(int i = 0; str[i] != 0; i++)
		x += drawChar(buffer, dst_width, x, y, str[i]);
}

void SmushFont::drawStringAbsolute(char *str, byte *buffer, int dst_width, int x, int y) {
	debug(9, "SmushFont::drawStringAbsolute(%s, %d, %d)", str, x, y);

	while(str) {
		char line[256];
		char *pos = strchr(str, '\n');
		if(pos) {
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

void SmushFont::drawStringCentered(char *str, byte *buffer, int dst_width, int dst_height, int y, int xmin, int width, int offset) {
	debug(9, "SmushFont::drawStringCentered(%s, %d, %d)", str, xmin, y);

	if ((strchr(str, '\n') != 0)) {
		char *z = strchr(str, '\n');
		*z = 0;
	}
	char **words = split(str, ' ');
	int nb_sub = 0;

	while(words[nb_sub])
		nb_sub++;

	int *sizes = new int[nb_sub];
	int i = 0, max_width = 0, height = 0, nb_subs = 0;

	for(i = 0; i < nb_sub; i++)
		sizes[i] = getStringWidth(words[i]);

	char **substrings = new char *[nb_sub];
	int *substr_widths = new int[nb_sub];
	int space_width = getCharWidth(' ');

	i = 0;
	while(i < nb_sub) {
		int substr_width = sizes[i];
		char *substr = new char[1000];
		strcpy(substr, words[i]);
		int j = i + 1;

		while(j < nb_sub && (substr_width + space_width + sizes[j]) < width) {
			substr_width += sizes[j++] + space_width;
		}

		for(int k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[nb_subs] = substr;
		substr_widths[nb_subs++] = substr_width;
		if(substr_width > max_width)
			max_width = substr_width;
		i = j;
		height += getStringHeight(substr);
	}

	delete []sizes;
	for(i = 0; i < nb_sub; i++) {
		delete []words[i];
	}
	delete []words;
	
	max_width = (max_width + 1) >> 1;
	int x = xmin + width / 2;
	x += offset - dst_width / 2;

	if(x < max_width) x = max_width;
	if(x + max_width > dst_width) {
		x = dst_width - max_width;
	}

	if(y + height > dst_height) {
		y = dst_height - height;
	}

	for(i = 0; i < nb_subs; i++) {
		int substr_width = substr_widths[i];
		drawSubstring(substrings[i], buffer, dst_width, x - substr_width / 2, y);
		y += getStringHeight(substrings[i]);
		delete []substrings[i];
	}

	delete []substr_widths;
	delete []substrings;
}

void SmushFont::drawStringWrap(char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int width) {
	debug(9, "SmushFont::drawStringWrap(%s, %d, %d)", str, x, y);

	if ((strchr(str, '\n') != 0)) {
		char *z = strchr(str, '\n');
		*z = 0;
	}
	char ** words = split(str, ' ');
	int nb_sub = 0;

	while(words[nb_sub])
		nb_sub++;

	int *sizes = new int[nb_sub];
	int i = 0, max_width = 0, height = 0, nb_subs = 0, left_x;

	for(i = 0; i < nb_sub; i++)
		sizes[i] = getStringWidth(words[i]);

	char **substrings = new char *[nb_sub];
	int *substr_widths = new int[nb_sub];
	int space_width = getCharWidth(' ');

	i = 0;
	while(i < nb_sub) {
		int substr_width = sizes[i];
		char *substr = new char[1000];
		strcpy(substr, words[i]);
		int j = i + 1;

		while(j < nb_sub && (substr_width + space_width + sizes[j]) < width) {
			substr_width += sizes[j++] + space_width;
		}

		for(int k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[nb_subs] = substr;
		substr_widths[nb_subs++] = substr_width;
		i = j;
		height += getStringHeight(substr);
	}

	delete []sizes;
	for(i = 0; i < nb_sub; i++) {
		delete []words[i];
	}
	delete []words;

	if(y + height > dst_height) {
		y = dst_height - height;
	}

	for(i = 0; i < nb_subs; i++)
		max_width = MAX(max_width, substr_widths[i]);

	if(max_width + x > dst_width)
		left_x = dst_width - max_width + getCharWidth(' ');
	else
		left_x = x;

	if(max_width + left_x > dst_height)
		left_x = dst_width - max_width;

	for(i = 0; i < nb_subs; i++) {
		drawSubstring(substrings[i], buffer, dst_width, left_x, y);
		y += getStringHeight(substrings[i]);
		delete []substrings[i];
	}

	delete []substr_widths;
	delete []substrings;
}

void SmushFont::drawStringWrapCentered(char *str, byte *buffer, int dst_width, int dst_height, int x, int32 y, int width) {
	debug(9, "SmushFont::drawStringWrapCentered(%s, %d, %d)", str, x, y);
	
	int max_substr_width = 0;
	if ((strchr(str, '\n') != 0)) {
		char *z = strchr(str, '\n');
		*z = 0;
	}
	char **words = split(str, ' ');
	int nb_sub = 0;

	while(words[nb_sub])
		nb_sub++;

	int *sizes = new int[nb_sub];
	int i = 0, height = 0, nb_subs = 0;

	for(i = 0; i < nb_sub; i++)
		sizes[i] = getStringWidth(words[i]);

	char **substrings = new char *[nb_sub];
	int *substr_widths = new int[nb_sub];
	int space_width = getCharWidth(' ');

	i = 0;
	width = MIN(width, dst_width);
	while(i < nb_sub) {
		int substr_width = sizes[i];
		char *substr = new char[1000];
		strcpy(substr, words[i]);
		int j = i + 1;

		while(j < nb_sub && (substr_width + space_width + sizes[j]) < width) {
			substr_width += sizes[j++] + space_width;
		}

		for(int k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[nb_subs] = substr;
		substr_widths[nb_subs++] = substr_width;
		max_substr_width = MAX(substr_width, max_substr_width);
		i = j;
		height += getStringHeight(substr);
	}

	delete []sizes;
	for(i = 0; i < nb_sub; i++) {
		delete []words[i];
	}
	delete []words;

	if(y + height > dst_height) {
		y = dst_height - height;
	}

	x = (dst_width - max_substr_width) / 2;

	for(i = 0; i < nb_subs; i++) {
		int substr_width = substr_widths[i];
		drawSubstring(substrings[i], buffer, dst_width, x + (max_substr_width - substr_width) / 2, y);
		y += getStringHeight(substrings[i]);
		delete []substrings[i];
	}

	delete []substr_widths;
	delete []substrings;
}
