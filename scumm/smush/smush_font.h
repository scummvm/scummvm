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

#ifndef SMUSH_FONT_H
#define SMUSH_FONT_H

#include "common/scummsys.h"

class SmushFont {
private:

	int _nbChars;
	int16 _color;
	bool _new_colors;
	bool _original;
	byte *_dataSrc;
	
	struct {
		int width;
		int height;
		byte *chr;
	} _chars[256];

public:

	SmushFont(bool use_original_colors, bool new_colors);
	~SmushFont();

protected:

	int getCharWidth(byte c);
	int getStringWidth(char *str);
	int getCharHeight(byte c);
	int getStringHeight(char *str);
	int drawChar(byte *buffer, int dst_width, int x, int y, byte chr);
	void drawSubstring(char *str, byte *buffer, int dst_width, int x, int y);
	void decodeCodec(byte *dst, byte *src, int length);

public:

	bool loadFont(const char *filename, const char *directory);
	void setColor(byte c) { _color = c; }
	void drawStringCentered(char *str, byte *buffer, int dst_width, int dst_height, int y, int xmin, int width, int offset);
	void drawStringWrap(char *str, byte *buffer, int dst_width, int dst_height, int x, int y, int width);
	void drawStringWrapCentered(char *str, byte *buffer, int dst_width, int dst_height, int x, int32 y, int width);
	void drawStringAbsolute(char *str, byte *buffer, int dst_width, int x, int y);
};

#endif
