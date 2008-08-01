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
 *
 * Text utility defines
 */

#ifndef TINSEL_TEXT_H     // prevent multiple includes
#define TINSEL_TEXT_H

#include "tinsel/object.h"	// object manager defines

namespace Tinsel {

/** text mode flags - defaults to left justify */
enum {
	TXT_CENTRE		= 0x0001,	//!< centre justify text
	TXT_RIGHT		= 0x0002,	//!< right justify text
	TXT_SHADOW		= 0x0004,	//!< shadow each character
	TXT_ABSOLUTE	= 0x0008	//!< position of text is absolute (only for object text)
};

/** maximum number of characters in a font */
#define	MAX_FONT_CHARS	256


#include "common/pack-start.h"	// START STRUCT PACKING

/**
 * Text font data structure.
 * @note only the pointer is used so the size of fontDef[] is not important.
 * It is currently set at 300 because it suited me for debugging.
 */
struct FONT {
	int xSpacing;			//!< x spacing between characters
	int ySpacing;			//!< y spacing between characters
	int xShadow;			//!< x shadow offset
	int yShadow;			//!< y shadow offset
	int spaceSize;			//!< x spacing to use for a space character
	OBJ_INIT fontInit;		//!< structure used to init text objects
	SCNHANDLE fontDef[300];	//!< image handle array for all characters in the font
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING


/** structure for passing the correct parameters to ObjectTextOut */
struct TEXTOUT {
	OBJECT *pList;		//!< object list to add text to
	char *szStr;		//!< string to output
	int colour;			//!< colour for monochrome text
	int xPos;			//!< x position of string
	int yPos;			//!< y position of string
	SCNHANDLE hFont;	//!< which font to use
	int mode;			//!< mode flags for the string
	int sleepTime;		//!< sleep time between each character (if non-zero)
};


/*----------------------------------------------------------------------*\
|*			Text Function Prototypes			*|
\*----------------------------------------------------------------------*/

OBJECT *ObjectTextOut(		// output a string of text
	OBJECT *pList,		// object list to add text to
	char *szStr,		// string to output
	int colour,		// colour for monochrome text
	int xPos,		// x position of string
	int yPos,		// y position of string
	SCNHANDLE hFont,	// which font to use
	int mode);		// mode flags for the string

OBJECT *ObjectTextOutIndirect(	// output a string of text
	TEXTOUT *pText);	// pointer to TextOut struct with all parameters

bool IsCharImage(		// Is there an image for this character in this font?
	SCNHANDLE hFont,	// which font to use
	char c);		// character to test

} // end of namespace Tinsel

#endif	// TINSEL_TEXT_H
