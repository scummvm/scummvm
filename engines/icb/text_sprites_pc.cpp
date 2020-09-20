/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/text_sprites.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"

namespace ICB {

#define SPACE ' ' // ASCII for space character

_rgb *psTempSpeechColour = NULL;

_TSrtn text_sprite::BuildTextSprite(int32 stopAtLine, bool8 bRemoraLeftFormatting) {
	uint8 *linePtr;                                                                // used to point to the start of each line of pixels in the text sprite
	uint8 *spritePtr;                                                              // used to point to data within the sprite
	uint32 lineNo;                                                                 // line no. of text line within sprite
	uint32 pos = 0;                                                                // keeps track of current character position within the whole text line
	uint32 posInLine;                                                              // for keeping track of character position within each line
	uint32 charHeight = CharHeight(params.fontResource, params.fontResource_hash); // height of each character in the font (should all be the same!)

	uint8 pnSavePalette[24]; // Room to save first 5 palette entries.

	_pxSprite *charHead;
	uint8 *pal;

	Zdebug("BuildTextSprite");

	// If a line to stop at has been passed in then use it, otherwise display every line.
	if (stopAtLine == -1)
		stopAtLine = lineInfo.noOfLines;

	// width of text sprite = width of widest line of output text
	spriteWidth = 0;

	for (lineNo = 0; lineNo < (uint32)stopAtLine; lineNo++)
		if (lineInfo.line[lineNo].width > spriteWidth)
			spriteWidth = lineInfo.line[lineNo].width;

	// height of text sprite = total height of lines + tot height of line spacing
	spriteHeight = charHeight * stopAtLine + params.lineSpacing * (stopAtLine - 1);

	// total size (no of bytes)
	size = (spriteWidth * 3) * spriteHeight; // params.bitDepth;

	Zdebug("size - w=%d h=%d mem=%d", spriteWidth, spriteHeight, size);

	if (size > TEXT_SPRITE_SIZE)
		Fatal_error("text sprite too big - w=%d h=%d mem=%d", spriteWidth, spriteHeight, size);

	// allocate memory for sprite & zero the contents (transparency always zeroes)
	memset(sprite, 0, size); // zero all the sprite data

	// ok, now point to the start (of the first line) of the sprite data itelf
	linePtr = sprite;

	_pxBitmap *charSet;

	charSet = (_pxBitmap *)rs_font->Res_open(const_cast<char *>(params.fontResource), params.fontResource_hash, font_cluster, font_cluster_hash); // open font file

	if (charSet->schema != PC_BITMAP_SCHEMA)
		Fatal_error("Incorrect versions loading [%s] (engine has %d, data has %d", const_cast<char *>(params.fontResource), PC_BITMAP_SCHEMA, charSet->schema);

	pal = (uint8 *)charSet->Fetch_palette_pointer();

	// If the temporary text colour is set, copy it into palette entry 1.  NB: This code assumes a 32-bit
	// palette.  If this ever changes then this code needs to change.  This should be done with schema numbers.
	if (psTempSpeechColour) {
		// Save the first 6 entries of the palette.
		memcpy(pnSavePalette, pal, 24);

		// Put in the main text colour.
		pal[12] = psTempSpeechColour->blue;
		pal[13] = psTempSpeechColour->green;
		pal[14] = psTempSpeechColour->red;

		// Put in first anti-alias colour.
		pal[16] = (uint8)((float)psTempSpeechColour->blue * 0.65);
		pal[17] = (uint8)((float)psTempSpeechColour->green * 0.65);
		pal[18] = (uint8)((float)psTempSpeechColour->red * 0.65);

		// Put in second anti-alias colour.
		pal[20] = (uint8)((float)psTempSpeechColour->blue * 0.3);
		pal[21] = (uint8)((float)psTempSpeechColour->green * 0.3);
		pal[22] = (uint8)((float)psTempSpeechColour->red * 0.3);
	}

	// fill sprite with characters, one line at a time
	for (lineNo = 0; lineNo < (uint32)stopAtLine; lineNo++) {
		// position the start of the line such that the line is centred across the sprite
		spritePtr = linePtr;
		if (!bRemoraLeftFormatting)
			spritePtr += (((spriteWidth - lineInfo.line[lineNo].width) / 2) * 3);
		Zdebug("line[%d].width %d", lineNo, lineInfo.line[lineNo].width);

		// copy the sprite for each character in this line to the text sprite
		// and inc the sprite ptr by the character's width minus the 'overlap'
		for (posInLine = 0; posInLine < lineInfo.line[lineNo].length; posInLine++) {
			charHead = FindChar(params.textLine[pos++], charSet);

			CopyChar(charHead, spritePtr, pal);

			spritePtr += ((charHead->width + params.charSpacing) * 3);
		}

		pos++; // skip space at end of last word in this line

		// move to start of next character line in text sprite
		linePtr += (charHeight + params.lineSpacing) * (spriteWidth * 3);
	}

	// We don't have a res_man.Res_close(params.fontResource) because it will age out of memory anyway.
	// As it happens we can PURGE resources but we know that a font resource is small
	// and is likely to be used a lot anyway, so there is no gain in forcing it out of memory here.

	// restore the palette if we overwrote it.
	if (psTempSpeechColour) {
		memcpy(pal, pnSavePalette, 24);

		psTempSpeechColour = NULL;
	}

	return TS_OK; // return with success
}

} // End of namespace ICB
