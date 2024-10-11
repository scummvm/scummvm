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

#include "m4/riddle/rooms/section8/room802.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room802::preload() {
	mouse_show();
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room802::init() {
	digi_preload("801_s02", -1);
	digi_play_loop("801_s02", 3, 35, -1, -1);
	if (!player_been_here(802)) {
		_G(flags)
		[V253] = 0;
		_G(flags)
		[V254] = 0;
		_G(flags)
		[V255] = 0;
	}

	digi_preload("802_s01", -1);
	digi_preload("802_s02", -1);
	digi_preload("802_s03", -1);
	digi_preload("802_s04", -1);
	digi_preload("802_s05", -1);
	_ripArmXPos3 = series_load("RIP TREK ARMS X POS3", -1, nullptr);
	_ripTalkOffTd33 = series_load("RIP HEAD DOWN TALK OFF TD33", -1, nullptr);

	if (_G(flags)[V254]) {
		_handInWall = series_load("HAND IN WALL", -1, nullptr);
		if (inv_player_has("farmer's shovel")) {
			_ripDigsWall = series_load("802 RIP DIGS AT WALL", -1, nullptr);
		}
	}

	if (_G(flags)[V255]) {
		_handInWallPartlyDug = series_load("HAND IN WALL PARTLY DUG ", -1, nullptr);
		if (inv_player_has("farmer's shovel")) {
			_ripTugsOnArm = series_load("RIP TUGS ON ARM", -1, nullptr);
		}
		_ripTugsBeforeDigging = series_load("RIP TUGS BEFORE DIGGING", -1, nullptr);
	}

	if (inv_player_has("farmer's shovel")) {
		_holeInWall = series_load("HOLE IN WALL", -1, nullptr);
	}

	if (_G(flags)[V253]) {
		_sackAgainstWall = series_load("SACK AGAINST WALL", -1, nullptr);
		_ripDragsSack = series_load("RIP DRAGS SACK ASIDE", -1, nullptr);
	}

	_802Sack2 = series_load("802SACK2", -1, nullptr);
	_ripUpStairs = series_load("802 RIP UP STAIRS", -1, nullptr);
	_ripDownStairs = series_load("802 RIP DOWN STAIRS", -1, nullptr);
	_ripLooksAtHand = series_load("RIP looks at hand in wall", -1, nullptr);

	if (inv_player_has("match") && !_G(flags)[V255]) {
		_lookWithMatch = series_load("802 LOOK WITH MATCH", -1, nullptr);
		hotspot_set_active(_G(currentSceneDef).hotspots, "HOLE", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "HOLE ", true);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "HOLE", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "HOLE ", false);
	}

	if (_G(flags)[V253]) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "RICE SACK", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "RICE SACK ", true);
	} else {
		_sackAgainstWallMach = series_place_sprite("SACK AGAINST WALL", 0, 0, 0, 100, 512);
		hotspot_set_active(_G(currentSceneDef).hotspots, "RICE SACK", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "RICE SACK ", false);
	}

	if (_G(flags)[V255]) {
		_sackAgainstWallMach = series_place_sprite("802SACK2", 0, 0, 0, 100, 768);
	}

	if (_G(flags)[V255] || !_G(flags)[V253])
		hotspot_set_active(_G(currentSceneDef).hotspots, "HAND", false);
	else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "HAND", true);
		if (_G(flags)[V254])
			_handInWallMach = series_place_sprite("HAND IN WALL PARTLY DUG ", 0, 0, 0, 100, 512);
		else
			_handInWallMach = series_place_sprite("HAND IN WALL", 0, 0, 0, 100, 512);
	}

	if (_G(flags)[V255])
		_holeInWallMach = series_place_sprite("HOLE IN WALL", 0, 0, 0, 100, 512);
	else
		hotspot_set_active(_G(currentSceneDef).hotspots, "HOLE", false);

	if (_G(flags)[V255])
		hotspot_set_active(_G(currentSceneDef).hotspots, "WALL", false);

	if (_G(game).previous_room == -2)
		return;

	if (_G(game).previous_room != 801) {
		_ripDigsWall = series_load("802 RIP DIGS AT WALL", -1, nullptr);
		_ripLooksAtHand = series_load("RIP looks at hand in wall", -1, nullptr);
		_ripTugsOnArm = series_load("RIP TUGS ON ARM", -1, nullptr);
		_ripTugsBeforeDigging = series_load("RIP TUGS BEFORE DIGGING", -1, nullptr);
		_holeInWall = series_load("HOLE IN WALL", -1, nullptr);
		_lookWithMatch = series_load("802 LOOK WITH MATCH", -1, nullptr);
	}

	ws_demand_facing(_G(my_walker), 3);
	ws_demand_location(_G(my_walker), 185, 263);
	ws_hide_walker(_G(my_walker));
	kernel_timing_trigger(30, 7, nullptr);
}

void Room802::pre_parser() {
	player_said("take"); // CHECKME: completely useless -> suspicious

	bool lookFl = false;
	if (player_said("look"))
		lookFl = true;

	if (player_said("push") || player_said("pull") || player_said("gear") || player_said("open") || player_said("close"))
		; // CHECKME: completely useless -> suspicious

	if (player_said("go") && player_said("root cellar")) {
		digi_play("802R13", 1, 255, -1, -1);
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (lookFl && player_said(" ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

}

void Room802::parser() {
}

void Room802::daemon() {
	switch (_G(kernel).trigger) {
	case 6:
	case 7:
		player_set_commands_allowed(false);
		_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, 0, triggerMachineByHashCallbackNegative, "rip walks down stairs");
		sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDownStairs, 1, 20, 51, _ripDownStairs, 20, 20, 0);
		break;
	case 14:
		player_set_commands_allowed(false);
		setGlobals1(1, _ripArmXPos3, 15, 15, 15, 0, 15, 1, 1, 1, 0, 1, 1, 1, 1, 0, 15, 15, 15, 15, 0);
		sendWSMessage_110000(_G(my_walker), 16);
		break;
	case 16:
		sendWSMessage_150000(_G(my_walker), 0);
		setGlobals1(1, _ripTalkOffTd33, 3, 3, 3, 0, 3, 1, 1, 1, 0, 1, 1, 1, 1, 0, 3, 3, 3, 3, 0);
		sendWSMessage_110000(_G(my_walker), 15);
		if (player_said("rice sack") || player_said("rice sack "))
			digi_play("802R01", 1, 255, 17, -1);
		else if (player_said("wall"))
			digi_play("802R02", 1, 255, 17, -1);
		break;
	case 17:
		sendWSMessage_140000(_G(my_walker), 18);
		break;
	case 18:
		setGlobals1(_ripArmXPos3, 15, 1, 1, 1, 0, 15, 1, 1, 1, 0, 1, 1, 1, 1, 0, 15, 15, 15, 15, 0);
		sendWSMessage_110000(_G(my_walker), 19);
		break;
	case 19:
		sendWSMessage_150000(_G(my_walker), 0);
		player_set_commands_allowed(true);
		break;
	case 51:
		terminateMachine(_ripWalksDownstairsMach);
		ws_unhide_walker(_G(my_walker));
		series_unload(_ripDownStairs);
		player_set_commands_allowed(true);
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
