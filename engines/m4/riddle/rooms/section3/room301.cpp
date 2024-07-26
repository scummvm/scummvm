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
	_G(globals)[V000] = 1;
	digi_preload("950_s01");

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = 0;
		_soundName = nullptr;
		_nextSound = nullptr;
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

		if (_val12 || (!player_been_here(401) && _G(globals)[V092] && !_G(globals)[V093])) {
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
	int frame;

	switch (_G(kernel).trigger) {
	// TODO: More cases

	case 21:
		if (!_val19) {
			switch (_val15) {
			case 0:
				sendWSMessage_10000(1, _machine4, _ripTrekTravel,
					10, 10, 20, _ripTrekTravel, 10, 10, 0);
				break;

			case 1:
				frame = imath_ranged_rand(11, 19);
				sendWSMessage_10000(1, _machine4, _ripTrekTravel,
					frame, frame, 20, _ripTrekTravel, frame, frame, 0);
				sendWSMessage_190000(_machine4, 13);
				sendWSMessage_1a0000(_machine4, 13);
				break;

			case 2:
				sendWSMessage_10000(1, _machine4, _ripTrekTravel,
					20, 26, 20, _ripTrekTravel, 10, 10, 0);
				break;

			case 3:
				sendWSMessage_10000(1, _machine4, _ripTrekTravel,
					37, 50, 20, _ripTrekTravel, 10, 10, 0);
				break;

			case 4:
				sendWSMessage_10000(1, _machine4, _ripTrekTravel,
					9, 1, 22, _ripTrekTravel, 1, 1, 0);
				break;

			case 5:
				_G(kernel).trigger_mode = KT_PARSE;
				conv_load("conv301a", 10, 10, 747, player_commands_allowed());
				_val1 = 0;

				conv_export_value(conv_get_handle(), _G(globals)[V088] >= 3 ? 1 : 0, 0);
				conv_export_value(conv_get_handle(), _G(globals)[V056], 1);
				conv_export_value(conv_get_handle(), _G(globals)[V110], 2);
				conv_export_pointer(conv_get_handle(), &_val1, 5);

				if (player_been_here(201))
					conv_export_value(conv_get_handle(), 2, 6);

				conv_play(conv_get_handle());
				_val15 = 0;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 20);
				break;

			default:
				break;
			}
		}
		break;

	case 22:
		terminateMachineAndNull(_machine4);
		terminateMachineAndNull(_machine3);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 25:
		sendWSMessage_10000(1, _machine1, _agentSlidesPaper, 50, 64, 26,
			_agentSlidesPaper, 64, 64, 0);
		digi_play("950_s35", 2);
		break;

	case 26:
		sendWSMessage_10000(1, _machine1, _agentSlidesPaper, 49, 1, 27,
			_agentStander, 1, 1, 0);
		break;

	case 27:
		sendWSMessage_10000(1, _machine1, _agentTakesMoney, 52, 9, 28,
			_agentTakesMoney, 9, 9, 0);
		_val14 = 4;
		_val13 = 4;
		_val8 = 0;
		_val15 = 0;
		break;

	case 28:
		conv_resume();
		kernel_timing_trigger(1, 13);
		break;

	case 51:
		digi_unload("301r37");
		setGlobals1(_ripTrekHandTalk3, 1, 7, 7, 7, 1);
		sendWSMessage_110000(52);
		digi_play("301r01", 1, 255, 53);
		break;

	case 52:
		sendWSMessage_140000(-1);
		break;

	case 53:
		_val14 = 6;
		_soundName = "301a01";
		_val16 = 56;
		break;

	case 56:
		_val14 = 3;
		kernel_timing_trigger(1, 11);
		player_set_commands_allowed(true);
		break;

	case 60:
		setGlobals1(_ripTrekHandTalk3, 1, 7, 7, 7, 1);
		sendWSMessage_110000(62);
		digi_play("301r01", 1);
		break;

	case 62:
		sendWSMessage_140000(63);
		break;

	case 63:
		_val14 = 6;
		_val7 = 2;
		_soundName = "301a01";
		_val16 = 72;
		break;

	case 64:
		if (_val12) {
			_val14 = 5;
			_soundName = (_val12 == 1) ? "301a03" : "301a04";
			_val7 = 2;
			_val16 = 71;
		} else if (!player_been_here(401) && _G(globals)[V092] &&
				!_G(globals)[V093]) {
			kernel_timing_trigger(1, 1000);
		}
		break;

	case 65:
		_G(global301) = 0;
		setGlobals1(_ripTrekTalker3, 1, 1, 1, 5, 1);
		sendWSMessage_110000(68);
		digi_play("301r01a", 1, 255, 68);
		break;

	case 68:
		if (_G(global301) >= 1) {
			_G(global301) = 0;
			sendWSMessage_140000(-1);
			_val14 = 0;
			_soundName = "301a04a";
			_val16 = 70;
		} else {
			++_G(global301);
		}
		break;

	case 70:
		_val14 = 0;
		player_set_commands_allowed(true);
		break;

	case 71:
		_val14 = 0;
		kernel_timing_trigger(1, 999);
		break;

	case 72:
		_val14 = 4;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 11);
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(10, 64);
		break;

	case 100:
		sendWSMessage_10000(1, _machine1, _agentCheckingList, 8,
			1, 10, _agentStander, 1, 1, 0);
		break;

	case 200:
		if (_val18 || _val17 || _val5 == -1) {
			kernel_timing_trigger(1, 201);
		} else {
			kernel_trigger_dispatchx(_val5);
			_val5 = -1;
		}

	case 201:
		if (_val18 <= 0) {
			if (_val17 <= 0) {
				kernel_timing_trigger(30, 201);
			} if (_val17 == 1) {
				sendWSMessage_10000(1, _machine2, _marshalMatt, 17, 51, 201,
					_marshalMatt, 51, 51, 0);
				_val18 = 1;
				_val17 = 1;
			}
		} else if (_val18 == 1) {
			if (_val17 == 1) {
				frame = imath_ranged_rand(52, 55);
				sendWSMessage_10000(1, _machine2, _marshalMatt, frame, frame, 201,
					_marshalMatt, frame, frame, 0);
				sendWSMessage_190000(_machine2, 13);
				sendWSMessage_1a0000(_machine2, 13);

			} else {
				sendWSMessage_10000(1, _machine2, _marshalMatt, 52, 52, 200,
					_marshalMatt, 52, 52, 0);
			}
		}
		break;

	case 202:
		digi_play("301s01", 1, 255, 203);
		sendWSMessage_10000(1, _machine2, _marshalMatt, 4, 16, 201,
			_marshalMatt, 16, 16, 0);
		_val17 = 0;
		_val18 = 0;
		break;

	case 203:
		setGlobals1(_ripTrekArms, 1, 15, 15, 15, 1);
		sendWSMessage_110000(-1);
		digi_play("301r22", 1, 255, 204);
		break;

	case 204:
		digi_play("301s02", 1, 255, 205);
		_val17 = 1;
		break;

	case 205:
		_val17 = 0;
		kernel_timing_trigger(60, 206);
		break;

	case 206:
		disable_player_commands_and_fade_init(207);
		break;

	case 207:
		sendWSMessage_150000(208);
		break;

	case 208:
		ws_demand_facing(3);
		kernel_timing_trigger(1, 209);
		break;

	case 209:
		_G(game).setRoom(413);
		break;

	case 300:
		terminateMachineAndNull(_machine1);
		_machine4 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, "rip");
		sendWSMessage_10000(1, _machine4, _agentStander, 7,
			1, 304, _agentStander, 1, 1, 0);
		break;

	case 304:
		ws_hide_walker();
		sendWSMessage(1, _machine4, _agentTakesTelegram,
			1, 53, 305, _agentTakesTelegram, 53, 53, 0);
		break;

	case 305:
		kernel_timing_trigger(1, 990);
		inv_give_to_player("MESSAGE LOG");
		break;

	case 320:
		sendWSMessage(1, _machine4, _agentTakesTelegram,
			54, 63, 322, _agentTakesTelegram, 63, 63, 0);
		break;

	case 322:
		sendWSMessage_10000(1, _machine4, _ripTrekTravel,
			10, 1, 324, _ripTrekTravel, 1, 1, 0);
		_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, "guy behind desk");
		_val13 = 0;
		_val14 = 0;
		sendWSMessage_10000(1, _machine1, _agentStander, 1,
			1, 10, _agentStander, 1, 1, 0);
		break;

	case 324:
		terminateMachineAndNull(_machine4);
		ws_unhide_walker();
		kernel_timing_trigger(1, 1000);
		break;

	case 990:
		_digiSound1 = _digiSound2 = 0;
		_digiSound3 = _digiSound4 = 0;

		if (_val12 <= 0) {
			kernel_timing_trigger(1, 320);
		} else if (_nextSound) {
			_digiSound1 = _nextSound;
			_nextSound = nullptr;
			kernel_timing_trigger(1, 991);
		} else  if (_G(globals)[V365] == 1) {
			_digiSound1 = "201R61";
			_G(globals)[V365] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(globals)[V366] == 1) {
			_digiSound1 = "401R31";
			_G(globals)[V366] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(globals)[V373] == 1) {
			_digiSound1 = "401R36";
			_G(globals)[V373] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(globals)[V371] == 1) {
			_digiSound1 = "501R03C";
			_G(globals)[V371] = 0;
			kernel_timing_trigger(1, 991);
		} else if (_G(globals)[V372] == 1) {
			_digiSound1 = "701R39";
			_digiSound2 = "701R39A";
			_G(globals)[V372] = 0;
			kernel_timing_trigger(1, 991);
		}
		break;

	case 991:
		if (_digiSound1)
			digi_play(_digiSound1, 1, 255, 993);
		else
			kernel_timing_trigger(1, 993);
		break;

	case 993:
		if (_digiSound2)
			digi_play(_digiSound2, 1, 255, 994);
		else
			kernel_timing_trigger(1, 994);
		break;

	case 994:
		if (_digiSound3)
			digi_play(_digiSound3, 1, 255, 995);
		else
			kernel_timing_trigger(1, 995);
		break;

	case 995:
		if (_digiSound4)
			digi_play(_digiSound4, 1, 255, 996);
		else
			kernel_timing_trigger(1, 996);
		break;

	case 996:
		if (--_val12 == 0)
			kernel_timing_trigger(1, 320);
		else
			kernel_timing_trigger(1, 990);
		break;

	case 999:
		kernel_timing_trigger(1, !_val12 ? 1000 : 300);
		break;

	case 1000:
		if (!player_been_here(401) && _G(globals)[V092]
			&& !_G(globals)[V093]) {
			_G(globals)[V092] = 0;
			_G(globals)[V093] = 1;
			kernel_timing_trigger(30, 65);
		}

		player_set_commands_allowed(true);
		break;
	}
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
				setGlobals1(_ripTrekHandTalk3, 1, 7, 8, 9, 1, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(-1);
				break;
			case 3:
				sendWSMessage_140000(-1);
				_val14 = 5;
				_val6 = 1;
				_val7 = 1;
				_val16 = 4;
				_soundName = "301a02";
				break;
			case 4:
				_val14 = 0;
				_val5 = kernel_trigger_create(5);
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 11);
				_G(kernel).trigger_mode = oldMode;
				break;
			case 5:
				player_set_commands_allowed(true);
				_G(globals)[V033] = 1;
				break;
			default:
				break;
			}
		}
	} else if (player_said_any("AMERICAN CURRENCY", "CHINESE CURRENCY") &&
			player_said("AGENT")) {
		digi_play("301r30", 1);
	} else if (lookFlag && player_said_any("sofa", "chair")) {
		digi_play("301r29", 1);
	} else if (_G(kernel).trigger == 747) {
		player_set_commands_allowed(false);
		_val14 = 0;
		_val15 = 4;

		if (_val1) {
			switch (_val1) {
			case 1:
				_G(globals)[V129] = 0;
				break;
			case 2:
				_G(globals)[V129] = 2;
				break;
			case 3:
				_G(globals)[V129] = 3;
				break;
			case 4:
				_G(globals)[V129] = 4;
				break;
			default:
				break;
			}

			disable_player_commands_and_fade_init(748);
		}

	} else if (_G(kernel).trigger == 748) {
		_G(game).setRoom(495);

	} else if (talkFlag && player_said("agent")) {
		ws_hide_walker();
		player_set_commands_allowed(false);
		player_update_info();

		_machine3 = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_machine4 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, "rip");

		_G(kernel).trigger_mode = KT_DAEMON;
		_val19 = 0;
		_val15 = 5;
		sendWSMessage_10000(1, _machine4, _ripTrekTravel, 1, 9, 20,
			_ripTrekTravel, 10, 10, 0);
		_val14 = 4;
	} else if (takeFlag && player_said("postcards")) {
		digi_play("301r09", 1);
	} else if (takeFlag && player_said("water cooler")) {
		digi_play("301r10", 1);
	} else if (takeFlag && player_said("magazines")) {
		digi_play("301r11", 1);
	} else if (useFlag && player_said_any("sofa", "chair")) {
		digi_play("301r31", 1);
	} else if (useFlag && player_said("telephone")) {
		digi_play("301r32", 1);
	} else if (useFlag && _G(player).click_y <= 374 &&
			!inv_player_has(_G(player).noun)) {
		digi_play("301r33", 1);
	} else {
		return;
	}

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
