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
#include "hugo/file.h"
#include "hugo/util.h"

namespace Hugo {

Screen_v1w::Screen_v1w(HugoEngine *vm) : Screen(vm) {
}

Screen_v1w::~Screen_v1w() {
}

/**
* Load font file, construct font ptrs and reverse data bytes
*/
void Screen_v1w::loadFont(int16 fontId) {
	debugC(2, kDebugDisplay, "loadFont(%d)", fontId);

	static bool fontLoadedFl[NUM_FONTS] = {false, false, false};

	_fnt = fontId - FIRST_FONT;                     // Set current font number

	if (fontLoadedFl[_fnt])                             // If already loaded, return
		return;

	fontLoadedFl[_fnt] = true;
	_vm->_file->readUIFItem(fontId, _fontdata[_fnt]);

	// Compile font ptrs.  Note: First ptr points to height,width of font
	_font[_fnt][0] = _fontdata[_fnt];               // Store height,width of fonts

	int16 offset = 2;                                       // Start at fontdata[2] ([0],[1] used for height,width)

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
} // End of namespace Hugo

