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

#include "common/debug.h"
#include "common/memstream.h"
#include "graphics/surface.h"

#include "saga2/saga2.h"
#include "saga2/gdraw.h"

namespace Saga2 {

void _BltPixels(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height) {
	for (uint y = 0; y < height; y++) {
		uint8 *src = srcPtr + srcMod * y;
		uint8 *dst = dstPtr + dstMod * y;
		for (uint x = 0; x < width; x++) {
			*dst++ = *src++;
		}
	}
}

void _BltPixelsT(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height) {
	for (uint y = 0; y < height; y++) {
		uint8 *src = srcPtr + srcMod * y;
		uint8 *dst = dstPtr + dstMod * y;
		for (uint x = 0; x < width; x++) {
			byte c = *src++;

			if (c == 0)
				dst++;
			else
				*dst++ = c;
		}
	}
}

void _FillRect(uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height, uint32 color) {
	for (uint y = 0; y < height; y++) {
		memset(dstPtr, color, width);

		dstPtr += dstMod;
	}
}

void _HLine(uint8 *dstPtr, uint32 width, uint32 color) {
	memset(dstPtr, color, width);
}

void unpackImage(gPixelMap &map, int16 width, int16 rowCount, int8 *srcData) {
	int8  *dest = (int8 *)map._data;
	int16 bytecount = (width + 1) & ~1;
	int16 rowMod = map._size.x - bytecount;

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

#define DEBUGPACK 0

void unpackSprite(gPixelMap *map, uint8 *sprData, uint32 dataSize) {
	byte *dst = map->_data;
	int bytes = map->_size.x * map->_size.y;
	bool fail = false;

	if (!sprData) {
		warning("unpackSprite(): empty sprData");
		return;
	}

	Common::MemoryReadStream stream(sprData, dataSize);

	while (true) {
		byte trans = stream.readByte();

		if (stream.eos()) {
			debug(8, "unpackSprite: premature end of data #1");
			fail = true;
			break;
		}

		if (bytes < trans) {
#if DEBUGPACK
			warning("unpackSprite: too many trans %d < %d for %dx%d (src %d bytes)", bytes, trans, map->_size.x, map->_size.y, dataSize);
#endif
			fail = true;
			break;
		}
		memset(dst, 0, trans);
		dst += trans;
		bytes -= trans;

		if (bytes < 0)
			break;

		byte fill = stream.readByte();

		if (stream.eos()) {
			debug(8, "unpackSprite: premature end of data #2");
			fail = true;
			break;
		}
		if (bytes < fill) {
#if DEBUGPACK
			warning("unpackSprite: too many fill %d < %d for %dx%d (src %d bytes)", bytes, fill, map->_size.x, map->_size.y, dataSize);
#endif
			fill = bytes;
			fail = true;
		}
		if (stream.read(dst, fill) != fill) {
			debug(8, "unpackSprite: premature end of data #3");
			fail = true;
			break;
		}
		dst += fill;
		bytes -= fill;

		if (bytes <= 0)
			break;
	}

	if (!stream.eos()) {
#if DEBUGPACK
		warning("unpackSprite: %d bytes left", stream.size() - stream.pos());
#endif
		fail = true;
	}

	if (fail) {
#if DEBUGPACK
		Graphics::Surface surf;
		surf.w = map->_size.x;
		surf.h = map->_size.y;
		surf.pitch = map->_size.x;
		surf.format = Graphics::PixelFormat::createFormatCLUT8();

		surf.setPixels(map->_data);

		surf.debugPrint();
#endif
	}
}

void drawTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData, bool mask) {
	const byte *tilePointer;
	const byte *readPointer;
	byte *drawPointer;
	Point32 drawPoint;
	int widthCount;
	int row, col, count, lowBound;
	int bgRunCount;
	int fgRunCount;
	const int32 SAGA_ISOTILE_WIDTH = 64;
	Point16 point(x - 32, y);

	if (point.x + SAGA_ISOTILE_WIDTH < 0)
		return;

	if (point.x - SAGA_ISOTILE_WIDTH >= map->_size.x)
		return;

	tilePointer = srcData;

	drawPoint = point;

	drawPoint.y -= height;

	if (drawPoint.y >= map->_size.y)
		return;

	readPointer = tilePointer;
	lowBound = MIN((int)(drawPoint.y + height), (int)map->_size.y);
	for (row = drawPoint.y; row < lowBound; row++) {
		widthCount = 0;
		if (row >= 0) {
			drawPointer = map->_data + drawPoint.x + (row * map->_size.x);
			col = drawPoint.x;
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH)
					break;

				drawPointer += bgRunCount;
				col += bgRunCount;
				fgRunCount = *readPointer++;
				widthCount += fgRunCount;

				count = 0;
				int colDiff = - col;
				if (colDiff > 0) {
					if (colDiff > fgRunCount)
						colDiff = fgRunCount;

					count = colDiff;
					col += colDiff;
				}

				colDiff = map->_size.x - col;
				if (colDiff > 0) {
					int countDiff = fgRunCount - count;
					if (colDiff > countDiff)
						colDiff = countDiff;

					if (colDiff > 0) {
						byte *dst = (byte *)(drawPointer + count);
						if (mask)
							memset(dst, 0, colDiff);
						else
							memcpy(dst, (readPointer + count), colDiff);
						col += colDiff;
					}
				}

				readPointer += fgRunCount;
				drawPointer += fgRunCount;
			}
		} else {
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH)
					break;

				fgRunCount = *readPointer++;
				widthCount += fgRunCount;

				readPointer += fgRunCount;
			}
		}
	}

	// Compute dirty rect
	int rectX = MAX<int>(drawPoint.x, 0);
	int rectY = MAX<int>(drawPoint.y, 0);
	int rectX2 = MIN<int>(drawPoint.x + SAGA_ISOTILE_WIDTH, map->_size.x);
	int rectY2 = lowBound;
	debugC(3, kDebugTiles, "Rect = (%d,%d,%d,%d)", rectX, rectY, rectX2, rectY2);

#if 0
	Graphics::Surface sur;
	sur.create(map->_size.x, map->_size.y, Graphics::PixelFormat::createFormatCLUT8());
	sur.setPixels(map->_data);
	//sur.debugPrint();
	g_system->copyRectToScreen(sur.getPixels(), sur.pitch, 0, 0, sur.w, sur.h);
	g_system->updateScreen();
#endif
}


void maskTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData) {
	drawTile(map, x, y, height, srcData, true);
}

void TBlit(gPixelMap *dstMap, gPixelMap *srcMap, int32 xpos, int32 ypos) {
	int16 w = srcMap->_size.x;
	int16 h = srcMap->_size.y;
	int32 offset = 0;

	if (ypos < 0) {
		h += ypos;
		offset -= (ypos * w);
		ypos = 0;
	}

	if (xpos < 0) {
		w += xpos;
		offset -= xpos;
		xpos = 0;
	}

	if (w > dstMap->_size.x - xpos)
		w = dstMap->_size.x - xpos;
	if (h > dstMap->_size.y - ypos)
		h = dstMap->_size.y - ypos;
	if (w < 0 || h < 0)
		return;

	int16 dstMod = dstMap->_size.x - w;
	int16 srcMod = srcMap->_size.x - w;

	byte *srcPtr = srcMap->_data + offset;
	byte *dstPtr = dstMap->_data + xpos + ypos * dstMap->_size.x;

	for (int16 y = 0; y < h; y++) {
		for (int16 x = 0; x < w; x++) {
			byte c = *srcPtr++;

			if (c == 0)
				dstPtr++;
			else
				*dstPtr++ = c;
		}
		dstPtr += dstMod;
		srcPtr += srcMod;
	}
}

void TBlit4(gPixelMap *d, gPixelMap *s, int32 x, int32 y) {
	TBlit(d, s, x, y);
}

void compositePixels(gPixelMap *compMap, gPixelMap *sprMap, int32 xpos, int32 ypos, byte *lookup) {
	byte *srcPtr = sprMap->_data;
	byte *dstPtr = compMap->_data + xpos + ypos * compMap->_size.x;
	int16 rowMod = compMap->_size.x - sprMap->_size.x;

	for (int16 y = 0; y < sprMap->_size.y; y++) {
		for (int16 x = 0; x < sprMap->_size.x; x++) {
			byte c = *srcPtr++;

			if (c == 0)
				dstPtr++;
			else
				*dstPtr++ = lookup[ c ];
		}
		dstPtr += rowMod;
	}
}

void compositePixelsRvs(gPixelMap *compMap, gPixelMap *sprMap, int32 xpos, int32 ypos, byte *lookup) {
	byte *srcPtr = sprMap->_data + sprMap->bytes();
	byte *dstPtr = compMap->_data + xpos + (ypos + sprMap->_size.y) * compMap->_size.x;

	int16 rowMod = compMap->_size.x + sprMap->_size.x;

	for (int16 y = 0; y < sprMap->_size.y; y++) {
		dstPtr -= rowMod;

		for (int16 x = 0; x < sprMap->_size.x; x++) {
			byte c = *--srcPtr;

			if (c == 0)
				dstPtr++;
			else
				*dstPtr++ = lookup[c];
		}
	}
}

bool initGraphics() {
	warning("STUB: initGraphics()");
	return false;
}

bool initProcessResources() {
        return true;
}

void termProcessResources() {
}

} // end of namespace Saga2
