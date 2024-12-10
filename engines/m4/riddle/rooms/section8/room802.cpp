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

static const char *const SAID[][2] = {
	{"urn", "802r10"},
	{"urn ", "802r11"},
	{"potatoes", "802r12"},
	{"stairs", "802r13"},
	{nullptr, nullptr}
};

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
		_G(flags)[V253] = 0;
		_G(flags)[V254] = 0;
		_G(flags)[V255] = 0;
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

	if (_G(game).previous_room == KERNEL_RESTORING_GAME)
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
	const bool lookFl = player_said("look");

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
	const bool lookFl = player_said_any("look", "look at");
	const bool takeFl = player_said("take");
	const bool gearFl = player_said("gear");
	const bool climbFl = player_said_any("go", "climb");

	if (lookFl && _G(walker).ripley_said(SAID)) {
		// Nothing on purpose
	}

	else if (lookFl && (player_said("rice sack") || player_said("rice sack "))) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(14));
		_G(kernel).trigger_mode = KT_PARSE;
	}

	else if (lookFl && player_said("wall") && _G(flags)[V255] == 0) {
		if (_G(flags)[V252]) {
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(14));
			_G(kernel).trigger_mode = KT_PARSE;
		} else {
			digi_play("802R03", 1, 255, -1, -1);
		}
	}

	else if ((climbFl && player_said("hole ")) || (lookFl && player_said("hole "))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			interface_hide();
			ws_unhide_walker(_G(my_walker));
			_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0, triggerMachineByHashCallback, "rip lights match");
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _lookWithMatch, 1, 12, 10, _lookWithMatch, 12, 12, 0);
			break;
		case 1:
			digi_play("802r16", 1, 255, 2, 802);
			break;
		case 2:
			kernel_timing_trigger(90, 3, nullptr);
			break;
		case 3:
			disable_player_commands_and_fade_init(4);
			break;
		case 4:
			inv_move_object("match", 999);
			terminateMachine(_ripWalksDownstairsMach);
			player_set_commands_allowed(true);
			_G(flags)[V260] = 1;
			adv_kill_digi_between_rooms(false);
			digi_play_loop("801_s02", 3, 100, -1, -1);
			_G(game).new_room = 801;
			break;
		case 10:
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _lookWithMatch, 13, 18, 1, _lookWithMatch, 19, 22, 1);
			digi_play("802_s05", 2, 255, -1, -1);
			break;
		default:
			break;
		}
	}

	else if (lookFl && (player_said("hand") || player_said("hole"))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0, triggerMachineByHashCallback, "rip bends down to look in hole / at hand");
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripLooksAtHand, 1, 17, 1, _ripLooksAtHand, 17, 17, 0);
			break;
		case 1:
			if (player_said("hole")) {
				digi_play("802r09", 1, 255, 2, -1);
			} else if (_G(flags)[V254]){
				digi_play("802r15", 1, 255, 2, -1);
			} else if (_G(flags)[V252]) {
				digi_play("802r02", 1, 255, 2, -1);
			} else {
				digi_play("802r03", 1, 255, 2, -1);
			}
			break;
		case 2:
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripLooksAtHand, 17, 1, 3, _ripLooksAtHand, 1, 1, 0);
			break;
		case 3:
			terminateMachine(_ripWalksDownstairsMach);
			ws_unhide_walker(_G(my_walker));
			break;
		default:
			break;
		}
	}

	else if (lookFl && player_said("root cellar")) {
		// Nothing on purpose
	}

	else if (gearFl && player_said("rice sack")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0, triggerMachineByHashCallback, "rip moves sack of rice");
			ws_hide_walker(_G(my_walker));
			terminateMachine(_sackAgainstWallMach);
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDragsSack, 1, 15, 10, _ripDragsSack, 15, 15, 0);
			break;
		case 1:
			digi_play("802r05", 1, 255, 2, -1);
			break;
		case 2:
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDragsSack, 41, 46, 3, _ripDragsSack, 46, 46, 0);
			break;
		case 3:
			terminateMachine(_ripWalksDownstairsMach);
			ws_unhide_walker(_G(my_walker));
			_handInWallMach = series_place_sprite("HAND IN WALL", 0, 0, 0, 100, 512);
			series_unload(_ripDragsSack);
			series_unload(_sackAgainstWall);
			_G(flags)[V253] = 1;
			hotspot_set_active(_G(currentSceneDef).hotspots, "rice sack", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "rice sack ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "hand", true);
			player_set_commands_allowed(true);
			break;
		case 10:
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDragsSack, 16, 25, 11, _ripDragsSack, 25, 25, 0);
			digi_play("802_s01", 2, 255, -1, -1);
			break;
		case 11:
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDragsSack, 26, 40, 1, _ripDragsSack, 40, 40, 0);
			digi_stop(2);
			break;
		default:
			break;
		}
	}

	else if ((player_said("farmer's shovel", "hand") || player_said("farmer's shovel", "wall")) && _G(flags)[V254] == 0) {
		if (_G(flags)[V253]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_handInWallMach);
				_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0, triggerMachineByHashCallback, "802 rip digs at wall");
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDigsWall, 1, 24, 10, _ripDigsWall, 24, 24, 0);
				break;
			case 1:
				terminateMachine(_ripWalksDownstairsMach);
				ws_unhide_walker((_G(my_walker)));
				_handInWallMach = series_place_sprite("HAND IN WALL PARTLY DUG ", 0, 0, 0, 100, 512);
				_G(flags)[V254] = 1;
				player_set_commands_allowed(true);
				break;
			case 10:
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDigsWall, 25, 54, 11, _ripDigsWall, 54, 54, 0);
				digi_play("802_s04", 2, 255, -1, -1);
				break;
			case 11:
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripDigsWall, 55, 66, 1, _ripDigsWall, 66, 66, 0);
				digi_stop(2);
				break;
			default:
				break;
			}
		} else {
			digi_play("802r04", 1, 255, -1, -1);
		}
	}

	else if ((gearFl || takeFl) && player_said("hand")) {
		if (inv_player_has("farmer's shovel") && _G(flags)[V254]) {
			if (_G(flags)[V254] >= 1) {
				switch (_G(kernel).trigger) {
				case -1:
					player_set_commands_allowed(false);
					ws_unhide_walker(_G(my_walker));
					_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0, triggerMachineByHashCallback, "rip tugs at hand and removes it");
					terminateMachine(_handInWallMach);
					sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsOnArm, 1, 36, 10, _ripTugsOnArm, 36, 36, 0);
					break;
				case 10:
					sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsOnArm, 37, 68, 20, _ripTugsOnArm, 68, 68, 0);
					digi_play("802_s03", 2, 255, -1, -1);
					break;
				case 20:
					terminateMachine(_ripWalksDownstairsMach);
					ws_unhide_walker(_G(my_walker));
					_holeInWallMach = series_place_sprite("HOLE IN WALL", 0, 0, 0, 100, 512);
					_sackAgainstWallMach = series_place_sprite("802SACK2", 0, 0, 0, 100, 768);

					if (inv_player_has("match")) {
						hotspot_set_active(_G(currentSceneDef).hotspots, "hole", true);
						hotspot_set_active(_G(currentSceneDef).hotspots, "hole ", false);
					} else {
						hotspot_set_active(_G(currentSceneDef).hotspots, "hole", false);
						hotspot_set_active(_G(currentSceneDef).hotspots, "hole ", true);
					}

					hotspot_set_active(_G(currentSceneDef).hotspots, "hand", false);
					hotspot_set_active(_G(currentSceneDef).hotspots, "wall", false);

					_G(flags)[V255] = 1;
					player_set_commands_allowed(true);
					break;
				default:
					break;
				}
			}

		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 512, 0, triggerMachineByHashCallback, "rip tugs at hand (noshovel)");
				terminateMachine(_handInWallMach);
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsBeforeDigging, 1, 15, 10, _ripTugsBeforeDigging, 15, 15, 0);
				break;
			case 1:
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsBeforeDigging, 26, 20, 2, _ripTugsBeforeDigging, 19, 19, 0);
				break;
			case 2:
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsBeforeDigging, 19, 15, 13, _ripTugsBeforeDigging, 15, 15, 0);
				if (inv_player_has("farmer's shovel")) {
					if (_G(flags)[V254] == 1)
						digi_play("802r07", 1, 255, -1, -1);
				} else {
					digi_play("802r06", 1, 255, -1, -1);
				}
				break;
			case 3:
				terminateMachine(_ripWalksDownstairsMach);
				ws_unhide_walker(_G(my_walker));
				_handInWallMach = series_place_sprite("HAND IN WALL", 0, 0, 0, 100, 512);
				player_set_commands_allowed(true);
				break;
			case 10:
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsBeforeDigging, 16, 26, 1, _ripTugsBeforeDigging, 26, 26, 0);
				digi_play("802_s02", 2, 255, -1, -1);
				break;
			case 13:
				sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripTugsBeforeDigging, 14, 1, 3, _ripTugsBeforeDigging, 1, 1, 0);
				digi_stop(2);
				break;
			default:
				break;
			}
		}
	}

	else if (takeFl && (player_said("urn") || player_said("urn ") || player_said("potatoes"))) {
		digi_play("802r14", 1, 255, -1, -1);
	}

	else if (climbFl && player_said("stairs")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, 0, triggerMachineByHashCallback, "rip walks up stairs");
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripWalksDownstairsMach, _ripUpStairs, 1, 23, 1, _ripUpStairs, 24, 29, 0);
			break;
		case 1:
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			terminateMachine(_ripWalksDownstairsMach);
			_G(game).new_room = 801;
			break;
		default:
			break;
		}
	}

	else if (player_said("journal") && (takeFl || lookFl || gearFl)) {
		digi_play("com042", 1, 255, -1, 997);
	} else
		return;

	_G(player).command_ready = false;
}

void Room802::daemon() {
	switch (_G(kernel).trigger) {
	case 6:
	case 7:
		player_set_commands_allowed(false);
		_ripWalksDownstairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 768, 0, triggerMachineByHashCallback, "rip walks down stairs");
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
