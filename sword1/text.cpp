/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "text.h"
#include "resman.h"
#include "objectman.h"
#include "common/util.h"
#include "swordres.h"
#include "sworddefs.h"

#define OVERLAP 3
#define SPACE ' '
#define BORDER_COL		200
#define LETTER_COL		193
#define NO_COL			0		// sprite background - 0 for transparency
#define MAX_LINES		30


SwordText::SwordText(ObjectMan *pObjMan, ResMan *pResMan, bool czechVersion) {
	_objMan = pObjMan;
	_resMan = pResMan;
	_textCount = 0;
	if (czechVersion)
		_font = (uint8*)_resMan->openFetchRes(CZECH_GAME_FONT);
	else
		_font = (uint8*)_resMan->openFetchRes(GAME_FONT);
	_joinWidth = charWidth( SPACE ) - 2 * OVERLAP;
	_charHeight = FROM_LE_16(_resMan->fetchFrame(_font, 0)->height); // all chars have the same height
	_textBlocks[0] = _textBlocks[1] = NULL;
}

SwordText::~SwordText(void) {
	if (_textBlocks[0])
		free(_textBlocks[0]);
	if (_textBlocks[1])
		free(_textBlocks[1]);
}

uint32 SwordText::lowTextManager(uint8 *ascii, int32 width, uint8 pen) {
	_textCount++;
	if (_textCount > MAX_TEXT_OBS)
		error("SwordText::lowTextManager: MAX_TEXT_OBS exceeded!");
	uint32 textObjId = (TEXT_sect * ITM_PER_SEC) - 1;
	do {
		textObjId++;
	} while(_objMan->fetchObject(textObjId)->o_status);
	// okay, found a free text object

	_objMan->fetchObject(textObjId)->o_status = STAT_FORE;
	makeTextSprite((uint8)textObjId, ascii, (uint16)width, pen);

	return textObjId;
}

void SwordText::makeTextSprite(uint8 slot, uint8 *text, uint16 maxWidth, uint8 pen) {
	LineInfo lines[MAX_LINES];
	uint16 numLines = analyzeSentence(text, maxWidth, lines);
	
	uint16 sprWidth = 0;
	uint16 lineCnt;
	for (lineCnt = 0; lineCnt < numLines; lineCnt++)
		if (lines[lineCnt].width > sprWidth)
			sprWidth = lines[lineCnt].width;
	uint16 sprHeight = _charHeight * numLines;
	uint32 sprSize = sprWidth * sprHeight;
	assert(!_textBlocks[slot]); // if this triggers, the speechDriver failed to call SwordText::releaseText.
	_textBlocks[slot] = (FrameHeader*)malloc(sprSize + sizeof(FrameHeader));

	memcpy( _textBlocks[slot]->runTimeComp, "Nu  ", 4);
	_textBlocks[slot]->compSize	= 0;
	_textBlocks[slot]->width	= TO_LE_16(sprWidth);
	_textBlocks[slot]->height	= TO_LE_16(sprHeight);
	_textBlocks[slot]->offsetX	= 0;
	_textBlocks[slot]->offsetY	= 0;

	uint8 *linePtr = ((uint8*)_textBlocks[slot]) + sizeof(FrameHeader);
	memset(linePtr, NO_COL, sprSize);
	for (lineCnt = 0; lineCnt < numLines; lineCnt++) {
		uint8 *sprPtr = linePtr + (sprWidth - lines[lineCnt].width) / 2; // center the text
		for (uint16 pos = 0; pos < lines[lineCnt].length; pos++)
			sprPtr += copyChar(*text++, sprPtr, sprWidth, pen) - OVERLAP;
		text++; // skip space at the end of the line
		linePtr += _charHeight * sprWidth;
	}
}

uint16 SwordText::charWidth(uint8 ch) {
	if (ch < SPACE)
		ch = 64;
	return FROM_LE_16(_resMan->fetchFrame(_font, ch - SPACE)->width);
}

uint16 SwordText::analyzeSentence(uint8 *text, uint16 maxWidth, LineInfo *line) {
	uint16 lineNo = 0;

	bool firstWord = true;
	while (*text) {
		uint16 wordWidth = 0;
		uint16 wordLength = 0;

		while ((*text != SPACE) && *text) {
			wordWidth += charWidth(*text) - OVERLAP;
			wordLength++;
			text++;
		}
		if (*text == SPACE)
			text++;

		wordWidth += OVERLAP; // no overlap on final letter of word!
		if( firstWord )	{ // first word on first line, so no separating SPACE needed
			line[0].width = wordWidth;
			line[0].length = wordLength;
			firstWord = false;
		} else {
			// see how much extra space this word will need to fit on current line
			// (with a separating space character - also overlapped)
			uint16 spaceNeeded = _joinWidth + wordWidth;

			if (line[lineNo].width + spaceNeeded <= maxWidth ) {
				line[lineNo].width += spaceNeeded;
				line[lineNo].length += 1 + wordLength; // NB. space+word characters
			} else {	// put word (without separating SPACE) at start of next line
				lineNo++;
				assert( lineNo < MAX_LINES );
				line[lineNo].width = wordWidth;
				line[lineNo].length = wordLength;
			}
		}
	}
	return lineNo+1;	// return no of lines
}

uint16 SwordText::copyChar(uint8 ch, uint8 *sprPtr, uint16 sprWidth, uint8 pen) {
	FrameHeader *chFrame = _resMan->fetchFrame(_font, ch - SPACE);
	uint8 *chData = ((uint8*)chFrame) + sizeof(FrameHeader);
	uint8 *dest = sprPtr;
	for (uint16 cnty = 0; cnty < FROM_LE_16(chFrame->height); cnty++) {
		for (uint16 cntx = 0; cntx < FROM_LE_16(chFrame->width); cntx++) {
			if (*chData == LETTER_COL)
				dest[cntx] = pen;
			else if ((*chData == BORDER_COL) && (!dest[cntx])) // don't do a border if there's already a color underneath (chars can overlap)
				dest[cntx] = BORDER_COL;
			chData++;
		}
		dest += sprWidth;
	}
	return FROM_LE_16(chFrame->width);
}

FrameHeader *SwordText::giveSpriteData(uint32 textTarget) {
	// textTarget is the resource ID of the Compact linking the textdata.
	// that's 0x950000 for slot 0 and 0x950001 for slot 1. easy, huh? :)
	textTarget &= ITM_ID;
	assert(textTarget <= 1);

	return _textBlocks[textTarget];
}

void SwordText::releaseText(uint32 id) {
	id &= ITM_ID;
	assert(id <= 1);
	if (_textBlocks[id]) {
		free(_textBlocks[id]);
		_textBlocks[id] = NULL;
		_textCount--;
	}
}
