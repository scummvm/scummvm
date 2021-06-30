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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"
#include "saga2/vdraw.h"

namespace Saga2 {

Extent16            gDisplaySize;
vDisplayPage        *drawPage;

/* ===================================================================== *
                    Member functions for gDisplayPort
 * ===================================================================== */

//  Your basic rectfill operation -- but this time in SVGA

void gDisplayPort::fillRect(const Rect16 r) {
	Rect16          sect;

	sect = intersect(clip, r);           // intersect with clip rect
	sect.x += origin.x;                     // apply origin translate
	sect.y += origin.y;

	if (!sect.empty()) {                    // if result is non-empty
		if (drawMode == drawModeComplement) // Complement drawing mode
			displayPage->invertRect(sect, fgPen);
		else displayPage->fillRect(sect, fgPen);     // regular drawing mode
	}
}

//  movePixels

void gDisplayPort::bltPixels(
    gPixelMap       &src,
    int             src_x,
    int             src_y,
    int             dst_x,
    int             dst_y,
    int             width,
    int             height) {
	Rect16          r = Rect16(dst_x, dst_y, width, height),
	                sect;
	uint8           *src_line;

	if (clip.empty())
		clip = Rect16(0, 0, map->size.x, map->size.y);
	sect = intersect(clip, r);

	if (!sect.empty()) {                        // if result is non-empty
		src_x += sect.x - r.x;
		src_y += sect.y - r.y;

		src_line = src.data + src_y * src.size.x + src_x;

		sect.x += origin.x;
		sect.y += origin.y;

		switch (drawMode) {
		case drawModeMatte:                     // use transparency
			displayPage->writeTransPixels(sect, src_line, src.size.x);
			break;
		case drawModeReplace:                   // don't use transparency
			displayPage->writePixels(sect, src_line, src.size.x);
			break;
		case drawModeColor:                     // solid color, use transparency
			displayPage->writeColorPixels(sect, src_line, src.size.x, fgPen);
			break;
		case drawModeComplement:                // blit in complement mode
			displayPage->writeComplementPixels(sect, src_line, src.size.x, fgPen);
			break;
		}
	}
}

/********* vdraw.cpp/gDisplayPort::scrollPixels **********************
*
*       NAME gDisplayPort::scrollPixels
*
*   SYNOPSIS
*
*   FUNCTION
*
*     INPUTS
*
*     RESULT
*
**********************************************************************
*/
void gDisplayPort::scrollPixels(
    const Rect16    r,                      // area to scroll
    int             dx,                     // amount to scroll by
    int             dy) {
	Rect16          sect;

	if (dx == 0 && dy == 0) return;         // quit of nothing to do

	sect = intersect(clip, r);           // apply cliping rect

	if (!sect.empty()) {                    // if result is non-empty
		Rect16      srcRect,
		            dstRect;
		gPixelMap   tempMap;

		sect.x += origin.x;
		sect.y += origin.y;
		srcRect = dstRect = sect;           // make copies of rect

		if (dx > 0) {
			dstRect.x += dx;
			srcRect.width = dstRect.width -= dx;
		} else {
			srcRect.x -= dx;
			srcRect.width = dstRect.width += dx;
		}

		if (dy > 0) {
			dstRect.y += dy;
			srcRect.height = dstRect.height -= dy;
		} else {
			srcRect.y -= dy;
			srcRect.height = dstRect.height += dy;
		}

		//  Quit if all data is completely scrolled off

		if (srcRect.width <= 0 || srcRect.height <= 0) return;

		//  Allocate temp map to hold scrolled pixels

		tempMap.size.x = srcRect.width;
		tempMap.size.y = srcRect.height;
		tempMap.data = (uint8 *)malloc(tempMap.bytes());
#if 0
		if (!tempMap.data) fatal("Out of memory.\n");
#endif

		//  Blit scrolled pixels to system ram and back to SVGA

		displayPage->readPixels(srcRect, tempMap.data, tempMap.size.x);
		displayPage->writePixels(dstRect, tempMap.data, tempMap.size.x);

		//  dispose of temp pixel map

		free(tempMap.data);
	}
}

//  Bresenham line-drawing functions

void gDisplayPort::line(int16 x1, int16 y1, int16 x2, int16 y2) {
#if 0
	bool            clipNeeded = false;

	int16           xAbs, yAbs,
	                xMove, yMove,
	                xDir, yDir,
	                i;

	int16           errTerm,
	                bank;

	int16           clipRight = clip.x + clip.width,
	                clipBottom = clip.y + clip.height;

	int32           offset;

	if (x1 > x2) {                      // drawing left
		if (x1 < clip.x || x2 >= clipRight) return;
		if (x2 < clip.x || x1 >= clipRight) clipNeeded = true;

		xDir = xMove = -1;              // amount to adjust address
		xAbs = x1 - x2;                 // length of line
	} else {                            // drawing right
		if (x2 < clip.x || x1 >= clipRight) return;
		if (x1 < clip.x || x2 >= clipRight) clipNeeded = true;

		xDir = xMove = 1;               // amount to adjust address
		xAbs = x2 - x1;                 // length of line
	}

	if (y1 > y2) {                      // drawing up
		if (y1 < clip.y || y2 >= clipBottom) return;
		if (y2 < clip.y || y1 >= clipBottom) clipNeeded = true;

		yDir = -1;
		yAbs = y1 - y2;
		yMove = -displayPage->size.x;
	} else {                            // drawing down
		if (y2 < clip.y || y1 >= clipBottom) return;
		if (y1 < clip.y || y2 >= clipBottom) clipNeeded = true;

		yDir = 1;
		yAbs = y2 - y1;
		yMove = displayPage->size.x;
	}

	if (clipNeeded) {                   // clipping versions
		if (xAbs > yAbs) {
			errTerm = yAbs - (xAbs >> 1);

			for (i = xAbs + 1; i > 0; i--) {
				if (x1 >= clip.x && x1 < clipRight
				        && y1 >= clip.y && y1 < clipBottom) {
					break;
				}

				if (errTerm >= 0) {
					y1 += yDir;
					errTerm -= xAbs;
				}

				x1 += xDir;
				errTerm += yAbs;
			}
		} else {
			errTerm = xAbs - (yAbs >> 1);

			for (i = yAbs + 1; i > 0; i--) {
				if (x1 >= clip.x && x1 < clipRight
				        && y1 >= clip.y && y1 < clipBottom) {
					break;
				}

				if (errTerm >= 0) {
					x1 += xDir;
					errTerm -= yAbs;
				}

				y1 += yDir;
				errTerm += xAbs;
			}
		}

		offset = (y1 + origin.y) * displayPage->size.x + x1 + origin.x;
		bank = offset >> 16;

		displayPage->setWriteBank(bank);
		if (drawMode == drawModeComplement) displayPage->setReadBank(bank);
		offset &= 0x0000ffff;

		if (xAbs > yAbs) {
			for (; i > 0; i--) {
				if (x1 < clip.x || x1 >= clipRight
				        || y1 < clip.y || y1 >= clipBottom) {
					break;
				}

				if (drawMode == drawModeComplement) {
					svgaWriteAddr[offset]
					    = svgaReadAddr[offset] ^ fgPen;
				} else svgaWriteAddr[offset] = fgPen;

				if (errTerm >= 0) {
					y1 += yDir;
					offset += yMove;
					errTerm -= xAbs;
				}

				x1 += xDir;
				offset += xMove;
				errTerm += yAbs;

				if (offset >= cBytesPerBank) {
					offset -= cBytesPerBank;
					displayPage->setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					displayPage->setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				}
			}
		} else {
			for (; i > 0; i--) {
				if (x1 < clip.x || x1 >= clipRight
				        || y1 < clip.y || y1 >= clipBottom) {
					break;
				}

				if (drawMode == drawModeComplement) {
					svgaWriteAddr[offset]
					    = svgaReadAddr[offset] ^ fgPen;
				} else svgaWriteAddr[offset] = fgPen;

				if (errTerm >= 0) {
					x1 += xDir;
					offset += xMove;
					errTerm -= yAbs;
				}

				y1 += yDir;
				offset += yMove;
				errTerm += xAbs;

				if (offset >= cBytesPerBank) {
					offset -= cBytesPerBank;
					displayPage->setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					displayPage->setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				}
			}
		}
	} else {                            // non-clipping versions
		offset = (y1 + origin.y) * displayPage->size.x + x1 + origin.x;

		bank = offset >> 16;

		displayPage->setWriteBank(bank);
		if (drawMode == drawModeComplement) displayPage->setReadBank(bank);
		offset &= 0x0000ffff;

		if (xAbs > yAbs) {
			errTerm = yAbs - (xAbs >> 1);

			for (i = xAbs + 1; i > 0; i--) {
				if (drawMode == drawModeComplement) {
					svgaWriteAddr[offset]
					    = svgaReadAddr[offset] ^ fgPen;
				} else svgaWriteAddr[offset] = fgPen;

				if (errTerm >= 0) {
					y1 += yDir;
					offset += yMove;
					errTerm -= xAbs;
				}

				x1 += xDir;
				offset += xMove;
				errTerm += yAbs;

				if (offset >= cBytesPerBank) {
					offset -= cBytesPerBank;
					displayPage->setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					displayPage->setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				}
			}
		} else {
			errTerm = xAbs - (yAbs >> 1);

			for (i = yAbs + 1; i > 0; i--) {
				if (drawMode == drawModeComplement) {
					svgaWriteAddr[offset]
					    = svgaReadAddr[offset] ^ fgPen;
				} else svgaWriteAddr[offset] = fgPen;

				if (errTerm >= 0) {
					x1 += xDir;
					offset += xMove;
					errTerm -= yAbs;
				}

				y1 += yDir;
				offset += yMove;
				errTerm += xAbs;

				if (offset >= cBytesPerBank) {
					offset -= cBytesPerBank;
					displayPage->setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					displayPage->setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						displayPage->setReadBank(bank);
				}
			}
		}
	}
#endif
}

/*  These are the breakpoint values

    0x10000 / 640   = 102
    0x10000 % 640   = 256

    0x20000 / 640   = 204
    0x20000 % 640   = 512

    0x30000 / 640   = 307
    0x30000 % 640   = 128

    0x40000 / 640   = 409
    0x40000 % 640   = 384
*/

} // end of namespace Saga2
