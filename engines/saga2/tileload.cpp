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

#include "saga2/saga2.h"
#include "saga2/idtypes.h"
#include "saga2/tile.h"
#include "saga2/hresmgr.h"
#include "saga2/oncall.h"
#include "saga2/blitters.h"

namespace Saga2 {

const uint16 tileBankCount = 64;

const uint32 tileImageID = MKTAG('T', 'I', 'L',  0);


extern hResource           *objResFile;
extern hResContext          *tileRes;       // tile resource handle

static byte *tileResLoad(hResID i, bool asynch = false) {
	if (tileRes)
		return tileRes->loadResource(i, "tile image bank");
	else
		return nullptr;
}

void freeAllTileBanks() {
	g_vm->_tileImageBanks->flush();
}

void updateHandleRefs(const TilePoint &) { //, StandingTileInfo *stiResult )
	g_vm->_tileImageBanks->flush();
}

void initTileBanks() {
	g_vm->_tileImageBanks = new HandleArray(tileBankCount, tileResLoad, tileImageID);
}

void drawPlatform(
	gPixelMap &drawMap,
    Platform        **pList,                // platforms to draw
    Point16         screenPos,              // screen position
    int16           uOrg,                   // for TAG search
    int16           vOrg) {                 // for TAG search

	int16           right = drawMap.size.x,
	                bottom = drawMap.size.y;

	Point16         tilePos;

	int16           x = screenPos.x,
	                x2 = x / kTileDX;
	int16           length = 1;
	TilePoint       origin(uOrg, vOrg, 0);

	tilePos.y = screenPos.y - (kPlatformWidth - 1) * kTileHeight;

	int16 u = kPlatformWidth - 1;
	int16 v = kPlatformWidth - 1;

	debugC(3, kDebugTiles, "drawPlatform: right = %d, bottom = %d, x = %d, x2 = %d, origin = %d,%d, tilePos.y=%d, u,v = %d,%d", right, bottom, x, x2, origin.u, origin.v,
	       tilePos.y, u, v);

	for (int row = 0; row < 15; row++) {
		if (tilePos.y > 0) {
			int16 col = 0;
			TilePoint pCoords(u, v, 0);

			tilePos.x = x;

			if (length > x2) {
				int16 offset = (length - x2) >> 1;

				pCoords.u += offset;
				pCoords.v -= offset;
				offset <<= 1;
				col += offset;
				tilePos.x += kTileDX * offset;
			}

			for (;
			        col < length && tilePos.x <= right;
			        col += 2,
			        pCoords.u++,
			        pCoords.v--,
			        tilePos.x += kTileWidth) {

				if (tilePos.x < 0)
					continue;

				for (Platform **pGet = pList; *pGet; pGet++) {
					Platform &p = **pGet;
					int16 h;
					uint8 *imageData;
					int16 trFlags;

					TileInfo *ti = p.fetchTile(g_vm->_currentMapNum, pCoords, origin, &imageData, h, trFlags);
					if (ti == nullptr)
						continue;

					int16 y = tilePos.y - h;

					if (ti->attrs.height > 0 && y < bottom + ti->attrs.height - 1) {
						drawTile(&drawMap, tilePos.x, y, ti->attrs.height, imageData);
					}
				}
			}
		}

		if (row < 7) {
			x -= kTileDX;
			x2++;
			length += 2;
			u--;
		} else {
			x += kTileDX;
			x2--;
			length -= 2;
			v--;
		}

		tilePos.y += kTileDY;
	}
}

} // end of namespace Saga2
