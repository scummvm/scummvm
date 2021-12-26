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

#include "graphics/surface.h"

#include "saga2/saga2.h"
#include "saga2/vdraw.h"
#include "saga2/blitters.h"

namespace Saga2 {

vDisplayPage *drawPage;

void gDisplayPort::fillRect(const Rect16 r) {
	Rect16          sect;

	sect = intersect(clip, r);           // intersect with clip rect
	sect.x += origin.x;                     // apply origin translate
	sect.y += origin.y;

	if (!sect.empty()) {                    // if result is non-empty
		if (drawMode == drawModeComplement) // Complement drawing mode
			protoPage.invertRect(sect, fgPen);
		else
			protoPage.fillRect(sect, fgPen);     // regular drawing mode
	}
}

//  movePixels

void gDisplayPort::bltPixels(
    const gPixelMap       &src,
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
			protoPage.writeTransPixels(sect, src_line, src.size.x);
			break;
		case drawModeReplace:                   // don't use transparency
			protoPage.writePixels(sect, src_line, src.size.x);
			break;
		case drawModeColor:                     // solid color, use transparency
			protoPage.writeColorPixels(sect, src_line, src.size.x, fgPen);
			break;
		case drawModeComplement:                // blit in complement mode
			protoPage.writeComplementPixels(sect, src_line, src.size.x, fgPen);
			break;
		default:
			error("bltPixels: Unknown drawMode: %d", drawMode);
		}
	}
}

void gDisplayPort::scrollPixels(
    const Rect16    r,                      // area to scroll
    int             dx,                     // amount to scroll by
    int             dy) {
	Rect16          sect;

	if (dx == 0 && dy == 0)        // quit of nothing to do
		return;

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

		if (srcRect.width <= 0 || srcRect.height <= 0)
			return;

		//  Allocate temp map to hold scrolled pixels

		tempMap.size.x = srcRect.width;
		tempMap.size.y = srcRect.height;
		tempMap.data = (uint8 *)malloc(tempMap.bytes());
#if 0
		if (!tempMap.data) fatal("Out of memory.\n");
#endif

		//  Blit scrolled pixels to system ram and back to SVGA

		protoPage.readPixels(srcRect, tempMap.data, tempMap.size.x);
		protoPage.writePixels(dstRect, tempMap.data, tempMap.size.x);

		//  dispose of temp pixel map

		free(tempMap.data);
	}
}

//  Bresenham line-drawing functions

void gDisplayPort::line(int16 x1, int16 y1, int16 x2, int16 y2) {
	warning("STUB: gDisplayPort::line()");
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
		yMove = -protoPage.size.x;
	} else {                            // drawing down
		if (y2 < clip.y || y1 >= clipBottom) return;
		if (y1 < clip.y || y2 >= clipBottom) clipNeeded = true;

		yDir = 1;
		yAbs = y2 - y1;
		yMove = protoPage.size.x;
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

		offset = (y1 + origin.y) * protoPage.size.x + x1 + origin.x;
		bank = offset >> 16;

		protoPage.setWriteBank(bank);
		if (drawMode == drawModeComplement) protoPage.setReadBank(bank);
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
					protoPage.setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					protoPage.setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
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
					protoPage.setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					protoPage.setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
				}
			}
		}
	} else {                            // non-clipping versions
		offset = (y1 + origin.y) * protoPage.size.x + x1 + origin.x;

		bank = offset >> 16;

		protoPage.setWriteBank(bank);
		if (drawMode == drawModeComplement) protoPage.setReadBank(bank);
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
					protoPage.setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					protoPage.setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
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
					protoPage.setWriteBank(++bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
				} else if (offset < 0) {
					offset += cBytesPerBank;
					protoPage.setWriteBank(--bank);
					if (drawMode == drawModeComplement)
						protoPage.setReadBank(bank);
				}
			}
		}
	}
#endif
}

void vDisplayPage::fillRect(Rect16 r, uint8 color) {
	Graphics::Surface *surf = g_system->lockScreen();

	_FillRect((byte *)surf->getBasePtr(r.x, r.y), surf->pitch, r.width, r.height, color);

	g_system->unlockScreen();
}

void vDisplayPage::invertRect(Rect16 r, uint8 color) {
}

void vDisplayPage::writePixels(Rect16 r, uint8 *pixPtr, uint16 pixMod) {
	g_system->copyRectToScreen(pixPtr, pixMod, r.x, r.y, r.width, r.height);
}

void vDisplayPage::writeTransPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod) {
	Graphics::Surface *surf = g_system->lockScreen();

	_BltPixelsT(pixPtr, pixMod, (byte *)surf->getBasePtr(r.x, r.y), surf->pitch, r.width, r.height );

	g_system->unlockScreen();
}

void vDisplayPage::readPixels(Rect16 &r, uint8 *pixPtr, uint16 pixMod) {
	warning("STUB: vWDisplayPage::readPixels()");
}

//  Function to quickly transfer pixels from an off-screen
//  buffer to a rectangle on the SVGA display;
void vDisplayPage::writeColorPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod, uint8 color) {
	warning("STUB: writeColorPixels");
	writePixels(r, pixPtr, pixMod);
}

//  Function to quickly transfer pixels from an off-screen
//  buffer to a rectangle on the SVGA display;
void vDisplayPage::writeComplementPixels(Rect16 r, uint8 *pixPtr, uint16 pixMod, uint8 color) {
	warning("STUB: writeComplementPixels");
	writePixels(r, pixPtr, pixMod);
}

} // end of namespace Saga2
