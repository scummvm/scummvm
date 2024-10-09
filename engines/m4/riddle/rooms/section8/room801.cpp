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

#include "section8.h"
#include "glk/zcode/glk_interface.h"
#include "m4/core/cstring.h"
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
		_roomStates_field9Eh = triggerMachineByHash_3000(8, 4, S8_SHADOW_DIRS2, S8_SHADOW_DIRS1,
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
		_G(player).command_ready = false;
		return;
	}

	if (_G(kernel).trigger == 747) {
		_roomStates_ripTalker = 4;
		_roomStates_pu = 8;
		_roomStates_field18 = 5;
		conv_shutdown();
		player_set_commands_allowed(true);
		_G(player).command_ready = false;
		return;
	}

	if (lookFl && _G(walker).ripley_said(SAID)) {
		_G(player).command_ready = false;
		return;
	}


	_G(player).command_ready = false;
}

void Room801::daemon() {
}

void Room801::room801_conv801a() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
