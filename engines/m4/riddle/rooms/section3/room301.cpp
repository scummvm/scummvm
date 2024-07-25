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

#include "m4/riddle/rooms/section3/room301.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room301::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	_G(player).walker_in_this_scene = true;
	LoadWSAssets("OTHER SCRIPT");
}

void Room301::init() {
	_G(flags)[V000] = 1;
	digi_preload("950_s01");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = 0;
		_soundName = nullptr;
		_val4 = 0;
		_val5 = -1;
		_val6 = 2;
		_val7 = 2;
		_val8 = 0;
		_val9 = 0;
		_val10 = 0;
		_val11 = 0;
		_val12 = 0;
	}

	_val13 = 0;
	_val14 = 0;

	_ripTrekHandTalk3 = series_load("rip trek hand talk pos3");
	_ripTrekTalker3 = series_load("RIP TREK TALKER POS3");
	_ripTrekTravel = series_load("rip trek travel disp");
	_agentStander = series_load("agent stander / talk disp");
	_agentCheckingList = series_load("agent checking list on desk");
	_agentSlidesPaper = series_load("AGENT SLIDES PAPER TO RIP");
	_agentTakesMoney = series_load("agent takes money from rip");
	_agentTakesTelegram = series_load("agent takes telegram from slot");
	_agentTalk = series_load("agent animated talk disp");
	_agentSalutes = series_load("agent salutes rip");
	_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
		triggerMachineByHashCallbackNegative, "guy behind desk");

	sendWSMessage(1, _machine1, _agentStander, 1, 1, 10, _agentStander, 1, 1, 0);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		if (inv_object_in_scene("ROMANOV EMERALD", 305)) {
			setFlag45();
			inv_move_object("ROMANOV EMERALD", NOWHERE);
		}

		_val12 = checkFlags(false);
		player_set_commands_allowed(false);
		ws_demand_location(55, 270, 3);

		if (_val12 || (!player_been_here(401) && _G(flags)[V092] && !_G(flags)[V093])) {
			ws_walk(352, 269, nullptr, 60, 3);
		} else if (!player_been_here(301)) {
			digi_preload("301r37");
			digi_play("301r37", 1, 255, 51);
			ws_walk(165, 270, nullptr, -1, 3);
		} else {
			ws_walk(165, 270, nullptr, 51, 3);
		}
	}

	digi_play_loop("950_s01", 3, 140);
}

void Room301::daemon() {
	// TODO
}

void Room301::pre_parser() {
	if (player_said("exit") && _G(globals)[GLB_TEMP_1]) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room301::parser() {
	auto oldMode = _G(kernel).trigger_mode;
	bool lookFlag = player_said_any("look", "look at");		// ecx
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");					// edi
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close"); // esi

	if (player_said("con301a")) {
		conv301a();
	} else if (player_said("exit")) {
		if (_G(globals)[GLB_TEMP_1]) {
			if (_G(kernel).trigger == -1) {
				player_set_commands_allowed(false);
				_marshalMatt = series_load("marshall matt");
				digi_preload("301s01");
				_ripTrekArms = series_load("rip trek arms x pos3");
				ws_walk(200, 269, nullptr, 1, 9);
			} else if (_G(kernel).trigger == 1) {
				_G(kernel).trigger_mode = KT_DAEMON;
				_machine2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
					triggerMachineByHashCallbackNegative, "marshal");
				sendWSMessage(1, _machine2, _marshalMatt, 1, 2, 202, _marshalMatt, 3, 3, 0);
				_val17 = 0;
				_val18 = 0;
			}
		} else {
			if (_G(kernel).trigger == -1) {
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(1);
			} else if (_G(kernel).trigger == 1) {
				_G(game).new_room = 303;
			}
		}
	} else if (lookFlag && player_said("poster")) {
		digi_play("301r04", 1);
	} else if (lookFlag && player_said("window")) {
		digi_play(_G(player).been_here_before ? "301r15" : "301r05", 1);
	} else if (lookFlag && player_said("magazines")) {
		digi_play("301r06", 1);
	} else if (lookFlag && player_said("water cooler")) {
		digi_play(_G(player).been_here_before ? "301r16" : "301r07", 1);
	} else if (lookFlag && player_said("agent")) {
		digi_play("301r08", 1);
	} else if (lookFlag && player_said("telephone")) {
		digi_play("301r27", 1);
	} else if (lookFlag && player_said("plant")) {
		digi_play("301r28", 1);
	} else if (lookFlag && player_said("postcards")) {
		if (_G(globals)[V033]) {
			digi_play("301r14", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				digi_play("301r03", 1, 255, 3);
				kernel_timing_trigger(260, 1);
				break;
			case 1:
				player_update_info();
				ws_walk(_G(player_info).x, _G(player_info).y,
					nullptr, 2, 3);
				break;
			case 2:
				// TODO
				break;
			default:
				break;
			}
		}
	}


	// TODO
	_G(player).command_ready = false;
}

void Room301::conv301a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();

	if (_G(kernel).trigger == 1 || !sound) {
		if (who <= 0) {
			if (node != 3) {
				_val14 = 4;
				conv_resume();
			}
		} else if (who == 1) {
			if (node == 11) {
				_val14 = 12;
			} else if (node != 13) {
				_val15 = 0;
				conv_resume();
			} else {
				conv_resume();
			}
		} else {
			conv_resume();
		}
	} else {
		if (who <= 0) {
			if (node == 3) {
				_val14 = 15;
			} else if (node != 2) {
				_val14 = imath_ranged_rand(5, 6);
			} else {
				_val14 = 2;
				_soundName = sound;
				_val16 = 1;
				return;
			}
		} else if (who == 1) {
			if (node == 13)
				_val14 = 13;
			else
				_val15 = 1;
		}

		digi_play(sound, 1, 255, 1);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
