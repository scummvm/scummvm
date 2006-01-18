/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $Header$
 */

// MAKETEXT	- Constructs a single-frame text sprite: returns a handle to a
//		  FLOATING memory block containing the sprite, given a
//		  null-terminated string, max width allowed, pen colour and
//		  pointer to required character set.
//
//		  NB 1) The routine does not create a standard file header or
//		  an anim header for the text sprite - the data simply begins
//		  with the frame header.
//
//		  NB 2) If pen colour is zero, it copies the characters into
//		  the sprite without remapping the colours.
//		  ie. It can handle both the standard 2-colour font for speech
//		  and any multicoloured fonts for control panels, etc.
//
//		  Based on textsprt.c as used for Broken Sword 1, but updated
//		  for new system by JEL on 9oct96 and updated again (for font
//		  as a resource) on 5dec96.

#include "common/stdafx.h"
#include "common/system.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/resman.h"

namespace Sword2 {

#define MAX_LINES	30	// max character lines in output sprite

#define BORDER_COL	200	// source colour for character border (only
				// needed for remapping colours)
#define LETTER_COL	193	// source colour for bulk of character ( " )
#define SPACE		' '
#define FIRST_CHAR	SPACE	// first character in character set
#define LAST_CHAR	255	// last character in character set
#define DUD		64	// the first "chequered flag" (dud) symbol in
				// our character set is in the '@' position

/**
 * This function creates a new text sprite. The sprite data contains a
 * FrameHeader, but not a standard file header.
 *
 * @param  sentence  pointer to a null-terminated string
 * @param  maxWidth  the maximum allowed text sprite width in pixels
 * @param  pen       the text colour, or zero to use the source colours
 * @param  fontRes   the font resource id
 * @param  border    the border colour; black by default
 * @return a handle to a floating memory block containing the text sprite
 * @note   The sentence must contain no leading, trailing or extra spaces.
 *         Out-of-range characters in the string are replaced by a special
 *         error-signal character (chequered flag)
 */

byte *FontRenderer::makeTextSprite(byte *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes, uint8 border) {
	debug(3, "makeTextSprite(\"%s\", maxWidth=%u)", sentence, maxWidth);

	_borderPen = border;

	// Line- and character spacing are hard-wired, rather than being part
	// of the resource.

	if (fontRes == _vm->_speechFontId) {
		_lineSpacing = -6;
		_charSpacing = -3;
	} else if (fontRes == CONSOLE_FONT_ID) {
		_lineSpacing = 0;
		_charSpacing = 1;
	} else {
		_lineSpacing = 0;
		_charSpacing = 0;
	}

	// Allocate memory for array of lineInfo structures

	byte *line = (byte *)malloc(MAX_LINES * sizeof(LineInfo));

	// Get details of sentence breakdown into array of LineInfo structures
	// and get the number of lines involved

	uint16 noOfLines = analyseSentence(sentence, maxWidth, fontRes, (LineInfo *)line);

	// Construct the sprite based on the info gathered - returns floating
	// mem block

	byte *textSprite = buildTextSprite(sentence, fontRes, pen, (LineInfo *)line, noOfLines);

	free(line);
	return textSprite;
}

uint16 FontRenderer::analyseSentence(byte *sentence, uint16 maxWidth, uint32 fontRes, LineInfo *line) {
	// joinWidth = how much extra space is needed to append a word to a
	// line. NB. SPACE requires TWICE the '_charSpacing' to join a word
	// to line

	uint16 joinWidth = charWidth(SPACE, fontRes) + 2 * _charSpacing;

	uint16 lineNo = 0;
	uint16 pos = 0;
	bool firstWord = true;

	byte ch;

	do {
		uint16 wordWidth = 0;
		uint16 wordLength = 0;

		// Calculate the width of the word.

		ch = sentence[pos++];

		while (ch && ch != SPACE) {
			wordWidth += charWidth(ch, fontRes) + _charSpacing;
			wordLength++;
			ch = sentence[pos++];
		}

		// Don't include any character spacing at the end of the word.
		wordWidth -= _charSpacing;

		// 'ch' is now the SPACE or NULL following the word
		// 'pos' indexes to the position following 'ch'

		if (firstWord) {
			// This is the first word on the line, so no separating
			// space is needed.

			line[0].width = wordWidth;
			line[0].length = wordLength;
			firstWord = false;
		} else {
			// See how much extra space this word will need to
			// fit on current line (with a separating space
			// character - also overlapped)

			uint16 spaceNeeded = joinWidth + wordWidth;

			if (line[lineNo].width + spaceNeeded <= maxWidth) {
				// The word fits on this line.
				line[lineNo].width += spaceNeeded;
				line[lineNo].length += (1 + wordLength);
			} else {
				// The word spills over to the next line, i.e.
				// no separating space.

				lineNo++;

				assert(lineNo < MAX_LINES);

				line[lineNo].width = wordWidth;
				line[lineNo].length = wordLength;
			}
		}
	} while (ch);

	return lineNo + 1;
}

/**
 * This function creates a new text sprite in a movable memory block. It must
 * be locked before use, i.e. lock, draw sprite, unlock/free. The sprite data
 * contains a FrameHeader, but not a standard file header.
 *
 * @param  sentence  pointer to a null-terminated string
 * @param  fontRes   the font resource id
 * @param  pen       the text colour, or zero to use the source colours
 * @param  line      array of LineInfo structures, created by analyseSentence()
 * @param  noOfLines the number of lines, i.e. the number of elements in 'line'
 * @return a handle to a floating memory block containing the text sprite
 * @note   The sentence must contain no leading, trailing or extra spaces.
 *         Out-of-range characters in the string are replaced by a special
 *         error-signal character (chequered flag)
 */

byte *FontRenderer::buildTextSprite(byte *sentence, uint32 fontRes, uint8 pen, LineInfo *line, uint16 noOfLines) {
	uint16 i;

	// Find the width of the widest line in the output text

	uint16 spriteWidth = 0;

	for (i = 0; i < noOfLines; i++)
		if (line[i].width > spriteWidth)
			spriteWidth = line[i].width;

	// Find the total height of the text sprite: the total height of the
	// text lines, plus the total height of the spacing between them.

	uint16 char_height = charHeight(fontRes);
	uint16 spriteHeight = char_height * noOfLines + _lineSpacing * (noOfLines - 1);

	// Allocate memory for the text sprite

	uint32 sizeOfSprite = spriteWidth * spriteHeight;
	byte *textSprite = (byte *)malloc(sizeof(FrameHeader) + sizeOfSprite);

	// At this stage, textSprite points to an unmovable memory block. Set
	// up the frame header.

	FrameHeader *frameHeadPtr = (FrameHeader *)textSprite;

	frameHeadPtr->compSize = 0;
	frameHeadPtr->width = spriteWidth;
	frameHeadPtr->height = spriteHeight;

	debug(4, "Text sprite size: %ux%u", spriteWidth, spriteHeight);

	// Clear the entire sprite to make it transparent.

	byte *linePtr = textSprite + sizeof(FrameHeader);
	memset(linePtr, 0, sizeOfSprite);

	byte *charSet = _vm->_resman->openResource(fontRes);

	// Build the sprite, one line at a time

	uint16 pos = 0;

	for (i = 0; i < noOfLines; i++) {
		// Center each line
		byte *spritePtr = linePtr + (spriteWidth - line[i].width) / 2;

		// copy the sprite for each character in this line to the
		// text sprite and inc the sprite ptr by the character's
		// width minus the 'overlap'

		for (uint j = 0; j < line[i].length; j++) {
			FrameHeader *charPtr = findChar(sentence[pos++], charSet);

			assert(charPtr->height == char_height);
			copyChar(charPtr, spritePtr, spriteWidth, pen);
			spritePtr += charPtr->width + _charSpacing;
		}

		// Skip space at end of last word in this line
		pos++;

		linePtr += (char_height + _lineSpacing) * spriteWidth;
	}

	_vm->_resman->closeResource(fontRes);

	return textSprite;
}

/**
 * @param  ch      the ASCII code of the character
 * @param  fontRes the font resource id
 * @return the width of the character
 */

uint16 FontRenderer::charWidth(byte ch, uint32 fontRes) {
	byte *charSet = _vm->_resman->openResource(fontRes);

	FrameHeader *charFrame = findChar(ch, charSet);
	uint16 width = charFrame->width;

	_vm->_resman->closeResource(fontRes);
	return width;
}

/**
 * @param  fontRes the font resource id
 * @return the height of a character sprite
 * @note   All characters in a font are assumed to have the same height, so
 *         there is no need to specify which one to look at.
 */

// Returns the height of a character sprite, given the character's ASCII code
// and a pointer to the start of the character set.

uint16 FontRenderer::charHeight(uint32 fontRes) {
	byte *charSet = _vm->_resman->openResource(fontRes);

	FrameHeader *charFrame = findChar(FIRST_CHAR, charSet);
	uint16 height = charFrame->height;

	_vm->_resman->closeResource(fontRes);
	return height;
}

/**
 * @param  ch      the ASCII code of the character to find
 * @param  charSet pointer to the start of the character set
 * @return pointer to the requested character or, if it's out of range, the
 *         'dud' character (chequered flag)
 */

FrameHeader* FontRenderer::findChar(byte ch, byte *charSet) {
	if (ch < FIRST_CHAR)
		ch = DUD;
	return _vm->fetchFrameHeader(charSet, ch - FIRST_CHAR);
}

/**
 * Copies a character sprite to the sprite buffer.
 * @param charPtr     pointer to the character sprite
 * @param spritePtr   pointer to the sprite buffer
 * @param spriteWidth the width of the character
 * @param pen         If zero, copy the data directly. Otherwise remap the
 *                    sprite's colours from BORDER_COL to _borderPen and from
 *                    LETTER_COL to pen.
 */

void FontRenderer::copyChar(FrameHeader *charPtr, byte *spritePtr, uint16 spriteWidth, uint8 pen) {
	byte *source = (byte *)charPtr + sizeof(FrameHeader);
	byte *rowPtr = spritePtr;

	for (uint i = 0; i < charPtr->height; i++) {
		byte *dest = rowPtr;

		if (pen) {
			// Use the specified colours
			for (uint j = 0; j < charPtr->width; j++) {
				switch (*source++) {
				case LETTER_COL:
					*dest = pen;
					break;
				case BORDER_COL:
					// Don't do a border pixel if there's
					// already a bit of another character
					// underneath (for overlapping!)
					if (!*dest)
						*dest = _borderPen;
					break;
				default:
					// Do nothing if source pixel is zero,
					// ie. transparent
					break;
				}
				dest++;
			}
		} else {
			// Pen is zero, so just copy character sprites
			// directly into text sprite without remapping colours.
			// Apparently overlapping is never considered here?
			memcpy(dest, source, charPtr->width);
			source += charPtr->width;
		}
		rowPtr += spriteWidth;
	}
}

// Distance to keep speech text from edges of screen
#define TEXT_MARGIN 12

/**
 * Creates a text bloc in the list and returns the bloc number. The list of
 * blocs is read and blitted at render time. Choose alignment type
 * RDSPR_DISPLAYALIGN or 0
 */

uint32 FontRenderer::buildNewBloc(byte *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification) {
	uint32 i = 0;

	while (i < MAX_text_blocs && _blocList[i].text_mem)
		i++;

	assert(i < MAX_text_blocs);

	// Create and position the sprite

	_blocList[i].text_mem = makeTextSprite(ascii, width, pen, fontRes);

	// 'NO_JUSTIFICATION' means print sprite with top-left at (x,y)
	// without margin checking - used for debug text

	if (justification != NO_JUSTIFICATION) {
		FrameHeader *frame_head = (FrameHeader *)_blocList[i].text_mem;

		switch (justification) {
		case POSITION_AT_CENTRE_OF_BASE:
			// This one is always used for SPEECH TEXT; possibly
			// also for pointer text
			x -= (frame_head->width / 2);
			y -= frame_head->height;
			break;
		case POSITION_AT_CENTRE_OF_TOP:
			x -= (frame_head->width / 2);
			break;
		case POSITION_AT_LEFT_OF_TOP:
			// The given coords are already correct for this!
			break;
		case POSITION_AT_RIGHT_OF_TOP:
			x -= frame_head->width;
			break;
		case POSITION_AT_LEFT_OF_BASE:
			y -= frame_head->height;
			break;
		case POSITION_AT_RIGHT_OF_BASE:
			x -= frame_head->width;
			y -= frame_head->height;
			break;
		case POSITION_AT_LEFT_OF_CENTRE:
			y -= (frame_head->height / 2);
			break;
		case POSITION_AT_RIGHT_OF_CENTRE:
			x -= frame_head->width;
			y -= (frame_head->height) / 2;
			break;
		}

		// Ensure text sprite is a few pixels inside the visible screen
		// remember - it's RDSPR_DISPLAYALIGN

		uint16 text_left_margin = TEXT_MARGIN;
		uint16 text_right_margin = 640 - TEXT_MARGIN - frame_head->width;
		uint16 text_top_margin = TEXT_MARGIN;
		uint16 text_bottom_margin = 400 - TEXT_MARGIN - frame_head->height;

		// Move if too far left or too far right

		if (x < text_left_margin)
			x = text_left_margin;
		else if (x > text_right_margin)
			x = text_right_margin;

		// Move if too high or too low

		if (y < text_top_margin)
			y = text_top_margin;
		else if (y > text_bottom_margin)
			y = text_bottom_margin;
	}

	// The sprite is always uncompressed
	_blocList[i].type = type | RDSPR_NOCOMPRESSION;

	_blocList[i].x = x;
	_blocList[i].y = y;

	return i + 1;
}

/**
 * Called by buildDisplay()
 */

void FontRenderer::printTextBlocs() {
	for (uint i = 0; i < MAX_text_blocs; i++) {
		if (_blocList[i].text_mem) {
			FrameHeader *frame = (FrameHeader *)_blocList[i].text_mem;
			SpriteInfo spriteInfo;

			spriteInfo.x = _blocList[i].x;
			spriteInfo.y = _blocList[i].y;
			spriteInfo.w = frame->width;
			spriteInfo.h = frame->height;
			spriteInfo.scale = 0;
			spriteInfo.scaledWidth = 0;
			spriteInfo.scaledHeight = 0;
			spriteInfo.type = _blocList[i].type;
			spriteInfo.blend = 0;
			spriteInfo.data = _blocList[i].text_mem + sizeof(FrameHeader);
			spriteInfo.colourTable = 0;

			uint32 rv = _vm->_screen->drawSprite(&spriteInfo);
			if (rv)
				error("Driver Error %.8x in printTextBlocs", rv);
		}
	}
}

void FontRenderer::killTextBloc(uint32 bloc_number) {
	bloc_number--;
	free(_blocList[bloc_number].text_mem);
	_blocList[bloc_number].text_mem = NULL;
}

// Resource 3258 contains text from location script for 152 (install, save &
// restore text, etc)

#define TEXT_RES	3258

// Local line number of "save" (actor no. 1826)

#define SAVE_LINE_NO	1

void Sword2Engine::initialiseFontResourceFlags() {
	byte *textFile = _resman->openResource(TEXT_RES);

	// If language is Polish or Finnish it requires alternate fonts.
	// Otherwise, use regular fonts

	// "tallenna"   Finnish for "save"
	// "zapisz"     Polish for "save"

	// Get the text line (& skip the 2 chars containing the wavId)
	char *textLine = (char *)fetchTextLine(textFile, SAVE_LINE_NO) + 2;

	if (strcmp(textLine, "tallenna") == 0)
		initialiseFontResourceFlags(FINNISH_TEXT);
	else if (strcmp(textLine, "zapisz") == 0)
		initialiseFontResourceFlags(POLISH_TEXT);
	else
		initialiseFontResourceFlags(DEFAULT_TEXT);

	// Get the game name for the windows application

	// According to the GetNameFunction(), which was never called and has
	// therefore been removed, the name of the game is:
	//
	// ENGLISH:  "Broken Sword II"
	// AMERICAN: "Circle of Blood II"
	// GERMAN:   "Baphomet's Fluch II"
	// default:  "Some game or other, part 86"
	//
	// But we get it from the text resource instead.

	if (Logic::_scriptVars[DEMO])
		textLine = (char *)fetchTextLine(textFile, 451) + 2;
	else
		textLine = (char *)fetchTextLine(textFile, 54) + 2;

	_system->setWindowCaption(textLine);
	_resman->closeResource(TEXT_RES);
}

/**
 * Called from initialiseFontResourceFlags(), and also from console.cpp
 */

void Sword2Engine::initialiseFontResourceFlags(uint8 language) {
	switch (language) {
	case FINNISH_TEXT:
		_speechFontId = FINNISH_SPEECH_FONT_ID;
		_controlsFontId = FINNISH_CONTROLS_FONT_ID;
		_redFontId = FINNISH_RED_FONT_ID;
		break;
	case POLISH_TEXT:
		_speechFontId = POLISH_SPEECH_FONT_ID;
		_controlsFontId = POLISH_CONTROLS_FONT_ID;
		_redFontId = POLISH_RED_FONT_ID;
		break;
	default:
		_speechFontId = ENGLISH_SPEECH_FONT_ID;
		_controlsFontId = ENGLISH_CONTROLS_FONT_ID;
		_redFontId = ENGLISH_RED_FONT_ID;
		break;
	}
}

} // End of namespace Sword2
