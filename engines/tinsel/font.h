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

#ifndef TINSEL_FONT_H	// prevent multiple includes
#define TINSEL_FONT_H

#include "tinsel/dw.h"

namespace Tinsel {

// A temporary buffer for extracting text into is defined in font.c
// Accessed using TextBufferAddr(), this is how big it is:
#define TBUFSZ	512

class Font {
public:
	Font() : _hTagFont(0), _hTalkFont(0), _hRegularTalkFont(0), _hRegularTagFont(0) {
	}

	/**
	 * Return address of tBuffer
	 */
	char* TextBufferAddr() { return _tBuffer; }

	/**
	 * Return hTagFont handle.
	 */
	SCNHANDLE GetTagFontHandle() { return _hTagFont; }

	/**
	 * Return hTalkFont handle.
	 */
	SCNHANDLE GetTalkFontHandle();

	/**
	 * Called from dec_tagfont() Glitter function. Store the tag font handle.
	 */
	void SetTagFontHandle(SCNHANDLE hFont);

	/**
	 * Called from dec_talkfont() Glitter function.
	 * Store the talk font handle.
	 */
	void SetTalkFontHandle(SCNHANDLE hFont) {
		_hTalkFont = _hRegularTalkFont = hFont;
	}

	/**
	 * Declare a temporary text font (DW2 only).
	 */
	void SetTempTagFontHandle(SCNHANDLE hFont) {
		_hTagFont = hFont;
	}

	/**
	 * Declare a temporary text font (DW2 only).
	 */
	void SetTempTalkFontHandle(SCNHANDLE hFont) {
		_hTalkFont = hFont;
	}

	void ResetFontHandles() {
		_hTagFont = _hRegularTagFont;
		_hTalkFont = _hRegularTalkFont;
	}

	/**
	 * Poke the background palette into character 0's images.
	 */
	void FettleFontPal(SCNHANDLE fontPal);

private:
	char _tBuffer[TBUFSZ];

	SCNHANDLE _hTagFont, _hTalkFont;
	SCNHANDLE _hRegularTalkFont, _hRegularTagFont;
};

} // End of namespace Tinsel

#endif		// TINSEL_FONT_H
