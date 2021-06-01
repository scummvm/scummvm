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

#include "common/debug.h"

#include "saga2/std.h"
#include "saga2/saga2.h"
#include "saga2/blitters.h"
#include "saga2/gdraw.h"

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
	//g_vm->_render->addDirtyRect(Common::Rect(rectX, rectY, rectX2, rectY2));
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
