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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//	MAKETEXT -	Constructs a single-frame text sprite: returns a handle to a
//				FLOATING memory block containing the sprite, given a
//				null-terminated string, max width allowed, pen colour and
//				pointer to required character set.
//
//				NB 1) The routine does not create a standard file header or
//				an anim header for the text sprite - the data simply begins
//				with the frame header.
//
//				NB 2) If pen colour is zero, it copies the characters into the
//				sprite without remapping the colours.
//				ie. It can handle both the standard 2-colour font for speech
//				and any multicoloured fonts for control panels, etc.
//
//	Based on textsprt.c as used for Broken Sword 1, but updated for new system
//	by JEL on 9oct96 and updated again (for font as a resource) on 5dec96.

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define	MAX_LINES		30		// max character lines in output sprite

#define BORDER_COL		200		// source colour for character border (only needed for remapping colours)
#define LETTER_COL		193		// source colour for bulk of character ( " )
#define BORDER_PEN		194		// output colour for character border - should be black ( " ) but note that we have to use a different pen number during sequences

#define NO_COL			0		// sprite background - 0 for transparency!
#define	SPACE			' '
#define	FIRST_CHAR		SPACE	// first character in character set
#define	LAST_CHAR		255		// last character in character set
#define	DUD				64		// the first "chequered flag" (dud) symbol in our character set is in the '@' position
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "driver/driver96.h"
#include "console.h"
#include "debug.h"
#include "defs.h"	// for SPEECH_FONT_ID & CONSOLE_FONT_ID
#include "header.h"
#include "maketext.h"
#include "memory.h"
#include "protocol.h"			// for FetchFrameHeader()
#include "resman.h"

extern uint32 sequenceTextLines;	// see anims.cpp

//-----------------------------------------------------------------------------
typedef struct	// info for each line of words in the output text sprite
{
	uint16	width;	// width of line in pixels
	uint16	length;	// length of line in characters
} _lineInfo;
//-----------------------------------------------------------------------------
// PROTOTYPES
uint16			AnalyseSentence( uint8 *sentence, uint16 maxWidth, uint32 fontRes, _lineInfo *line );
mem*			BuildTextSprite( uint8 *sentence, uint32 fontRes, uint8 pen, _lineInfo *line, uint16 noOfLines );
uint16			CharWidth( uint8 ch, uint32 fontRes );
uint16			CharHeight( uint32 fontRes );
_frameHeader*	FindChar( uint8 ch, uint8 *charSet );
void			CopyChar( _frameHeader *charPtr, uint8 *spritePtr, uint16 spriteWidth, uint8 pen );
//-----------------------------------------------------------------------------
// global layout variables - these used to be defines, but now we're dealing with 2 character sets (10dec96 JEL)

int8	line_spacing;	// no. of pixels to separate lines of characters in the output sprite	- negative for overlap
int8	char_spacing;	// no. of pixels to separate characters along each line					- negative for overlap
uint8	border_pen;		// output pen colour of character borders

//-----------------------------------------------------------------------------
// Global font resource id variables, set up in 'SetUpFontResources()' at bottom of this file

uint32 speech_font_id;
uint32 controls_font_id;
uint32 red_font_id;
uint32 death_font_id;

//-----------------------------------------------------------------------------
mem* MakeTextSprite( uint8 *sentence, uint16 maxWidth, uint8 pen, uint32 fontRes )
{
	mem		*line;			// handle for the memory block which will contain the array of lineInfo structures
	mem		*textSprite;	// handle for the block to contain the text sprite itself
	uint16	noOfLines;		// no of lines of text required to fit within a sprite of width 'maxWidth' pixels

//	Zdebug("MakeTextSprite( \"%s\", maxWidth=%u )", sentence, maxWidth );

	/////////////////////////////////////////////////////////////////////////////
	// NB. ensure sentence contains no leading/tailing/extra spaces
	// - if necessary, copy to another array first, missing the extra spaces.
	/////////////////////////////////////////////////////////////////////////////

	//----------------------------------------------
	// set the global layout variables (10dec96 JEL)

	if (fontRes == speech_font_id)
	{
		line_spacing	= -6;	// overlap lines by 6 pixels
		char_spacing	= -3;	// overlap characters by 3 pixels
	}
	else if (fontRes == CONSOLE_FONT_ID)
	{
		line_spacing	= 0;	// no space or overlap between lines
		char_spacing	= 1;	// 1 pixel spacing between each character
	}
	else
	{
		line_spacing	= 0;
		char_spacing	= 0;
	}

	if (sequenceTextLines)	// if rendering text over a sequence
		border_pen = 1;		// need a different colour number to BORDER_PEN
	else
		border_pen = BORDER_PEN;

	//----------------------------------------------

	// allocate memory for array of lineInfo structures
	line = Twalloc( MAX_LINES*sizeof(_lineInfo), MEM_locked, UID_temp );	// last param is an optional id for type of mem block

	// get details of sentence breakdown into array of _lineInfo structures
	// and get the no of lines involved
	noOfLines = AnalyseSentence( sentence, maxWidth, fontRes, (_lineInfo *)line->ad );

	// construct the sprite based on the info gathered - returns floating mem block
	textSprite = BuildTextSprite( sentence, fontRes, pen, (_lineInfo *)line->ad, noOfLines );

	// free up the lineInfo array now
	Free_mem( line );

	return( textSprite );
}
//-----------------------------------------------------------------------------
uint16 AnalyseSentence( uint8 *sentence, uint16 maxWidth, uint32 fontRes, _lineInfo *line )
{
	uint16 pos=0, wordWidth, wordLength, spaceNeeded, firstWord=TRUE, lineNo=0;
	uint8 ch;
	// joinWidth = how much extra space is needed to append a word to a line
	// NB. SPACE requires TWICE the 'char_spacing' to join a word to line
	uint16 joinWidth = CharWidth( SPACE, fontRes ) + 2*char_spacing;
	

	do
	{
		wordWidth = 0;					// new word
		wordLength = 0;

		ch = sentence[pos++];			// get first char of word (at position 'pos')

		while( (ch != SPACE) && ch )	// while not SPACE or NULL terminator
		{
			// inc wordWidth by (character width + char_spacing) pixels
			wordWidth += CharWidth( ch, fontRes ) + char_spacing;
			wordLength++;
			ch = sentence[pos++];		// get next char
		}

		wordWidth -= char_spacing;		// no char_spacing after final letter of word!

		// 'ch' is now the SPACE or NULL following the word
		// 'pos' indexes to the position following 'ch'


		if( firstWord )		// first word on first line, so no separating SPACE needed
		{
			line[0].width = wordWidth;
			line[0].length = wordLength;
			firstWord = FALSE;
		}
		else
		{
			// see how much extra space this word will need to fit on current line
			// (with a separating space character - also overlapped)
			spaceNeeded = joinWidth + wordWidth;

			if( (line[lineNo].width + spaceNeeded) <= maxWidth )	// fits this line
			{
				line[lineNo].width += spaceNeeded;
				line[lineNo].length += 1+wordLength;	// NB. space+word characters
			}
			else	// put word (without separating SPACE) at start of next line
			{
				lineNo++;							// for next _lineInfo structure in the array
				//debug_only( lineNo < MAX_LINES );	// exception if lineNo >= MAX_LINES
				line[lineNo].width = wordWidth;
				line[lineNo].length = wordLength;
			}
		}
	}
	while( ch );		// while not reached the NULL terminator

	return lineNo+1;	// return no of lines
}

//-----------------------------------------------------------------------------
// Returns a handle to a floating memory block containing a text sprite, given
// a pointer to a null-terminated string, pointer to required character set,
// required text pen colour (or zero to use source colours), pointer to the
// array of linInfo structures created by 'AnalyseSentence()', and the number
// of lines (ie. no. of elements in the 'line' array).

//
//
//	PC Version of BuildTextSprite
//
//

mem* BuildTextSprite( uint8 *sentence, uint32 fontRes, uint8 pen, _lineInfo *line, uint16 noOfLines )
{
	uint8 *linePtr, *spritePtr;
	uint16 lineNo, pos=0, posInLine, spriteWidth=0, spriteHeight, sizeOfSprite;
	uint16 charHeight = CharHeight(fontRes);
	_frameHeader *frameHeadPtr, *charPtr;
	mem *textSprite;
	uint8 *charSet;

	// spriteWidth = width of widest line of output text
	for( lineNo=0; lineNo < noOfLines; lineNo++)
		if( line[lineNo].width > spriteWidth )
			spriteWidth = line[lineNo].width;

	// spriteHeight = tot height of char lines + tot height of separating lines
	spriteHeight = (charHeight*noOfLines + line_spacing*(noOfLines-1));

	// total size (no of pixels)
	sizeOfSprite = spriteWidth * spriteHeight;

	// allocate memory for sprite, and lock it ready for use
	// NB. 'textSprite' is the given pointer to the handle to be used
	textSprite = Twalloc( sizeof(_frameHeader) + sizeOfSprite, MEM_locked, UID_text_sprite );
	// the handle (*textSprite) now points to UNMOVABLE memory block

	// set up the frame header
	frameHeadPtr = (_frameHeader *)textSprite->ad;	// point to the start of our memory block

	frameHeadPtr->compSize	= 0;
	frameHeadPtr->width		= spriteWidth;
	frameHeadPtr->height	= spriteHeight;
	
//	Zdebug("spriteWidth=%u",spriteWidth);
//	Zdebug("spriteHeight=%u",spriteHeight);

	// ok, now point to the start (of the first line) of the sprite data itelf
	linePtr = textSprite->ad + sizeof(_frameHeader);

	// start with transparent sprite (no colour)
	memset( linePtr, NO_COL, sizeOfSprite );


	charSet = res_man.Res_open(fontRes);			// open font file


	// fill sprite with characters, one line at a time
	for( lineNo=0; lineNo < noOfLines; lineNo++ )
	{
		// position the start of the line so that it is centred across the sprite
		spritePtr = linePtr + (spriteWidth - line[lineNo].width) / 2;

		// copy the sprite for each character in this line to the text sprite
		// and inc the sprite ptr by the character's width minus the 'overlap'
		for( posInLine=0; posInLine < line[lineNo].length; posInLine++ )
		{
			charPtr = FindChar( sentence[pos++], charSet );

			#ifdef _DEBUG			
			if ((charPtr->height) != charHeight)
				Con_fatal_error("FONT ERROR: '%c' is not same height as the space (%s line %u)",sentence[pos-1],__FILE__,__LINE__);
			#endif

			CopyChar( charPtr, spritePtr, spriteWidth, pen );
			spritePtr += charPtr->width + char_spacing;
		}

		pos++;	// skip space at end of last word in this line

		// move to start of next character line in text sprite
		linePtr += (charHeight + line_spacing) * spriteWidth;
	}


	res_man.Res_close(fontRes);						// close font file


	// unlock the sprite memory block, so it's movable
	Float_mem( textSprite );

	return( textSprite );
}

//-----------------------------------------------------------------------------
// Returns the width of a character sprite, given the character's ASCII code
// and a pointer to the start of the character set.

uint16 CharWidth( uint8 ch, uint32 fontRes )
{
	_frameHeader *charFrame;
	uint8 *charSet;
	uint16 width;


	charSet = res_man.Res_open(fontRes);	// open font file

	charFrame = FindChar( ch, charSet );	// move to approp. sprite (header)

	width = charFrame->width;

 	res_man.Res_close(fontRes);				// close font file

	return (width);							// return its width
}
//-----------------------------------------------------------------------------
// Returns the height of a character sprite, given the character's ASCII code
// and a pointer to the start of the character set.

uint16 CharHeight( uint32 fontRes )		// assume all chars the same height!
{
	_frameHeader *charFrame;
	uint8 *charSet;
	uint16 height;


	charSet = res_man.Res_open(fontRes);			// open font file

	charFrame = FindChar( FIRST_CHAR, charSet );	// FIRST_CHAR as good as any

	height = charFrame->height;

	res_man.Res_close(fontRes);						// close font file

	return (height);								// return its height
}
//-----------------------------------------------------------------------------
// Returns a pointer to the header of a character sprite, given the character's
// ASCII code and a pointer to the start of the character set.

_frameHeader* FindChar( uint8 ch, uint8 *charSet )
{
	// charSet details:
	// ---------------
	// starts with the standard file header			ie. sizeof(_header) bytes
	// then an int32 giving the no of sprites		ie. 4 bytes
	// then the offset table (an int32 offset for each sprite)
	//  - each offset counting from the start of the file

	if( (ch<FIRST_CHAR) )		// if 'ch' out of range
//	if( (ch<FIRST_CHAR) || (ch>LAST_CHAR) )		// if 'ch' out of range
		ch = DUD;								// then print the 'dud' character (chequered flag)

	// address of char = address of charSet + offset to char
	//return (charSet + *(int32 *)(charSet + sizeof(_header) + 4 + 4*(ch - FIRST_CHAR)));
	return (FetchFrameHeader( charSet, ch-FIRST_CHAR ));
}
//-----------------------------------------------------------------------------
// Copies a character sprite from 'charPtr' to the sprite buffer at 'spritePtr'
// of width 'spriteWidth'. If pen is zero, it copies the data across directly,
// otherwise it maps pixels of BORDER_COL to 'border_pen', and LETTER_COL to 'pen'.

void CopyChar( _frameHeader *charPtr, uint8 *spritePtr, uint16 spriteWidth, uint8 pen )
{
	uint8 *rowPtr, *source, *dest;
	uint16 rows, cols;
	

	source = (uint8 *)charPtr + sizeof(_frameHeader);	// now pts to sprite data for char 'ch'
	rowPtr = spritePtr;					// pts to start of first row of char within text sprite

	for( rows=0; rows < charPtr->height; rows++ )
	{
		dest = rowPtr;				// start at beginning of row


		if (pen)	// if required output pen is non-zero
		{
			for( cols=0; cols < charPtr->width; cols++ )
			{
				switch( *source++ )		// inc source ptr along sprite data
				{
					case LETTER_COL:
						*dest = pen;
					break;

					case BORDER_COL:
						if (!(*dest))	// don't do a border pixel if there already a bit of another character underneath (for overlapping!)
							*dest = border_pen;
					break;

					// do nothing if source pixel is zero - ie. transparent
				}
				dest++;			// inc dest ptr to next pixel along row
			}
		}

		else	// pen is zero, so just copy character sprites directly into text sprite without remapping colours
		{
			memcpy( dest, source, charPtr->width );
			source += charPtr->width;
		}			

		rowPtr += spriteWidth;	// next row down (add width of text sprite)
	}
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define	MAX_text_blocs	MAX_DEBUG_TEXT_BLOCKS+1	// allow enough for all the debug text blocks (see debug.cpp)
#else
#define	MAX_text_blocs	2	// only need one for speech, and possibly one for "PAUSED"
#endif	// _DEBUG

typedef	struct
{
	int16	x;
	int16	y;
	uint16	type;	// RDSPR_ status bits - see defintion of _spriteInfo structure for correct size!
	mem	*text_mem;
}	text_bloc;

text_bloc	text_sprite_list[MAX_text_blocs];
//-----------------------------------------------------------------------------
void	Init_text_bloc_system(void)	//Tony16Oct96
{
	uint32	j;

	for	(j=0;j<MAX_text_blocs;j++)
		text_sprite_list[j].text_mem=0;
}
//-----------------------------------------------------------------------------
#define TEXT_MARGIN	12		// distance to keep speech text from edges of screen

uint32 Build_new_block(uint8 *ascii, int16 x, int16 y, uint16 width, uint8 pen, uint32 type, uint32 fontRes, uint8 justification)	//Tony31Oct96
{
//creates a text bloc in the list and returns the bloc number
//the list of blocs are read and blitted at render time
//choose alignment type RDSPR_DISPLAYALIGN or 0

	uint32	j=0;
 	_frameHeader *frame_head;
	int16 text_left_margin;
	int16 text_right_margin;
	int16 text_top_margin;
	int16 text_bottom_margin;


//find a free slot
	while((j<MAX_text_blocs)&&(text_sprite_list[j].text_mem))
		j++;

#ifdef _DEBUG
	if	(j==MAX_text_blocs)	//we've run out
		Con_fatal_error("Build_new_block ran out of blocks! (%s line %u)",__FILE__,__LINE__);	//might as well stop the system
#endif


	text_sprite_list[j].text_mem = MakeTextSprite( ascii, width, pen, fontRes );	// make the sprite!


	// speech to be centred above point (x,y), but kept on-screen
	// where (x,y) is a point somewhere just above the talker's head

  	// debug text just to be printed normally from point (x,y)

	//-----------------------------------------------------------
	// JUSTIFICATION & POSITIONING (James updated 20jun97)

	if (justification != NO_JUSTIFICATION)	// 'NO_JUSTIFICATION' means print sprite with top-left at (x,y) without margin checking - used for debug text
	{								
		frame_head = (_frameHeader*) text_sprite_list[j].text_mem->ad;

		switch (justification)
		{
			// this one is always used for SPEECH TEXT; possibly also for pointer text
			case POSITION_AT_CENTRE_OF_BASE:
				x -= (frame_head->width)/2;	// subtract half the sprite-width from the given x-coord
				y -= frame_head->height;	// and the sprite-height from the given y-coord
				break;

 			case POSITION_AT_CENTRE_OF_TOP:
				x -= (frame_head->width)/2;
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
				y -= (frame_head->height)/2;
   				break;

			case POSITION_AT_RIGHT_OF_CENTRE:
				x -= frame_head->width;
				y -= (frame_head->height)/2;
 				break;
		}

		// ensure text sprite is a few pixels inside the visible screen
		text_left_margin	= TEXT_MARGIN;
		text_right_margin	= 640 - TEXT_MARGIN - frame_head->width;
		text_top_margin		= 0 + TEXT_MARGIN;		// remember - it's RDSPR_DISPLAYALIGN
		text_bottom_margin	= 400 - TEXT_MARGIN - frame_head->height;

		if (x < text_left_margin)	// move if too far left or too far right
			x = text_left_margin;
		else if (x > text_right_margin)
			x = text_right_margin;
			
		if (y < text_top_margin)	// move if too high or too low
			y = text_top_margin;
		else if (y > text_bottom_margin)
			y = text_bottom_margin;
	}
	//-----------------------------------------------------------

	text_sprite_list[j].x = x;
	text_sprite_list[j].y = y;
  	text_sprite_list[j].type = type+RDSPR_NOCOMPRESSION;	// always uncompressed


	return(j+1);
}
//-----------------------------------------------------------------------------

//
//
//	PC Version of Print_text_blocs
//
//

void	Print_text_blocs(void)	//Tony16Oct96
{
//called by build_display

	_frameHeader *frame;
	_spriteInfo		spriteInfo;
	uint32	j;
	uint32	rv;

	for	(j=0;j<MAX_text_blocs;j++)
	{
		if	(text_sprite_list[j].text_mem)
		{
			frame = (_frameHeader*) text_sprite_list[j].text_mem->ad;

			spriteInfo.x			= text_sprite_list[j].x;
			spriteInfo.y			= text_sprite_list[j].y;
			spriteInfo.w			= frame->width;
			spriteInfo.h			= frame->height;
			spriteInfo.scale		= 0;
			spriteInfo.scaledWidth	= 0;
			spriteInfo.scaledHeight	= 0;
			spriteInfo.type			= text_sprite_list[j].type;
			spriteInfo.blend		= 0;
			spriteInfo.data			= text_sprite_list[j].text_mem->ad+sizeof(_frameHeader);
			spriteInfo.colourTable	= 0;

			rv = DrawSprite( &spriteInfo );
			if (rv)
				ExitWithReport("Driver Error %.8x in Print_text_blocs [%s line %u]", rv, __FILE__, __LINE__);
		}
	}
}

//-----------------------------------------------------------------------------
void	Kill_text_bloc(uint32	bloc_number)	//Tony18Oct96
{
	bloc_number--;	//back to real

	if	(text_sprite_list[bloc_number].text_mem)
	{
		Free_mem(text_sprite_list[bloc_number].text_mem);	//release the floating memory
		text_sprite_list[bloc_number].text_mem=0;	//this is how we know the bloc is free
	}
	else
		Con_fatal_error("closing closed text bloc number %d", bloc_number);	//illegal kill - stop the system

}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// called from InitialiseGame() in sword2.cpp
void InitialiseFontResourceFlags(void)	// (James31july97)
{
	uint8 *textFile, *textLine;
	uint8 language;

	#define TEXT_RES		3258	// resource 3258 contains text from location script for 152 (install, save & restore text, etc)
	#define SAVE_LINE_NO	1		// local line number of "save" (actor no. 1826)

#ifndef _DEMO	// normal game
	#define NAME_LINE_NO	54		// local line number of game name (actor no. 3550)
#else
	#define NAME_LINE_NO	451		// local line number of demo game name
#endif	// _DEMO

	//---------------------------------------------------------------------------------
	textFile = res_man.Res_open(TEXT_RES);					// open the text resource
	//---------------------------------------------------------------------------------
	// check if language is Polish or Finnish, and therefore requires alternate fonts

	textLine = FetchTextLine(textFile, SAVE_LINE_NO )+2;	// get the text line (& skip the 2 chars containing the wavId)

	if (strcmp((char*)textLine,"tallenna")==0)		// if this line contains the Finnish for "save"
		language = FINNISH_TEXT;					// - then this version must be Finnish
	else if (strcmp((char*)textLine,"zapisz")==0)	// if this line contains the Polish for "save"
		language = POLISH_TEXT;						// - then this version must be Polish
	else											// neither Finnish nor Polish
		language = DEFAULT_TEXT;					// - use regular fonts

	InitialiseFontResourceFlags(language);			// Set the game to use the appropriate fonts

	//---------------------------------------------------------------------------------
	// Get the game name for the windows application

	textLine = FetchTextLine(textFile, NAME_LINE_NO )+2;	// get the text line (& skip the 2 chars containing the wavId)
	SetWindowName((char*)textLine);							// driver function
	//---------------------------------------------------------------------------------
	res_man.Res_close(TEXT_RES);					// now ok to close the text file
	//---------------------------------------------------------------------------------
}
//------------------------------------------------------------------------------------
// called from the above function, and also from console.cpp
void InitialiseFontResourceFlags(uint8 language)	// (James31july97)
{
	switch (language)
	{
		case FINNISH_TEXT:		// special Finnish fonts
		{
			speech_font_id		= FINNISH_SPEECH_FONT_ID;
			controls_font_id	= FINNISH_CONTROLS_FONT_ID;
			red_font_id			= FINNISH_RED_FONT_ID;
			break;
		}

		case POLISH_TEXT:		// special Polish fonts
		{
			speech_font_id		= POLISH_SPEECH_FONT_ID;
			controls_font_id	= POLISH_CONTROLS_FONT_ID;
			red_font_id			= POLISH_RED_FONT_ID;
			break;
		}

		default:// DEFAULT_TEXT	// regular fonts
		{
			speech_font_id		= ENGLISH_SPEECH_FONT_ID;
			controls_font_id	= ENGLISH_CONTROLS_FONT_ID;
			red_font_id			= ENGLISH_RED_FONT_ID;
			break;
		}
	}
}
//------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------






