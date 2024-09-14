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

#include "ags/engine/ac/global_region.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_version.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/region.h"
#include "ags/engine/ac/room.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/dynobj/cc_region.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/script/script.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

int GetRegionIDAtRoom(int xxx, int yyy) {
	// if the co-ordinates are off the edge of the screen,
	// correct them to be just within
	// this fixes walk-off-screen problems
	xxx = room_to_mask_coord(xxx);
	yyy = room_to_mask_coord(yyy);

	if (_G(loaded_game_file_version) >= kGameVersion_262) { // Version 2.6.2+
		if (xxx >= _GP(thisroom).RegionMask->GetWidth())
			xxx = _GP(thisroom).RegionMask->GetWidth() - 1;
		if (yyy >= _GP(thisroom).RegionMask->GetHeight())
			yyy = _GP(thisroom).RegionMask->GetHeight() - 1;
		if (xxx < 0)
			xxx = 0;
		if (yyy < 0)
			yyy = 0;
	}

	int hsthere = _GP(thisroom).RegionMask->GetPixel(xxx, yyy);
	if (hsthere <= 0 || hsthere >= MAX_ROOM_REGIONS) return 0;
	if (_G(croom)->region_enabled[hsthere] == 0) return 0;
	return hsthere;
}

void SetAreaLightLevel(int area, int brightness) {
	if ((area < 0) || (area > MAX_ROOM_REGIONS))
		quit("!SetAreaLightLevel: invalid region");
	if (brightness < -100) brightness = -100;
	if (brightness > 100) brightness = 100;
	_GP(thisroom).Regions[area].Light = brightness;
	// disable RGB tint for this area
	_GP(thisroom).Regions[area].Tint = 0;
	debug_script_log("Region %d light level set to %d", area, brightness);
}

void SetRegionTint(int area, int red, int green, int blue, int amount, int luminance) {
	if ((area < 0) || (area > MAX_ROOM_REGIONS))
		quit("!SetRegionTint: invalid region");

	if ((red < 0) || (red > 255) || (green < 0) || (green > 255) ||
	        (blue < 0) || (blue > 255)) {
		quit("!SetRegionTint: RGB values must be 0-255");
	}

	// originally the value was passed as 0
	// TODO: find out which versions had this; fixup only for past versions in the future!
	if (amount == 0)
		amount = 100;

	if ((amount < 1) || (amount > 100))
		quit("!SetRegionTint: amount must be 1-100");
	if ((luminance < 0) || (luminance > 100))
		quit("!SetRegionTint: luminance must be 0-100");

	debug_script_log("Region %d tint set to %d,%d,%d", area, red, green, blue);

	/*red -= 100;
	green -= 100;
	blue -= 100;*/

	_GP(thisroom).Regions[area].Tint = (red & 0xFF) |
	                                   ((green & 0xFF) << 8) |
	                                   ((blue & 0XFF) << 16) |
	                                   ((amount & 0xFF) << 24);
	_GP(thisroom).Regions[area].Light = (luminance * 25) / 10;
}

void DisableRegion(int hsnum) {
	if ((hsnum < 0) || (hsnum >= MAX_ROOM_REGIONS))
		quit("!DisableRegion: invalid region specified");

	_G(croom)->region_enabled[hsnum] = 0;
	debug_script_log("Region %d disabled", hsnum);
}

void EnableRegion(int hsnum) {
	if ((hsnum < 0) || (hsnum >= MAX_ROOM_REGIONS))
		quit("!EnableRegion: invalid region specified");

	_G(croom)->region_enabled[hsnum] = 1;
	debug_script_log("Region %d enabled", hsnum);
}

void DisableGroundLevelAreas(int alsoEffects) {
	if ((alsoEffects < 0) || (alsoEffects > 1))
		quit("!DisableGroundLevelAreas: invalid parameter: must be 0 or 1");

	_GP(play).ground_level_areas_disabled = GLED_INTERACTION;

	if (alsoEffects)
		_GP(play).ground_level_areas_disabled |= GLED_EFFECTS;

	debug_script_log("Ground-level areas disabled");
}

void EnableGroundLevelAreas() {
	_GP(play).ground_level_areas_disabled = 0;

	debug_script_log("Ground-level areas re-enabled");
}

void RunRegionInteraction(int regnum, int mood) {
	if ((regnum < 0) || (regnum >= MAX_ROOM_REGIONS))
		quit("!RunRegionInteraction: invalid region speicfied");
	if ((mood < 0) || (mood > 2))
		quit("!RunRegionInteraction: invalid event specified");

	// NOTE: for Regions the mode has specific meanings (NOT verbs):
	// 0 - stands on region, 1 - walks onto region, 2 - walks off region
	const auto obj_evt = ObjectEvent("region%d", regnum,
									 RuntimeScriptValue().SetScriptObject(&_G(scrRegion)[regnum], &_GP(ccDynamicRegion)), mood);

	if (_G(loaded_game_file_version) > kGameVersion_272) {
		run_interaction_script(obj_evt, _GP(thisroom).Regions[regnum].EventHandlers.get(), mood);
	} else {
		run_interaction_event(obj_evt, &_G(croom)->intrRegion[regnum], mood);
	}
}

} // namespace AGS3
