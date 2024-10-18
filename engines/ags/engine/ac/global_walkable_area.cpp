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

#include "ags/engine/ac/global_walkable_area.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

int GetScalingAt(int x, int y) {
	int onarea = get_walkable_area_pixel(x, y);
	if (onarea < 0)
		return 100;

	return get_area_scaling(onarea, x, y);
}

void SetAreaScaling(int area, int min, int max) {
	if ((area < 0) || (area >= MAX_WALK_AREAS))
		quit("!SetAreaScaling: invalid walkalbe area");

	if (min > max)
		quit("!SetAreaScaling: min > max");

	if ((min < 5) || (max < 5) || (min > 200) || (max > 200))
		quit("!SetAreaScaling: min and max must be in range 5-200");

	// the values are stored differently
	min -= 100;
	max -= 100;

	if (min == max) {
		_GP(thisroom).WalkAreas[area].ScalingFar = min;
		_GP(thisroom).WalkAreas[area].ScalingNear = NOT_VECTOR_SCALED;
	} else {
		_GP(thisroom).WalkAreas[area].ScalingFar = min;
		_GP(thisroom).WalkAreas[area].ScalingNear = max;
	}
}

void RemoveWalkableArea(int areanum) {
	if ((areanum < 1) | (areanum > 15))
		quit("!RemoveWalkableArea: invalid area number specified (1-15).");
	_GP(play).walkable_areas_on[areanum] = 0;
	redo_walkable_areas();
	debug_script_log("Walkable area %d removed", areanum);
}

void RestoreWalkableArea(int areanum) {
	if ((areanum < 1) | (areanum > 15))
		quit("!RestoreWalkableArea: invalid area number specified (1-15).");
	_GP(play).walkable_areas_on[areanum] = 1;
	redo_walkable_areas();
	debug_script_log("Walkable area %d restored", areanum);
}

int GetWalkableAreaAtScreen(int x, int y) {
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(x, y);
	if (vpt.second < 0)
		return 0;
	return GetWalkableAreaAtRoom(vpt.first.X, vpt.first.Y);
}

int GetWalkableAreaAtRoom(int x, int y) {
	int area = get_walkable_area_pixel(x, y);
	// IMPORTANT: disabled walkable areas are actually erased completely from the mask;
	// see: RemoveWalkableArea() and RestoreWalkableArea().
	return (area >= 0 && area < MAX_WALK_AREAS) ? area : 0;
}

} // namespace AGS3
