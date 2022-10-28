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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/gdraw.h"
#include "saga2/gblitter.h"

namespace Saga2 {

#define TempAlloc       malloc
#define TempFree        free


void gPort::setMap(gPixelMap *newmap, bool inverted) {
	_map = newmap;
	_clip = Rect16(0, 0, _map->_size.x, _map->_size.y);

	//  Added by Talin to support inverted maps
	if (inverted) {
		_baseRow = _map->_data + _map->bytes() - _map->_size.x;
		_rowMod = -_map->_size.x;
	} else {
		_baseRow = _map->_data;
		_rowMod = _map->_size.x;
	}
}

/****** gdraw.cpp/gPort::setState *********************************
*
*   NAME
*       gPort::setState -- restore the state of a gPort
*
*   SYNOPSIS
*       port.setState( state );
*
*       void gPort::setState( gPenState & );
*
*   FUNCTION
*       This function restores the state of a gPort from a gPenState
*       record. The record should have been filled in by an earlier
*       call to gPort::getState. These two functions are intended for
*       situations where it is necessary to save and restore the
*       drawing state of a gPort.
*
*   INPUTS
*       state       A reference to the saved state record.
*
*   RESULT
*       none
*
*   SEE ALSO
*       gPort class
*       gPort::getState
*
**********************************************************************
*/
void gPort::setState(gPenState &state) {
	setColor(state.fgPen);
	setBgColor(state.bgPen);
	setOutlineColor(state.olPen);
	setShadowColor(state.shPen);
	setMode((DrawModes)state.drawMode);
}

/****** gdraw.cpp/gPort::getState *********************************
*
*   NAME
*       gPort::getState -- save the drawing state of a gPort
*
*   SYNOPSIS
*       port.getState( state );
*
*       void gPort::getState( gPenState & );
*
*   FUNCTION
*       This function saves the "state" of a gPort into a small record.
*       The values saved are all of the pens and the drawing mode.
*
*       When used in conjunction with gPort::setState, this allows you
*       to save and restore the state of a gPort.
*
*   INPUTS
*       state       A reference to the state record to be filled in.
*
*   RESULT
*       none
*
*   SEE ALSO
*       gPort class
*       gPort::setState
*
**********************************************************************
*/
void gPort::getState(gPenState &state) {
	state.fgPen = _fgPen;
	state.bgPen = _bgPen;
	state.olPen = _olPen;
	state.shPen = _shPen;
	state.drawMode = _drawMode;
}

/****** gdraw.cpp/gPort::fillRect *********************************
*
*   NAME
*       gPort::fillRect -- fill a rectangle with a solid color.
*
*   SYNOPSIS
*       port.fillRect( rect );
*       port.fillRect( x, y, w, h );
*
*       void gPort::fillRect( const Rect16 );
*       void gPort::fillRect( int16, int16, int16, int16 );
*
*   FUNCTION
*       Fill a rectangular area with a solid color.
*       drawModeComplement is supported; All other draw modes cause
*       a solid filling of pixels.
*
*   INPUTS
*       x,y,w,h     The coordinates of the rectangle to fill
*
*       rect        The rectangle to fill as a Rect16
*
*   RESULT
*       none
*
*   NOTES
*       This function is currently in C and uses memset (for the gPort
*       version only -- the subclasses all have more optimized routines).
*       The base class rectfill should probably also be optimized.
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/

//  Your basic rectfill operation
//  REM: Probably better to recode this in assembly?

void gPort::fillRect(const Rect16 r) {
	Rect16          sect;

	sect = intersect(_clip, r);           // intersect with clip rect
	sect.x += _origin.x;                     // apply origin translate
	sect.y += _origin.y;

	if (!sect.empty()) {                    // if result is non-empty
		uint8 *addr = _baseRow + sect.y * _rowMod + sect.x;

		if (_drawMode == drawModeComplement) { // Complement drawing mode
			for (int h = sect.height;
			        h > 0;
			        h--,
			        addr += _rowMod) {
				uint16  w = sect.width;
				uint8   *put = addr;

				while (w--) *put++ ^= _fgPen;
			}
		} else {
			_FillRect(addr, _rowMod, sect.width, sect.height, _fgPen);
			/*
			            for (int h = sect.height;
			                 h > 0;
			                 h--,
			                    addr += _rowMod)
			            {
			                memset( addr, _fgPen, sect.width );
			            }
			*/
		}
	}
}

/****** gdraw.cpp/gPort::frameRect ********************************
*
*       NAME gPort::frameRect
*
*   SYNOPSIS
*       port.frameRect( rect, thickness );
*       port.frameRect( x, y, w, h, thickness );
*
*       void gPort::frameRect( const Rect16, int16 );
*       void gPort::frameRect( int16, int16, int16, int16, int16 );
*
*   FUNCTION
*       This function draws a frame around a rectangle. The frame is
*       contained completely within the area of the rectangle. The
*       thickness parameter specifies how many pixel thick the frame
*       is -- The thickness increases towards the center of the rectangle.
*
*       For example, if the rectangle has an X of 0 and a width of 100,
*       and the frame is 10 pixels thick, then the left edge of the frame
*       will span pixels 0-9 and the right edge pixels 90-99.
*
*     INPUTS
*       x,y,w,h     The coordinates of the rectangle to frame
*
*       rect        The rectangle to frame as a Rect16
*
*       thickness   The thickness of the frame.
*
*   RESULT
*       none
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::frameRect(const Rect16 r, int16 thick) {
	int16       dThick = thick * 2;

	if (dThick >= r.width || dThick >= r.height) {
		fillRect(r);
	} else {
		fillRect(Rect16(r.x, r.y, r.width, thick));
		fillRect(Rect16(r.x, r.y + r.height - thick, r.width, thick));
		fillRect(Rect16(r.x, r.y + thick, thick, r.height - dThick));
		fillRect(Rect16(r.x + r.width - thick, r.y + thick,
		                thick, r.height - dThick));
	}
}

/****** gdraw.cpp/gPort::hLine ************************************
*
*   NAME
*       gPort::hLine -- optimized rendering of horizontal lines
*
*   SYNOPSIS
*       port.hLine( x, y, width );
*
*       void gPort::hLine( int16, int16, int16 );
*
*   FUNCTION
*       This function draws a 1-pixel-thick horizontal line starting
*       from the coordinates at (x,y) and extending for "width" pixels.
*       drawModeComplement is supported.
*
*       Calling this function is faster that the general "line" routine.
*       It is used in many of the GTools bevel-drawing functions.
*
*   INPUTS
*       x,y     The left edge of the line
*
*       width   width of the line in pixels.
*
*   RESULT
*       none
*
*   NOTES
*       This function needs to be downcoded to assembly.
*       Note: The VGA and SVGA subclass versions of this function
*       are not as efficient (they just call RectFill).
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::hLine(int16 x, int16 y, int16 width) {
	Rect16          sect;

	//  Temporarily convert the coords into a rectangle, for
	//  easy clipping

	sect = intersect(_clip, Rect16(x, y, width, 1));
	sect.x += _origin.x;                     // apply origin translate
	sect.y += _origin.y;

	if (!sect.empty()) {                        // if result is non-empty
		if (_drawMode == drawModeComplement) {
			uint8 *addr = _baseRow + (y + _origin.y) * _rowMod + x + _origin.x;

			while (sect.width--) *addr++ ^= _fgPen;
		} else {
			_HLine(_baseRow + sect.y * _rowMod + sect.x, sect.width, _fgPen);

			/*          memset( _baseRow + sect.y * _rowMod + sect.x,
			                    _fgPen,
			                    sect.width );
			*/
		}
	}
}

/****** gdraw.cpp/gPort::vLine ************************************
*
*   NAME
*       gPort::vLine -- optimized rendering of vertical lines
*
*   SYNOPSIS
*       port.vLine( x, y, height );
*
*       void gPort::vLine( int16, int16, int16 );
*
*   FUNCTION
*       This function draws a 1-pixel-thick vertical line starting
*       from the coordinates at (x,y) and extending downward for
*       "height" pixels. drawModeComplement is supported.
*
*       Calling this function is faster that the general "line" routine.
*       It is used in many of the GTools bevel-drawing functions.
*
*   INPUTS
*       x,y     The left edge of the line
*
*       height  height of the line in pixels.
*
*   RESULT
*       none
*
*   NOTES
*       This function needs to be downcoded to assembly.
*       Note: The VGA and SVGA subclass versions of this function
*       are not as efficient (they just call RectFill).
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::vLine(int16 x, int16 y, int16 height) {
	int16           bottom = y + height;
	uint8           *addr;

	//  Just for laughs, we'll do the clipping a different way

	if (x < _clip.x || x >= _clip.x + _clip.width) return;
	if (y < _clip.y) y = _clip.y;
	if (bottom > _clip.y + _clip.height) bottom = _clip.y + _clip.height;

	//  And now, draw the line

	if (_drawMode == drawModeComplement) {
		for (addr = _baseRow + (y + _origin.y) * _rowMod + x + _origin.x;
		        y < bottom;
		        y++) {
			*addr ^= _fgPen;
			addr += _rowMod;
		}
	} else {
		for (addr = _baseRow + (y + _origin.y) * _rowMod + x + _origin.x;
		        y < bottom;
		        y++) {
			*addr = _fgPen;
			addr += _rowMod;
		}
	}
}

/* ======================================================================= *
   gPort class (graphics drawing port)
 * ======================================================================= */

//  Bresenham line-drawing functions

/****** gdraw.cpp/gPort::line *************************************
*
*   NAME
*       gPort::line -- general line-drawing routine
*
*   SYNOPSIS
*       port.line( x1, y1, x2, y2 );
*       port.line( startPoint, destPoint );
*
*       void gPort::line( int16 x1, int16 y1, int16 x2, int16 y2 );
*       void gPort::line( Point16 p, Point16 p );
*
*   FUNCTION
*       This is the general bresenham line-drawing function. It draws
*       from a source point to a destination point. The pen position
*       is not affected. Clipping and drawModeComplement are supported.
*
*     INPUTS
*       (first form)
*
*       x1, y1      The starting point of the line
*
*       x2, y2      The ending point of the line
*
*       (second form)
*
*       startPoint  The starting point of the line as a Point16
*
*       endPoint    The ending point of the line as a Point16
*
*     RESULT
*
*   NOTES
*       This function is still in C and not very efficient. The VGA and
*       SVGA versions are also fairly inefficient.
*
*       Also, I have not yet worked out the math to properly determine
*       the error terms for clipped lines. This potentially could cause
*       "jaggies" in the lines to match poorly when lines drawn in two
*       adjacent clip rectangles meet. To fix this I interate the
*       algorithm even for clipped pixels, which is a waste...
*
*   SEE ALSO
*       gPort class
*       gPort::move
*       gPort::moveTo
*       gPort::draw
*       gPort::drawTo
*
**********************************************************************
*/
void gPort::line(int16 x1, int16 y1, int16 x2, int16 y2) {
	bool            clipNeeded = false;

	int16           xAbs, yAbs,
	                xMove, yMove,
	                xDir, yDir,
	                i;

	int16           errTerm;

	int16           clipRight = _clip.x + _clip.width,
	                clipBottom = _clip.y + _clip.height;

	uint8           *addr;

	if (x1 > x2) {                      // drawing left
		if (x1 < _clip.x || x2 >= clipRight) return;
		if (x2 < _clip.x || x1 >= clipRight) clipNeeded = true;

		xDir = xMove = -1;              // amount to adjust address
		xAbs = x1 - x2;                 // length of line
	} else {                            // drawing right
		if (x2 < _clip.x || x1 >= clipRight) return;
		if (x1 < _clip.x || x2 >= clipRight) clipNeeded = true;

		xDir = xMove = 1;               // amount to adjust address
		xAbs = x2 - x1;                 // length of line
	}

	if (y1 > y2) {                      // drawing up
		if (y1 < _clip.y || y2 >= clipBottom) return;
		if (y2 < _clip.y || y1 >= clipBottom) clipNeeded = true;

		yDir = -1;
		yAbs = y1 - y2;
		yMove = -_rowMod;
	} else {                                    // drawing down
		if (y2 < _clip.y || y1 >= clipBottom) return;
		if (y1 < _clip.y || y2 >= clipBottom) clipNeeded = true;

		yDir = 1;
		yAbs = y2 - y1;
		yMove = _rowMod;
	}

	addr = _baseRow + (y1 + _origin.y) * _rowMod + x1 + _origin.x;

	if (clipNeeded) {                   // clipping versions
		if (xAbs > yAbs) {
			errTerm = yAbs - (xAbs >> 1);

			for (i = xAbs + 1; i > 0; i--) {
				if (x1 >= _clip.x && x1 < clipRight
				        && y1 >= _clip.y && y1 < clipBottom) {
					if (_drawMode == drawModeComplement)
						*addr ^= _fgPen;
					else *addr = _fgPen;
				}

				if (errTerm > 0) {
					y1 += yDir;
					addr += yMove;
					errTerm -= xAbs;
				}

				x1 += xDir;
				addr += xMove;
				errTerm += yAbs;
			}
		} else {
			errTerm = xAbs - (yAbs >> 1);

			for (i = yAbs + 1; i > 0; i--) {
				if (x1 >= _clip.x && x1 < clipRight
				        && y1 >= _clip.y && y1 < clipBottom) {
					if (_drawMode == drawModeComplement)
						*addr ^= _fgPen;
					else *addr = _fgPen;
				}

				if (errTerm > 0) {
					x1 += xDir;
					addr += xMove;
					errTerm -= yAbs;
				}

				y1 += yDir;
				addr += yMove;
				errTerm += xAbs;
			}
		}
	} else {                            // non-clipping versions
		if (xAbs > yAbs) {
			errTerm = yAbs - (xAbs >> 1);

			for (i = xAbs + 1; i > 0; i--) {
				if (_drawMode == drawModeComplement)
					*addr ^= _fgPen;
				else *addr = _fgPen;

				if (errTerm > 0) {
					y1 += yDir;
					addr += yMove;
					errTerm -= xAbs;
				}

				x1 += xDir;
				addr += xMove;
				errTerm += yAbs;
			}
		} else {
			errTerm = xAbs - (yAbs >> 1);

			for (i = yAbs + 1; i > 0; i--) {
				if (_drawMode == drawModeComplement)
					*addr ^= _fgPen;
				else *addr = _fgPen;

				if (errTerm > 0) {
					x1 += xDir;
					addr += xMove;
					errTerm -= yAbs;
				}

				y1 += yDir;
				addr += yMove;
				errTerm += xAbs;
			}
		}
	}
}

/****** gdraw.cpp/gPort::bltPixels ********************************
*
*   NAME
*       gPort::bltPixels -- general blitting routine
*
*   SYNOPSIS
*       port.bltPixels( sourceMap, srcX, srcY, dstX, dstY, width, height );
*
*       void gPort::bltPixels( gPixelMap &, int, int, int, int, int, int );
*
*   FUNCTION
*       This is it! The general blitting routine for gPorts. It obeys
*       clipping and the drawModes (see gPort::setMode for more details
*       on THAT).
*
*   INPUTS
*       sourceMap       A gPixelMap containing the source image to blit.
*
*       srcX, srcY      The coordinates of the source image to start
*                       blitting from.
*
*       dstX, dstY      The
*
*   RESULT
*       none
*
*   NOTES
*       This is still in C. However, for C it's not bad. The subclass
*       versions (VGA and SVGA) are in highly optimized assembly, though.
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::bltPixels(
    const gPixelMap       &src,
    int             src_x,
    int             src_y,
    int             dst_x,
    int             dst_y,
    int             width,
    int             height) {
	Rect16          r = Rect16(dst_x, dst_y, width, height),
	                sect;
	uint8           *src_line,
	                *dst_line;

	sect = intersect(_clip, r);

	if (!sect.empty()) {                        // if result is non-empty
		src_x += sect.x - r.x;
		src_y += sect.y - r.y;

		src_line = src._data + src_y   * src._size.x + src_x;
		dst_line = _baseRow
		           + (sect.y + _origin.y) * _rowMod
		           + sect.x + _origin.x;

		if (_drawMode == drawModeMatte) {        // Matte drawing mode
			for (int h = sect.height; h > 0; h--, src_line += src._size.x, dst_line += _rowMod) {
				uint8   *src_ptr = src_line,
				*dst_ptr = dst_line;

				for (int w = sect.width; w > 0; w--) {
					if (*src_ptr)
						*dst_ptr++ = *src_ptr++;
					else
						dst_ptr++, src_ptr++;
				}
			}
		} else if (_drawMode == drawModeColor) { // Color drawing mode
			// Draws single color, except where
			for (int h = sect.height;           // src pixels are transparent
			        h > 0;
			        h--,
			        src_line += src._size.x,
			        dst_line += _rowMod) {
				uint8   *src_ptr = src_line,
				         *dst_ptr = dst_line;

				for (int w = sect.width;
				        w > 0;
				        w--) {
					if (*src_ptr++)
						*dst_ptr++ = _fgPen;
					else
						dst_ptr++;
				}
			}
		} else if (_drawMode == drawModeReplace) { // Replacement drawing mode
            for (int h = sect.height; h > 0; h--, src_line += src._size.x, dst_line += _rowMod) {
				memcpy(dst_line, src_line, sect.width);
            }
		} else if (_drawMode == drawModeComplement) { // Complement drawing mode
			// Inverts pixels, except where
			for (int h = sect.height;           // src is transparent
			        h > 0;
			        h--,
			        src_line += src._size.x,
			        dst_line += _rowMod) {
				uint8   *src_ptr = src_line,
				         *dst_ptr = dst_line;

				for (int w = sect.width;
				        w > 0;
				        w--) {
					if (*src_ptr++) *dst_ptr++ ^= _fgPen;
					else dst_ptr++;
				}
			}
		}
	}
}

/****** gdraw.cpp/gPort::bltPixelMask ********************************
*
*   NAME
*       gPort::bltPixelMask -- blit pixels through a mask
*
*   SYNOPSIS
*       port.bltPixelMask( sourceMap, maskMap, srcX, srcY,
*       /c/     dstX, dstY, width, height );
*
*       void gPort::bltPixels( gPixelMap &, gPixelMap &, int, int,
*       /c/     int, int, int, int );
*
*   FUNCTION
*       This is similar to the bltPixels function, except it takes
*       an additional parameter, which specifies a "mask" bitmap. The mask
*       bitmap is assumed to be the same size and congruent to the
*       the source pixel map.
*
*       The mask map affects the blit as follows: Any time a pixel within
*       the mask map is zero, the corresponding source pixel will not
*       be blitted.
*
*   INPUTS
*       sourceMap       A gPixelMap containing the source image to blit.
*
*       maskMap         A gPixelMap containing the image mask.
*
*       srcX, srcY      The coordinates of the source image to start
*                       blitting from.
*
*       dstX, dstY      The
*
*   RESULT
*       none
*
*   NOTES
*       This is still in C. However, for C it's not bad. The subclass
*       versions (VGA and SVGA) are in highly optimized assembly, though.
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::bltPixelMask(
    gPixelMap       &src,
    gPixelMap       &msk,
    int             src_x,
    int             src_y,
    int             dst_x,
    int             dst_y,
    int             width,
    int             height) {
	Rect16          r = Rect16(dst_x, dst_y, width, height),
	                sect;
	uint8           *src_line,
	                *dst_line,
	                *msk_line;

	sect = intersect(_clip, r);

	if (!sect.empty()) {                        // if result is non-empty
		src_x += sect.x - r.x;
		src_y += sect.y - r.y;

		src_line = src._data + src_y   * src._size.x + src_x;
		msk_line = msk._data + src_y   * msk._size.x + src_x;
		dst_line = _baseRow
		           + (sect.y + _origin.y) * _rowMod
		           + sect.x + _origin.x;

		for (int h = sect.height;
		        h > 0;
		        h--,
		        src_line += src._size.x,
		        dst_line += _rowMod,
		        msk_line += msk._size.x) {
			uint8   *src_ptr = src_line,
			         *dst_ptr = dst_line,
			          *msk_ptr = msk_line;

			for (int w = sect.width;
			        w > 0;
			        w--) {
				if (*msk_ptr++) *dst_ptr++ = *src_ptr++;
				else dst_ptr++, src_ptr++;
			}
		}
	}
}

/****** gdraw.cpp/gPort::scrollPixels *****************************
*
*   NAME
*       gPort::scrollPixels -- scroll pixels within a rectangle
*
*   SYNOPSIS
*       port.scrollPixels( rect, dx, dy );
*
*       void gPort::scrollPixels( Rect16, int, int );
*
*   FUNCTION
*       This function scrolls the pixels within the rectangle "rect"
*       in the direction (dx, dy). Thus, if dx and dy are positive, the
*       image within the rectangle will appear to move to the lower right.
*
*       The function does not attempt to clear or otherwise deal with
*       "newly revealed" pixels.
*
*       The rectangle is intersected with the current clip region
*       before the operation proceeds.
*
*   INPUTS
*       rect        The rectangle to limit the scrolling to.
*
*       dx,dy       The direction of the scroll.
*
*   RESULT
*       none
*
*   NOTES
*       This function is NOT IMPLEMENTED for the gDisplayPorts (i.e.
*       the SVGA and VGA subclasses). The reason for this is the
*       difficulty of implementing this on all SVGA cards (For example,
*       if a card only has a single memory window, it would require an
*       extra off-screen buffer to insure that all possible scrollPixels
*       operations could succeed).
*
*   SEE ALSO
*       gPort class
*
**********************************************************************
*/
void gPort::scrollPixels(
    const Rect16    r,                          // area to scroll
    int             dx,                         // amount to scroll by
    int             dy) {
	Rect16          sect;
	uint8           *src_ptr,
	                *dst_ptr;

	sect = intersect(_clip, r);
	if (dx == 0 && dy == 0) return;

	if (!sect.empty()) {                        // if result is non-empty
		uint16      w = sect.width,
		            h = sect.height,
		            mod = _rowMod;
		Point16     src(sect.x + _origin.x, sect.y + _origin.y),
		            dst(sect.x + _origin.x, sect.y + _origin.y);

		if (dx > 0) {
			dst.x += dx;
			w -= dx;
		} else {
			src.x -= dx;
			w += dx;
		}

		if (dy > 0) {
			dst.y += dy;
			h -= dy;
		} else {
			src.y -= dy;
			h += dy;
		}

		if (w <= 0 || h <= 0) return;

		if (src.y > dst.y || (src.y == dst.y && src.x > dst.x)) {
			src_ptr = _baseRow + src.y * mod + src.x;
			dst_ptr = _baseRow + dst.y * mod + dst.x;

			mod -= w;

			while (h--) {
				for (int w1 = w; w1 > 0; w1--) *dst_ptr++ = *src_ptr++;
				src_ptr += mod;
				dst_ptr += mod;
			}
		} else {
			src_ptr = _baseRow + (src.y + h - 1) * mod + src.x + w;
			dst_ptr = _baseRow + (dst.y + h - 1) * mod + dst.x + w;

			mod -= w;

			while (h--) {
				for (int w1 = w; w1 > 0; w1--) *--dst_ptr = *--src_ptr;
				src_ptr -= mod;
				dst_ptr -= mod;
			}
		}
	}
}


/* ======================================================================= *
   Image Mapping
 * ======================================================================= */

/****** gdraw.cpp/mapImage ****************************************
*
*   NAME
*       mapImage -- translate an image via a translation table
*
*   SYNOPSIS
*       mapImage( sourceMap, destMap, translation );
*       void mapImage( sourePort, destPort, translation );
*
*       void mapImage( gPixelMap &, gPixelMap &, gPen map[] )
*       void mapImage( gPort, gPort, gPen map[] )
*
*   FUNCTION
*       Runs an image through a color translation. Right now it assumes
*       that the source image has exactly the same dimensions as the
*       destination image.
*
*   INPUTS
*       sourceMap   The map to translate
*
*       destMap     Where to put the translated pixels
*
*       translation The lookup table to translate the pixels through.
*
*       (second form, which takes gPorts instead of gPixelMaps)
*
*       sourcePort  The source pixels (in a gPort)
*
*       destPort    The destination pixels (in a gPort)
*
*   RESULT
*
*   NOTES
*       This could easily be downcoded.
*
**********************************************************************
*/
void mapImage(gPixelMap &from, gPixelMap &to, gPen map[]) {
	int32       bytes = to.bytes();
	uint8       *get = from._data,
	             *put = to._data;

	while (bytes--) *put++ = map[*get++];
}

void mapImage(gPort &from, gPort &to, gPen map[]) {
	mapImage(*from._map, *to._map, map);
}

/* ======================================================================= *
   Temporary images
 * ======================================================================= */

/****** gdraw.cpp/NewTempPort *************************************
*
*   NAME
*       NewTempPort -- create a new temporary gPort
*
*   SYNOPSIS
*       error = NewTempPort( port, width, height );
*
*       errorCode NewTempPort( gPort &, int, int );
*
*   FUNCTION
*       This function takes a raw, uninitialized gPort, creates
*       a temporary gPixelMap, and attaches it to the gPort. It's
*       mainly used internally by gTools when it needs somewhere to
*       temporarily stash an image.
*
*   INPUTS
*       port        The port object to attach the temporary pixel map to.
*
*       width,height    The size of the pixel map to be created.
*
*   RESULT
*       If it succeed, the error will be "errOK".
*
*   NOTES
*       We should phase out the old method of handling errors and
*       use the new exception-based error handling.
*
*   SEE ALSO
*       gPort class
*       gPixelMap class
*       DisposeTempPort
*
**********************************************************************
*/
bool NewTempPort(gPort &port, int width, int height) {
	gPixelMap       *map;

	map = (gPixelMap *)TempAlloc(width * height + sizeof(gPixelMap));
	if (map != nullptr) {
		map->_data = (uint8 *)(map + 1);
		map->_size.x = width;
		map->_size.y = height;
		port.setMap(map);
		return true;
	} else
		return false;
}

/****** gdraw.cpp/DisposeTempPort *********************************
*
*   NAME
*       DisposeTempPort -- disposes of temporary pixel map
*
*   SYNOPSIS
*       DisposeTempPort( port );
*
*       void DisposeTempPort( gPort & );
*
*   FUNCTION
*       This function disposes of the temporary pixel map which
*       was created by NewTempPort.
*
*   INPUTS
*       port        The same port that was passed to NewTempPort
*
*   RESULT
*       none
*
*   SEE ALSO
*       gPort class
*       gPixelMap class
*       NewTempPort
*
**********************************************************************
*/
void DisposeTempPort(gPort &port) {
	if (port._map) TempFree(port._map);
	port._map = nullptr;
}

} // end of namespace Saga2
