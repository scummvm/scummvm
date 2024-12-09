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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section7/room706.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room706::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room706::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #13", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #14", false);
		// Suspicious but present in the original. Should it be #16?
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #14", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #15", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #14", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);

		series_show("THE ALL IMPORTANT NOTE", 3840, 16, -1, -1, 0, 100, 0, 0);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, "  ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "Note", false);

	if (_G(flags[V219])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #15", false);
	}

	switch (_G(flags[V219])) {
	case 1:
		_prayerWheelMach = series_place_sprite("706 PEACE PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("706 INSIGHT PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("706 SERENITY PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("706 TRUTH PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("706 WISDOM PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	default:
		break;
	}

	if (_G(flags[V222])) {
		series_show("706 ENLIGHTEN DOOR OPEN", 3840, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active(_G(currentSceneDef).hotspots, "  ", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #14", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "empty cell", false);
	}

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "Note", true);
	}

	switch (_G(game).previous_room) {
	case 703:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		player_first_walk(1299, 300, 9, 1170, 318, 9, true);
		break;

	case 705:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 635, 357);
		ws_demand_facing(_G(my_walker), 7);
		ws_hide_walker(_G(my_walker));
		_706RipGoesDownStairsSeries = series_load("706 RIP GOES DOWN STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _706RipGoesDownStairsSeries, 4, 39, 140, _706RipGoesDownStairsSeries, 39, 39, 0);
		break;

	case 709:
	case 711:
		digi_preload("950_s39", -1);
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 420, 288);
		ws_demand_facing(_G(my_walker), 4);
		_ripShieldsFaceSeries = series_load("RIP SHIELDS FACE", -1, nullptr);
		setGlobals1(_ripShieldsFaceSeries, 1, 14, 14, 14, 0, 14, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 131);

		break;

	default: // Including KERNEL_RESTORING_GAME
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room706::pre_parser() {
	// Nothing
}

void Room706::parser() {
	// TODO Not implemented yet
}

void Room706::daemon() {
	// TODO Not implemented yet
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
