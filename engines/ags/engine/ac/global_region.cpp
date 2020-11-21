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

#include "ac/global_region.h"
#include "ac/common.h"
#include "ac/game_version.h"
#include "ac/region.h"
#include "ac/room.h"
#include "ac/roomstatus.h"
#include "debug/debug_log.h"
#include "game/roomstruct.h"
#include "gfx/bitmap.h"
#include "script/script.h"


using namespace AGS::Common;

extern RoomStruct thisroom;
extern RoomStatus *croom;
extern const char *evblockbasename;
extern int evblocknum;

int GetRegionIDAtRoom(int xxx, int yyy) {
	// if the co-ordinates are off the edge of the screen,
	// correct them to be just within
	// this fixes walk-off-screen problems
	xxx = room_to_mask_coord(xxx);
	yyy = room_to_mask_coord(yyy);

	if (loaded_game_file_version >= kGameVersion_262) { // Version 2.6.2+
		if (xxx >= thisroom.RegionMask->GetWidth())
			xxx = thisroom.RegionMask->GetWidth() - 1;
		if (yyy >= thisroom.RegionMask->GetHeight())
			yyy = thisroom.RegionMask->GetHeight() - 1;
		if (xxx < 0)
			xxx = 0;
		if (yyy < 0)
			yyy = 0;
	}

	int hsthere = thisroom.RegionMask->GetPixel(xxx, yyy);
	if (hsthere <= 0 || hsthere >= MAX_ROOM_REGIONS) return 0;
	if (croom->region_enabled[hsthere] == 0) return 0;
	return hsthere;
}

void SetAreaLightLevel(int area, int brightness) {
	if ((area < 0) || (area > MAX_ROOM_REGIONS))
		quit("!SetAreaLightLevel: invalid region");
	if (brightness < -100) brightness = -100;
	if (brightness > 100) brightness = 100;
	thisroom.Regions[area].Light = brightness;
	// disable RGB tint for this area
	thisroom.Regions[area].Tint  = 0;
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

	thisroom.Regions[area].Tint = (red & 0xFF) |
	                              ((green & 0xFF) << 8) |
	                              ((blue & 0XFF) << 16) |
	                              ((amount & 0xFF) << 24);
	thisroom.Regions[area].Light = (luminance * 25) / 10;
}

void DisableRegion(int hsnum) {
	if ((hsnum < 0) || (hsnum >= MAX_ROOM_REGIONS))
		quit("!DisableRegion: invalid region specified");

	croom->region_enabled[hsnum] = 0;
	debug_script_log("Region %d disabled", hsnum);
}

void EnableRegion(int hsnum) {
	if ((hsnum < 0) || (hsnum >= MAX_ROOM_REGIONS))
		quit("!EnableRegion: invalid region specified");

	croom->region_enabled[hsnum] = 1;
	debug_script_log("Region %d enabled", hsnum);
}

void DisableGroundLevelAreas(int alsoEffects) {
	if ((alsoEffects < 0) || (alsoEffects > 1))
		quit("!DisableGroundLevelAreas: invalid parameter: must be 0 or 1");

	play.ground_level_areas_disabled = GLED_INTERACTION;

	if (alsoEffects)
		play.ground_level_areas_disabled |= GLED_EFFECTS;

	debug_script_log("Ground-level areas disabled");
}

void EnableGroundLevelAreas() {
	play.ground_level_areas_disabled = 0;

	debug_script_log("Ground-level areas re-enabled");
}

void RunRegionInteraction(int regnum, int mood) {
	if ((regnum < 0) || (regnum >= MAX_ROOM_REGIONS))
		quit("!RunRegionInteraction: invalid region speicfied");
	if ((mood < 0) || (mood > 2))
		quit("!RunRegionInteraction: invalid event specified");

	// We need a backup, because region interactions can run
	// while another interaction (eg. hotspot) is in a Wait
	// command, and leaving our basename would call the wrong
	// script later on
	const char *oldbasename = evblockbasename;
	int   oldblocknum = evblocknum;

	evblockbasename = "region%d";
	evblocknum = regnum;

	if (thisroom.Regions[regnum].EventHandlers != nullptr) {
		run_interaction_script(thisroom.Regions[regnum].EventHandlers.get(), mood);
	} else {
		run_interaction_event(&croom->intrRegion[regnum], mood);
	}

	evblockbasename = oldbasename;
	evblocknum = oldblocknum;
}
