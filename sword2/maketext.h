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

#include "memory.h"

namespace Sword2 {

// only for debug text, since it doesn't keep text inside the screen margin!
#define	NO_JUSTIFICATION		0
// these all force text inside the screen edge margin when necessary
#define POSITION_AT_CENTRE_OF_BASE	1
#define POSITION_AT_CENTRE_OF_TOP	2
#define POSITION_AT_LEFT_OF_TOP		3
#define POSITION_AT_RIGHT_OF_TOP	4
#define POSITION_AT_LEFT_OF_BASE	5
#define POSITION_AT_RIGHT_OF_BASE	6
#define POSITION_AT_LEFT_OF_CENTRE	7
#define POSITION_AT_RIGHT_OF_CENTRE	8

mem* MakeTextSprite(uint8 *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes);
void Init_text_bloc_system(void);

void Kill_text_bloc(uint32 bloc_number);
void Print_text_blocs(void);	// Tony16Oct96

uint32 Build_new_block(uint8 *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification);

#define	DEFAULT_TEXT	0
#define FINNISH_TEXT	1
#define POLISH_TEXT	2

// this one works out the language from the text cluster (James31july97)
void InitialiseFontResourceFlags(void);
// this one allow you to select the fonts yourself (James31july97)
void InitialiseFontResourceFlags(uint8 language);

extern uint32 speech_font_id;
extern uint32 controls_font_id;
extern uint32 red_font_id;

} // End of namespace Sword2

#endif
