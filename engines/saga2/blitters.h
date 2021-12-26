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

#ifndef SAGA2_BLITTERS_H
#define SAGA2_BLITTERS_H

namespace Saga2 {

class gPixelMap;

void _BltPixels(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height);

void _BltPixelsT(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height);

void _FillRect(uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height, uint32 color);
void _HLine(uint8 *dstPtr, uint32 width, uint32 color);


void unpackImage(gPixelMap *map, int32 width, int32 rowCount, int8 *srcData);
void unpackImage(gPixelMap &map, int16 width, int16 rowCount, int8 *srcData);

void unpackSprite(gPixelMap *map, uint8 *sprData, uint32 dataSize);
void compositePixels(gPixelMap *compMap, gPixelMap *sprMap, int32 xpos, int32 ypos, uint8 *lookup);
void compositePixelsRvs(gPixelMap *compMap, gPixelMap *sprMap, int32 xpos, int32 ypos, uint8 *lookup);

//  Fast transparent blitting routine in assembly
void TBlit(gPixelMap *d, gPixelMap *s, int32 x, int32 y);
void TBlit4(gPixelMap *d, gPixelMap *s, int32 x, int32 y);

void drawTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData, bool mask = false);
void maskTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData);

} // end of namespace Saga2

#endif
