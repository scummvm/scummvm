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

#ifndef ICB_INCLUDED_TEXT_SPRITES_H
#define ICB_INCLUDED_TEXT_SPRITES_H

#include "engines/icb/p4_generic.h"
#include "engines/icb/common/px_bitmap.h"
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/debug.h"

namespace ICB {

// PC limit
#define TEXT_SPRITE_SIZE (300 * 150 * 4)

#define TS_NON_SPOKEN_LINE '&'
#define TS_SPOKEN_LINE '*'
#define TS_LINENO_OPEN '{'
#define TS_LINENO_CLOSE '}'

// This pointer can be set to force a different RGB to be used for speech text.  Remora uses
// this to do text colouring.
extern _rgb *psTempSpeechColour;

// return codes for text sprite functions
enum _TSrtn {
	TS_OK,              // it worked
	TS_ILLEGAL_SPACING, // line contains leading/trailing spaces or extra spaces between words
	TS_TOO_MANY_LINES,  // exceeded lineInfo array while analysing sentence
	TS_INVALID_FONT,    // the resource id passed is not a valid font resource
	TS_OUT_OF_MEM,      // couldn't allocate memory for text sprite
	TS_ILLEGAL_PIN,     // unrecognised value of '_pin_position'
	TS_ILLEGAL_MARGIN   // sprite cannot fit within given screen margin
};

// "Pin position" is the point on the sprite that we want to fix to the screen coordinates.
// eg. Speech text traditionally uses "PIN_AT_CENTRE_OF_BASE". That is, for calculating the
// required render coordinates, we will specify the coordinates of a point just above the
// talker's head, and then whatever the width and height of the text sprite, the render
// coordinates will be calculated such that the centre of the text sprite's base will be
// exactly at the specified coordinates - ie. text is centred just above the talkers head.
// The other pin positions may be used for pointer text, data readouts on PPC, or alternate
// positions for speech text, etc.
enum _pin_position {
	PIN_AT_CENTRE,
	PIN_AT_CENTRE_OF_TOP,
	PIN_AT_CENTRE_OF_BASE,
	PIN_AT_CENTRE_OF_LEFT,
	PIN_AT_CENTRE_OF_RIGHT,
	PIN_AT_TOP_LEFT,
	PIN_AT_TOP_RIGHT,
	PIN_AT_BOTTOM_LEFT,
	PIN_AT_BOTTOM_RIGHT
};

// This must now be a multiple of 4!
#define MAX_LINES 48 // max character lines in output sprite

typedef struct {       // info for each line of words in the output text sprite
	uint16 width;  // width of line in pixels
	uint16 length; // length of line in characters
} _textLine;

typedef struct { // info describing whole set of lines used in the sprite
	_textLine line[MAX_LINES];
	uint8 noOfLines;
	uint8 m_stopAtLine;
	uint8 m_bLeftFormatted;
	uint8 padding;
} _lineInfo;

// This is the structure that must be filled in before calling MakeTextSprite()
// which takes a reference to a structure of this type as it's only parameter
typedef struct {                  // info for each line of words in the output text sprite
	uint8 *textLine;          // null-terminated text sprite (must not contain any leading/tailing/extra spaces
	const char *fontResource; // name of the font
	uint32 fontResource_hash; // hash of the font file
	uint32 maxWidth;          // maximum allowed pixel width of text sprite
	uint32 bitDepth;          // 16,24 or 32 bits per pixel (2,3 or 4 bytes)
	int32 lineSpacing;        // no. of pixels to separate lines of characters in the output sprite   - negative for overlap
	int32 charSpacing;        // no. of pixels to separate characters along each line                                 - negative for overlap
	_rgb bodyColour;          // colour required for character bodies
	_rgb borderColour;        // colour required for character borders
	int32 errorChecking;      // perform error checking during analysesetence on/off
} _TSparams;

class text_sprite {
	// created by routines:
	uint8 sprite[TEXT_SPRITE_SIZE];
	uint32 spriteWidth;  // width of text sprite
	uint32 spriteHeight; // height of text sprite
	uint32 size;         // in bytes (= width * height * bit_depth)      NOT SURE WE NEED THIS FOR ANYTHING
	uint32 surfaceId; // The surface we want to eventually draw this sprite on
	_lineInfo lineInfo; // information about the lines in the text_block
	_TSparams params;

	// private functions:
	_TSrtn BuildTextSprite(int32 stopAtLine = -1, bool8 bRemoraLeftFormatting = FALSE8); // construct the sprite
	uint32 CharWidth(const uint8 ch, const char *fontRes, uint32 fontRes_hash);          // get width of a character
	void CopyChar(_pxSprite *charPtr, uint8 *spritePtr, uint8 *pal);    // copy character into sprite, based on params
	_TSrtn CheckFontResource(const char *fontRes, uint32 fontRes_hash); // check that it's a valid font resource
	_pxBitmap *LoadFont(const char *fontRes, uint32 fontRes_hash);

public:
	int32 renderX; // render coordinate
	int32 renderY;

	// the usual:
	text_sprite();  // constructor -- default constructor
	~text_sprite(); // destructor

	// Made this public, so Remora can work out how its formatting will be done.
	_TSrtn AnalyseSentence(void); // fill in the lineInfo structure

	// the main routine to create the sprite (added flag to stop AnalyseSentence() getting called if you need to)
	// Also, you can now tell it to stop short of displaying every line in the text.
	_TSrtn MakeTextSprite(bool8 analysisAlreadyDone = FALSE8, int32 stopAtLine = -1, bool8 bRemoraLeftFormatting = FALSE8);

	// the support routine to calculate suitable render coordinates
	_TSrtn GetRenderCoords(const int32 pinX,           // screen x-coord where we want to position the pin
	                       const int32 pinY,           // y-coord -"-
	                       const _pin_position pinPos, // position of pin on text sprite
	                       const int32 margin);        // margin to keep sprite within edge of screen, or -1 if allowed anywhere

	uint32 CharHeight(const char *fontRes, uint32 fontRes_hash); // get character height (all the same)
	_pxSprite *FindChar(uint8 ch, _pxBitmap *charSet);           // get pointer to header of required character data in font

	// the usual set of routines to return required values
	uint8 *GetSprite(void) {
		return sprite; // get pointer to text sprite in memory
	}
	uint32 GetWidth(void) {
		return spriteWidth; // get dimensions of text sprite
	}
	uint32 GetHeight(void) {
		return spriteHeight; // -"-
	}
	uint32 GetSize(void) {
		return size; // get byte-length of text sprite data
	}
	_lineInfo *GetLineInfo(void) {
		return &lineInfo; // information about the lines in the text_block
	}
	_TSparams *GetParams(void) {
		return &params; // information about the lines in the text_block
	}
	void SetSurface(uint32 sid) { surfaceId = sid; }
	uint32 GetSurface() const { return surfaceId; }

	// This a variable stuck way down here on it's little lonesome ?
	bool8 please_render; // draw yes/no
};

} // End of namespace ICB

#endif // INCLUDED_TEXT_SPRITES_H
