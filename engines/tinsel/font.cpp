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
 */

#include "tinsel/dw.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/object.h"
#include "tinsel/text.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

static char tBuffer[TBUFSZ];

static SCNHANDLE hTagFont = 0, hTalkFont = 0;


/**
 * Return address of tBuffer
 */
char *tBufferAddr() {
	return tBuffer;
}

/**
 * Return hTagFont handle.
 */
SCNHANDLE hTagFontHandle() {
	return hTagFont;
}

/**
 * Return hTalkFont handle.
 */
SCNHANDLE hTalkFontHandle() {
	return hTalkFont;
}

/**
 * Called from dec_tagfont() Glitter function. Store the tag font handle.
 */
void TagFontHandle(SCNHANDLE hf) {
	hTagFont = hf;		// Store the font handle
}

/**
 * Called from dec_talkfont() Glitter function.
 * Store the talk font handle.
 */
void TalkFontHandle(SCNHANDLE hf) {
	hTalkFont = hf;		// Store the font handle
}

/**
 * Poke the background palette into character 0's images.
 */
void fettleFontPal(SCNHANDLE fontPal) {
	const FONT *pFont;
	PIMAGE	pImg;

	assert(fontPal);
	assert(hTagFont); // Tag font not declared
	assert(hTalkFont); // Talk font not declared

	pFont = (const FONT *)LockMem(hTagFont);
	pImg = (PIMAGE)LockMem(FROM_LE_32(pFont->fontInit.hObjImg));	// get image for char 0
	pImg->hImgPal = TO_LE_32(fontPal);

	pFont = (const FONT *)LockMem(hTalkFont);
	pImg = (PIMAGE)LockMem(FROM_LE_32(pFont->fontInit.hObjImg));	// get image for char 0
	pImg->hImgPal = TO_LE_32(fontPal);
}

} // End of namespace Tinsel
