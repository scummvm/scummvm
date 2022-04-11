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

#include "ags/engine/ac/global_hotspot.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/event.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/hotspot.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/room_status.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/script/script.h"

namespace AGS3 {

using namespace AGS::Shared;

void DisableHotspot(int hsnum) {
	if ((hsnum < 1) | (hsnum >= MAX_ROOM_HOTSPOTS))
		quit("!DisableHotspot: invalid hotspot specified");
	_G(croom)->hotspot[hsnum].Enabled = false;
	debug_script_log("Hotspot %d disabled", hsnum);
}

void EnableHotspot(int hsnum) {
	if ((hsnum < 1) | (hsnum >= MAX_ROOM_HOTSPOTS))
		quit("!EnableHotspot: invalid hotspot specified");
	_G(croom)->hotspot[hsnum].Enabled = true;
	debug_script_log("Hotspot %d re-enabled", hsnum);
}

int GetHotspotPointX(int hotspot) {
	if ((hotspot < 0) || (hotspot >= MAX_ROOM_HOTSPOTS))
		quit("!GetHotspotPointX: invalid hotspot");

	if (_GP(thisroom).Hotspots[hotspot].WalkTo.X < 1)
		return -1;

	return _GP(thisroom).Hotspots[hotspot].WalkTo.X;
}

int GetHotspotPointY(int hotspot) {
	if ((hotspot < 0) || (hotspot >= MAX_ROOM_HOTSPOTS))
		quit("!GetHotspotPointY: invalid hotspot");

	if (_GP(thisroom).Hotspots[hotspot].WalkTo.X < 1) // TODO: there was "x" here, why?
		return -1;

	return _GP(thisroom).Hotspots[hotspot].WalkTo.Y;
}

int GetHotspotIDAtScreen(int scrx, int scry) {
	VpPoint vpt = _GP(play).ScreenToRoomDivDown(scrx, scry);
	if (vpt.second < 0) return 0;
	return get_hotspot_at(vpt.first.X, vpt.first.Y);
}

void GetHotspotName(int hotspot, char *buffer) {
	VALIDATE_STRING(buffer);
	if ((hotspot < 0) || (hotspot >= MAX_ROOM_HOTSPOTS))
		quit("!GetHotspotName: invalid hotspot number");

	strcpy(buffer, get_translation(_G(croom)->hotspot[hotspot].Name.GetCStr()));
}

void RunHotspotInteraction(int hotspothere, int mood) {

	int passon = -1, cdata = -1;
	if (mood == MODE_TALK) passon = 4;
	else if (mood == MODE_WALK) passon = 0;
	else if (mood == MODE_LOOK) passon = 1;
	else if (mood == MODE_HAND) passon = 2;
	else if (mood == MODE_PICKUP) passon = 7;
	else if (mood == MODE_CUSTOM1) passon = 8;
	else if (mood == MODE_CUSTOM2) passon = 9;
	else if (mood == MODE_USE) {
		passon = 3;
		cdata = _G(playerchar)->activeinv;
		_GP(play).usedinv = cdata;
	}

	if ((_GP(game).options[OPT_WALKONLOOK] == 0) & (mood == MODE_LOOK));
	else if (_GP(play).auto_use_walkto_points == 0);
	else if ((mood != MODE_WALK) && (_GP(play).check_interaction_only == 0))
		MoveCharacterToHotspot(_GP(game).playercharacter, hotspothere);

	// can't use the setevent functions because this ProcessClick is only
	// executed once in a eventlist
	const char *oldbasename = _G(evblockbasename);
	int   oldblocknum = _G(evblocknum);

	_G(evblockbasename) = "hotspot%d";
	_G(evblocknum) = hotspothere;

	if (_GP(thisroom).Hotspots[hotspothere].EventHandlers != nullptr) {
		if (passon >= 0)
			run_interaction_script(_GP(thisroom).Hotspots[hotspothere].EventHandlers.get(), passon, 5, (passon == 3));
		run_interaction_script(_GP(thisroom).Hotspots[hotspothere].EventHandlers.get(), 5);  // any click on hotspot
	} else {
		if (passon >= 0) {
			if (run_interaction_event(&_G(croom)->intrHotspot[hotspothere], passon, 5, (passon == 3))) {
				_G(evblockbasename) = oldbasename;
				_G(evblocknum) = oldblocknum;
				return;
			}
		}
		// run the 'any click on hs' event
		run_interaction_event(&_G(croom)->intrHotspot[hotspothere], 5);
	}

	_G(evblockbasename) = oldbasename;
	_G(evblocknum) = oldblocknum;
}

int GetHotspotProperty(int hss, const char *property) {
	return get_int_property(_GP(thisroom).Hotspots[hss].Properties, _G(croom)->hsProps[hss], property);
}

void GetHotspotPropertyText(int item, const char *property, char *bufer) {
	get_text_property(_GP(thisroom).Hotspots[item].Properties, _G(croom)->hsProps[item], property, bufer);
}

} // namespace AGS3
