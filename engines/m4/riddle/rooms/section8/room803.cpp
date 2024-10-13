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

#include "m4/riddle/rooms/section8/section8.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room803::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("other script", _G(master_palette));
}

void Room803::init() {
	digi_preload("950_s29", -1);
	_var1 = 0;

	if (_G(flags)[V276])
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
	else
		loadWalkSeries();

	initWalker();
	digi_play_loop("950_s29", 2, 127, -1, 950);
}

void Room803::parser() {
	bool cl = false;
	if (player_said("look") || player_said("look at")) {
		cl = true;
	}

	bool ch = false;
	if (player_said("take")) {
		ch = true;
	}

	bool talkFl = false;
	if (player_said("talk") || player_said("talk to")) {
		talkFl = true;
	}

	player_said("gear");

	if (cl && player_said("HOLE IN THE WALL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripTalkerPos5 = series_load("RIP LOOKS UP POS3", -1, nullptr);
			setGlobals1(_ripTalkerPos5, 1, 6, 6, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;

		case 1:
			digi_play("803R04", 1, 255, 2, -1);
			break;

		case 2:
			sendWSMessage_140000(_G(my_walker), 3);
			break;

		case 3:
			player_set_commands_allowed(true);

		default:
			break;
		}
	}

	else if (cl && player_said("MURAL")) {
		digi_play("803R05", 1, 255, -1, -1);
	}

	else if (cl && player_said("SOLDIER")) {
		digi_play("COM074", 1, 255, -1, 997);
	}

	else if (cl && player_said("FALLEN SOLDIER")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripTalkerPos5 = series_load("RIP TREK LOOK DOWN POS3", -1, nullptr);
			setGlobals1(_ripTalkerPos5, 1, 11, 11, 11, 0, 11, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;

		case 1:
			digi_play("COM062", 1, 255, 2, 997);
			break;

		case 2:
			sendWSMessage_120000(_G(my_walker), 3);
			break;

		case 3:
			sendWSMessage_150000(_G(my_walker), 4);
			break;

		case 4:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	}

	else if (cl && player_said("FALLEN STATUE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripTalkerPos5 = series_load("RIP TREK LOOK DOWN POS3", -1, nullptr);
			setGlobals1(_ripTalkerPos5, 1, 11, 11, 11, 0, 11, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;

		case 1:
			digi_play("803R06", 1, 255, 2, -1);
			break;

		case 2:
			sendWSMessage_120000(_G(my_walker), 3);
			break;

		case 3:
			sendWSMessage_150000(_G(my_walker), 4);
			break;

		case 4:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	}

	else if (cl && player_said("BROKEN STATUE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripTalkerPos5 = series_load("RIP TREK LOOK DOWN POS3", -1, nullptr);
			setGlobals1(_ripTalkerPos5, 1, 11, 11, 11, 0, 12, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 1);
			break;

		case 1:
			digi_play("803R07", 1, 255, 2, -1);
			break;

		case 2:
			sendWSMessage_120000(_G(my_walker), 3);
			break;

		case 3:
			sendWSMessage_150000(_G(my_walker), 4);
			break;

		case 4:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	}

	else if (cl && player_said("URN")) {
		digi_play("803R08", 1, 255, -1, -1);
	}

	else if (cl && player_said("UNLIT URN")) {
		digi_play("COM061", 1, 255, -1, 997);
	}

	else if (cl && player_said("FALLEN URN")) {
		digi_play("803R10", 1, 255, -1, -1);
	}

	else if (cl && player_said(" ")) {
		digi_play("803R11", 1, 255, -1, -1);
	}

	else if (cl && player_said("MEI CHEN")) {
		digi_play("COM043", 1, 255, -1, 997);
	}

	else if (cl && player_said("BROKEN BEAM")) {
		digi_play("844R12", 1, 255, -1, -1);
	}

	else if (cl && player_said("tipped soldier")) {
		digi_play("com062", 1, 255, -1, 997);
	}

	else if (cl && player_said(" ")) { // Previously checked??...
		digi_play("803R11", 1, 255, -1, -1);
	}

	else if (talkFl && player_said("MEI CHEN")) {
		player_set_commands_allowed(false);
		_G(kernel).trigger_mode = KT_DAEMON;
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("COM044", 1, 255, 33, 997);
			break;

		case 2:
			digi_play("COM045", 1, 255, 33, 997);
			break;

		case 3:
			digi_play("COM046", 1, 255, 33, 997);
			break;

		case 4:
			digi_play("COM047", 1, 255, 33, 997);
			break;

		default:
			break;
		}

		ws_demand_facing(_G(my_walker), 3);
		_ripTalkerPos5 = series_load("RIP TREK TALKER POS3", -1, nullptr);
		setGlobals1(_ripTalkerPos5, 1, 5, 1, 6, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
	}

	else if (ch && (player_said("BROKEN STATUE") || player_said("FALLEN STATUE"))) {
		digi_play("803R13", 1, 255, -1, -1);
	}

	else if (player_said("climb through", "Hole in the Wall")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("803R19", 1, 255, 1, -1);
			break;

		case 1:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	}

	else if (player_said("Fifth Door")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 2, 180, -1, 950);
			_G(game).new_room = 844;
			break;

		default:
			break;
		}
	}

	else if (player_said("Fourth Door")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 3, 180, -1, 950);
			_G(game).new_room = 834;
			break;

		default:
			break;
		}
	}

	else if (player_said("Third Door")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 3, 180, -1, 950);
			_G(game).new_room = 814;
			break;

		default:
			break;
		}
	}

	else if (player_said("Second Door")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 3, 180, -1, 950);
			_G(game).new_room = 824;
			break;

		default:
			break;
		}
	}

	else if (player_said("First Door")) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_timing_trigger(15, 1, nullptr);
			break;

		case 1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;

		case 2:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 3, 180, -1, 950);
			_G(game).new_room = 804;
			break;

		default:
			break;
		}
	}

	else if (player_said("journal") && player_said("look at")) {
		digi_play("com042", 1, 255, -1, 997);
	} else
		return;


	_G(player).command_ready = false;
}

void Room803::daemon() {
	int32 trigger = _G(kernel).trigger;

	if ((trigger >= 1) && (trigger < 15))
		daemonSub1();
	else if (trigger == 15) {
		sendWSMessage_150000(_mcWalkerMach, -1);
		player_set_commands_allowed(true);
	} else if (trigger == 16) {
		sendWSMessage_150000(_G(my_walker), -1);
	} else if (trigger == 33) {
		sendWSMessage_150000(_G(my_walker), -1);
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("COM048", 1, 255, 15, 997);
			break;
		case 2:
			digi_play("COM049", 1, 255, 15, 997);
			break;
		case 3:
			digi_play("COM050", 1, 255, 15, 997);
			break;
		case 4:
			digi_play("COM051", 1, 255, 15, 997);
			break;
		default:
			break;
		}

		_meiTalksPos3 = series_load("Mei talks pos3", -1, nullptr);
		setGlobals1(_meiTalksPos3, 1, 4, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcWalkerMach, -1);
	} else if ((trigger >= 50) && ((trigger <= 56) || (trigger == 999)))
		daemonSub1();
}

void Room803::initWalker() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[V276] == 0) {
			_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 357, 175, 3, Walker::player_walker_callback, "mc");
		}
		break;

	case 804:
		ws_demand_location(_G(my_walker), 639, 260);
		ws_demand_facing(_G(my_walker), 9);
		if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 639, 260, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 579, 260, nullptr, 50, 9, true);
		} else {
			ws_walk(_G(my_walker), 202, 309, nullptr, -1, 7, true);
		}
		break;

	case 814:
		ws_demand_location(_G(my_walker), 513, 192);
		ws_demand_facing(_G(my_walker), 9);
		if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 513, 192, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 463, 192, nullptr, 51, 9, true);
		} else {
			ws_walk(_G(my_walker), 202, 309, nullptr, -1, 7, true);
		}
		break;

	case 824:
		ws_demand_location(_G(my_walker), 570, 220);
		ws_demand_facing(_G(my_walker), 9);
		if (_G(flags[V276]) == 0) {
			player_set_commands_allowed(false);
			_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 570, 220, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 520, 220, nullptr, 52, 9, true);
		} else {
			ws_walk(_G(my_walker), 202, 309, nullptr, -1, 7, true);
		}
		break;

	case 834:
		ws_demand_location(_G(my_walker), 491, 174);
		ws_demand_facing(_G(my_walker), 9);
		if (_G(flags[V276]) == 0) {
			player_set_commands_allowed(false);
			_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 491, 174, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 441, 174, nullptr, 53, 9, true);
		} else {
			ws_walk(_G(my_walker), 202, 309, nullptr, -1, 7, true);
		}
		break;

	case 844:
		ws_demand_location(_G(my_walker), 475, 161);
		ws_demand_facing(_G(my_walker), 9);
		if (_G(flags[V276]) == 0) {
			player_set_commands_allowed(false);
			_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 475, 161, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 425, 161, nullptr, 54, 9, true);
		} else {
			ws_walk(_G(my_walker), 202, 309, nullptr, -1, 7, true);
		}
		break;

	default:
		ws_demand_location(_G(my_walker), 202, 239);
		ws_demand_facing(_G(my_walker), 7);
		if (_G(flags[V276]) == 0) {
			ws_hide_walker(_G(my_walker));
			_meiStepOffPileMach = series_play("MEI STEPS OFF PILE", 0, 15, 2, 5, 0, 100, 0, 0, 0, 0);
			kernel_timing_trigger(15, 1, nullptr);
		}

		break;
	}

	series_play("BRAZIERS 1-6 BURNING", 3840, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 8 BURNING", 2304, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 9 BURNING", 1792, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 10 BURNING", 1024, 0, -1, 5, -1, 100, 0, 0, 0, -1);
}

void Room803::loadWalkSeries() {
	ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
}

void Room803::daemonSub1() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(false);
		terminateMachine(_meiStepOffPileMach);
		_meiStepOffPileMach = series_play("MEI STEPS OFF PILE", 0, 15, 2, 5, 0, 100, 0, 0, 0, 56);
		break;

	case 2:
		digi_play("803m01", 1, 255, 3, -1);
		break;

	case 3:
		if (_meiStepOffPileMach != nullptr)
			terminateMachine(_meiStepOffPileMach);

		_meiStepOffPileMach = series_play("MEI STEPS OFF PILE", 0, 16, 4, 5, 0, 100, 0, 0, 56, 57);
		break;

	case 4:
		if (_meiStepOffPileMach != nullptr)
			terminateMachine(_meiStepOffPileMach);

		_meiStepOffPile = series_load("MEI STEPS OFF PILE", -1, nullptr);
		_meiStepOffPileMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, 0, triggerMachineByHashCallback, "rip");
		sendWSMessage_10000(1, _meiStepOffPileMach, _meiStepOffPile, 57, 50, -1, _meiStepOffPile, 57, 60, 1);
		digi_play("803r01", 1, 255, 5, -1);
		break;

	case 5:
		sendWSMessage_10000(1, _meiStepOffPileMach, _meiStepOffPile, 57, 57, -1, _meiStepOffPile, 57, 57, 0);
		digi_play("803m02", 1, 255, 6, -1);
		break;

	case 6:
		sendWSMessage_10000(1, _meiStepOffPileMach, _meiStepOffPile, 61, 67, 7, _meiStepOffPile, 67, 67, 0);
		digi_play("803r02", 1, 255, -1, -1);
		break;

	case 7:
		sendWSMessage_10000(1, _meiStepOffPileMach, _meiStepOffPile, 67, 57, 8, _meiStepOffPile, 57, 57, 0);
		break;

	case 8:
		sendWSMessage_150000(_meiStepOffPileMach, -1);

		if (_meiStepOffPileMach != nullptr) {
			terminateMachine(_meiStepOffPileMach);
			series_unload(_meiStepOffPile);
		}

		_ripStanderMach = series_play("RIP STANDER", 0, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		_meiLooksAboutMach = series_play("MEI LOOKS ABOUT", 0, 16, 9, 5, 0, 100, 0, 0, 0, 32);
		break;

	case 9:
		digi_play("803m03", 1, 255, 10, -1);
		break;

	case 10:
		sendWSMessage_150000(_meiLooksAboutMach, -1);
		if (_meiLooksAboutMach != nullptr)
			terminateMachine(_meiLooksAboutMach);

		_ripTalkerPos5 = series_load("RIP TALKER POS 5", -1, nullptr);
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 7);
		setGlobals1(_ripTalkerPos5, 1, 4, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("803r03", 1, 255, 11, -1);
		_mcWalkerMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 186, 252, 1, Walker::player_walker_callback, "mc");
		sendWSMessage_10000(_mcWalkerMach, 244, 252, 3, 12, 1);
		if (_ripStanderMach != nullptr)
			terminateMachine(_ripStanderMach);
		break;

	case 11:
		sendWSMessage_150000(_G(my_walker), -1);
		ws_unhide_walker(_G(my_walker));
		break;

	case 12:
		sendWSMessage_10000(_mcWalkerMach, 357, 175, 1, 13, 1);
		break;

	case 13:
	case 56:
		player_set_commands_allowed(true);
		break;

	case 14:
		// Nothing
		break;

	case 50:
		ws_walk(_G(my_walker), 202, 239, nullptr, -1, 7, true);
		ws_walk(_mcWalkerMach, 450, 260, nullptr, 55, 1, true);
		break;

	case 51:
		ws_walk(_G(my_walker), 202, 239, nullptr, -1, 7, true);
		ws_walk(_mcWalkerMach, 400, 191, nullptr, 55, 1, true);
		break;

	case 52:
		ws_walk(_G(my_walker), 202, 239, nullptr, -1, 7, true);
		ws_walk(_mcWalkerMach, 390, 220, nullptr, 55, 1, true);
		break;

	case 53:
	case 54:
		ws_walk(_G(my_walker), 202, 239, nullptr, -1, 7, true);
		sendWSMessage_10000(_mcWalkerMach, 357, 175, 1, 55, 1);
		break;

	case 55:
		sendWSMessage_10000(_mcWalkerMach, 357, 175, 1, 56, 1);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
