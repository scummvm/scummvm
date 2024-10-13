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

#include "m4/riddle/rooms/section8/room801.h"
#include "m4/riddle/rooms/section8/section8.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *const SAID[][2] = {
{ "farmer",        "801r10"  },
{ "rice paddies",  "801r11"  },
{ "root cellar",   "801r29"  },
{ "root cellar ",  "801r29"  },
{ "root cellar  ", "801r14"  },
{ "garden fence",  "801r17"  },
{ "earthen jugs",  "801r18"  },
{ nullptr,         nullptr   }
};


void Room801::preload() {
	mouse_show();

	if (!_G(flags)[V260])
		interface_show();

	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room801::init() {
	if (!player_been_here(201) && !player_been_here(801)) {
		inv_give_to_player("CHINESE YUAN");
	}

	digi_preload("801_s02", -1);
	digi_play_loop("801_s02", 3, 100, -1, -1);
	_roomStates_field642 = series_load("farmer displacement", -1, nullptr);
	_roomStates_field742 = series_load("farmer  talk loop", -1, nullptr);
	_roomStates_field6C2 = series_load("farmer give take bow", -1, nullptr);
	_roomStates_field782 = series_load("mei prattle loop", -1, nullptr);
	_roomStates_loop1 = series_load("rip kneel to pottery", -1, nullptr);
	_roomStates_loop2 = series_load("rip enters root cellar", -1, nullptr);
	_roomStates_loop62 = series_load("rip trek head turn pos5", -1, nullptr);
	_roomStates_loop72 = series_load("rptmr15", -1, nullptr);
	_roomStates_val1 = series_load("rip trek hand on chin pos2", -1, nullptr);
	_roomStates_ear2 = series_load("rip trek hands beh back pos1", -1, nullptr);
	_roomStates_val3 = series_load("rip trek nod pos4", -1, nullptr);
	_roomStates_val2 = series_load("rip looks l r down", -1, nullptr);
	series_load("SAFARI SHADOW 5", -1, nullptr);
	_roomStates_loop4 = series_load("MCTSH1", -1, nullptr);
	_roomStates_field96h = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1024, 0, triggerMachineByHashCallback, "farmer displacement");
	sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field642, 1, 14, 0, _roomStates_field642, 14, 14, 0);
	_roomStates_field9Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 151, 317, 100, 1280, 0, triggerMachineByHashCallback, "farmer shadow");
	sendWSMessage_10000(1, _roomStates_field9Ah, _roomStates_loop4, 1, 1, -1, _roomStates_loop4, 1, 1, 0);
	if (inv_player_has("farmer's shovel"))
		hotspot_set_active(_G(currentSceneDef).hotspots, "farmer's shovel", false);
	else {
		_roomStates_field80 = series_load("801 SHOVEL ", -1, nullptr);
		_roomStates_field86h = series_place_sprite("801 SHOVEL ", 0, 0, 0, 100, 1024);
	}

	if (_G(game).previous_room == 850) {
		_G(flags)[V250] = 0;
		_G(flags)[V251] = 0;
		_G(flags)[V252] = 0;
		_G(flags)[V273] = 0;

		_roomStates_tt = 0;
		hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar  ", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "ROOT CELLAR", false);
		_roomStates_ripTalk = series_load("CELLAR DOOR CLOSED", -1, nullptr);
		_roomStates_fieldA6h = series_place_sprite("CELLAR DOOR CLOSED", 0, 0, -53, 100, 1280);
		_G(player).disable_hyperwalk = true;
		player_set_commands_allowed(false);
		_G(kernel).call_daemon_every_loop = true;
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), 20, 270);
		_roomStates_field682 = series_load("farmer  earthquake gesture", -1, nullptr);
		_roomStates_loop0 = series_load("rip looks to zero", -1, nullptr);
		_roomStates_field7C = series_load("mei looks to zero", -1, nullptr);
		_roomStates_field702 = series_load("farmer looks to zero", -1, nullptr);
		_roomStates_field60 = series_load("shadow of de plane", -1, nullptr);
		ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);
		_roomStates_field9Eh = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1,
														 20, 310, 3, triggerMachineByHashCallback3000, "mc_trek");
		kernel_timing_trigger(60, 1, nullptr);
	} else if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (_roomStates_tt) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar  ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "ROOT CELLAR", true);
			_roomStates_field24 = series_load("CELLAR DOOR OPEN", -1, nullptr);
			_roomStates_fieldAAh = series_place_sprite("CELLAR DOOR OPEN", 0, 0, -53, 100, 1024);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar  ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "ROOT CELLAR", false);
			_roomStates_ripTalk = series_load("CELLAR DOOR CLOSED", -1, nullptr);
			_roomStates_fieldA6h = series_place_sprite("CELLAR DOOR CLOSED", 0, 0, -53, 100, 1280);
		}

		_roomStates_field9Eh = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1280, 0, triggerMachineByHashCallback, "mc talk frames");
		_roomStates_fieldA2h = series_show("SAFARI SHADOW 5", 1280, 16, -1, -1, 0, 50, 124, 304);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 0, _roomStates_field782, 42, 42, 0);
	} else {
		ws_demand_facing(_G(my_walker), 8);
		ws_demand_location(_G(my_walker), 525, 301);
		_roomStates_loop3 = series_load("rip returns from root cellar", -1, nullptr);
		_roomStates_tt = 1;
		hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar ", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar  ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "ROOT CELLAR", true);
		_roomStates_field24 = series_load("CELLAR DOOR OPEN", -1, nullptr);
		_roomStates_fieldAAh = series_place_sprite("CELLAR DOOR OPEN", 0, 0, -53, 100, 1024);
		kernel_timing_trigger(10, 7, nullptr);
	}
}

void Room801::pre_parser() {
	player_said("take");
	bool lookCheck = false;
	if (player_said("look") || player_said("look at")) {
		lookCheck = true;
	}

	if (player_said("push") || player_said("pull")
		|| player_said("gear") || player_said("open")) {
		player_said("close");
	}

	player_said("go");
	if (lookCheck && player_said("farm")) {
		digi_play("801R23", 1, 255, -1, -1);
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (lookCheck && player_said(" ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room801::parser() {
	bool lookFl = false;
	bool takeFl = false;
	bool gearFl = false;
	bool talkFl = false;
	bool goFl = false;

	if (player_said("look") || player_said("look at")) {
		lookFl = true;
	}

	if (player_said("talk") || player_said("talk TO")) {
		talkFl = true;
	}

	if (player_said("take")) {
		takeFl = true;
	}

	if (player_said("gear")) {
		gearFl = true;
	}

	if (player_said("go")) {
		goFl = true;
	}

	if (player_said("conv801a")) {
		room801_conv801a();
	}

	else if (_G(kernel).trigger == 747) {
		_roomStates_ripTalker = 4;
		_roomStates_pu = 8;
		_roomStates_field18 = 5;
		conv_shutdown();
		player_set_commands_allowed(true);
	}

	else if (lookFl && _G(walker).ripley_said(SAID)) {
		// Nothing (though it's important to keep it because of the cascade of checks
	}

	else if (lookFl && player_said("house")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_roomStates_val1, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 1, 1, 1, 1, 0, 16, 16, 16, 16, 0);
			sendWSMessage_110000(_G(my_walker), 4);
			digi_play("801R12", 1, 255, 1, -1);
			break;
		case 1:
			sendWSMessage_140000(_G(my_walker), 2);
			break;
		case 2: player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	}

	else if (lookFl && player_said("mount li")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags)[V250]) {
				digi_play("801R13A", 1, 255, 0);
			} else {
				player_set_commands_allowed(false);
				setGlobals1(_roomStates_ear2, 1, 11, 11, 11, 0, 11, 1, 1, 1, 0, 1, 1, 1, 1, 0, 11, 11, 11, 11, 0);
				sendWSMessage_110000(_G(my_walker), 4);
				digi_play("801R13", 1, 255, 1, -1);
				_G(flags)[V250] = 1;
			}
			break;
		case 1:
			sendWSMessage_140000(_G(my_walker), 2);
			break;
		case 2:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	}

	else if (lookFl && player_said("farmer's shovel") && !inv_player_has("farmer's shovel")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_roomStates_val2, 1, 4, 4, 4, 0, 4, 1, 1, 1, 0, 1, 1, 1, 1, 0, 3, 3, 3, 3, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;
		case 1:
			if (_G(flags)[V253]) {
				digi_play("801R15A", 1, 255, 2, -1);
			} else {
				digi_play("801R15", 1, 255, 2, -1);
			}
			break;
		case 2:
			sendWSMessage_140000(_G(my_walker), 3);
			break;
		case 3:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	}

	else if (lookFl && player_said("garden")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_roomStates_val3, 1, 3, 3, 3, 0, 3, 1, 1, 1, 0, 1, 1, 1, 1, 0, 3, 3, 3, 3, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			digi_play("801R16", 1, 255, 2, -1);
			break;
		case 2:
			sendWSMessage_140000(_G(my_walker), 3);
			break;
		case 3:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	}

	else if (lookFl && player_said("pottery")) {
		switch (_G(kernel).trigger) {
		case -1:
			break;
		case 1:
			if (_var1 == 0)
				++_var1;
			else
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop2, 3, 42, 2, _roomStates_loop2, 42, 42, 0);
			break;
		case 2:
			digi_play("801r20", 1, 255, 3, -1);
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 40, 3, _roomStates_field782, 40, 40, 0);
			break;
		case 3:
			if (_var1 == 0)
				++_var1;
			else {
				_var1 = 0;
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop2, 42, 26, 4, _roomStates_loop2, 26, 26, 0);
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 37, 40, 4, _roomStates_field782, 37, 40, 1);
				digi_play("801m10", 1, 255, 4, -1);
			}
			break;
		case 4:
			if (_var1 == 2)
				++_var1;
			else {
				_var1 = 0;
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 40, 40, 5, _roomStates_field782, 40, 40, 0);
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop1, 26, 42, 5, _roomStates_loop1, 42, 42, 0);
			}
			break;
		case 5:
			if (_var1 == 1)
				++_var1;
			else {
				_var1 = 0;
				digi_play("801r21", 1, 255, 6, -1);
			}
			break;
		case 6:
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop1, 42, 26, 7, _roomStates_loop1, 26, 26, 0);
			break;
		case 7:
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop1, 26, 1, 8, _roomStates_loop1, 1, 1, 0);
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 37, 40, 9, _roomStates_field782, 37, 40, 1);
			digi_play("801m11", 1, 255, 9, -1);
			break;
		case 8:
			terminateMachine(_roomStates_field8Ah);
			_roomStates_field8Ah = nullptr;
			ws_unhide_walker(_G(my_walker));
			break;
		case 9:
			if (_var1 == 1)
				++_var1;
			else {
				_var1 = 0;
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 40, 31, 10, _roomStates_field782, 31, 31, 0);
			}
			break;
		case 10:
			_G(flags)[V252] = 1;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	}

	else if (lookFl && player_said("mei chen")) {
		digi_play("COM043", 1, 255, -1, 997);
	}

	else if (lookFl && player_said("farm")) {
		; // I bet there was something at some point, but it's really empty now
	}

	else if (gearFl && player_said("MATCH")) {
		digi_play("801R09", 1, 255, -1, -1);
	}

	else if (player_said("US DOLLARS", "FARMER")) {
		if (_G(flags)[V273]) {
			digi_play("801r43b", 1, 255, -1, -1);
		} else {
			digi_play("801r43c", 1, 255, -1, -1);
			_G(flags)[V273] = 1;
		}
	}

	else if (player_said("CHINESE YUAN", "FARMER")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_roomStates_loop62, 1, 1, 1, 1, 0, 1, 7, 5, 7, 1, 7, 1, 1, 1, 0, 7, 7, 7, 7, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;
		case 1:
			digi_play("801r27", 1, 255, 3, -1);
			kernel_timing_trigger(430, 2, nullptr);
			break;
		case 2:
			sendWSMessage_120000(_G(my_walker), 0);
			break;
		case 3:
			sendWSMessage_130000(_G(my_walker), 0);
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 55, 0, _roomStates_field782, 53, 55, 1);
			digi_play("801m12", 1, 255, 4, -1);
			break;
		case 4:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 55, 55, 0, _roomStates_field782, 55, 55, 0);
			digi_play("801f06", 1, 255, 5, -1);
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 1, 0, _roomStates_field742, 1, 3, 4);
			break;
		case 5:
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 1, 0, _roomStates_field742, 1, 1, 0);
			sendWSMessage_150000(_G(my_walker), 0);
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 40, 6, _roomStates_field782, 40, 40, 0);
			break;
		case 6:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 40, 41, 0, _roomStates_field782, 39, 41, 4);
			digi_play("801m13", 1, 255, 7, -1);
			break;
		case 7:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 41, 31, 0, _roomStates_field782, 31, 31, 0);
			setGlobals1(_roomStates_loop72, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 1, 1, 1, 1, 1, 16, 16, 16, 16, 0);
			sendWSMessage_110000(_G(my_walker), 9);
			kernel_timing_trigger(50, 8, nullptr);
			inv_move_object("CHINESE YUAN", 801);
			digi_play("801r28", 1, 255, 9, -1);
			break;
		case 8:
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field6C2, 71, 110, 10, _roomStates_field742, 110, 110, 0);
			break;
		case 9:
			sendWSMessage_120000(_G(my_walker), 0);
			break;
		case 10:
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 1, 0, _roomStates_field742, 1, 1, 0);
			sendWSMessage_150000(_G(my_walker), 12);
			break;
		case 12:
			player_update_info(_G(my_walker), &_G(player_info));
			ws_walk(_G(my_walker), 8, 6, nullptr, 13, 5, true);
			break;
		case 13:
			hotspot_set_active(_G(currentSceneDef).hotspots, "farmer's shovel", false);
			setGlobals1(_roomStates_loop72, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 1, 1, 1, 1, 1, 16, 16, 16, 16, 0);
			sendWSMessage_110000(_G(my_walker), 16);
			break;
		case 16:
			kernel_examine_inventory_object("PING FARMER'S SHOVEL", _G(master_palette), 5, 1, 110, 250, 17, nullptr, -1);
			terminateMachine(_roomStates_field86h);
			inv_give_to_player("farmer's shovel");
			break;
		case 17:
			sendWSMessage_140000(_G(my_walker), 18);
			break;
		case 18:
			series_unload(_roomStates_field80);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	}

	else if (gearFl && player_said("root cellar  ")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_roomStates_tt2 == 0) {
				player_set_commands_allowed(false);
				ws_hide_walker();
				terminateMachine(_roomStates_fieldA6h);
				_roomStates_field18 = series_load("RIP OPENS CELLAR", -1, nullptr);
				_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, 0, triggerMachineByHashCallback, "rip opens cellar");
				sendWSMessage_10000(1, _roomStates_field8Ah, 1, _roomStates_field18, 28, 10, _roomStates_field18, 28, 28, 0);
			}
			break;
		case 1:
			_roomStates_tt2 = 1;
			terminateMachine(_roomStates_field8Ah);
			series_unload(_roomStates_field18);
			_roomStates_fieldAAh = series_place_sprite("CELLAR DOOR OPEN", 0, 0, -53, 100, 1024);
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar  ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "ROOT CELLAR", true);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			break;

		case 10:
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_field18, 29, 45, 11, _roomStates_field18, 45, 45, 0);
			digi_play("801_s03", 2, 255, -1, -1);
			break;

		case 11:
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_field18, 46, 56, 1, _roomStates_field18, 56, 56, 0);
			digi_stop(1);
			break;
		default:
			break;
		}
	}

	else if (gearFl && player_said("root cellar ")) {
	// Not the same as the previous one : there's one space instead of two at the end
		switch (_G(kernel).trigger) {
		case -1:
			if (_roomStates_tt2 != 0) {
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_roomStates_fieldAAh);
				_roomStates_ripTalking = series_load("RIP CLOSES CELLAR", -1, nullptr);
				_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, 0, triggerMachineByHashCallback, "rip closes cellar");
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_ripTalking, 1, 19, 10, _roomStates_ripTalking, 34, 34, 0);
			}
			break;
		case 1:
			_roomStates_tt2 = 0;
			terminateMachine(_roomStates_field8Ah);
			ws_unhide_walker(_G(my_walker));
			digi_stop(1);
			series_unload(_roomStates_ripTalking);
			_roomStates_fieldA6h = series_place_sprite("CELLAR DOOR CLOSED", 0, 0, -53, 100, 1280);
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar  ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "root cellar ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "ROOT CELLAR", false);
			player_set_commands_allowed(true);
			break;
		case 10:
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_ripTalking, 20, 34, 1, _roomStates_ripTalking, 34, 34, 0);
			digi_play("801_s04", 2, 255, -1, -1);
			break;
		default:
			break;
		}
	}

	else if ((gearFl || takeFl) && player_said("mei chen")) {
		digi_play("com013", 1, 255, -1, 997);
	}

	else if (talkFl && player_said("farmer")) {
		if (_G(flags)[V252] || _G(flags)[V253] || _G(flags)[V255]) {
			ws_hide_walker(_G(my_walker));
			player_set_commands_allowed(false);
			_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 167, 303, 49, 1280, 1, triggerMachineByHashCallback, "rip talking to farmer");
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop62, 1, 1, -1, _roomStates_loop62, 1, 1, 0);
			_roomStates_field8Eh = series_show("SAFARI SHADOW 5", 1280, 144, -1, -1, 0, 48, 167, 303);
			_roomStates_pu2 = 1;
			_roomStates_ripTalker = 1;
			_roomStates_untie2 = 1;
			_roomStates_pu = 1;
			_roomStates_ripTalker2 = 1;
			_roomStates_field18 = 1;
			_G(kernel).trigger_mode = KT_DAEMON;

			kernel_timing_trigger(1, 300, nullptr);
			kernel_timing_trigger(1, 200, nullptr);
			kernel_timing_trigger(1, 400, nullptr);

			_G(kernel).trigger_mode = KT_PARSE;
			conv_load("conv801a", 10, 10, 747);
			conv_set_shading(65);
			conv_export_value(conv_get_handle(), _G(flags)[V252], 0);
			conv_export_value(conv_get_handle(), _G(flags)[V255], 1);
			conv_export_value(conv_get_handle(), _G(flags)[V253], 2);

			conv_play(conv_get_handle());
		} else {
			digi_play("801r06", 1, 255, -1, -1);
		}
	}

	else if (talkFl && player_said("mei chen")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_roomStates_loop62, 1, 4, 5, 7, 1, 4, 4, 4, 4, 1, 4, 1, 1, 1, 0, 4, 1, 1, 1, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;
		case 1:
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("com034", 1, 255, 2, 997);
				break;
			case 2:
				digi_play("com035", 1, 255, 2, 997);
				break;
			case 3:
				digi_play("com036", 1, 255, 2, 997);
				break;
			case 4:
				digi_play("com037", 1, 255, 2, 997);
				break;
			default:
				break;
			}
			break;
		case 2:
			sendWSMessage_120000(_G(my_walker), 4);
			break;
		case 4:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 40, 5, _roomStates_field782, 40, 40, 0);
			break;
		case 5:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 40, 41, 0, _roomStates_field782, 40, 41, 4);
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				digi_play("com039", 1, 255, 6, 997);
				break;
			case 2:
				digi_play("com040", 1, 255, 6, 997);
				break;
			case 3:
				digi_play("com041", 1, 255, 6, 997);
				break;
			default:
				break;
			}
			break;
		case 6:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 41, 31, 7, _roomStates_field782, 31, 31, 0);
			sendWSMessage_130000(_G(my_walker), 0);
			break;
		case 7:
			sendWSMessage_150000(_G(my_walker), 0);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	}

	else if (gearFl && player_said("farmer")) {
		digi_play("com017", 1, 255, -1, 997);
	}

	else if (takeFl && player_said("pottery")) {
		if (_G(flags)[V251]) {
			digi_play("801r25", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				digi_play("801r25", 1, 255, 1, -1);
				break;
			case 1:
				digi_play("801r25a", 1, 255, 2, -1);
				break;
			case 2:
				player_set_commands_allowed(true);
				_G(flags)[V251] = 1;
			default:
				break;
			}
		}
	}

	else if (goFl && player_said("root cellar")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, 0, triggerMachineByHashCallback, "rip enters cellar");
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop2, 1, 14, 5, _roomStates_loop2, 15, 19, 0);
			break;
		case 5:
			disable_player_commands_and_fade_init(6);
			break;
		case 6:
			terminateMachine(_roomStates_field8Ah);
			ws_unhide_walker(_G(my_walker));
			adv_kill_digi_between_rooms(false);
			digi_play_loop("801_s02", 3, 35, -1, -1);
			_G(game).new_room = 802;
			break;
		default:
			break;
		}
	}

	else if (takeFl && player_said("farmer's shovel") && !inv_player_has("farmer's shovel")) {
		digi_play("801r26", 1, 255, -1, -1);
	}

	else if (player_said("journal") && !takeFl && !lookFl && !gearFl) {
		digi_play("com042", 1, 255, -1, 997);
	} else
		return;

	_G(player).command_ready = false;
}

void Room801::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		sendWSMessage_10000(_roomStates_field9Eh, 124, 304, 4, 11, 1);
		ws_walk(_G(my_walker), 172, 302, nullptr, 10, 7, true);
		break;
	case 2:
		terminateMachine(_roomStates_field92h);
		series_unload(_roomStates_field60);
		_roomStates_field4 = 1;
		kernel_timing_trigger(70, 31, nullptr);
		break;
	case 4:
		// nothing
		break;
	case 5:
		sendWSMessage_150000(_G(my_walker), -1);
		_G(player).disable_hyperwalk = false;
		_G(kernel).call_daemon_every_loop = false;
		player_set_commands_allowed(true);
		break;
	case 6:
	case 7:
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_roomStates_field9Eh = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1280, 0, triggerMachineByHashCallback, "mc talk frames");
		_roomStates_fieldA2h = series_show("SAFARI SHADOW 5", 1280, 16, -1, -1, 0, 50, 124, 304);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 0, _roomStates_field782, 42, 42, 0);

		if (_G(flags)[V260]) {
			kernel_timing_trigger(30, 81, nullptr);
		} else {
			_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0, triggerMachineByHashCallback, "rip returns from cellar");
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop3, 1, 22, 8, _roomStates_loop3, 22, 22, 0);
		}
		break;
	case 8:
		terminateMachine(_roomStates_field8Ah);
		ws_unhide_walker(_G(my_walker));
		series_unload(_roomStates_loop3);
		player_set_commands_allowed(true);
		break;
	case 9:
		mouse_lock_sprite(5);
		conv_resume(conv_get_handle());
		break;
	case 10:
		terminateMachine(_roomStates_field96h);
		_roomStates_field92h = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 2048, 0, triggerMachineByHashCallback, "shadow of de plane");
		_roomStates_field96h = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1024, 0, triggerMachineByHashCallback, "farmer looks to zero");
		_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 1536, 0, triggerMachineByHashCallback, "rip looks to zero");
		sendWSMessage_10000(1, _roomStates_field92h, _roomStates_field60, 1, 47, 2, _roomStates_field60, 47, 47, 0);
		_roomStates_field4 = 0;
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field702, 1, 48, 12, _roomStates_field702, 48, 48, 0);
		_roomStates_field6 = 0;
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field7E, 1, 18, 0, _roomStates_field7E, 18, 18, 0);
		ws_hide_walker(_G(my_walker));
		sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_field2A, 1, 13, 51, _roomStates_field2A, 13, 13, 0);
		_roomStates_field8Eh = series_show("SAFARI SHADOW 5", 1280, 144, -1, -1, 0, 48, 172, 302);
		break;
	case 11:
		digi_play("801_s01", 2, 255, -1, -1);
		sendWSMessage_60000(_roomStates_field9Eh);
		_roomStates_field9Eh = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1280, 0, triggerMachineByHashCallback, "mei chen looks to zero");
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 1, 7, 0, _roomStates_field782, 7, 7, 0);
		_roomStates_fieldA2h = series_show("SAFARI SHADOW 5", 1280, 16, -1, -1, 0, 50, 124, 304);
		break;
	case 12:
		_roomStates_field6 = 1;
		break;
	case 13:
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 4, 1, 0, _roomStates_field742, 1, 1, 0);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 0, _roomStates_field782, 42, 55, 4);
		digi_play("801m02", 1, 255, 35, -1);
		break;
	case 14:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field682, 1, 20, 15, _roomStates_field682, 20, 20, 0);
		digi_play("801f02a", 1, 255, 16, -1);
		break;
	case 15:
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field682, 20, 1, 12, _roomStates_field742, 1, 1, 0);
		break;
	case 16:
		if (_roomStates_field6) {
			_roomStates_field6 = 0;
			_roomStates_loop62 = series_load("RIP TREK HEAD TURN POS5", -1, nullptr);
			setGlobals1(_roomStates_loop62, 1, 7, 5, 7, 1, 7, 1, 1, 1, 0, 1, 1, 1, 1, 0, 7, 7, 7, 7, 0);
			sendWSMessage_110000(_G(my_walker), 4);
			digi_play("801r02", 1, 255, 53, -1);
		} else {
			kernel_timing_trigger(15, 16, nullptr);
		}
		break;
	case 17:
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field682, 20, 1, 18, _roomStates_field682, 1, 1, 0);
		break;
	case 18:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 37, 0, _roomStates_field782, 37, 41, 4);
		digi_play("801m04", 1, 255, 41, -1);
		break;
	case 19:
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field642, 1, 1, 0, _roomStates_field642, 1, 1, 0);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 37, 0, _roomStates_field782, 37, 41, 4);
		digi_play("801m06", 1, 255, 43, -1);
		break;
	case 31:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field7E, 18, 1, 32, _roomStates_field7E, 1, 1, 0);
		sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_field2A, 13, 1, 52, _roomStates_field2A, 1, 1, 0);
		break;
	case 32:
		_roomStates_tt2 = 1;
		break;
	case 33:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 17, 8, 49, _roomStates_field782, 8, 8, 0);
		break;
	case 34:
		if (_roomStates_tt2) {
			_roomStates_tt2 = 0;
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 0, _roomStates_field782, 42, 42, 0);
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 4, 0, _roomStates_field742, 5, 7, 4);
			digi_play("801f01", 1, 255, 13, -1);
		} else {
			kernel_timing_trigger(15, 34, nullptr);
		}
		break;
	case 35:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 0, _roomStates_field782, 42, 42, 0);
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 4, 0, _roomStates_field742, 5, 7, 4);
		digi_play("801f02", 1, 255, 14, -1);
		break;
	case 36:
		if (_roomStates_tt2) {
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, -1, _roomStates_field782, 42, 42, 0);
		} else {
			kernel_timing_trigger(15, 36, nullptr);
		}
		break;
	case 37:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 83, 64, 38, _roomStates_field782, 64, 64, 0);
		kernel_timing_trigger(0, 39, nullptr);
		break;
	case 38:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 64, 56, 32, _roomStates_field782, 56, 56, 0);
		break;
	case 39:
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field682, 1, 20, 17, _roomStates_field682, 20, 20, 0);
		break;
	case 41:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 37, 37, 0, _roomStates_field782, 37, 37, 0);
		sendWSMessage_110000(_G(my_walker), 4);
		digi_play("801r03", 1, 255, 54, -1);
		break;
	case 42:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 41, 31, 0, _roomStates_field782, 31, 31, 0);
		digi_play("801r04", 1, 255, 55, -1);
		break;
	case 43:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 41, 31, 0, _roomStates_field782, 42, 42, 0);
		_roomStates_field4C2 = series_load("RIP TREK HANDS HIP POS5", -1, nullptr);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("801r05", 1, 255, 56, -1);
		break;
	case 48:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 56, 83, 37, _roomStates_field782, 83, 83, 0);
		break;
	case 49:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 18, 30, 50, _roomStates_field782, 30, 30, 0);
		break;
	case 50:
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 30, 18, 32, _roomStates_field782, 42, 55, 4);
		break;
	case 51:
		digi_play("801r01", 1, 255, -1, -1);
		break;
	case 52:
		if (_roomStates_field6 && _roomStates_field4 && _roomStates_tt2) {
			terminateMachine(_roomStates_field9Eh);
			terminateMachine(_roomStates_field96h);
			terminateMachine(_roomStates_field8Ah);
			terminateMachine(_roomStates_field8Eh);
			ws_unhide_walker(_G(my_walker));
			series_unload(_roomStates_field702);
			series_unload(_roomStates_field7E);
			series_unload(_roomStates_field2A);
			_roomStates_field96h = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1024, 0, triggerMachineByHashCallback, "farmer talk frames");
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 1, 0, _roomStates_field742, 1, 1, 0);
			_roomStates_field9Eh = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1280, 0, triggerMachineByHashCallback, "mc talk frames");
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 8, 17, 33, _roomStates_field782, 17, 17, 0);
			_roomStates_field502 = series_load("RIP TREK HAT TIP POS5", -1, nullptr);
			kernel_timing_trigger(210, 62, nullptr);
			digi_play("801m01", 1, 255, 34, -1);
			_roomStates_field4 = 0;
			_roomStates_field6 = 0;
			_roomStates_tt2 = 0;
		} else {
			kernel_timing_trigger(15, 52, nullptr);
		}
		break;
	case 53:
		sendWSMessage_120000(_G(my_walker), -1);
		kernel_timing_trigger(330, 48, nullptr);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 37, 0, _roomStates_field782, 37, 41, 4);
		digi_play("801m03", 1, 255, 36, -1);
		break;
	case 54:
		sendWSMessage_120000(_G(my_walker), -1);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 37, 37, 0, _roomStates_field782, 37, 41, 4);
		digi_play("801m05", 1, 255, 42, -1);
		break;
	case 55:
		sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 4, 0, _roomStates_field742, 5, 7, 4);
		digi_play("801f03", 1, 255, 19, -1);
		break;
	case 56:
		sendWSMessage_120000(_G(my_walker), 5);
		break;
	case 60:
		sendWSMessage_140000(_G(my_walker), 63);
		break;
	case 61:
		sendWSMessage_150000(_G(my_walker), -1);
		break;
	case 62:
		setGlobals1(_roomStates_field502, 1, 12, 12, 12, 0, 12, 1, 1, 1, 0, 1, 12, 12, 12, 0, 12, 12, 12, 12, 0);
		sendWSMessage_110000(_G(my_walker), 60);
		break;
	case 63:
		series_unload(_roomStates_field502);
		break;
	case 81:
		digi_play("801R30", 1, 255, 82, 801);
		break;
	case 82:
		disable_player_commands_and_fade_init(83);
		break;
	case 83:
		_G(game).new_room = 852;
		break;
	case 200:
		switch (_roomStates_untie2) {
		case 1:
			switch (_roomStates_pu) {
			case 1:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, -1, _roomStates_field782, 42, 42, 0);
				kernel_timing_trigger(60, 200, nullptr);
				break;
			case 2:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 31, 37, 200, _roomStates_field782, 37, 41, 4);
				_roomStates_untie2 = 2;
				break;
			case 3:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 200, _roomStates_field782, 42, 55, 4);
				_roomStates_untie2 = 3;
				break;
			case 4:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 56, 84, 200, _roomStates_field782, 84, 84, 0);
				_roomStates_untie2 = 4;
				break;
			case 8:
				_roomStates_untie2 = 8;
				break;
			default:
				break;
			}
			break;
		case 2:
			switch (_roomStates_pu) {
			case 1:
			case 3:
			case 4:
			case 8:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 37, 31, 200, _roomStates_field782, 42, 42, 0);
				_roomStates_untie2 = 1;
				break;
			case 2:
				kernel_timing_trigger(30, 200, nullptr);
				break;
			default:
				break;
			}
			break;
		case 3:
			switch (_roomStates_pu) {
			case 1:
			case 2:
			case 4:
			case 8:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 42, 42, 200, _roomStates_field782, 42, 42, 0);
				_roomStates_untie2 = 1;
				break;
			case 3:
				kernel_timing_trigger(30, 200, nullptr);
				break;
			default:
				break;
			}
			break;
		case 4:
			switch (_roomStates_pu) {
			case 4:
				_roomStates_pu = 1;
				// Fallthrough
			case 1:
			case 2:
			case 3:
			case 8:
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field782, 84, 56, 200, _roomStates_field782, 42, 42, 0);
				_roomStates_untie2 = 1;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case 300:
		switch (_roomStates_pu2) {
		case 1:
			switch (_roomStates_ripTalker) {
			case 1:
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop62, 1, 1, -1, _roomStates_loop62, 1, 1, 0);
				kernel_timing_trigger(60, 300, nullptr);
				break;
			case 2:
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop62, 1, 4, 300, _roomStates_loop62, 5, 7, 4);
				_roomStates_pu2 = 2;
				break;
			case 3:
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop72, 1, 16, 300, _roomStates_loop72, 16, 16, 0);
				_roomStates_pu2 = 3;
				break;
			case 4:
				terminateMachine(_roomStates_field8Ah);
				terminateMachine(_roomStates_field8Eh);
				ws_unhide_walker(_G(my_walker));
				_roomStates_pu2 = 4;
				break;
			default:
				break;
			}

			break;
		case 2:
			switch (_roomStates_ripTalker) {
			case 1:
			case 3:
			case 4:
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop62, 4, 1, 300, _roomStates_loop62, 1, 1, 0);
				_roomStates_pu2 = 1;
				break;

			case 2:
				kernel_timing_trigger(30, 300, nullptr);
				break;
			default:
				break;
			}
			break;
		case 3:
			switch (_roomStates_ripTalker) {
			case 3:
				_roomStates_ripTalker = 1;
				// Fallthrough
			case 1:
			case 2:
			case 4:
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop72, 16, 1, 300, _roomStates_loop72, 1, 1, 0);
				mouse_lock_sprite(false);
				kernel_examine_inventory_object("PING MATCH", _G(master_palette), 5, 1, 110, 220, 9, "801R44", -1);
				_roomStates_pu2 = 1;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case 310:
		player_set_commands_allowed(true);
		_roomStates_ripTalker = 3;
		break;
	case 400:
		if (_roomStates_field18 == 5)
			_roomStates_ripTalker2 = 5;

		switch (_roomStates_ripTalker2) {
		case 1:
			switch (_roomStates_field18) {
			case 1:
				sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field642, 14, 14, -1, _roomStates_field642, 14, 14, 0);
				kernel_timing_trigger(60, 400, nullptr);
				break;
			case 2:
				sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 1, 4, 400, _roomStates_field742, 5, 7, 4);
				_roomStates_ripTalker2 = 2;
				break;
			case 3:
				sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field6C2, 69, 110, 400, _roomStates_field642, 14, 14, 0);
				_roomStates_field18 = 1;
				break;
			case 4:
				sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field6C2, 2, 68, 400, _roomStates_field642, 14, 14, 0);
				_roomStates_field18 = 1;
				break;
			case 5:
				_roomStates_ripTalker2 = 5;
				break;
			default:
				break;
			}
			break;
		case 2:
			switch (_roomStates_field18) {
			case 4:
			case 5:
				sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field742, 4, 1, 400, _roomStates_field642, 14, 14, 0);
				_roomStates_ripTalker2 = 1;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		break;

	default:
		break;
	}
}

void Room801::room801_conv801a() {
	int32 entry = conv_current_entry();
	int32 node = conv_current_node();

	if (_G(kernel).trigger == 1) {
		int32 who = conv_whos_talking();
		if (who <= 0) {
			_roomStates_pu = 1;
			_roomStates_field18 = 1;
			if (node == 3 && entry == 0) {
				inv_give_to_player("MATCH");
				_roomStates_field18 = 4;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(90, 310, nullptr);
				_G(kernel).trigger_mode = KT_PARSE;
				return;
			}
		} else if (who == 1) {
			_roomStates_ripTalker = 1;
		}

		conv_resume(conv_get_handle());
		return;
	}

	if (conv_sound_to_play() == nullptr) {
		conv_resume(conv_get_handle());
		return;
	}

	int32 who = conv_whos_talking();
	if (who <= 0) {
		if ((node == 1 && entry == 0) || (node == 1 && entry == 1) || (node == 1 && entry == 2) || (node == 4 && entry == 2))
			_roomStates_pu = 3;

		if ((node == 2 && entry == 1) || (node == 2 && entry == 2) || (node == 4 && entry == 1))
			_roomStates_pu = 2;

		if ((node == 4 && entry == 0) || (node == 2 && entry == 0) || (node == 3 && entry == 0))
			_roomStates_pu = 2;

	} else if (who == 1) {
		if (node != 1 || entry != 3)
			_roomStates_field18 = 2;
	}

	digi_play(conv_sound_to_play(), 1, 255, 1, -1);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
