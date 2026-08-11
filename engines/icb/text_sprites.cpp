/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/icb/common/px_common.h"
#include "engines/icb/text_sprites.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/res_man.h"

namespace ICB {

#define SPACE ' ' // ASCII for space character (the word-divider!)

text_sprite::text_sprite() {
	// initialise the object:
	spriteWidth = 0;  // width of text sprite
	spriteHeight = 0; // height of text sprite
	size = 0;         // in bytes (= width * height * bit_depth)
	surfaceId = working_buffer_id;
}

text_sprite::~text_sprite() { Zdebug("**destructing text sprite**"); }

_TSrtn text_sprite::GetRenderCoords(const int32 pinX,           // screen x-coord where we want to position the pin
									const int32 pinY,           // y-coord -"-
									const _pin_position pinPos, // position of pin on text sprite
									const int32 margin)         // margin to keep sprite within edge of screen, or -1 if allowed anywhere

{
	// first, calculate the render coords based on the pin position & desired pin coords:
	Zdebug("GetRenderCoords (x=%d y=%d)", pinX, pinY);

	switch (pinPos) {
	case PIN_AT_CENTRE:
		renderX = pinX - spriteWidth / 2;
		renderY = pinY - spriteHeight / 2;
		break;

	case PIN_AT_CENTRE_OF_TOP:
		renderX = pinX - spriteWidth / 2;
		renderY = pinY;
		break;

	case PIN_AT_CENTRE_OF_BASE:               // this one is used for SPEECH TEXT to keep it centred above the talker's head
		renderX = pinX - spriteWidth / 2; // subtract half the sprite-width from the pin x-coordinate
		renderY = pinY - spriteHeight;    // subtract the sprite-height from the pin y-coordinate
		break;

	case PIN_AT_CENTRE_OF_LEFT:
		renderX = pinX;
		renderY = pinY - spriteHeight / 2;
		break;

	case PIN_AT_CENTRE_OF_RIGHT:
		renderX = pinX - spriteWidth;
		renderY = pinY - spriteHeight / 2;
		break;

	case PIN_AT_TOP_LEFT:
		renderX = pinX;
		renderY = pinY;
		break;

	case PIN_AT_TOP_RIGHT:
		renderX = pinX - spriteWidth;
		renderY = pinY;
		break;

	case PIN_AT_BOTTOM_LEFT:
		renderX = pinX;
		renderY = pinY - spriteHeight;
		break;

	case PIN_AT_BOTTOM_RIGHT:
		renderX = pinX - spriteWidth;
		renderY = pinY - spriteHeight;
		break;

	default: // unrecognised PIN type
		return TS_ILLEGAL_PIN;
	}

	// now check if we want to ensure this sprite is fully on-screen:

	if (margin > -1) { // (-ve value means we don't want sprite forced on screen)
		Zdebug("fix position");
		Zdebug("render x= %d, render y=%d", renderX, renderY);

		// calculate the limits for the render coordinates, based on the desired margin
		int32 left_limit = SCREEN_LEFT_EDGE + margin;
		int32 right_limit = SCREEN_RIGHT_EDGE - margin - (spriteWidth - 1);
		int32 top_limit = SCREEN_TOP_EDGE + margin;
		int32 bottom_limit = SCREEN_BOTTOM_EDGE - margin - (spriteHeight - 1);

		// adjust 'renderX' if sprite too far left or right
		// NB. if sprite wider than screen, it will go off the right, not the left
		if (renderX < left_limit) {
			renderX = left_limit;
			Zdebug("fixleft setting renderX to %d", left_limit);
		} else if (renderX > right_limit) {
			renderX = right_limit;
			Zdebug("fixright setting renderX to %d", right_limit);
		}
		// adjust 'renderY' if sprite too far up or down
		// NB. if sprite higher than the screen, it will go off the bottom, not the top
		if (renderY < top_limit)
			renderY = top_limit;
		else if (renderY > bottom_limit)
			renderY = bottom_limit;

		if (((2 * margin + spriteWidth) > SCREEN_WIDTH) || ((2 * margin + spriteHeight) > SCREEN_DEPTH)) {
			return TS_ILLEGAL_MARGIN; // render coords still set up, but this tells us that
		}                                 // the sprite couldn't quite fit within the given margin!
	}

	return TS_OK;
}

_TSrtn text_sprite::MakeTextSprite(bool8 analysisAlreadyDone, int32 stopAtLine, bool8 bRemoraLeftFormatting) {
	_TSrtn rtnCode; // for keeping copy of text sprite function return codes
	const char *pcTextLine;
	uint32 nCloseBracePos;
	uint32 nLineLength;

	Zdebug("\n\nmake text sprite");

	rtnCode = CheckFontResource(params.fontResource, params.fontResource_hash); // first make sure it's a valid font resource

	if (rtnCode == TS_OK) {
		spriteWidth = 0;  // reset width of text sprite
		spriteHeight = 0; // reset height of text sprite
		size = 0;         // reset size

		// Ignore lines beginning with '&' as this means a non-spoken line
		if (params.textLine[0] == TS_NON_SPOKEN_LINE)
			params.textLine++;

		// Look for speech line numbers.  If present, make sure they parse correctly then either skip
		// them or display them depending on whether or not the feature is turned on.
		nLineLength = strlen((const char *)params.textLine);

		// To be a line number, there must be an open brace as the first string character.
		if (params.textLine[0] == TS_LINENO_OPEN) {
			// Okay, we appear to have a legal line number.  Find the close brace for it.
			nCloseBracePos = 1;
			while ((nCloseBracePos < nLineLength) && (params.textLine[nCloseBracePos] != TS_LINENO_CLOSE))
				++nCloseBracePos;

			// If we didn't find one then this is an error.
			if (nCloseBracePos == nLineLength)
				Fatal_error("Failed to find the end of the line number in [%s]", params.textLine);

			// Right we appear to have a present-and-correct line number.  To display it we don't have
			// to do anything special.  If the displaying of line numbers is turned off then we must skip
			// past the line number.
			if (!g_px->speechLineNumbers) {
				// Skip to first non-space after the line number.
				pcTextLine = (const char *)(&params.textLine[nCloseBracePos + 1]);
				while ((*pcTextLine != '\0') && (*pcTextLine == ' '))
					++pcTextLine;

				// If we got to the end of the string then we have a line number with no text following it.
				if (*pcTextLine == '\0')
					Fatal_error("Found line number [%s] with no text", params.textLine);

				// Write the modified pointer back into the text block.
				params.textLine = (uint8 *)const_cast<char *>(pcTextLine);
			}
		}

		// The analysis of the sentence can be switched out now.  The Remora uses this because it has already
		// done its text formatting and doesn't need to do it again.
		if (!analysisAlreadyDone) {
			lineInfo.noOfLines = 0;      // zero-out the number of lines
			rtnCode = AnalyseSentence(); // create info about layout of lines in sprite
		} else {
			rtnCode = TS_OK;
		}

		if (rtnCode == TS_OK) { // if that went ok
			Zdebug("sentence ok");
			rtnCode = BuildTextSprite(stopAtLine, bRemoraLeftFormatting); // then construct the sprite
		} else {
			if (params.errorChecking < 2) {
				Zdebug("sentence analyses failed");
				Fatal_error("AnalyseSentence failed with return code %d for sentence '%s'", rtnCode, params.textLine);
			}
		}
	}

	Zdebug("\nmade text sprite\n\n");
	return rtnCode;
}

_TSrtn text_sprite::CheckFontResource(const char *fontRes, uint32 fontRes_hash) {

	Zdebug("check font [%s] %d", fontRes, fontRes_hash);

	return TS_OK;
}

_TSrtn text_sprite::AnalyseSentence() {
	uint32 pos = 0;     // keeps track of current character position within the text line
	uint32 wordWidth;   // for adding up the pixel-width of each word in the text line
	uint32 wordLength;  // length (in characters) of each word
	uint32 spaceNeeded; // amount of space needed to add a word to a line
	uint32 lineNo = 0;  // keeps track of line no (as the whole line becomes split into several)
	// joinWidth = how much extra space is needed to append a word to a line (ie. in addition to the word's pixel width)
	// ie. charSpacing + width of SPACE + charSpacing (NB. Depending on charSpacing, it's possible this could actually be negative, hence int32
	int32 joinWidth = CharWidth(SPACE, params.fontResource, params.fontResource_hash) + 2 * params.charSpacing;
	uint8 firstWord = TRUE8; // whether or not this is the first word of the whole text line
	uint8 ch;                // for storing ASCII code of each character read from text line

	Zdebug("AnalyseSentence");

	Zdebug("joinWidth= %d", joinWidth);

	do {
		// new word:
		wordWidth = 0;  // start with zero pixel width
		wordLength = 0; // start with zero no. of chars

		ch = params.textLine[pos++]; // get first char of word (at position 'pos' within text line) & increment 'pos'

		if ((params.errorChecking == 1) && (ch == SPACE)) // should never have a space at the start of a word - it means we have illegal extra spaces (leading/trailing/etc)
			return TS_ILLEGAL_SPACING;

		while ((ch != SPACE) && ch) {                                                                           // while not SPACE or NULL terminator
			wordWidth += CharWidth(ch, params.fontResource, params.fontResource_hash) + params.charSpacing; // inc pixel-width of word by width of char + 'charSpacing'
			wordLength++;                                                                                   // inc length of word by 1 char
			ch = params.textLine[pos++];                                                                    // get next char
		}

		if (wordWidth > (uint32)params.charSpacing)
			wordWidth -= params.charSpacing; // no char_spacing after final letter of word!
		else
			wordWidth = 0;

		// 'ch' is now the SPACE or NULL following the word
		// 'pos' indexes to the position following 'ch'

		if (firstWord) { // first word on first line, so no separating SPACE needed
			lineInfo.line[0].width = (uint16)wordWidth;
			lineInfo.line[0].length = (uint16)wordLength;
			firstWord = FALSE8;
		} else {
			// see how much extra space this word will need to fit on current line
			// (with a separating space character - also overlapped)
			spaceNeeded = joinWidth + wordWidth;

			if ((lineInfo.line[lineNo].width + spaceNeeded) <= params.maxWidth) {                      // if we've room for this word on the current line
				lineInfo.line[lineNo].width = (uint16)(lineInfo.line[lineNo].width + spaceNeeded); // inc pixel-width of line by the appropriate amount
				lineInfo.line[lineNo].length =
				    (uint16)(lineInfo.line[lineNo].length + (1 + wordLength)); // inc length of line by 1 char for space + no. of chars in word
			} else {                                                               // put word (without separating SPACE) at start of next line
				lineNo++;                                                      // for next _textLine structure in the array with the _lineInfo structure

				if (lineNo >= MAX_LINES)          // we've exceeded the MAX_LINES limit
					return TS_TOO_MANY_LINES; // return the error - we will want to check this text line or extend the maximum

				lineInfo.line[lineNo].width = (uint16)wordWidth;   // new line's pixel width is set to pixel width of this word
				lineInfo.line[lineNo].length = (uint16)wordLength; // similar for length in chars
			}
		}
	} while (ch); // while not reached the NULL terminator

	lineInfo.noOfLines = (uint8)(lineNo + 1); // set the no. of lines in the lineInfo structure

	return TS_OK; // return with success
}

uint32 text_sprite::CharWidth(const uint8 ch, const char *fontRes, uint32 fontRes_hash) {
	_pxBitmap *charSet = LoadFont(fontRes, fontRes_hash);

	uint32 nNumber = ch - ' ';
	assert(nNumber < FROM_LE_32(charSet->num_sprites));

	_pxSprite *spr = (_pxSprite *)((byte *)charSet + FROM_LE_32(charSet->sprite_offsets[nNumber]));
	return (spr->width);
}

uint32 text_sprite::CharHeight(const char *fontRes, uint32 fontRes_hash) { // assume all chars the same height!
	_pxBitmap *charSet = LoadFont(fontRes, fontRes_hash);
	_pxSprite *spr = (_pxSprite *)((byte *)charSet + FROM_LE_32(charSet->sprite_offsets[0]));
	return spr->height;
}

_pxSprite *text_sprite::FindChar(uint8 ch, _pxBitmap *charSet) {
	uint32 nNumber = ch - ' ';
	assert(nNumber < FROM_LE_32(charSet->num_sprites));
	return ((_pxSprite *)((byte *)charSet + FROM_LE_32(charSet->sprite_offsets[nNumber])));
}

void text_sprite::CopyChar(_pxSprite *charPtr, uint8 *spritePtr, uint8 *pal) { // copy character into sprite, based on params
	uint8 *rowPtr;
	uint8 *source;
	uint8 *dest;
	uint32 rows;
	uint32 cols;
	uint32 charHeight = CharHeight(params.fontResource, params.fontResource_hash); // height of each character in the font (should all be the same!)

	source = &charPtr->data[0];

	rowPtr = (uint8 *)spritePtr; // pts to start of first row of char within text sprite

	for (rows = 0; rows < charHeight; rows++) {
		dest = rowPtr; // start at beginning of row

		for (cols = 0; cols < charPtr->width; cols++) { // *charPtr is width of char
			if (*source) {
				*dest++ = (uint8)(pal[((*source) * 4)]); // b
				*dest++ = (uint8)(pal[((*source) * 4) + 1]); // g
				*dest++ = (uint8)(pal[((*source) * 4) + 2]); // r
			} else {
				dest += 3;
			}

			++source;
		}
		rowPtr += (spriteWidth * 3); // next row down (add width of text sprite)
	}
}

_pxBitmap *text_sprite::LoadFont(const char *fontRes, uint32 fontRes_hash) {
	pxString font_cluster = FONT_CLUSTER_PATH;
	_pxBitmap *font = (_pxBitmap *)rs_font->Res_open(const_cast<char *>(fontRes), fontRes_hash, font_cluster, font_cluster_hash); // open font file

	return (font);
}

} // End of namespace ICB
