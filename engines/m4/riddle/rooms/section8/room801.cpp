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
	_roomStates_field64 = series_load("farmer displacement", -1, nullptr);
	_roomStates_field74 = series_load("farmer  talk loop", -1, nullptr);
	_roomStates_field6C = series_load("farmer give take bow", -1, nullptr);
	_roomStates_field78 = series_load("mei prattle loop", -1, nullptr);
	_roomStates_loop1 = series_load("rip kneel to pottery", -1, nullptr);
	_roomStates_loop2 = series_load("rip enters root cellar", -1, nullptr);
	_roomStates_loop6 = series_load("rip trek head turn pos5", -1, nullptr);
	_roomStates_loop7 = series_load("rptmr15", -1, nullptr);
	_roomStates_val1 = series_load("rip trek hand on chin pos2", -1, nullptr);
	_roomStates_ear2 = series_load("rip trek hands beh back pos1", -1, nullptr);
	_roomStates_val3 = series_load("rip trek nod pos4", -1, nullptr);
	_roomStates_val2 = series_load("rip looks l r down", -1, nullptr);
	series_load("SAFARI SHADOW 5", -1, nullptr);
	_roomStates_loop4 = series_load("MCTSH1", -1, nullptr);
	_roomStates_field96h = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1024, 0, triggerMachineByHashCallbackNegative, "farmer displacement");
	sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field64, 1, 14, 0, _roomStates_field64, 14, 14, 0);
	_roomStates_field9Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 151, 317, 100, 1280, 0, triggerMachineByHashCallbackNegative, "farmer shadow");
	sendWSMessage_10000(1, _roomStates_field9Ah, _roomStates_loop4, 1, 1, -1, _roomStates_loop4, 1, 1, 0);
	if (inv_player_has("farmer's shovel"))
		hotspot_set_active(_G(currentSceneDef).hotspots, "farmer's shovel", false);
	else {
		_roomStates_field80 = series_load("801 SHOVEL ", -1, nullptr);
		_roomStates_field86h = series_place_sprite("801 SHOVEL ", 0, 0, 0, 100, 1024);
	}

	if (_G(game).previous_room == 850) {
		_G(flags)	[V250] = 0;
		_G(flags)	[V251] = 0;
		_G(flags)	[V252] = 0;
		_G(flags)	[V273] = 0;

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
		_roomStates_field68 = series_load("farmer  earthquake gesture", -1, nullptr);
		_roomStates_loop0 = series_load("rip looks to zero", -1, nullptr);
		_roomStates_field7C = series_load("mei looks to zero", -1, nullptr);
		_roomStates_field70 = series_load("farmer looks to zero", -1, nullptr);
		_roomStates_field60 = series_load("shadow of de plane", -1, nullptr);
		ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);
		_roomStates_field9Eh = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1,
														 20, 310, 3, triggerMachineByHashCallback3000, "mc_trek");
		kernel_timing_trigger(60, 1, nullptr);
	} else if (_G(game).previous_room < 850 || _G(game).previous_room != -2) {
		// CHECKME: the second part of the check is useless. That's suspicious.
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
	} else {
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

		_roomStates_field9Eh = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1280, 0, triggerMachineByHashCallbackNegative, "mc talk frames");
		_roomStates_fieldA2h = series_show("SAFARI SHADOW 5", 1280, 16, -1, -1, 0, 50, 124, 304);
		sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 42, 42, 0, _roomStates_field78, 42, 42, 0);
	}
}

void Room801::pre_parser() {
	player_said("take", nullptr, nullptr);
	bool lookCheck = false;
	if (player_said("look", nullptr, nullptr) || player_said("look at", nullptr, nullptr)) {
		lookCheck = true;
	}

	if (player_said("push", nullptr, nullptr) || player_said("pull", nullptr, nullptr)
		|| player_said("gear", nullptr, nullptr) || player_said("open", nullptr, nullptr)) {
		player_said("close", nullptr, nullptr);
	}

	player_said("go", nullptr, nullptr);
	if (lookCheck && player_said("farm", nullptr, nullptr)) {
		digi_play("801R23", 1, 255, -1, -1);
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (lookCheck && player_said(" ", nullptr, nullptr)) {
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

	if (player_said("look", nullptr, nullptr) || player_said("look at", nullptr, nullptr)) {
		lookFl = true;
	}

	if (player_said("talk", nullptr, nullptr) || player_said("talk TO", nullptr, nullptr)) {
		talkFl = true;
	}

	if (player_said("take", nullptr, nullptr)) {
		takeFl = true;
	}

	if (player_said("gear", nullptr, nullptr)) {
		gearFl = true;
	}

	if (player_said("go", nullptr, nullptr)) {
		goFl = true;
	}

	if (player_said("conv801a", nullptr, nullptr)) {
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
		; // Yep, nothing
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
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 31, 40, 3, _roomStates_field78, 40, 40, 0);
			break;
		case 3:
			if (_var1 == 0)
				++_var1;
			else {
				_var1 = 0;
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop2, 42, 26, 4, _roomStates_loop2, 26, 26, 0);
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 37, 40, 4, _roomStates_field78, 37, 40, 1);
				digi_play("801m10", 1, 255, 4, -1);
			}
			break;
		case 4:
			if (_var1 == 2)
				++_var1;
			else {
				_var1 = 0;
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 40, 40, 5, _roomStates_field78, 40, 40, 0);
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
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 37, 40, 9, _roomStates_field78, 37, 40, 1);
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
				sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 40, 31, 10, _roomStates_field78, 31, 31, 0);
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
			setGlobals1(_roomStates_loop6, 1, 1, 1, 1, 0, 1, 7, 5, 7, 1, 7, 1, 1, 1, 0, 7, 7, 7, 7, 0);
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
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 42, 55, 0, _roomStates_field78, 53, 55, 1);
			digi_play("801m12", 1, 255, 4, -1);
			break;
		case 4:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 55, 55, 0, _roomStates_field78, 55, 55, 0);
			digi_play("801f06", 1, 255, 5, -1);
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field74, 1, 1, 0, _roomStates_field74, 1, 3, 4);
			break;
		case 5:
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field74, 1, 1, 0, _roomStates_field74, 1, 1, 0);
			sendWSMessage_150000(_G(my_walker), 0);
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 31, 40, 6, _roomStates_field78, 40, 40, 0);
			break;
		case 6:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 40, 41, 0, _roomStates_field78, 39, 41, 4);
			digi_play("801m13", 1, 255, 7, -1);
			break;
		case 7:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 41, 31, 0, _roomStates_field78, 31, 31, 0);
			setGlobals1(_roomStates_loop7, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 1, 1, 1, 1, 1, 16, 16, 16, 16, 0);
			sendWSMessage_110000(_G(my_walker), 9);
			kernel_timing_trigger(50, 8, nullptr);
			inv_move_object("CHINESE YUAN", 801);
			digi_play("801r28", 1, 255, 9, -1);
			break;
		case 8:
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field6C, 71, 110, 10, _roomStates_field74, 110, 110, 0);
			break;
		case 9:
			sendWSMessage_120000(_G(my_walker), 0);
			break;
		case 10:
			sendWSMessage_10000(1, _roomStates_field96h, _roomStates_field74, 1, 1, 0, _roomStates_field74, 1, 1, 0);
			sendWSMessage_150000(_G(my_walker), 12);
			break;
		case 12:
			player_update_info(_G(my_walker), &_G(player_info));
			ws_walk(_G(my_walker), 8, 6, nullptr, 13, 5, true);
			break;
		case 13:
			hotspot_set_active(_G(currentSceneDef).hotspots, "farmer's shovel", false);
			setGlobals1(_roomStates_loop7, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 1, 1, 1, 1, 1, 16, 16, 16, 16, 0);
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
			if (_roomStates_tt == 0) {
				player_set_commands_allowed(false);
				ws_hide_walker();
				terminateMachine(_roomStates_fieldA6h);
				_roomStates_field18 = series_load("RIP OPENS CELLAR", -1, nullptr);
				_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, 0, triggerMachineByHashCallbackNegative, "rip opens cellar");
				sendWSMessage_10000(1, _roomStates_field8Ah, 1, _roomStates_field18, 28, 10, _roomStates_field18, 28, 28, 0);
			}
			break;
		case 1:
			_roomStates_tt = 1;
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
			if (_roomStates_tt != 0) {
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_roomStates_fieldAAh);
				_roomStates_ripTalking = series_load("RIP CLOSES CELLAR", -1, nullptr);
				_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, 0, triggerMachineByHashCallbackNegative, "rip closes cellar");
				sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_ripTalking, 1, 19, 10, _roomStates_ripTalking, 34, 34, 0);
			}
			break;
		case 1:
			_roomStates_tt = 0;
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
			_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 167, 303, 49, 1280, 1, triggerMachineByHashCallbackNegative, "rip talking to farmer");
			sendWSMessage_10000(1, _roomStates_field8Ah, _roomStates_loop6, 1, 1, -1, _roomStates_loop6, 1, 1, 0);
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
			setGlobals1(_roomStates_loop6, 1, 4, 5, 7, 1, 4, 4, 4, 4, 1, 4, 1, 1, 1, 0, 4, 1, 1, 1, 0);
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
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 31, 40, 5, _roomStates_field78, 40, 40, 0);
			break;
		case 5:
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 40, 41, 0, _roomStates_field78, 40, 41, 4);
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
			sendWSMessage_10000(1, _roomStates_field9Eh, _roomStates_field78, 41, 31, 7, _roomStates_field78, 31, 31, 0);
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
			_roomStates_field8Ah = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, 0, triggerMachineByHashCallbackNegative, "rip enters cellar");
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
}

void Room801::room801_conv801a() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
