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

/****************************************************************************
 * MAKETEXT.H - Function prototype for text sprite builder routine JEL Oct96
 *
 * The routine returns a memory handle to a movable memory block containing
 * the required sprite, which must be locked before use. ie. lock, draw
 * sprite, unlock/free.
 * 
 * The sprite data contains a frameHeader, but not a standard file header.
 *
 * Debugger will trap error when word too big for line (maxWidth) or when
 * more lines needed than max expected (MAX_LINES)
 *
 * PARAMETERS:
 *
 * 'sentence' points to a NULL-TERMINATED STRING
 *      - string must contain no leading/tailing/extra spaces
 *      - out-of-range characters in the string are forced to the output as
 *        a special error-signal character (chequered flag)
 *
 * 'maxWidth' is the maximum allowed text sprite width, in PIXELS
 *
 * 'pen' is the desired colour (0-255) for the main body of each character
 * 
 * NB. Border colour is #DEFINEd in textsprt.c (to a colour value for BLACK)
 * if 'pen' is zero, the characters are copied directly and NOT remapped.
 *
 * 'charSet' points to the beginning of the standard file header for the
 * desired character set
 *
 * NB. The first and last characters in the set are #DEFINEd in textsprt.c
 *
 * RETURNS:
 *
 * 'textSprite' points to the handle to be used for the text sprite
 *
 ****************************************************************************/

#ifndef _MAKETEXT_H
#define _MAKETEXT_H

#include "bs2/memory.h"
#include "bs2/debug.h"

// Output colour for character border - should be be black but note that we
// have to use a different pen number during sequences

#define BORDER_PEN 194

namespace Sword2 {

// allow enough for all the debug text blocks (see debug.cpp)
#define MAX_text_blocs MAX_DEBUG_TEXT_BLOCKS + 1

enum {
	// only for debug text, since it doesn't keep text inside the screen
	// margin!
	NO_JUSTIFICATION = 0,

	// these all force text inside the screen edge margin when necessary
	POSITION_AT_CENTRE_OF_BASE = 1,
	POSITION_AT_CENTRE_OF_TOP = 2,
	POSITION_AT_LEFT_OF_TOP = 3,
	POSITION_AT_RIGHT_OF_TOP = 4,
	POSITION_AT_LEFT_OF_BASE = 5,
	POSITION_AT_RIGHT_OF_BASE = 6,
	POSITION_AT_LEFT_OF_CENTRE = 7,
	POSITION_AT_RIGHT_OF_CENTRE = 8
};

enum {
	DEFAULT_TEXT = 0,
	FINNISH_TEXT = 1,
	POLISH_TEXT = 2
};

typedef	struct {
	int16 x;
	int16 y;
	// RDSPR_ status bits - see defintion of _spriteInfo structure for
	// correct size!
	uint16 type;
	mem *text_mem;
} TextBloc;

typedef struct {
	uint16 width;	// width of line in pixels
	uint16 length;	// length of line in characters
} LineInfo;

class FontRenderer {
private:
	TextBloc _blocList[MAX_text_blocs];

	// layout variables - these used to be defines, but now we're dealing
	// with 2 character sets

	int8 _lineSpacing;	// no. of pixels to separate lines of
				// characters in the output sprite - negative
				// for overlap
	int8 _charSpacing;	// no. of pixels to separate characters along
				// each line - negative for overlap
	uint8 _borderPen;	// output pen colour of character borders

	uint16 analyseSentence(uint8 *sentence, uint16 maxWidth, uint32 fontRes, LineInfo *line);
	mem* buildTextSprite(uint8 *sentence, uint32 fontRes, uint8 pen, LineInfo *line, uint16 noOfLines);
	uint16 charWidth(uint8 ch, uint32 fontRes);
	uint16 charHeight(uint32 fontRes);
	_frameHeader* findChar(uint8 ch, uint8 *charSet);
	void copyChar(_frameHeader *charPtr, uint8 *spritePtr, uint16 spriteWidth, uint8 pen);
	
public:
	FontRenderer() {
		for (int i = 0; i < MAX_text_blocs; i++)
			_blocList[i].text_mem = NULL;
	}

	mem* makeTextSprite(uint8 *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes, uint8 border = BORDER_PEN);

	void killTextBloc(uint32 bloc_number);
	void printTextBlocs(void);

	uint32 buildNewBloc(uint8 *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification);
};

extern FontRenderer fontRenderer;

} // End of namespace Sword2

#endif
