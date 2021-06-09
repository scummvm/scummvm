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

#include "common/debug.h"
#include "graphics/surface.h"

#include "saga2/std.h"
#include "saga2/gdraw.h"

namespace Saga2 {

void _BltPixels(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height) {
	uint8 *src, *dst;
	for (uint y = 0; y < height; y++) {
		src = srcPtr + srcMod * y;
		dst = dstPtr + dstMod * y;
		for (uint x = 0; x < width; x++) {
			*dst++ = *src++;
		}
	}
}

void _BltPixelsT(uint8 *srcPtr, uint32 srcMod, uint8 *dstPtr, uint32 dstMod, uint32 width, uint32 height) {
	uint8 *src, *dst;
	for (uint y = 0; y < height; y++) {
		src = srcPtr + srcMod * y;
		dst = dstPtr + dstMod * y;
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

//void drawTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData) {
//	warning("STUB: drawTile()");
//}

void drawTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData) {
	warning("STUB: drawTile()");
	const byte *tilePointer;
	const byte *readPointer;
	byte *drawPointer;
	Point32 drawPoint;
	int widthCount = 0;
	int row, col, count, lowBound;
	int bgRunCount;
	int fgRunCount;
	const int32 SAGA_ISOTILE_WIDTH = 32;
	Point16 point(x, y);


//	if (tileIndex >= _tilesTable.size()) {
//		error("IsoMap::drawTile wrong tileIndex");
//	}


	if (point.x + SAGA_ISOTILE_WIDTH < 0) {
		return;
	}

	if (point.x - SAGA_ISOTILE_WIDTH >= map->size.x) {
		return;
	}

	if ((height <= 8) || (height > 64)) {
		return;
	}

	tilePointer = srcData;

	drawPoint = point;

	drawPoint.y -= height;

	if (drawPoint.y >= map->size.y) {
		return;
	}

#if 0
	if (location != NULL) {
		if (location->z <= -16) {
			if (location->z <= -48) {
				if (location->u() < -THRESH8 || location->v() < -THRESH8) {
					return;
				}
			} else {
				if (location->u() < THRESH0 || location->v() < THRESH0) {
					return;
				}
			}
		} else {
			if (location->z >= 16) {
				return;
			} else {
				switch (_tilesTable[tileIndex].getMaskRule()) {
				case kMaskRuleNever:
					return;
				case kMaskRuleAlways:
				default:
					break;
				case kMaskRuleUMIN:
					if (location->u() < THRESH0) {
						return;
					}
					break;
				case kMaskRuleUMID:
					if (location->u() < THRESH8) {
						return;
					}
					break;
				case kMaskRuleUMAX:
					if (location->u() < THRESH16) {
						return;
					}
					break;
				case kMaskRuleVMIN:
					if (location->v() < THRESH0) {
						return;
					}
					break;
				case kMaskRuleVMID:
					if (location->v() < THRESH8) {
						return;
					}
					break;
				case kMaskRuleVMAX:
					if (location->v() < THRESH16) {
						return;
					}
					break;
				case kMaskRuleYMIN:
					if (location->uv() < THRESH0 * 2) {
						return;
					}
					break;
				case kMaskRuleYMID:
					if (location->uv() < THRESH8 * 2) {
						return;
					}
					break;
				case kMaskRuleYMAX:
					if (location->uv() < THRESH16 * 2) {
						return;
					}
					break;
				case kMaskRuleUVMAX:
					if (location->u() < THRESH16 && location->v() < THRESH16) {
						return;
					}
					break;
				case kMaskRuleUVMIN:
					if (location->u() < THRESH0 || location->v() < THRESH0) {
						return;
					}
					break;
				case kMaskRuleUorV:
					if (location->u() < THRESH8 && location->v() < THRESH8) {
						return;
					}
					break;
				case kMaskRuleUandV:
					if (location->u() < THRESH8 || location->v() < THRESH8) {
						return;
					}
					break;
				}
			}
		}
	}
#endif

	readPointer = tilePointer;
	lowBound = MIN((int)(drawPoint.y + height), (int)map->size.y);
	for (row = drawPoint.y; row < lowBound; row++) {
		widthCount = 0;
		if (row >= 0) {
			drawPointer = map->data + drawPoint.x + (row * map->size.x);
			col = drawPoint.x;
			for (;;) {
				bgRunCount = *readPointer++;
				widthCount += bgRunCount;
				if (widthCount >= SAGA_ISOTILE_WIDTH) {
					break;
				}

				drawPointer += bgRunCount;
				col += bgRunCount;
				fgRunCount = *readPointer++;
				widthCount += fgRunCount;

				count = 0;
				int colDiff = - col;
				if (colDiff > 0) {
					if (colDiff > fgRunCount) {
						colDiff = fgRunCount;
					}
					count = colDiff;
					col += colDiff;
				}

				colDiff = map->size.x - col;
				if (colDiff > 0) {
					int countDiff = fgRunCount - count;
					if (colDiff > countDiff) {
						colDiff = countDiff;
					}
					if (colDiff > 0) {
						byte *dst = (byte *)(drawPointer + count);
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
				if (widthCount >= SAGA_ISOTILE_WIDTH) {
					break;
				}

				fgRunCount = *readPointer++;
				widthCount += fgRunCount;

				readPointer += fgRunCount;
			}
		}
	}

	// Compute dirty rect
	int rectX = MAX<int>(drawPoint.x, 0);
	int rectY = MAX<int>(drawPoint.y, 0);
	int rectX2 = MIN<int>(drawPoint.x + SAGA_ISOTILE_WIDTH, map->size.x);
	int rectY2 = lowBound;
	debugC(3, kDebugTiles, "Rect = (%d,%d,%d,%d)", rectX, rectY, rectX2, rectY2);

	// FIXME: Debug purposes-code for displaying things on the screen
	// updateScreen should not be called here
	warning("FIXME: drawTile");
	Graphics::Surface sur;
	sur.create(map->size.x, map->size.y, Graphics::PixelFormat::createFormatCLUT8());
	sur.setPixels(map->data);
	//sur.debugPrint();
	g_system->copyRectToScreen(sur.getPixels(), sur.pitch, 0, 0, sur.w, sur.h);
	g_system->updateScreen();
	//g_vm->_render->addDirtyRect(Common::Rect(rectX, rectY, rectX2, rectY2));
}


void maskTile(gPixelMap *map, int32 x, int32 y, int32 height, uint8 *srcData) {
	warning("STUB: maskTile()");
}

void TBlit(gPixelMap *dstMap, gPixelMap *srcMap, int xpos, int ypos) {
	byte			*srcPtr,
					*dstPtr;
	int16			srcMod,
					dstMod;
	int16			x, y, w, h;
	int32			offset = 0;

	w = srcMap->size.x;
	h = srcMap->size.y;

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

	if (w > dstMap->size.x - xpos)
		w = dstMap->size.x - xpos;
	if (h > dstMap->size.y - ypos)
		h = dstMap->size.y - ypos;
	if (w < 0 || h < 0)
		return;

	dstMod = dstMap->size.x - w;
	srcMod = srcMap->size.x - w;

	srcPtr = srcMap->data + offset;
	dstPtr = dstMap->data + xpos + ypos * dstMap->size.x;

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
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

void compositePixels(gPixelMap *compMap, gPixelMap *sprMap, int xpos, int ypos, byte *lookup) {
	byte			*srcPtr,
					*dstPtr;
	int16			rowMod;
	int16			x, y;

		//	Blit the temp map onto the composite map

	srcPtr	= sprMap->data;
	dstPtr	= compMap->data + xpos + ypos * compMap->size.x;
	rowMod = compMap->size.x - sprMap->size.x;

	for (y = 0; y < sprMap->size.y; y++) {
		for (x = 0; x < sprMap->size.x; x++) {
			byte c = *srcPtr++;

			if (c == 0)
				dstPtr++;
			else
				*dstPtr++ = lookup[ c ];
		}
		dstPtr += rowMod;
	}
}

void compositePixelsRvs(gPixelMap *compMap, gPixelMap *sprMap, int xpos, int ypos, byte *lookup) {
	byte			*srcPtr,
					*dstPtr;
	int16			rowMod;
	int16			x, y;

		//	Blit the temp map onto the composite map

	srcPtr	= sprMap->data + sprMap->bytes();
	dstPtr	= compMap->data	+ xpos + (ypos + sprMap->size.y) * compMap->size.x;

	rowMod = compMap->size.x + sprMap->size.x;

	for (y = 0; y < sprMap->size.y; y++) {
		dstPtr -= rowMod;

		for (x = 0; x < sprMap->size.x; x++) {
			byte c = *--srcPtr;

			if (c == 0)
				dstPtr++;
			else
				*dstPtr++ = lookup[ c ];
		}
	}
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
