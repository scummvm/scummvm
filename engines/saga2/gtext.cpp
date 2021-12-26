/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/gdraw.h"

namespace Saga2 {

#define textStyleBar    (textStyleUnderBar|textStyleHiLiteBar)

#define TempAlloc       malloc
#define TempFree        free

/* ============================================================================ *
                            Text Blitting Routines
 * ============================================================================ */

/*  Notes:

    These functions attempt to render planar fonts onto a chunky bitmap.

    All non-pointer variables are 16 bits or less. (I don't know how
    big pointers need to be).
*/

/*  DrawChar: This function renders a single bitmapped character
    into an offscreen buffer.

    'drawchar' is the ascii code of the character to be drawn.

    'xpos' is the position in the buffer to draw the outline.

    'baseline' is the address of the first scanline in the buffer.
    (it can actually be any scanline in the buffer if we want to
    draw the character at a different y position).
    Note that this has nothing to do with the Amiga "BASELINE" field,
    this routine always renders relative to the top of the font.

    'color' is the pixel value to render into the buffer.

    Note that this code does unaligned int16 writes to byte addresses,
    and will not run on an 68000 or 68010 processor.
*/

void DrawChar(gFont *font, int drawchar, int xpos, uint8 *baseline, uint8 color,
              uint16 destwidth) {
	short   w,
	        font_mod;               // width of font in bytes

	uint8   *src,                   // start of char data
	        *dst;                   // start of dest row

	/*
	    This code assumes that the font characters are all byte-aligned,
	    and that there is no extra junk after the character in the bits
	    that pad to the next byte. Also, the code currently makes no
	    provision for "LoChar" or "HiChar" (i.e. there are blank table
	    entries for characters 0-1f). This can be changed if desired.
	*/

	font_mod = font->rowMod;
	uint16 offset = font->charXOffset[drawchar];
	src = &font->fontdata[offset];
	dst = baseline + xpos;

	for (w = font->charWidth[drawchar]; w > 0; w -= 8) {
		uint8   *src_row,
		        *dst_row;
		short   h;

		src_row = src;
		dst_row = dst;

		for (h = font->height; h > 0; h--) {
			uint8   *b;
			uint8   s;

			b = dst_row;

			for (s = *src_row; s != 0; s <<= 1) {
				if ((s & 0x80) != 0)
					*b = color;
				++b;
			}
			src_row += font_mod;
			dst_row += destwidth;
		}

		++src;
		dst += 8;
	}
}
/*  DrawChar3x3Outline:

    This function renders a single bitmapped character into an offscreen
    buffer. The character will be "ballooned", i.e. expanded, by 1 pixel
    in each direction. It does not render the center part of the outlined
    character in a different color -- that must be done as a separate
    step.

    'drawchar' is the ascii code of the character to be drawn.

    'xpos' is the position in the buffer to draw the outline. Note that
    the first pixel of the outline may be drawn at xpos-1, since the
    outline expands in both directions.

    'baseline' is the address of the first scanline in the buffer.
    (it can actually be any scanline in the buffer if we want to
    draw the character at a different y position).
    Note that this has nothing to do with the Amiga "BASELINE" field,
    this routine always renders relative to the top of the font.

    'color' is the pixel value to render into the buffer.

    This code assumes that the font characters are all byte-aligned,
    and that there is no extra junk after the character in the bits
    that pad to the next byte. Also, the code currently makes no
    provision for "LoChar" or "HiChar" (i.e. there are blank table
    entries for characters 0-1f). This can be changed if desired.

    Note that this code does unaligned int16 writes to byte addresses,
    and will not run on an 68000 or 68010 processor. (Even with the
    speed penalty, this is still pretty fast).
*/

#define SQUARE_OUTLINES 1

void DrawChar3x3Outline(gFont *font, int drawchar, int xpos, uint8 *baseline,
                        uint8 color, uint16 destwidth) {
	uint8           *d;
	short           h,              // font height counter
	                rowmod;

	short           charwidth, w;   // width of character in pixels

	uint8           *chardata;      // pointer to start of char data

	uint8           *src, *dst;
	unsigned short  s;
	unsigned short  txt1, txt2, txt3;

	//  point to the first byte of the first scanline of the source char
	uint16 offset = font->charXOffset[drawchar];
	chardata = &font->fontdata[offset];

	//  get the width of the character in pixels
	charwidth = font->charWidth[drawchar];

	//  point to the first byte of where we want to place the character
	baseline += xpos - 1;

	//  this loop goes once for each 8 pixels wide that the character is
	rowmod = font->rowMod;

	for (w = (charwidth + 7) >> 3; w > 0; w--) {
		src = chardata;
		dst = baseline;

		txt1 = txt2 = 0;

		for (h = font->height + 2; h; h--) {
			d = dst;

			txt3 = txt2;
			txt2 = txt1;
			txt1 = h > 2 ? *src : 0;

			s = txt1 | txt2 | txt3;

			s = s | (s << 1) | (s << 2);

			while (s) {
				if (s & 0x200)
					*d = color;
				++d;
				s <<= 1;
			}

			src += rowmod;
			dst += destwidth;
		}

		chardata++;
		baseline += 8;
	}

}

void DrawChar5x5Outline(gFont *font, int drawchar, int xpos, uint8 *baseline,
                        uint8 color, uint16 destwidth) {
	uint8           *d;
	short           h,              /* font height counter              */
	                rowmod;

	short           charwidth, w;   /* width of character in pixels     */

	uint8           *chardata;      /* pointer to start of char data    */

	uint8           *src, *dst;
	unsigned short  s0, s1;
	unsigned short  txt[5];

	//  point to the first byte of the first scanline of the source char
	uint16 offset = font->charXOffset[drawchar];
	chardata = &font->fontdata[offset];

	//  get the width of the character in pixels
	charwidth = font->charWidth[drawchar];

	//  point to the first byte of where we want to place the character
	baseline += xpos - 2;

	//  this loop goes once for each 8 pixels wide that the character is
	rowmod = font->rowMod;

	for (w = (charwidth + 7) >> 3; w > 0; w--) {
		src = chardata;
		dst = baseline;

		txt[0] = txt[1] = txt[2] = txt[3 ] = txt[ 4 ] = 0;

		for (h = font->height + 4; h; h--) {
			d = dst;

			txt[4] = txt[3];
			txt[3] = txt[2];
			txt[2] = txt[1];
			txt[1] = txt[0];
			txt[0] = h > 4 ? *src : 0;

			s0 = txt[1] | txt[2] | txt[3];
			s1 = s0 | txt[0] | txt[4];

			s0 = s0 | (s0 << 1) | (s0 << 2) | (s0 << 3) | (s0 << 4);
			s0 |= (s1 << 1) | (s1 << 2) | (s1 << 3);

			while (s0) {
				if (s0 & 0x800)
					*d = color;
				++d;
				s0 <<= 1;
			}

			src += rowmod;
			dst += destwidth;
		}

		chardata++;
		baseline += 8;
	}

}

/*  A private routine to render a string of characters into a temp
    buffer.
*/

void gPort::drawStringChars(
    const char      *str,                   // string to draw
    int16           len,                    // length of string
    gPixelMap       &dest,
    int             xpos,                   // x position to draw it
    int             ypos) {                 // y position to draw it
	const char     *s;                     // pointer to string
	uint8           drawchar;               // char to draw
	int16           x;                      // current x position
	uint8           *buffer,                // buffer to render to
	                *uBuffer;               // underline buffer
	uint16          drowMod = dest.size.x;  // row modulus of dest
	int16           i;                      // loop index
	uint8           underbar = (textStyles & textStyleBar) != 0;
	bool            underscore;
	int16           underPos;

	// the address to start rendering pixels to.

	underPos = font->baseLine + 2;
	if (underPos > font->height) underPos = font->height;
	buffer = dest.data + (ypos * drowMod);
	uBuffer = buffer + (underPos * drowMod);

	// draw drop-shadow, if any

	if (textStyles & textStyleShadow) {
		x = xpos - 1;
		s = str;

		if (textStyles & textStyleOutline) { // if outlining
			for (i = 0; i < len; i++) {
				drawchar = *s++;            // draw thick drop shadow
				x += font->charKern[drawchar];
				DrawChar3x3Outline(font, drawchar, x, buffer, shPen, drowMod);
				x += font->charSpace[drawchar] + textSpacing;
			}
		} else if (textStyles & textStyleThickOutline) { // if outlining
			for (i = 0; i < len; i++) {
				drawchar = *s++;                // draw thick drop shadow
				x += font->charKern[drawchar];
				DrawChar5x5Outline(font, drawchar, x, buffer, shPen, drowMod);
				x += font->charSpace[drawchar] + textSpacing;
			}
		} else {
			for (i = 0; i < len; i++) {
				drawchar = *s++;            // draw thick drop shadow
				x += font->charKern[drawchar];
				DrawChar(font, drawchar, x, buffer + drowMod,
				         shPen, drowMod);
				x += font->charSpace[drawchar] + textSpacing;
			}
		}
	}

	// draw outline, if any

	if (textStyles & textStyleOutline) { // if outlining
		x = xpos;
		s = str;

		for (i = 0; i < len; i++) {
			drawchar = *s++;                // draw thick text
			x += font->charKern[drawchar];
			DrawChar3x3Outline(font, drawchar, x, buffer - drowMod,
			                   olPen, drowMod);
			x += font->charSpace[drawchar] + textSpacing;
		}
	} else if (textStyles & textStyleThickOutline) { // if thick outlining
		x = xpos;
		s = str;

		for (i = 0; i < len; i++) {
			drawchar = *s++;                // draw extra thick text
			x += font->charKern[drawchar];
			DrawChar5x5Outline(font, drawchar, x, buffer - drowMod * 2,
			                   olPen, drowMod);
			x += font->charSpace[drawchar] + textSpacing;
		}
	}

	// draw inner part

	x = xpos;
	s = str;
	underscore = textStyles & textStyleUnderScore ? true : false;

	for (i = 0; i < len; i++) {
		int16       last_x = x;
		uint8       color = fgPen;

		drawchar = *s++;                // draw thick drop shadow
		if (drawchar == '_' && underbar) {
			len--;
			drawchar = *s++;
			if (textStyles & textStyleUnderBar)
				underscore = true;
			if (textStyles & textStyleHiLiteBar)
				color = bgPen;
		}
		x += font->charKern[drawchar];
		DrawChar(font, drawchar, x, buffer, color, drowMod);
		x += font->charSpace[drawchar] + textSpacing;

		if (underscore) {               // draw underscore
			uint8   *put = uBuffer + last_x;
			int16   width = x - last_x;

			while (width-- > 0) {
				*put++ = color;
			}

			if (!(textStyles & textStyleUnderScore))
				underscore = false;
		}
	}
}

//  Draws a string clipped to the current clipping rectangle.
//  Note that if several clipping rectangles were to be used,
//  we would probably do this differently...

int16 gPort::drawClippedString(
    const char      *s,                     // string to draw
    int16           len,                    // length of string
    int             xpos,                   // x position to draw it
    int             ypos) {                 // y position to draw it
	int16           clipWidth = 0,          // width of clipped string
	                clipLen;                // chars to draw
	gPixelMap       tempMap;                // temp buffer for text
	uint8           underbar = (textStyles & textStyleBar) != 0;
	int16           xoff = 0,               // offset for outlines
	                yoff = 0;               // offset for outlines
	int16           penMove = 0;            // keep track of pen movement

	//  First, we want to avoid rendering any characters to the
	//  left of the clipping rectangle. Scan the string until
	//  we find a character that is not completely to the left
	//  of the clip.

	while (len > 0) {
		int16       drawchar = *s,
		            charwidth;

		if (drawchar == '_' && underbar) {
			drawchar = s[1];
			charwidth   = font->charKern[drawchar]
			              + font->charSpace[drawchar] + textSpacing;

			if (xpos + charwidth >= clip.x)
				break;
			s++;
		} else {
			charwidth   = font->charKern[drawchar]
			              + font->charSpace[drawchar] + textSpacing;
			if (xpos + charwidth >= clip.x)
				break;
		}

		s++;
		len--;
		xpos += charwidth;
		penMove += charwidth;
	}

	//  Now, we also want to only draw that portion of the string
	//  that actually appears in the clip, so scan the rest of the
	//  string until we find a character who's left edge is past
	//  the right edge of the clip.

	for (clipLen = 0; clipLen < len; clipLen++) {
		int16       drawchar = s[clipLen];

		if (drawchar == '_' && underbar)
			continue;

		clipWidth += font->charKern[drawchar]
		             + font->charSpace[drawchar] + textSpacing;

		if (xpos > clip.x + clip.width)
			break;
	}

	//  Handle special case of negative kern value of 1st character

	if (font->charKern[(byte)s[0]] < 0) {
		int16       kern = - font->charKern[(byte)s[0]];

		clipWidth += kern;              // increase size of map to render
		xoff += kern;                   // offset text into map right
		xpos -= kern;                   // offset map into port left
	}

	//  Set up a temporary bitmap to hold the string.

	tempMap.size.x = clipWidth;
	tempMap.size.y = font->height;

	//  Adjust the size and positioning of the temp map due
	//  to text style effects.

	if (textStyles & textStyleOutline) {
		xoff = yoff = 1;
		xpos--;
		ypos--;
		tempMap.size.x += 2;
		tempMap.size.y += 2;
	} else if (textStyles & textStyleThickOutline) {
		xoff = yoff = 2;
		xpos -= 2;
		ypos -= 2;
		tempMap.size.x += 4;
		tempMap.size.y += 4;
	}

	if (textStyles & (textStyleShadow | textStyleUnderScore | textStyleUnderBar)) {
		tempMap.size.x += 1;
		tempMap.size.y += 1;
	}

	if (textStyles & textStyleItalics) {
		int n = (font->height - font->baseLine - 1) / 2;

		if (n > 0) xpos += n;
		tempMap.size.x += tempMap.size.y / 2;
	}

	//  Allocate a temporary bitmap

	if (tempMap.bytes() == 0)
		return 0;
	tempMap.data = (uint8 *)TempAlloc(tempMap.bytes());
	if (tempMap.data != nullptr) {
		//  Fill the buffer with background pen if we're
		//  not doing a transparent blit.

		memset(tempMap.data,
		       (drawMode == drawModeReplace) ? bgPen : 0,
		       tempMap.bytes());

		//  Draw the characters into the buffer

		drawStringChars(s, clipLen, tempMap, xoff, yoff);

		//  apply slant if italics

		if (textStyles & textStyleItalics) {
			int n = (font->height - font->baseLine - 1) / 2;
			int shift = (n > 0 ? n : 0);
			int flag = (font->height - font->baseLine - 1) & 1;

			shift = -shift;

			for (int k = font->height - 1; k >= 0; k--) {
				if (shift < 0) {
					uint8   *dest = tempMap.data + k * tempMap.size.x,
					         *src = dest - shift;
					int     j;

					for (j = 0; j < tempMap.size.x + shift; j++) {
						*dest++ = *src++;
					}
					for (; j < tempMap.size.x; j++) {
						*dest++ = 0;
					}
				} else if (shift > 0) {
					uint8   *dest = tempMap.data + (k + 1) * tempMap.size.x,
					         *src = dest - shift;
					int     j;

					for (j = 0; j < tempMap.size.x - shift; j++) {
						*--dest = *--src;
					}
					for (; j < tempMap.size.x; j++) {
						*--dest = 0;
					}
				}

				flag ^= 1;
				if (flag)
					shift++;
			}
		}

		//  blit the temp buffer onto the screen.

		bltPixels(tempMap, 0, 0,
		          xpos, ypos,
		          tempMap.size.x, tempMap.size.y);

		TempFree(tempMap.data);
	}

	//  Now, we still need to scan the rest of the string
	//  so that we can move the pen position by the right amount.

//	penMove += clipWidth;
	for (clipLen = 0; clipLen < len; clipLen++) {
		int16       drawchar = s[clipLen];

		if (drawchar == '_' && underbar)
			continue;

		penMove += font->charKern[drawchar]
		           + font->charSpace[drawchar] + textSpacing;
	}

	return penMove;
}

/********* gtext.cpp/gPort::drawText *********************************
*
*   NAME
*       gPort::drawText -- draw a text string into a gPort
*
*   SYNOPSIS
*       port.drawText( str, length );
*
*       void gPort::drawText( char *, int16 = -1 );
*
*   FUNCTION
*       This function draws a string of text at the current pen
*       position in the current pen color. The pen position is
*       updated to the end of the text.
*
*       The text will be positioned such that the top-left corner
*       of the first character will be at the pen position.
*
*   INPUTS
*       str         A string of characters.
*
*       length      [Optional parameter] If supplied, it indicates
*                   the length of the string; Otherwise, strlen()
*                   is used.
*
*   RESULT
*       none
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::drawText(
    const char      *str,                   /* string to draw               */
    int16           length) {
	if (length < 0)
		length = strlen(str);

	if (length > 0)
		penPos.x += drawClippedString(str, length, penPos.x, penPos.y);
}

/********* gtext.cpp/gPort::drawTextInBox *********************************
*
*   NAME
*       gPort::drawTextInBox -- draw text within a box
*
*   SYNOPSIS
*       port.drawTextInBox( str, len, rect, pos, borderSpace );
*
*       void gPort::drawTextInBox( char *, int16, const Rect16 &,
*       /c/ int16, Point16 );
*
*   FUNCTION
*       This function can draw a text string centered or justified
*       within a rectangular area, and clipped to that area as well.
*       The text string is drawn in the current pen color, and with
*       the current draw mode.
*
*   INPUTS
*       str         The text to draw.
*
*       len         The length of the string or -1 to indicate a
*                   null-terminated string.
*
*       rect        The rectangle to draw the text within.
*
*       pos         How to position the text string within the
*                   rectangle. The default (0) is to center the
*                   string both horizontally and vertically; However,
*                   the following flags will modify this:
*
*       /i/         textPosLeft -- draw text left-justified.
*
*       /i/         textPosRight -- draw text right-justified.
*
*       /i/         textPosHigh -- draw text flush with top edge.
*
*       /i/         textPosLow -- draw text flush with bottom edge.
*
*       borderSpace A Point16 object, which indicates how much space
*                   (in both x and y) to place between the text and
*                   the border when the text is justified to a
*                   particular edge. Does not apply when text is centered.
*
*   RESULT
*       none
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::drawTextInBox(
    const char      *str,
    int16           length,
    const Rect16    &r,
    int16           pos,
    Point16         borders) {
	int16           height, width;
	int16           x, y;
	Rect16          newClip,
	                saveClip = clip;

	if (!font)
		return;

	height = font->height;
	width  = TextWidth(font, str, length, textStyles);

	if (textStyles & (textStyleUnderScore | textStyleUnderBar)) {
		if (font->baseLine + 2 >= font->height)
			height++;
	}

	//  Calculate x position of text string

	if (pos & textPosLeft)
		x = r.x + borders.x;
	else if (pos & textPosRight)
		x = r.x + r.width - width - borders.x;
	else
		x = r.x + (r.width - width) / 2;

	//  Calculate y position of text string

	if (pos & textPosHigh)
		y = r.y + borders.y;
	else if (pos & textPosLow)
		y = r.y + r.height - height - borders.y;
	else
		y = r.y + (r.height - height) / 2;

	//  Calculate clipping region

	clip = intersect(clip, r);

	//  Draw the text

	moveTo(x, y);
	drawText(str, length);

	//  Restore the clipping region

	clip = saveClip;
}

//  Attach to gFont?

/********* gtext.cpp/TextWidth ***************************************
*
*   NAME
*       TextWidth -- returns length of text string in pixels
*
*   SYNOPSIS
*       width = TextWidth( font, str, len, styles );
*
*       int16 TextWidth( gFont *, char *, int16, int16 );
*
*   FUNCTION
*       This function computes the length of a text string in pixels
*       for a given font and text style.
*
*   INPUTS
*       font        The text font of the text.
*
*       str         The text string to measure.
*
*       len         The length of the string, or -1 to use strlen()
*
*       styles      The text rendering style (see gPort::setStyle).
*                   0 = normal.
*
*   RESULT
*       The width of the text.
*
**********************************************************************
*/
int16 TextWidth(gFont *font, const char *s, int16 length, int16 styles) {
	int16           count = 0;

	if (length < 0)
		length = strlen(s);

	while (length--) {
		uint8       chr = *s++;

		if (chr == '_' && (styles & textStyleBar))
			continue;

		count += font->charKern[chr] + font->charSpace[chr];
	}

	if (styles & textStyleItalics) {
		count += (font->baseLine + 1) / 2 +
		         (font->height - font->baseLine - 1) / 2;
	}
	if (styles & textStyleOutline)
		count += 2;
	else if (styles & textStyleThickOutline)
		count += 4;
	if (styles & textStyleShadow)
		count += 1;

	return count;
}

//  Searches for the insertion point between chars under the cursor

/********* gtext.cpp/WhichIChar **************************************
*
*   NAME
*       WhichIChar -- search for insertion point between characters
*
*   SYNOPSIS
*       charNum = WhichIChar( font, str, pick, maxLen );
*
*       int16 WhichIChar( gFont *, uint8 *, int16, int16 );
*
*   FUNCTION
*       This function is used by the gTextBox class to select
*       the position of the insertion point when the text box
*       is clicked on. It computes the width of each character
*       in the string (as it would be rendered
*       on the screen) and determines which two characters the
*       pick position would fall between, in other words it finds
*       the nearest insertion point between characters.
*
*   INPUTS
*       font        The font to use in the character-width calculation.
*
*       str         The string to search.
*
*       pick        The pick position, relative to the start of the string.
*
*       maxLen      The length of the string.
*
*   RESULT
*       The index of the selected character.
*
**********************************************************************
*/
int16 WhichIChar(gFont *font, uint8 *s, int16 length, int16 maxLen) {
	int16           count = 0;

	if (maxLen == -1)
		maxLen = strlen((char *)s);

	for (count = 0; count < maxLen; count++) {
		uint8       chr = *s++;
		int16       width;

		width = font->charKern[chr] + font->charSpace[chr];

		if (length < width / 2)
			break;
		length -= width;
	}
	return count;
}

/****** gtext.cpp/GTextWrap *****************************************
*
*   NAME
*       GTextWrap -- given text buffer and pixel width, find wrap point
*
*   SYNOPSIS
*       offset = GTextWrap( font, buffer, count, width, styles );
*
*       int32 GTextWrap( gFont *, char *, uint16 &, uint16, int16 );
*
*   FUNCTION
*       This function finds the point in a text buffer that text
*       wrapping would have to occur given the indicated pixel width.
*       Linefeeds are considered implied wrap points. Tabs are not
*       expanded.
*
*   INPUTS
*       font    a gFont that the text will be rendered in.
*       buffer  a NULL-terminated text buffer.
*       count   reference to variable to store count of characters
*               to render before wrap point.
*       width   width in pixels to wrap point.
*       styles  possible text styles.
*
*   RESULT
*       offset  amount to add to buffer for next call to GTextWrap,
*               or -1 if the NULL-terminator was reached.
*
**********************************************************************
*/
int32 GTextWrap(gFont *font, char *mark, uint16 &count, uint16 width, int16 styles) {
	char *text = mark;
	char *atext;
	uint16 pixlen;
	int aTextIndex = 0;

	if (!strchr(text, '\n')) {
		count = strlen(text);
		pixlen = TextWidth(font, text, count, styles);
		if (pixlen <= width)
			return -1;
	}

	atext = text;
	while (1) {
		Common::String s = atext;

		int nTextIndex = aTextIndex + s.findFirstOf(' ');
		int lTextIndex = aTextIndex + s.findFirstOf('\n');

		if (!s.contains(' ') || (s.contains('\n') && lTextIndex < nTextIndex)) {
			pixlen = TextWidth(font, text, lTextIndex, styles);
			if (pixlen <= width) {
				count = lTextIndex;
				return count + 1;
			}

			if (!s.contains(' ')) {
				if (atext == text)
					break;

				count = aTextIndex - 1;
				return count + 1;
			}
		}

		pixlen = TextWidth(font, text, nTextIndex, styles);
		if (pixlen > width) {
			if (atext == text)
				break;

			count = aTextIndex - 1;
			return count + 1;
		}

		atext = text + nTextIndex + 1;
		aTextIndex = nTextIndex + 1;
	}

	if (atext == text) {
		// current text has no spaces AND doesn't fit

		count = strlen(text);
		while (--count) {
			pixlen = TextWidth(font, text, count, styles);
			if (pixlen <= width)
				return count;
		}
	} else {
		count = aTextIndex - 1;
		return count + 1;
	}

	return -1;
}

} // end of namespace Saga2
