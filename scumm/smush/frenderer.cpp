/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

#include <stdafx.h>
#include "common/util.h"
#include "common/engine.h" // for debug, warning, error

#include "frenderer.h"

#include <assert.h>
#include <string.h>

FontRenderer::FontRenderer(bool use_original_colors) :
	_nbChars(0),
	_color(-1),
	_original(use_original_colors) {
}

FontRenderer::~FontRenderer() {
	for(int32 i = 0; i < _nbChars; i++) {
		if(_chars[i].chr) delete []_chars[i].chr;
	}
}

void FontRenderer::save(int32 frame) {
	_chars[_nbChars].width = getWidth();
	_chars[_nbChars].height = getHeight();
	int size = getWidth() * getHeight();
	_chars[_nbChars].chr = new char[size];
	memcpy(_chars[_nbChars].chr, data(), size);
	_nbChars++;
}

int32 FontRenderer::charWidth(int32 v) const {
	if(v < 0) v = 256 + v;
	if(v < 0 || v >= _nbChars) error("invalid character in FontRenderer::charWidth : %d (%d)", v, _nbChars);
	return _chars[v].width;
}

int32 FontRenderer::charHeight(int32 v) const {
	if(v < 0) v = 256 + v;
	if(v < 0 || v >= _nbChars) error("invalid character in FontRenderer::charHeight : %d (%d)", v, _nbChars);
	return _chars[v].height;
}

int32 FontRenderer::stringWidth(const char * str) const {
	int32 ret = 0;

	while(*str) {
		ret += charWidth(*str++);
	}

	return ret;
}

int32 FontRenderer::stringHeight(const char * str) const {
	int32 ret = 0;

	for(int32 i = 0; str[i] != 0; i++) {
		int32 h = charHeight(str[i]);
		ret = MAX(ret, h);
	}

	return ret;
}

int32 FontRenderer::drawChar(char * buffer, const Point & size, int32 x, int32 y, int32 chr) const {
	int32 w = _chars[chr].width;
	int32 h = _chars[chr].height;
	char * src = _chars[chr].chr;
	char * dst = buffer + size.getX() * y + x;

	if(_original) {
		for(int32 j = 0; j < h; j++) {
			for(int32 i = 0; i < w; i++) {
				char value = *src++;
				if(value) dst[i] = value;
			}
			dst += size.getX();
		}
	} else {
		char color = (_color != -1) ? _color : 1;
		for(int32 j = 0; j < h; j++) {
			for(int32 i = 0; i < w; i++) {
				char value = *src++;
				if(value == 1) {
					dst[i] = color;
				} else if(value) {
					dst[i] = 0;
				}
			}
			dst += size.getX();
		}
	}
	return w;
}

static char * * split(const char * str, char sep) {
	char * * ret = new char *[62];
	int32 n = 0;
	const char * i = str, * j = strchr(i, sep);

	while(j != NULL) {
		assert(n < 60);
		ret[n] = new char[j - i + 1];
		memcpy(ret[n], i, j - i);
		ret[n++][j - i] = 0;
		i = j+1;
		j = strchr(i, sep);
	}

	ret[n] = new char[strlen(i) + 1];
	memcpy(ret[n], i, strlen(i));
	ret[n++][strlen(i)] = 0;
	ret[n] = 0;
	return ret;
}

void FontRenderer::drawSubstring(const byte * str, char * buffer, const Point & size, int32 x, int32 y) const {
	for(int32 i = 0; str[i] != 0; i++)
		x += drawChar(buffer, size, x, y, str[i]);
}

bool FontRenderer::drawStringAbsolute(const char * str, char * buffer, const Point & size, int32 x, int32 y) const {
	debug(9, "FontRenderer::drawStringAbsolute(%s, %d, %d)", str, x, y);
	while(str) {
		char line[256];
		char * pos = strchr(str, '\n');
		if(pos) {
			memcpy(line, str, pos - str - 1);
			line[pos - str - 1] = 0;
			str = pos + 1;
		} else {
			strcpy(line, str);
			str = 0;
		}
		drawSubstring((const byte *)line, buffer, size, x, y);
		y += stringHeight(line);
	}
	return true;
}

bool FontRenderer::drawStringCentered(const char * str, char * buffer, const Point & size, int32 y, int32 xmin, int32 width, int32 offset) const {
	debug(9, "FontRenderer::drawStringCentered(%s, %d, %d)", str, xmin, y);
	if ((strchr(str, '\n') != 0)) {
		char * j = strchr(str, '\n');
		*j = 0;
	}
	char * * words = split(str, ' ');
	int32 nb_sub = 0;

	while(words[nb_sub]) nb_sub++;

	int32 * sizes = new int32[nb_sub];
	int32 i = 0, max_width = 0, height = 0, nb_subs = 0;

	for(i = 0; i < nb_sub; i++)
		sizes[i] = stringWidth(words[i]);

	char * * substrings = new char *[nb_sub];
	int32 * substr_widths = new int32[nb_sub];
	int32 space_width = charWidth(' ');

	i = 0;
	while(i < nb_sub) {
		int32 substr_width = sizes[i];
		char * substr = new char[1000];
		strcpy(substr, words[i]);
		int32 j = i + 1;

		while(j < nb_sub && (substr_width + space_width + sizes[j]) < width) {
			substr_width += sizes[j++] + space_width;
		}

		for(int32 k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[nb_subs] = substr;
		substr_widths[nb_subs++] = substr_width;
		if(substr_width > max_width)
			max_width = substr_width;
		i = j;
		height += stringHeight(substr);
	}

	delete []sizes;
	for(i = 0; i < nb_sub; i++) {
		delete []words[i];
	}
	delete []words;
	
	max_width = (max_width + 1) >> 1;
	// we have a box from 0 -> max_width
	// we want a box from (xmin + offset) - max_width / 2, (xmin + offset) + max_width / 2
	int x = xmin + width / 2;
	x += offset - size.getX() / 2;

	if(x < max_width) x = max_width;
	if(x + max_width > size.getX()) {
		x = size.getX() - max_width;
	}

	if(y + height > size.getY()) {
		y = size.getY() - height;
	}

	for(i = 0; i < nb_subs; i++) {
		int32 substr_width = substr_widths[i];
		drawSubstring((const byte *)substrings[i], buffer, size, x - substr_width / 2, y);
		y += stringHeight(substrings[i]);
		delete []substrings[i];
	}

	delete []substr_widths;
	delete []substrings;
	return true;
}

bool FontRenderer::drawStringWrap(const char * str, char * buffer, const Point & size, int32 x, int32 y, int32 width) const {
	debug(9, "FontRenderer::drawStringWrap(%s, %d, %d)", str, x, y);
	if ((strchr(str, '\n') != 0)) {
		char * j = strchr(str, '\n');
		*j = 0;
	}
	char * * words = split(str, ' ');
	int32 nb_sub = 0;

	while(words[nb_sub]) nb_sub++;

	int32 * sizes = new int32[nb_sub];
	int32 i = 0, max_width = 0, height = 0, nb_subs = 0, left_x;

	for(i = 0; i < nb_sub; i++)
		sizes[i] = stringWidth(words[i]);

	char * * substrings = new char *[nb_sub];
	int32 * substr_widths = new int32[nb_sub];
	int32 space_width = charWidth(' ');

	i = 0;
	while(i < nb_sub) {
		int32 substr_width = sizes[i];
		char * substr = new char[1000];
		strcpy(substr, words[i]);
		int32 j = i + 1;

		while(j < nb_sub && (substr_width + space_width + sizes[j]) < width) {
			substr_width += sizes[j++] + space_width;
		}

		for(int32 k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[nb_subs] = substr;
		substr_widths[nb_subs++] = substr_width;
		i = j;
		height += stringHeight(substr);
	}

	delete []sizes;
	for(i = 0; i < nb_sub; i++) {
		delete []words[i];
	}
	delete []words;

	if(y + height > size.getY()) {
		y = size.getY() - height;
	}

	for(i = 0; i < nb_subs; i++)
		max_width = MAX(max_width, substr_widths[i]);

	if(max_width + x > size.getX())
		left_x = size.getX() - max_width + charWidth(' ');
	else
		left_x = x;

	if(max_width + left_x > size.getX())
		left_x = size.getX() - max_width;

	for(i = 0; i < nb_subs; i++) {
		drawSubstring((const byte *)substrings[i], buffer, size, left_x, y);
		y += stringHeight(substrings[i]);
		delete []substrings[i];
	}

	delete []substr_widths;
	delete []substrings;
	return true;
}

bool FontRenderer::drawStringWrapCentered(const char * str, char * buffer, const Point & size, int32 x, int32 y, int32 width) const {
	int32 max_substr_width = 0;
	debug(9, "FontRenderer::drawStringWrapCentered(%s, %d, %d)", str, x, y);
	if ((strchr(str, '\n') != 0)) {
		char * j = strchr(str, '\n');
		*j = 0;
	}
	char * * words = split(str, ' ');
	int32 nb_sub = 0;

	while(words[nb_sub]) nb_sub++;

	int32 * sizes = new int32[nb_sub];
	int32 i = 0, height = 0, nb_subs = 0;

	for(i = 0; i < nb_sub; i++)
		sizes[i] = stringWidth(words[i]);

	char * * substrings = new char *[nb_sub];
	int32 * substr_widths = new int32[nb_sub];
	int32 space_width = charWidth(' ');

	i = 0;
	width = MIN(width, size.getX());
	while(i < nb_sub) {
		int32 substr_width = sizes[i];
		char * substr = new char[1000];
		strcpy(substr, words[i]);
		int32 j = i + 1;

		while(j < nb_sub && (substr_width + space_width + sizes[j]) < width) {
			substr_width += sizes[j++] + space_width;
		}

		for(int32 k = i + 1; k < j; k++) {
			strcat(substr, " ");
			strcat(substr, words[k]);
		}

		substrings[nb_subs] = substr;
		substr_widths[nb_subs++] = substr_width;
		max_substr_width = MAX(substr_width, max_substr_width);
		i = j;
		height += stringHeight(substr);
	}

	delete []sizes;
	for(i = 0; i < nb_sub; i++) {
		delete []words[i];
	}
	delete []words;

	if(y + height > size.getY()) {
		y = size.getY() - height;
	}

	if(x - max_substr_width / 2 < 0) {
		x = max_substr_width / 2;
	} else if (x + max_substr_width / 2 >= size.getX()) {
		x = size.getX() - 1 - max_substr_width / 2;
	}

	for(i = 0; i < nb_subs; i++) {
		int32 substr_width = substr_widths[i];
		drawSubstring((const byte *)substrings[i], buffer, size, x - substr_width / 2, y);
		y += stringHeight(substrings[i]);
		delete []substrings[i];
	}

	delete []substr_widths;
	delete []substrings;
	return true;
}
