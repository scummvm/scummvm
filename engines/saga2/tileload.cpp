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
#include "saga2/input.h"
#include "saga2/cmisc.h"
#include "saga2/tcoords.h"
#include "saga2/tile.h"
#include "saga2/objects.h"
#include "saga2/tileload.h"
#include "saga2/oncall.h"
#include "common/debug.h"

namespace Saga2 {

const uint16 tileBankCount = 25;

const uint32            tileImageID     = MKTAG('T', 'I', 'L',  0);

/* ===================================================================== *
   Prototypes
 * ===================================================================== */

//  Assembly-language function to copy pixel to SVGA

#if DEBUG
void TPLine(const TilePoint &start, const TilePoint &stop);
#endif


/* ===================================================================== *
   Imports
 * ===================================================================== */

extern gPixelMap            tileDrawMap;

extern int16                currentMapNum;

extern hResource           *objResFile;

/* ===================================================================== *
   Tile structure management
 * ===================================================================== */

extern hResContext          *tileRes;       // tile resource handle

/* ===================================================================== *
   Debugging displays
 * ===================================================================== */

void sprintBA(char buf[], BitArray *x);

/* ===================================================================== *
   Debugging displays
 * ===================================================================== */


static byte *tileResLoad(hResID i, bool asynch = FALSE) {
	if (tileRes)
		return tileRes->loadResource(i, "tile image bank");
	else
		return nullptr;
}


//LoadOnCall<UByteHandle> tileImageBanks(64, tileResLoad, tileImageID);
HandleArray tileImageBanks(64, tileResLoad, tileImageID);

/* ===================================================================== *
   Debugging displays
 * ===================================================================== */


void showBanks() {
}

void showBank(BankBits bb) {
	WriteStatusF(4, "%8.8X:%8.8X  ", bb.b[1], bb.b[0]);
}

void sprintBA(char buf[], BitArray *x) {
	uint32 v;
	if (x->currentSize())
		for (int16 i = (uint16) x->currentSize() / 32 ; i >= 0; i--) {
			v = x->getChunk(i);
			sprintf(buf, "%8.8X", v);
			if (i) sprintf(buf, ",");
		}
}

/* ===================================================================== *
   load tile resources as needed
 * ===================================================================== */

void initTileBank(int16 bankNum) {
	byte *th;
	th = tileImageBanks[bankNum];
}


void RHeapsAMess(void);

void freeAllTileBanks(void) {
	tileImageBanks.flush();
	tileImageBanks.flush();
	RHeapsAMess();
}

void unlockAllTileBanks(void) {
	tileImageBanks.flush();
}

void freeTileBank(int16) {
}

void updateHandleRefs(const TilePoint &) { //, StandingTileInfo *stiResult )
	tileImageBanks.flush();
}

#if DEBUG
void tileFault(int bank, int num) {
	WriteStatusF(0, "Bad Tile: %d/%d", bank, num);
}
#else
void tileFault(int, int) {
}
#endif

void initTileBanks(void) {
	for (int16 i = 0; i < tileBankCount; i++)
		initTileBank(i);
}

void drawPlatform(
    Platform        **pList,                // platforms to draw
    Point16         screenPos,              // screen position
    int16           uOrg,                   // for TAG search
    int16           vOrg) {                 // for TAG search
	int16           u, v;

	int16           right = tileDrawMap.size.x,
	                bottom = tileDrawMap.size.y;

	Point16         tilePos;

	int16           x = screenPos.x,
	                x2 = x / tileDX;
	int16           length = 1;
	TilePoint       origin(uOrg, vOrg, 0);

	tilePos.y = screenPos.y - (platformWidth - 1) * tileHeight;

	u = platformWidth - 1;
	v = platformWidth - 1;

	debugC(3, kDebugTiles, "drawPlatform: right = %d, bottom = %d, x = %d, x2 = %d, origin = %d,%d, tilePos.y=%d, u,v = %d,%d", right, bottom, x, x2, origin.u, origin.v,
	       tilePos.y, u, v);

	for (int row = 0; row < 15; row++) {
		if (tilePos.y > 0) {
			int16   col = 0;
			TilePoint pCoords(u, v, 0);

			tilePos.x = x;

			if (length > x2) {
				int16   offset = (length - x2) >> 1;

				pCoords.u += offset;
				pCoords.v -= offset;
				offset <<= 1;
				col += offset;
				tilePos.x += tileDX * offset;
			}

			for (;
			        col < length && tilePos.x <= right;
			        col += 2,
			        pCoords.u++,
			        pCoords.v--,
			        tilePos.x += tileWidth
			    ) {
				Platform    **pGet;

				if (tilePos.x < 0) continue;

				for (pGet = pList; *pGet; pGet++) {
					Platform    &p = **pGet;
					int16       h,
					            y;
					TileInfo    *ti;
					uint8       *imageData;
					int16       trFlags;

					ti =    p.fetchTile(
					            currentMapNum,
					            pCoords,
					            origin,
					            &imageData,
					            h,
					            trFlags);
					if (ti == NULL) continue;

					y = tilePos.y - h;

					if (ti->attrs.height > 0
					        && y < bottom + ti->attrs.height - 1) {
						drawTile(&tileDrawMap,
						         tilePos.x, y, ti->attrs.height,
						         imageData);
					}
				}
			}
		}

		if (row < 7) {
			x -= tileDX;
			x2++;
			length += 2;
			u--;
		} else {
			x += tileDX;
			x2--;
			length -= 2;
			v--;
		}

		tilePos.y += tileDY;
	}
}

} // end of namespace Saga2
