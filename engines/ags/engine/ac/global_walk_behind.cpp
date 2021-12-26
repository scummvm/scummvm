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

#include "ags/engine/ac/global_walk_behind.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/walk_behind.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/globals.h"

namespace AGS3 {

void SetWalkBehindBase(int wa, int bl) {
	if ((wa < 1) || (wa >= MAX_WALK_BEHINDS))
		quit("!SetWalkBehindBase: invalid walk-behind area specified");

	if (bl != _G(croom)->walkbehind_base[wa]) {
		_G(walk_behind_baselines_changed) = 1;
		invalidate_cached_walkbehinds();
		_G(croom)->walkbehind_base[wa] = bl;
		debug_script_log("Walk-behind %d baseline changed to %d", wa, bl);
	}
}

} // namespace AGS3
