/* Copyright (C) 1994-2003 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#define MAX_LINES	30	// max character lines in output sprite

#define BORDER_COL	200	// source colour for character border (only
				// needed for remapping colours)
#define LETTER_COL	193	// source colour for bulk of character ( " )
#define NO_COL		0	// sprite background - 0 for transparency!
#define SPACE		' '
#define FIRST_CHAR	SPACE	// first character in character set
#define LAST_CHAR	255	// last character in character set
#define DUD		64	// the first "chequered flag" (dud) symbol in
				// our character set is in the '@' position
#include "stdafx.h"
#include "sword2/driver/driver96.h"
#include "sword2/console.h"
#include "sword2/debug.h"
#include "sword2/defs.h"		// for SPEECH_FONT_ID & CONSOLE_FONT_ID
#include "sword2/header.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/protocol.h"	// for fetchFrameHeader()
#include "sword2/resman.h"
#include "sword2/sword2.h"

namespace Sword2 {

FontRenderer *fontRenderer;

// info for each line of words in the output text sprite

mem* FontRenderer::makeTextSprite(uint8 *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes, uint8 border) {
	mem *line;		// handle for the memory block which will
				// contain the array of lineInfo structures
	mem *textSprite;	// handle for the block to contain the text
				// sprite itself
	uint16 noOfLines;	// no of lines of text required to fit within
				// a sprite of width 'maxWidth' pixels

	debug(5, "makeTextSprite(\"%s\", maxWidth=%u)", sentence, maxWidth);

	_borderPen = border;

	// NB. ensure sentence contains no leading/tailing/extra spaces - if
	// necessary, copy to another array first, missing the extra spaces.

	// set the global layout variables

	if (fontRes == g_sword2->_speechFontId) {
		_lineSpacing = -6;  // overlap lines by 6 pixels
		_charSpacing = -3;  // overlap characters by 3 pixels
	} else if (fontRes == CONSOLE_FONT_ID) {
		_lineSpacing = 0;   // no space or overlap between lines
		_charSpacing = 1;   // 1 pixel spacing between each character
	} else {
		_lineSpacing = 0;
		_charSpacing = 0;
	}

	// allocate memory for array of lineInfo structures

	line = memory->allocMemory(MAX_LINES * sizeof(LineInfo), MEM_locked, UID_temp);

	// get details of sentence breakdown into array of LineInfo structures
	// and get the no of lines involved

	noOfLines = analyseSentence(sentence, maxWidth, fontRes, (LineInfo *) line->ad);

	// construct the sprite based on the info gathered - returns floating
	// mem block

	textSprite = buildTextSprite(sentence, fontRes, pen, (LineInfo *) line->ad, noOfLines);

	// free up the lineInfo array now
	memory->freeMemory(line);

	return textSprite;
}

uint16 FontRenderer::analyseSentence(uint8 *sentence, uint16 maxWidth, uint32 fontRes, LineInfo *line) {
	uint16 pos = 0, wordWidth, wordLength, spaceNeeded;
	uint16 lineNo = 0;
	uint8 ch;
	bool firstWord = true;

	// joinWidth = how much extra space is needed to append a word to a
	// line. NB. SPACE requires TWICE the '_charSpacing' to join a word
	// to line

	uint16 joinWidth = charWidth(SPACE, fontRes) + 2 * _charSpacing;
	
	// while not reached the NULL terminator

	do {
		// new word
		wordWidth = 0;
		wordLength = 0;

		// get first char of word (at position 'pos')
		ch = sentence[pos++];

		// while not SPACE or NULL terminator

		while ((ch != SPACE) && ch) {
			wordWidth += charWidth(ch, fontRes) + _charSpacing;
			wordLength++;
			ch = sentence[pos++];
		}

		// no _charSpacing after final letter of word!
		wordWidth -= _charSpacing;

		// 'ch' is now the SPACE or NULL following the word
		// 'pos' indexes to the position following 'ch'

		if (firstWord) {
			// first word on first line, so no separating SPACE
			// needed

			line[0].width = wordWidth;
			line[0].length = wordLength;
			firstWord = false;
		} else {
			// see how much extra space this word will need to
			// fit on current line (with a separating space
			// character - also overlapped)

			spaceNeeded = joinWidth + wordWidth;

			if (line[lineNo].width + spaceNeeded <= maxWidth) {
				// fits this line
				line[lineNo].width += spaceNeeded;
				// NB. space+word characters
				line[lineNo].length += 1 + wordLength;
			} else {
				// put word (without separating SPACE) at
				// start of next line

				// for next LineInfo structure in the array
				lineNo++;

				// exception if lineNo >= MAX_LINES
				// debug_only( lineNo < MAX_LINES );

				line[lineNo].width = wordWidth;
				line[lineNo].length = wordLength;
			}
		}
	} while (ch);

	// return no of lines
	return lineNo + 1;
}

// Returns a handle to a floating memory block containing a text sprite, given
// a pointer to a null-terminated string, pointer to required character set,
// required text pen colour (or zero to use source colours), pointer to the
// array of linInfo structures created by 'analyseSentence()', and the number
// of lines (ie. no. of elements in the 'line' array).

// PC Version of BuildTextSprite

mem* FontRenderer::buildTextSprite(uint8 *sentence, uint32 fontRes, uint8 pen, LineInfo *line, uint16 noOfLines) {
	uint8 *linePtr, *spritePtr;
	uint16 lineNo, pos = 0, posInLine, spriteWidth = 0, spriteHeight;
	uint16 sizeOfSprite;
	uint16 char_height = charHeight(fontRes);
	_frameHeader *frameHeadPtr, *charPtr;
	mem *textSprite;
	uint8 *charSet;

	// spriteWidth = width of widest line of output text

	for (lineNo = 0; lineNo < noOfLines; lineNo++)
		if (line[lineNo].width > spriteWidth)
			spriteWidth = line[lineNo].width;

	// spriteHeight = tot height of char lines + tot height of separating
	// lines

	spriteHeight = char_height * noOfLines + _lineSpacing * (noOfLines - 1);

	// total size (no of pixels)
	sizeOfSprite = spriteWidth * spriteHeight;

	// allocate memory for sprite, and lock it ready for use
	// NB. 'textSprite' is the given pointer to the handle to be used
	textSprite = memory->allocMemory(sizeof(_frameHeader) + sizeOfSprite, MEM_locked, UID_text_sprite);

	// the handle (*textSprite) now points to UNMOVABLE memory block
	// set up the frame header

	// point to the start of our memory block
	frameHeadPtr = (_frameHeader *) textSprite->ad;

	frameHeadPtr->compSize = 0;
	frameHeadPtr->width = spriteWidth;
	frameHeadPtr->height = spriteHeight;
	
	debug(5, "spriteWidth=%u", spriteWidth);
	debug(5, "spriteHeight=%u", spriteHeight);

	// ok, now point to the start (of the first line) of the sprite data
	// itelf

	linePtr = textSprite->ad + sizeof(_frameHeader);

	// start with transparent sprite (no colour)
	memset(linePtr, NO_COL, sizeOfSprite);

	// open font file
	charSet = res_man->openResource(fontRes);

	// fill sprite with characters, one line at a time

	for (lineNo = 0; lineNo < noOfLines; lineNo++) {
		// position the start of the line so that it is centred
		// across the sprite

		spritePtr = linePtr + (spriteWidth - line[lineNo].width) / 2;

		// copy the sprite for each character in this line to the
		// text sprite and inc the sprite ptr by the character's
		// width minus the 'overlap'

		for (posInLine = 0; posInLine < line[lineNo].length; posInLine++) {
			charPtr = findChar(sentence[pos++], charSet);

#ifdef _SWORD2_DEBUG			
			if (charPtr->height != char_height)
				error("FONT ERROR: '%c' is not same height as the space", sentence[pos - 1]);
#endif

			copyChar(charPtr, spritePtr, spriteWidth, pen);
			spritePtr += charPtr->width + _charSpacing;
		}

		// skip space at end of last word in this line
		pos++;

		// move to start of next character line in text sprite
		linePtr += (char_height + _lineSpacing) * spriteWidth;
	}

	// close font file
	res_man->closeResource(fontRes);

	// unlock the sprite memory block, so it's movable
	memory->floatMemory(textSprite);

	return textSprite;
}

// Returns the width of a character sprite, given the character's ASCII code
// and a pointer to the start of the character set.

uint16 FontRenderer::charWidth(uint8 ch, uint32 fontRes) {
	_frameHeader *charFrame;
	uint8 *charSet;
	uint16 width;

	// open font file
	charSet = res_man->openResource(fontRes);

	// move to approp. sprite (header)
	charFrame = findChar(ch, charSet);
	width = charFrame->width;

	// close font file
 	res_man->closeResource(fontRes);

	// return its width
	return width;
}

// Returns the height of a character sprite, given the character's ASCII code
// and a pointer to the start of the character set.

uint16 FontRenderer::charHeight(uint32 fontRes) {
	_frameHeader *charFrame;
	uint8 *charSet;
	uint16 height;

	// open font file
	charSet = res_man->openResource(fontRes);

	// assume all chars the same height, i.e. FIRST_CHAR is as good as any
	charFrame = findChar(FIRST_CHAR, charSet);
	height = charFrame->height;

	// close font file
	res_man->closeResource(fontRes);

	// return its height
	return height;
}

// Returns a pointer to the header of a character sprite, given the character's
// ASCII code and a pointer to the start of the character set.

_frameHeader* FontRenderer::findChar(uint8 ch, uint8 *charSet) {
	// if 'ch' out of range, print the 'dud' character (chequered flag)
	if (ch < FIRST_CHAR)
		ch = DUD;

	return g_sword2->fetchFrameHeader(charSet, ch - FIRST_CHAR);
}

// Copies a character sprite from 'charPtr' to the sprite buffer at 'spritePtr'
// of width 'spriteWidth'. If pen is zero, it copies the data across directly,
// otherwise it maps pixels of BORDER_COL to '_borderPen', and LETTER_COL to
// 'pen'.

void FontRenderer::copyChar(_frameHeader *charPtr, uint8 *spritePtr, uint16 spriteWidth, uint8 pen) {
	uint8 *rowPtr, *source, *dest;
	uint16 rows, cols;

	// now pts to sprite data for char 'ch'
	source = (uint8 *) charPtr + sizeof(_frameHeader);

	// pts to start of first row of char within text sprite
	rowPtr = spritePtr;

	for (rows = 0; rows < charPtr->height; rows++) {
		// start at beginning of row
		dest = rowPtr;

		// if required output pen is non-zero
		if (pen) {
			for (cols = 0; cols < charPtr->width; cols++) {
				// inc source ptr along sprite data
				switch (*source++) {
				case LETTER_COL:
					*dest = pen;
					break;
				case BORDER_COL:
					// don't do a border pixel if there's
					// already a bit of another character
					// underneath (for overlapping!)

					if (!*dest)
						*dest = _borderPen;
					break;

					// do nothing if source pixel is zero,
					// ie. transparent
				}

				// inc dest ptr to next pixel along row
				dest++;
			}
		} else {
			// pen is zero, so just copy character sprites
			// directly into text sprite without remapping colours
			memcpy(dest, source, charPtr->width);
			source += charPtr->width;
		}			

		// next row down (add width of text sprite)
		rowPtr += spriteWidth;
	}
}

// distance to keep speech text from edges of screen
#define TEXT_MARGIN 12

// creates a text bloc in the list and returns the bloc number the list of
// blocs are read and blitted at render time choose alignment type
// RDSPR_DISPLAYALIGN or 0

uint32 FontRenderer::buildNewBloc(uint8 *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification) {
	uint32	j = 0;
 	_frameHeader *frame_head;
	int16 text_left_margin;
	int16 text_right_margin;
	int16 text_top_margin;
	int16 text_bottom_margin;

	// find a free slot
	while (j < MAX_text_blocs && _blocList[j].text_mem)
		j++;

#ifdef _SWORD2_DEBUG
	// we've run out - might as well stop the system
	if (j == MAX_text_blocs)
		error("Build_new_block ran out of blocks!");
#endif

	// make the sprite!
	_blocList[j].text_mem = makeTextSprite(ascii, width, pen, fontRes);

	// speech to be centred above point (x,y), but kept on-screen
	// where (x,y) is a point somewhere just above the talker's head

  	// debug text just to be printed normally from point (x,y)

	// JUSTIFICATION & POSITIONING

	// 'NO_JUSTIFICATION' means print sprite with top-left at (x,y)
	// without margin checking - used for debug text

	if (justification != NO_JUSTIFICATION) {
		frame_head = (_frameHeader *) _blocList[j].text_mem->ad;

		switch (justification) {
		// this one is always used for SPEECH TEXT; possibly
		// also for pointer text
		case POSITION_AT_CENTRE_OF_BASE:
			x -= (frame_head->width) / 2;
			y -= frame_head->height;
			break;
		case POSITION_AT_CENTRE_OF_TOP:
			x -= (frame_head->width) / 2;
			break;
		case POSITION_AT_LEFT_OF_TOP:
			// the given coords are already correct for this!
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
			y -= (frame_head->height) / 2;
			break;
		case POSITION_AT_RIGHT_OF_CENTRE:
			x -= frame_head->width;
			y -= (frame_head->height) / 2;
			break;
		}

		// ensure text sprite is a few pixels inside the visible screen
		// remember - it's RDSPR_DISPLAYALIGN

		text_left_margin = TEXT_MARGIN;
		text_right_margin = 640 - TEXT_MARGIN - frame_head->width;
		text_top_margin = TEXT_MARGIN;
		text_bottom_margin = 400 - TEXT_MARGIN - frame_head->height;

		// move if too far left or too far right

		if (x < text_left_margin)
			x = text_left_margin;
		else if (x > text_right_margin)
			x = text_right_margin;
			
		// move if too high or too low

		if (y < text_top_margin)
			y = text_top_margin;
		else if (y > text_bottom_margin)
			y = text_bottom_margin;
	}

	_blocList[j].x = x;
	_blocList[j].y = y;

	// always uncompressed
  	_blocList[j].type = type | RDSPR_NOCOMPRESSION;

	return j + 1;
}

void FontRenderer::printTextBlocs(void) {
	//called by build_display

	_frameHeader *frame;
	_spriteInfo spriteInfo;
	uint32 rv;
	uint32 j;

	for (j = 0; j < MAX_text_blocs; j++) {
		if (_blocList[j].text_mem) {
			frame = (_frameHeader*) _blocList[j].text_mem->ad;

			spriteInfo.x = _blocList[j].x;
			spriteInfo.y = _blocList[j].y;
			spriteInfo.w = frame->width;
			spriteInfo.h = frame->height;
			spriteInfo.scale = 0;
			spriteInfo.scaledWidth = 0;
			spriteInfo.scaledHeight = 0;
			spriteInfo.type = _blocList[j].type;
			spriteInfo.blend = 0;
			spriteInfo.data = _blocList[j].text_mem->ad + sizeof(_frameHeader);
			spriteInfo.colourTable = 0;

			rv = g_graphics->drawSprite(&spriteInfo);
			if (rv)
				error("Driver Error %.8x in Print_text_blocs", rv);
		}
	}
}

void FontRenderer::killTextBloc(uint32 bloc_number) {
	//back to real
	bloc_number--;

	if (_blocList[bloc_number].text_mem) {
		// release the floating memory and mark it as free
		memory->freeMemory(_blocList[bloc_number].text_mem);
		_blocList[bloc_number].text_mem = 0;
	} else {
		// illegal kill - stop the system
		error("closing closed text bloc number %d", bloc_number);
	}
}

// The rest of this file doesn't belong in the FontRenderer class!

// called from InitialiseGame() in sword2.cpp

// resource 3258 contains text from location script for 152 (install, save &
// restore text, etc)

#define TEXT_RES	3258

// local line number of "save" (actor no. 1826)

#define SAVE_LINE_NO	1

void Sword2Engine::initialiseFontResourceFlags(void) {
	uint8 *textFile, *textLine;
	uint8 language;

	// open the text resource
	textFile = res_man->openResource(TEXT_RES);

	// If language is Polish or Finnish it requires alternate fonts.
	// Otherwise, use regular fonts

	// get the text line (& skip the 2 chars containing the wavId)

	textLine = fetchTextLine(textFile, SAVE_LINE_NO) + 2;

	// "talenna"	Finnish for "save"
	// "zapisz"	Polish for "save"

	if (strcmp((char *) textLine, "tallenna") == 0)
		language = FINNISH_TEXT;
	else if (strcmp((char *) textLine, "zapisz") == 0)
		language = POLISH_TEXT;
	else
		language = DEFAULT_TEXT;

	// Set the game to use the appropriate fonts
	initialiseFontResourceFlags(language);

	// Get the game name for the windows application

	// Get the text line - skip the 2 chars containing the wavId

	if (_features & GF_DEMO)
		textLine = fetchTextLine(textFile, 451) + 2;
	else
		textLine = fetchTextLine(textFile, 54) + 2;

	// According to the GetNameFunction(), which was never called and has
	// therefore been removed, the name of the game is:
	//
	// ENGLISH:  "Broken Sword II"
	// AMERICAN: "Circle of Blood II"
	// GERMAN:   "Baphomet's Fluch II"
	// default:  "Some game or other, part 86"

	g_graphics->setWindowName((char *) textLine);

	// now ok to close the text file
	res_man->closeResource(TEXT_RES);
}

// called from the above function, and also from console.cpp

void Sword2Engine::initialiseFontResourceFlags(uint8 language) {
	switch (language) {
	case FINNISH_TEXT:	// special Finnish fonts
		_speechFontId = FINNISH_SPEECH_FONT_ID;
		_controlsFontId = FINNISH_CONTROLS_FONT_ID;
		_redFontId = FINNISH_RED_FONT_ID;
		break;
	case POLISH_TEXT:	// special Polish fonts
		_speechFontId = POLISH_SPEECH_FONT_ID;
		_controlsFontId = POLISH_CONTROLS_FONT_ID;
		_redFontId = POLISH_RED_FONT_ID;
		break;
	default:		// DEFAULT_TEXT	- regular fonts
		_speechFontId = ENGLISH_SPEECH_FONT_ID;
		_controlsFontId = ENGLISH_CONTROLS_FONT_ID;
		_redFontId = ENGLISH_RED_FONT_ID;
		break;
	}
}

} // End of namespace Sword2
