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

void unpackImage(gPixelMap *map, int32 width, int32 rowCount, int8 *srcData) {
	warning("STUB: unpackImage()");
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
	warning("STUB: initGraphics");
}

bool initProcessResources(void) {
        return TRUE;
}

void termProcessResources(void) {
}

} // end of namespace Saga2
