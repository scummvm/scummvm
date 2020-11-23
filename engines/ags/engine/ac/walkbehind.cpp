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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/shared/ac/walkbehind.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/gfx/graphicsdriver.h"
#include "ags/shared/gfx/bitmap.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern RoomStruct thisroom;
extern GameState play;
extern IGraphicsDriver *gfxDriver;


char *walkBehindExists = nullptr;  // whether a WB area is in this column
int *walkBehindStartY = nullptr, *walkBehindEndY = nullptr;
char noWalkBehindsAtAll = 0;
int walkBehindLeft[MAX_WALK_BEHINDS], walkBehindTop[MAX_WALK_BEHINDS];
int walkBehindRight[MAX_WALK_BEHINDS], walkBehindBottom[MAX_WALK_BEHINDS];
IDriverDependantBitmap *walkBehindBitmap[MAX_WALK_BEHINDS];
int walkBehindsCachedForBgNum = 0;
WalkBehindMethodEnum walkBehindMethod = DrawOverCharSprite;
int walk_behind_baselines_changed = 0;

void update_walk_behind_images() {
	int ee, rr;
	int bpp = (thisroom.BgFrames[play.bg_frame].Graphic->GetColorDepth() + 7) / 8;
	Bitmap *wbbmp;
	for (ee = 1; ee < MAX_WALK_BEHINDS; ee++) {
		update_polled_stuff_if_runtime();

		if (walkBehindRight[ee] > 0) {
			wbbmp = BitmapHelper::CreateTransparentBitmap(
				(walkBehindRight[ee] - walkBehindLeft[ee]) + 1,
				(walkBehindBottom[ee] - walkBehindTop[ee]) + 1,
				thisroom.BgFrames[play.bg_frame].Graphic->GetColorDepth());
			int yy, startX = walkBehindLeft[ee], startY = walkBehindTop[ee];
			for (rr = startX; rr <= walkBehindRight[ee]; rr++) {
				for (yy = startY; yy <= walkBehindBottom[ee]; yy++) {
					if (thisroom.WalkBehindMask->GetScanLine(yy)[rr] == ee) {
						for (int ii = 0; ii < bpp; ii++)
							wbbmp->GetScanLineForWriting(yy - startY)[(rr - startX) * bpp + ii] = thisroom.BgFrames[play.bg_frame].Graphic->GetScanLine(yy)[rr * bpp + ii];
					}
				}
			}

			update_polled_stuff_if_runtime();

			if (walkBehindBitmap[ee] != nullptr) {
				gfxDriver->DestroyDDB(walkBehindBitmap[ee]);
			}
			walkBehindBitmap[ee] = gfxDriver->CreateDDBFromBitmap(wbbmp, false);
			delete wbbmp;
		}
	}

	walkBehindsCachedForBgNum = play.bg_frame;
}


void recache_walk_behinds() {
	if (walkBehindExists) {
		free(walkBehindExists);
		free(walkBehindStartY);
		free(walkBehindEndY);
	}

	walkBehindExists = (char *)malloc(thisroom.WalkBehindMask->GetWidth());
	walkBehindStartY = (int *)malloc(thisroom.WalkBehindMask->GetWidth() * sizeof(int));
	walkBehindEndY = (int *)malloc(thisroom.WalkBehindMask->GetWidth() * sizeof(int));
	noWalkBehindsAtAll = 1;

	int ee, rr, tmm;
	const int NO_WALK_BEHIND = 100000;
	for (ee = 0; ee < MAX_WALK_BEHINDS; ee++) {
		walkBehindLeft[ee] = NO_WALK_BEHIND;
		walkBehindTop[ee] = NO_WALK_BEHIND;
		walkBehindRight[ee] = 0;
		walkBehindBottom[ee] = 0;

		if (walkBehindBitmap[ee] != nullptr) {
			gfxDriver->DestroyDDB(walkBehindBitmap[ee]);
			walkBehindBitmap[ee] = nullptr;
		}
	}

	update_polled_stuff_if_runtime();

	// since this is an 8-bit memory bitmap, we can just use direct
	// memory access
	if ((!thisroom.WalkBehindMask->IsLinearBitmap()) || (thisroom.WalkBehindMask->GetColorDepth() != 8))
		quit("Walk behinds bitmap not linear");

	for (ee = 0; ee < thisroom.WalkBehindMask->GetWidth(); ee++) {
		walkBehindExists[ee] = 0;
		for (rr = 0; rr < thisroom.WalkBehindMask->GetHeight(); rr++) {
			tmm = thisroom.WalkBehindMask->GetScanLine(rr)[ee];
			//tmm = _getpixel(thisroom.WalkBehindMask,ee,rr);
			if ((tmm >= 1) && (tmm < MAX_WALK_BEHINDS)) {
				if (!walkBehindExists[ee]) {
					walkBehindStartY[ee] = rr;
					walkBehindExists[ee] = tmm;
					noWalkBehindsAtAll = 0;
				}
				walkBehindEndY[ee] = rr + 1;  // +1 to allow bottom line of screen to work

				if (ee < walkBehindLeft[tmm]) walkBehindLeft[tmm] = ee;
				if (rr < walkBehindTop[tmm]) walkBehindTop[tmm] = rr;
				if (ee > walkBehindRight[tmm]) walkBehindRight[tmm] = ee;
				if (rr > walkBehindBottom[tmm]) walkBehindBottom[tmm] = rr;
			}
		}
	}

	if (walkBehindMethod == DrawAsSeparateSprite) {
		update_walk_behind_images();
	}
}

} // namespace AGS3
