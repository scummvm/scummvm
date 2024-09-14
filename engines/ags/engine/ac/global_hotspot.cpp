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
#include "ags/engine/ac/dynobj/cc_hotspot.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/script/script.h"
#include "ags/globals.h"

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

	snprintf(buffer, MAX_MAXSTRLEN, "%s", get_translation(_G(croom)->hotspot[hotspot].Name.GetCStr()));
}

void RunHotspotInteraction(int hotspothere, int mood) {

	// convert cursor mode to event index (in hotspot event table)
	// TODO: probably move this conversion table elsewhere? should be a global info
	// TODO: find out what is hotspot event with index 6 (5 is any-click)
	int evnt;
	switch (mood) {
		case MODE_WALK:	evnt = 0; break;
		case MODE_LOOK:	evnt = 1; break;
		case MODE_HAND:	evnt = 2; break;
		case MODE_TALK:	evnt = 4; break;
		case MODE_USE: evnt = 3; break;
		case MODE_PICKUP: evnt = 7; break;
		case MODE_CUSTOM1: evnt = 8; break;
		case MODE_CUSTOM2: evnt = 9; break;
		default: evnt = -1; break;
	}
	const int anyclick_evt = 5; // TODO: make global constant (hotspot any-click evt)

	// For USE verb: remember active inventory
	if (mood == MODE_USE) {
		_GP(play).usedinv = _G(playerchar)->activeinv;
	}

	if ((_GP(game).options[OPT_WALKONLOOK] == 0) & (mood == MODE_LOOK));
	else if (_GP(play).auto_use_walkto_points == 0);
	else if ((mood != MODE_WALK) && (_GP(play).check_interaction_only == 0))
		MoveCharacterToHotspot(_GP(game).playercharacter, hotspothere);

	const auto obj_evt = ObjectEvent("hotspot%d", hotspothere,
									 RuntimeScriptValue().SetScriptObject(&_G(scrHotspot)[hotspothere], &_GP(ccDynamicHotspot)), mood);

	if (_G(loaded_game_file_version) > kGameVersion_272) {
		if ((evnt >= 0) &&
			run_interaction_script(obj_evt, _GP(thisroom).Hotspots[hotspothere].EventHandlers.get(), evnt, anyclick_evt) < 0)
			return; // game state changed, don't do "any click"
		run_interaction_script(obj_evt, _GP(thisroom).Hotspots[hotspothere].EventHandlers.get(), anyclick_evt); // any click on hotspot
	} else {
		if ((evnt >= 0) &&
			run_interaction_event(obj_evt, &_G(croom)->intrHotspot[hotspothere], evnt, anyclick_evt, (mood == MODE_USE)) < 0)
			return; // game state changed, don't do "any click"
		run_interaction_event(obj_evt, &_G(croom)->intrHotspot[hotspothere], anyclick_evt); // any click on hotspot
	}
}

int GetHotspotProperty(int hss, const char *property) {
	if (!AssertHotspot("GetHotspotProperty", hss))
		return 0;
	return get_int_property(_GP(thisroom).Hotspots[hss].Properties, _G(croom)->hsProps[hss], property);
}

void GetHotspotPropertyText(int item, const char *property, char *bufer) {
	if (!AssertHotspot("GetHotspotPropertyText", item))
		return;
	get_text_property(_GP(thisroom).Hotspots[item].Properties, _G(croom)->hsProps[item], property, bufer);
}

} // namespace AGS3
