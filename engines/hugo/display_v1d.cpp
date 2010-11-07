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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// Display.c - DIB related code for HUGOWIN

#include "common/system.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/util.h"

namespace Hugo {

Screen_v1d::Screen_v1d(HugoEngine *vm) : Screen(vm) {
}

Screen_v1d::~Screen_v1d() {
}

/**
* Load font file, construct font ptrs and reverse data bytes
* TODO: This uses hardcoded fonts in hugo.dat, it should be replaced
*       by a proper implementation of .FON files
*/
void Screen_v1d::loadFont(int16 fontId) {
	debugC(2, kDebugDisplay, "loadFont(%d)", fontId);

	assert(fontId < NUM_FONTS);

	_fnt = fontId - FIRST_FONT;                     // Set current font number

	if (fontLoadedFl[_fnt])                         // If already loaded, return
		return;

	fontLoadedFl[_fnt] = true;

	memcpy(_fontdata[_fnt], _arrayFont[_fnt], _arrayFontSize[_fnt]);
	_font[_fnt][0] = _fontdata[_fnt];               // Store height,width of fonts

	int16 offset = 2;                               // Start at fontdata[2] ([0],[1] used for height,width)

	// Setup the font array (127 characters)
	for (int i = 1; i < 128; i++) {
		_font[_fnt][i] = _fontdata[_fnt] + offset;
		byte height = *(_fontdata[_fnt] + offset);
		byte width  = *(_fontdata[_fnt] + offset + 1);

		int16 size = height * ((width + 7) >> 3);
		for (int j = 0; j < size; j++)
			Utils::reverseByte(&_fontdata[_fnt][offset + 2 + j]);

		offset += 2 + size;
	}
}

/**
* Load fonts from Hugo.dat
* These fonts are a workaround to avoid handling TTF fonts used by DOS versions
* TODO: Properly handle the vector based font files (win31)
*/
void Screen_v1d::loadFontArr(Common::File &in) {
	for (int i = 0; i < NUM_FONTS; i++) {
		_arrayFontSize[i] = in.readUint16BE();
		_arrayFont[i] = (byte *)malloc(sizeof(byte) * _arrayFontSize[i]);
		for (int j = 0; j < _arrayFontSize[i]; j++) {
			_arrayFont[i][j] = in.readByte();
		}
	}
}
} // End of namespace Hugo

