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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ags/engine/ac/walk_behind.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

void update_walk_behind_images() {
	int ee, rr;
	int bpp = (_GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic->GetColorDepth() + 7) / 8;
	Bitmap *wbbmp;
	for (ee = 1; ee < MAX_WALK_BEHINDS; ee++) {
		update_polled_stuff_if_runtime();

		if (_G(walkBehindRight)[ee] > 0) {
			wbbmp = BitmapHelper::CreateTransparentBitmap(
			            (_G(walkBehindRight)[ee] - _G(walkBehindLeft)[ee]) + 1,
			            (_G(walkBehindBottom)[ee] - _G(walkBehindTop)[ee]) + 1,
			            _GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic->GetColorDepth());
			int yy, startX = _G(walkBehindLeft)[ee], startY = _G(walkBehindTop)[ee];
			for (rr = startX; rr <= _G(walkBehindRight)[ee]; rr++) {
				for (yy = startY; yy <= _G(walkBehindBottom)[ee]; yy++) {
					if (_GP(thisroom).WalkBehindMask->GetScanLine(yy)[rr] == ee) {
						for (int ii = 0; ii < bpp; ii++)
							wbbmp->GetScanLineForWriting(yy - startY)[(rr - startX) * bpp + ii] = _GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic->GetScanLine(yy)[rr * bpp + ii];
					}
				}
			}

			update_polled_stuff_if_runtime();

			if (_G(walkBehindBitmap)[ee] != nullptr) {
				_G(gfxDriver)->DestroyDDB(_G(walkBehindBitmap)[ee]);
			}
			_G(walkBehindBitmap)[ee] = _G(gfxDriver)->CreateDDBFromBitmap(wbbmp, false);
			delete wbbmp;
		}
	}

	_G(walkBehindsCachedForBgNum) = _GP(play).bg_frame;
}


void recache_walk_behinds() {
	if (_G(walkBehindExists)) {
		free(_G(walkBehindExists));
		free(_G(walkBehindStartY));
		free(_G(walkBehindEndY));
	}

	_G(walkBehindExists) = (char *)malloc(_GP(thisroom).WalkBehindMask->GetWidth());
	_G(walkBehindStartY) = (int *)malloc(_GP(thisroom).WalkBehindMask->GetWidth() * sizeof(int));
	_G(walkBehindEndY) = (int *)malloc(_GP(thisroom).WalkBehindMask->GetWidth() * sizeof(int));
	_G(noWalkBehindsAtAll) = 1;

	int ee, rr, tmm;
	const int NO_WALK_BEHIND = 100000;
	for (ee = 0; ee < MAX_WALK_BEHINDS; ee++) {
		_G(walkBehindLeft)[ee] = NO_WALK_BEHIND;
		_G(walkBehindTop)[ee] = NO_WALK_BEHIND;
		_G(walkBehindRight)[ee] = 0;
		_G(walkBehindBottom)[ee] = 0;

		if (_G(walkBehindBitmap)[ee] != nullptr) {
			_G(gfxDriver)->DestroyDDB(_G(walkBehindBitmap)[ee]);
			_G(walkBehindBitmap)[ee] = nullptr;
		}
	}

	update_polled_stuff_if_runtime();

	for (ee = 0; ee < _GP(thisroom).WalkBehindMask->GetWidth(); ee++) {
		_G(walkBehindExists)[ee] = 0;
		for (rr = 0; rr < _GP(thisroom).WalkBehindMask->GetHeight(); rr++) {
			tmm = _GP(thisroom).WalkBehindMask->GetScanLine(rr)[ee];
			//tmm = _getpixel(_GP(thisroom).WalkBehindMask,ee,rr);
			if ((tmm >= 1) && (tmm < MAX_WALK_BEHINDS)) {
				if (!_G(walkBehindExists)[ee]) {
					_G(walkBehindStartY)[ee] = rr;
					_G(walkBehindExists)[ee] = tmm;
					_G(noWalkBehindsAtAll) = 0;
				}
				_G(walkBehindEndY)[ee] = rr + 1;  // +1 to allow bottom line of screen to work

				if (ee < _G(walkBehindLeft)[tmm]) _G(walkBehindLeft)[tmm] = ee;
				if (rr < _G(walkBehindTop)[tmm]) _G(walkBehindTop)[tmm] = rr;
				if (ee > _G(walkBehindRight)[tmm]) _G(walkBehindRight)[tmm] = ee;
				if (rr > _G(walkBehindBottom)[tmm]) _G(walkBehindBottom)[tmm] = rr;
			}
		}
	}

	if (_G(walkBehindMethod) == DrawAsSeparateSprite) {
		update_walk_behind_images();
	}
}

} // namespace AGS3
