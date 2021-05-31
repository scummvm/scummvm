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

#define FORBIDDEN_SYMBOL_ALLOW_ALL // FIXME: Remove

#include "saga2/std.h"
#include "saga2/saga2.h"
#include "saga2/blitters.h"
#include "common/debug.h"

namespace Saga2 {

void _BltPixels(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height) {
	warning("STUB: _BltPixels()");
}

void _BltPixelsT(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height) {
	warning("STUB: _BltPixelsT()");
}

void _FillRect(uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height, uint32 color) {
	warning("STUB: _FillRect()");
}

void _HLine(uint8 *dstPtr, uint32 width, uint32 color) {
	warning("STUB: _HLine()");
}

void unpackImage(gPixelMap &map, int16 width, int16 rowCount, int8 *srcData) {
	int8  *dest     = (int8 *)map.data;
	int16 bytecount = (width + 1) & ~1;
	int16 rowMod    = map.size.x - bytecount;

	while (rowCount--) {
		for (int16 k = 0; k < bytecount;) {
			int16 p = *srcData++;

			if (p == -128)
				continue;
			else if (p >= 0) {
				p++;
				k += p;
				while (p--)
					*dest++ = *srcData++;
			} else {
				p = 1 - p;
				k += p;
				while (p--)
					*dest++ = *srcData;
				srcData++;
			}
		}

		if (bytecount & 1)
			srcData++;
		dest += rowMod;
	}
}

void unpackImage(gPixelMap *map, int32 width, int32 rowCount, int8 *srcData) {
	unpackImage(*map, (int16)width, (int16)rowCount, srcData);
}

void unpackSprite(gPixelMap *map, uint8 *sprData) {
	warning("STUB: unpackSprite()");
}

void drawTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData) {
	warning("STUB: drawTile()");
}

void maskTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData) {
	warning("STUB: maskTile()");
}

void TBlit(gPixelMap *d, gPixelMap *s, int32 x, int32 y) {
	warning("STUB: TBlit()");
}

void TBlit4(gPixelMap *d, gPixelMap *s, int32 x, int32 y) {
	warning("STUB: TBlit4()");
}

void compositePixels(gPixelMap *compMap, gPixelMap *sprMap, int32 xpos, int32 ypos, uint8 *lookup) {
	warning("STUB: compositePixels()");
}

void compositePixelsRvs(gPixelMap *compMap, gPixelMap *sprMap, int32 xpos, int32 ypos, uint8 *lookup) {
	warning("STUB: compositePixelsRvs()");
}

void _LoadPalette(uint8 *rgbArray, uint32 startColor, uint32 numColors) {
	warning("STUB: _LoadPalette()");
}

bool initGraphics(void) {
	warning("STUB: initGraphics()");
	return false;
}

bool initProcessResources(void) {
        return TRUE;
}

void termProcessResources(void) {
}

} // end of namespace Saga2
