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

#include "common/std/algorithm.h"
#include "ags/engine/ac/walk_behind.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

// Generates walk-behinds as separate sprites
void walkbehinds_generate_sprites() {
	const Bitmap *mask = _GP(thisroom).WalkBehindMask.get();
	const Bitmap *bg = _GP(thisroom).BgFrames[_GP(play).bg_frame].Graphic.get();

	const int coldepth = bg->GetColorDepth();
	Bitmap wbbmp; // temp buffer
	// Iterate through walk-behinds and generate a texture for each of them
	for (int wb = 1 /* 0 is "no area" */; wb < MAX_WALK_BEHINDS; ++wb) {
		const Rect pos = _G(walkBehindAABB)[wb];
		if (pos.Right > 0) {
			wbbmp.CreateTransparent(pos.GetWidth(), pos.GetHeight(), coldepth);
			// Copy over all solid pixels belonging to this WB area
			const int sx = pos.Left, ex = pos.Right, sy = pos.Top, ey = pos.Bottom;
			for (int y = sy; y <= ey; ++y) {
				const uint8_t *check_line = mask->GetScanLine(y);
				const uint8_t *src_line = bg->GetScanLine(y);
				uint8_t *dst_line = wbbmp.GetScanLineForWriting(y - sy);
				for (int x = sx; x <= ex; ++x) {
					if (check_line[x] != wb) continue;
					switch (coldepth) {
					case 8:
						dst_line[(x - sx)] = src_line[x];
						break;
					case 16:
						reinterpret_cast<uint16_t *>(dst_line)[(x - sx)] =
							reinterpret_cast<const uint16_t *>(src_line)[x];
						break;
					case 32:
						reinterpret_cast<uint32_t *>(dst_line)[(x - sx)] =
							reinterpret_cast<const uint32_t *>(src_line)[x];
						break;
					default: assert(0); break;
					}
				}
			}
			// Add to walk-behinds image list
			add_walkbehind_image(wb, &wbbmp, pos.Left, pos.Top);
		}
	}

	_G(walkBehindsCachedForBgNum) = _GP(play).bg_frame;
}

// Edits the given game object's sprite, cutting out pixels covered by walk-behinds;
// returns whether any pixels were updated;
bool walkbehinds_cropout(Bitmap *sprit, int sprx, int spry, int basel) {
	if (_G(noWalkBehindsAtAll))
		return false;

	const int maskcol = sprit->GetMaskColor();
	const int spcoldep = sprit->GetColorDepth();

	bool pixels_changed = false;
	// pass along the sprite's pixels, but skip those that lie outside the mask
	for (int x = MAX(0, 0 - sprx);
		(x < sprit->GetWidth()) && (x + sprx < _GP(thisroom).WalkBehindMask->GetWidth()); ++x) {
		// select the WB column at this x
		const auto &wbcol = _G(walkBehindCols)[x + sprx];
		// skip if no area, or sprite lies outside of all areas in this column
		if ((!wbcol.Exists) ||
			(wbcol.Y2 <= spry) ||
			(wbcol.Y1 > spry + sprit->GetHeight()))
			continue;

		// ensure we only check within the valid areas (between Y1 and Y2)
		// we assume that Y1 and Y2 are always within the mask
		for (int y = MAX(0, wbcol.Y1 - spry);
				(y < sprit->GetHeight()) && (y + spry < wbcol.Y2); ++y) {
			const int wb = _GP(thisroom).WalkBehindMask->GetScanLine(y + spry)[x + sprx];
			if (wb < 1) continue; // "no area"
			if (_G(croom)->walkbehind_base[wb] <= basel) continue;

			pixels_changed = true;
			uint8_t *dst_line = sprit->GetScanLineForWriting(y);
			switch (spcoldep) {
			case 8:
				dst_line[x] = maskcol;
				break;
			case 16:
				reinterpret_cast<uint16_t *>(dst_line)[x] = maskcol;
				break;
			case 32:
				reinterpret_cast<uint32_t *>(dst_line)[x] = maskcol;
				break;
			default:
				assert(0);
				break;
			}
		}
	}
	return pixels_changed;
}

void walkbehinds_recalc() {
	// Reset all data
	_G(walkBehindCols).clear();
	for (int wb = 0; wb < MAX_WALK_BEHINDS; ++wb) {
		_G(walkBehindAABB)[wb] = Rect(INT32_MAX, INT32_MAX, INT32_MIN, INT32_MIN);
	}
	_G(noWalkBehindsAtAll) = true;

	// Recalculate everything; note that mask is always 8-bit
	const Bitmap *mask = _GP(thisroom).WalkBehindMask.get();
	_G(walkBehindCols).resize(mask->GetWidth());
	for (int col = 0; col < mask->GetWidth(); ++col) {
		auto &wbcol = _G(walkBehindCols)[col];
		for (int y = 0; y < mask->GetHeight(); ++y) {
			int wb = mask->GetScanLine(y)[col];
			// Valid areas start with index 1, 0 = no area
			if ((wb >= 1) && (wb < MAX_WALK_BEHINDS)) {
				if (!wbcol.Exists) {
					wbcol.Y1 = y;
					wbcol.Exists = true;
					_G(noWalkBehindsAtAll) = false;
				}
				wbcol.Y2 = y + 1; // +1 to allow bottom line of screen to work (CHECKME??)
				// resize the bounding rect
				_G(walkBehindAABB)[wb].Left = MIN(col, _G(walkBehindAABB)[wb].Left);
				_G(walkBehindAABB)[wb].Top = MIN(y, _G(walkBehindAABB)[wb].Top);
				_G(walkBehindAABB)[wb].Right = MAX(col, _G(walkBehindAABB)[wb].Right);
				_G(walkBehindAABB)[wb].Bottom = MAX(y, _G(walkBehindAABB)[wb].Bottom);
			}
		}
	}
	_G(walkBehindsCachedForBgNum) = -1;
}

} // namespace AGS3
