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

#include "m4/riddle/rooms/section4/room402.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *const SAID[][2] = {
	{ "CASTLE",      "402r09" },
	{ "CASTLE DOOR", "402r09" },
	{ "FLOWERBED",   "402r10" },
	{ "WINDOW",      "402r11" },
	{ "GATE",        "402r14" },
	{ "WHEELBARROW", "402r15" },
	{ nullptr, nullptr }
};

void Room402::init() {
	player_set_commands_allowed(false);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val4 = 0;
		_val5 = -1;
		_val6 = 0;
		_val8 = 0;
		_currentNode = -1;
		_val10 = 0;
		_val11 = 0;
		_val12 = 0;
		_val13 = 0;
		_sound1.clear();
		_sound2.clear();
	}

	hotspot_set_active("WOLF", false);
	hotspot_set_active("TOPIARY", false);
	hotspot_set_active("TOPIARY ", false);

	_wolfie = series_place_sprite("WOLFIE WHLBRW OUTSIDE CASTLE", 0, 0, 0, 100, 0x300);
	_shadow3 = series_load("SAFARI SHADOW 3");

	digi_preload("950_s19");
	digi_preload("950_s20");
	digi_preload("950_s21");
	digi_preload("950_s23");

	if (!_G(flags)[GLB_TEMP_12] && !_G(flags)[V117]) {
		if (!_G(flags)[V110])
			_G(flags)[V131] = 402;
		else if (inv_player_has("TURTLE"))
			_G(flags)[V131] = 408;
		else
			_G(flags)[V131] = _G(flags)[V124] ? 402 : 403;
	}

	if (inv_player_has("PLANK")) {
		_G(flags)[V139] = 1;
	} else if (inv_player_has("STEP LADDER")) {
		_G(flags)[V139] = 2;
	} else if (inv_player_has("EDGER")) {
		_G(flags)[V139] = inv_player_has("TURTLE") ? 3 : 4;
	} else {
		ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
		ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);

		if (!player_been_here(402)) {
			digi_preload("950_s22");
			hotspot_set_active("WOLF", true);
			hotspot_set_active("TOPIARY", true);

			_G(flags)[V131] = 402;
			_G(flags)[V318] = 0;

			_ripPaysWolfie = series_load("RIP PAYS WOLFIE");
			_ripTalkWolf = series_load("RIP TALKS TO WOLF");
			_ripTalker = series_load("RIP TREK TALKER POS3");
			_wolfClipping = series_load("WOLF CLIPPING LOOP");
			_wolfClippersDown = series_load("WOLF CLPRS DOWN TURNS POS9");
			_wolfShakesHead = series_load("WOLF TALKS SHAKES HEAD");
			_wolfWantsMoney = series_load("WOLF WANTS MONEY");
			_ripHeadTurn = series_load("RIP TREK HEAD TURN POS3");

			_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
				triggerMachineByHashCallback, "WOLFIE");
			sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
				_wolfClipping, 10, 10, 0);
			_val12 = 2001;
			_val13 = 2300;
			ws_demand_facing(11);

			if (!_G(kittyScreaming)) {
				ws_demand_location(660, 290);
				digi_play("402_S03", 1, 255, 19);
			} else {
				ws_demand_location(425, 285);
				player_set_commands_allowed(true);
			}

		} else if (_G(flags)[V131] != 402) {
			hotspot_set_active("TOPIARY ", true);

			switch (_G(game).previous_room) {
			case KERNEL_RESTORING_GAME:
				digi_preload("950_s22");

				if (_G(flags)[V117]) {
					_branch = series_place_sprite("sprite of the pulled out branch",
						0, 0, -53, 100, 0x300);
				}

				player_set_commands_allowed(true);
				break;

			case 404:
				digi_preload("950_s22");
				_ripDownStairs = series_load("RIP DOWN STAIRS");
				ws_hide_walker();
				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
					triggerMachineByHashCallback, "rip leaving castle");
				sendWSMessage_10000(1, _ripEnterLeave, _ripDownStairs, 1, 27, 55,
					_ripDownStairs, 27, 27, 0);
				break;

			case 408:
				ws_demand_location(517, 239, 3);
				ws_walk(510, 260, nullptr, 50, 8);
				break;

			default:
				digi_preload("950_s22");
				ws_demand_location(660, 290, 3);
				ws_walk(612, 287, nullptr, 50, 9);
				midi_fade_volume(0, 120);
				break;
			}
		} else {
			if (!_G(flags)[V117]) {
				hotspot_set_active("WOLF", true);
				hotspot_set_active("TOPIARY", true);

				_ripPaysWolfie = series_load("RIP PAYS WOLFIE");
				_ripTalkWolf = series_load("RIP TALKS TO WOLF");
				_ripTalker = series_load("RIP TREK TALKER POS3");
				_wolfClipping = series_load("WOLF CLIPPING LOOP");
				_wolfClippersDown = series_load("WOLF CLPRS DOWN TURNS POS9");
				_wolfShakesHead = series_load("WOLF TALKS SHAKES HEAD");
				_wolfWantsMoney = series_load("WOLF WANTS MONEY");

				_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
					triggerMachineByHashCallback, "WOLFIE");

				if (_val6) {
					sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 16, -1,
						_wolfWantsMoney, 16, 16, 0);
				} else {
					_val12 = 2001;
					_val13 = 2300;
					sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
						_wolfClipping, 10, 10, 0);
				}
			}

			switch (_G(game).previous_room) {
			case KERNEL_RESTORING_GAME:
				digi_preload("950_s22");

				if (_G(flags)[V117])
					_branch = series_place_sprite("sprite of the pulled out branch", 0, 0, -53, 100, 0x300);
				player_set_commands_allowed(true);
				break;

			case 404:
				digi_preload("950_s22");
				_ripDownStairs = series_load("RIP DOWN STAIRS");
				ws_hide_walker();

				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
					triggerMachineByHashCallback, "rip leaving castle");
				sendWSMessage_10000(1, _ripEnterLeave, _ripDownStairs, 1, 27, 55,
					_ripDownStairs, 27, 27, 0);
				break;

			case 408:
				if (_G(flags)[V125] == 3) {
					_G(flags)[GLB_TEMP_12] = 1;
					_G(flags)[V125] = 4;
					_G(flags)[V131] = 999;

					_wolfClipping = series_load("WOLF CLIPPING LOOP");
					_wolfClippersDown = series_load("WOLF CLPRS DOWN TURNS POS9");
					_wolfTurnsClipping = series_load("WOLF TURNS CLPNG TO POS1");
					_turtlePopup = series_load("402 TURTLE POPUP");
					_ripLeanWall = series_load("Rip leans against far wall");

					_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
						triggerMachineByHashCallback, "WOLFIE");
					_val12 = 2001;
					_val13 = 2300;

					sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 100,
						_wolfClipping, 10, 10, 0);
					ws_demand_location(517, 239, 3);
					ws_walk(503, 248, nullptr, 350, 7);
				} else if (_G(flags)[V117] != 0 && inv_player_has("TURTLE")) {
					_G(flags)[V117] = 0;
					hotspot_set_active("TOPIARY ", true);
					_G(flags)[V131] = 408;
					ws_demand_location(517, 239, 3);

					_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS,
						475, 300, 11, triggerMachineByHashCallback3000, "wolf_walker");
					sendWSMessage_10000(_wolfWalker, 549, 239, 9, 42, 0);
					kernel_timing_trigger(90, 40);
				} else {
					ws_demand_location(517, 239, 3);
					ws_walk(449, 317, nullptr, 30, 3);
				}
				break;

			default:
				digi_preload("950_s22");
				ws_demand_location(660, 290, 3);
				ws_walk(449, 317, nullptr, 30, 3);
				midi_fade_volume(0, 120);

				if (inv_player_has("TURTLE"))
					inv_move_object("STEP LADDER", NOWHERE);
				break;
			}
		}
	}

	if (_G(flags)[V139]) {
		_wolfClipping = series_load("WOLF CLIPPING LOOP");
		_wolfieMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
			triggerMachineByHashCallback, "WOLFIE");
		_val12 = 2001;
		_val13 = 2300;

		sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
			_wolfClipping, 10, 10, 0);
		ws_demand_location(517, 239, 3);
		ws_walk(510, 260, nullptr, 370, 8);
	}

	digi_play_loop("950_s22", 3, 120);
}

void Room402::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 19:
		ws_walk(449, 317, nullptr, 20, 3);
		midi_fade_volume(0, 120);
		break;

	case 20:
		midi_stop();
		setGlobals1(_ripTalker, 1, 5, 1, 5, 1, 1, 1, 1, 1);
		sendWSMessage_110000(-1);
		digi_play("402r01", 1, 255, 21);
		break;

	case 21:
		sendWSMessage_120000(-1);
		digi_play("402w01", 1, 255, 22);
		break;

	case 22:
		sendWSMessage_150000(23);
		break;

	case 23:
		setGlobals1(_ripHeadTurn, 1, 5, 5, 5, 0, 5, 1, 1, 1, 0, 1, 1, 1, 1, 1);
		sendWSMessage_110000(24);
		digi_play("402r02", 1, 255, 25);
		break;

	case 24:
		sendWSMessage_120000(-1);
		break;

	case 25:
		sendWSMessage_130000(-1);
		digi_play("402w02", 1, 255, 26);
		break;

	case 26:
		sendWSMessage_150000(27);
		break;

	case 27:
		ws_hide_walker();
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "RIP talks to Wolife");
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 1, 13, 28,
			_ripTalkWolf, 13, 13, 0);
		digi_play("402r03", 1);
		break;

	case 28:
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 13, 1, 29,
			_ripTalkWolf, 1, 1, 0);
		break;

	case 29:
		terminateMachineAndNull(_ripEnterLeave);
		series_unload(_ripHeadTurn);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 30:
		setGlobals1(_ripTalker, 0, 0, 1, 1, 0, 1, 5, 1, 5, 1);
		sendWSMessage_110000(32);
		break;

	case 32:
		digi_play("402r29", 1, 255, 33);
		sendWSMessage_120000(-1);
		break;

	case 33:
		sendWSMessage_110000(-1);
		digi_play("402w09", 1, 255, 34);
		break;

	case 34:
		sendWSMessage_150000(35);
		break;

	case 35:
	case 43:
		player_set_commands_allowed(true);
		break;

	case 40:
		ws_walk(510, 260, nullptr, -1, 8);
		break;

	case 42:
		sendWSMessage_10000(_wolfWalker, 517, 239, 9, 43, 0);
		break;

	case 50:
		midi_stop();
		player_set_commands_allowed(true);
		break;

	case 55:
		terminateMachineAndNull(_ripEnterLeave);
		series_unload(_ripDownStairs);
		ws_unhide_walker();
		ws_demand_location(345, 275, 3);
		ws_walk(375, 279, nullptr,
			(_G(flags)[V131] == 402) ? 56 : 50,
			4);
		break;

	case 56:
		ws_walk(449, 317, nullptr, 30, 3);
		break;

	case 100:
		kernel_timing_trigger(1, 102);
		break;

	case 101:
		switch (_val10) {
		case 1000:
			_val11 = 1105;
			break;
		case 1001:
			_val5 = (_G(flags)[V132]) ? 300 : 1112;
			break;

		default:
			break;
		}
		break;

	case 102:
		if (_val2 == -1) {
			kernel_timing_trigger(1, 103);
		} else {
			kernel_timing_trigger(1, _val2);
			_val2 = -1;
		}
		break;

	case 103:
		switch (_val10) {
		case 1000:
			switch (_val11) {
			case 1100:
				ws_hide_walker();
				player_set_commands_allowed(false);
				player_update_info();

				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");
				_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf SHADOW");

				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, 1, 1, -1,
					_ripTalker, 1, 1, 0);
				sendWSMessage_10000(1, _safariShadow, _shadow3, 1, 1, 102,
					_shadow3, 1, 1, 0);
				_val11 = 1101;
				_val12 = 2000;
				_val13 = 2100;
				kernel_timing_trigger(1, 110);
				break;

			case 1101:
				_val11 = 1103;
				kernel_timing_trigger(1, 102);
				conv_load("conv402a", 10, 10, 101);

				conv_export_value_curr(inv_player_has("POMERANIAN MARKS") ? 1 : 0, 0);
				conv_export_pointer_curr(&_G(flags)[V111], 1);
				conv_export_value_curr(_G(flags)[V112], 2);
				conv_export_value_curr(_G(flags)[V114], 4);
				conv_export_pointer_curr(&_G(flags)[V115], 5);
				conv_export_pointer_curr(&_G(flags)[V135], 7);
				conv_export_pointer_curr(&_G(flags)[V337], 8);
				conv_export_value_curr(_G(flags)[V110], 9);
				conv_export_pointer_curr(&_G(flags)[V314], 10);
				conv_play();
				break;

			case 1102:
				frame = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, frame, frame, 102,
					_ripTalker, frame, frame, 0);
				break;

			case 1103:
				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, 1, 1, 102,
					_ripTalker, 1, 1, 0);
				break;

			case 1104:
				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, 1, 1, -1,
					_ripTalker, 1, 1, 0);
				break;

			case 1105:
				terminateMachineAndNull(_ripEnterLeave);
				terminateMachineAndNull(_safariShadow);
				ws_unhide_walker();

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1);
					_sound1.clear();
				}

				_G(flags)[V114] = 0;
				_G(flags)[V112] = 0;

				if (_G(flags)[V115] && !_val8) {
					_val6 = 1;
					_val12 = 2000;
					_val13 = 2230;
				} else {
					_val8 = 0;
					_val12 = 2001;
					_val13 = 2300;
				}

				player_set_commands_allowed(true);
				break;

			case 1120:
				terminateMachineAndNull(_ripEnterLeave);
				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");
				_val11 = 1122;
				_letter = series_load("RIP SHOWS WOLF THE LETTER");
				sendWSMessage_10000(1, _ripEnterLeave, _letter, 1, 12, 103,
					_letter, 12, 12, 0);
				break;

			case 1122:
				if (!_sound1.empty()) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_sound1.c_str(), 1, 255, 777);
					_G(kernel).trigger_mode = KT_DAEMON;
					_sound1.clear();
				}
				break;

			case 1123:
				_val11 = 1124;
				sendWSMessage_10000(1, _ripEnterLeave, _letter, 12, 1, 103,
					_ripTalker, 1, 1, 0);
				break;

			case 1124:
				terminateMachineAndNull(_ripEnterLeave);
				series_unload(_letter);
				player_update_info();

				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");
				_val10 = 1000;
				_val11 = 1103;
				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, 1, 1, 102,
					_ripTalker, 1, 1, 0);
				conv_resume();
				break;

			default:
				break;
			}
			break;

		case 1001:
			switch (_val11) {
			case 1110:
				player_update_info();
				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");

				player_update_info();
				_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x200, 0,
					triggerMachineByHashCallback, "rip talks wolf SHADOW");
				sendWSMessage_10000(1, _safariShadow, _shadow3, 1, 1, -1,
					_shadow3, 1, 1, 0);
				ws_hide_walker();

				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, 1, 1, -1,
					_ripTalker, 1, 1, 0);
				player_set_commands_allowed(false);
				_val12 = 2002;
				_val13 = 2141;
				kernel_timing_trigger(1, 110);

				_val11 = 1111;
				kernel_timing_trigger(1, 102);
				break;

			case 1111:
				_val10 = 1001;
				_val11 = 1115;
				kernel_timing_trigger(1, 102);

				conv_load("conv402a", 0, 10, 101);
				conv_export_value_curr(inv_player_has("POMERANIAN MARKS") ? 1 : 0, 0);
				conv_export_pointer_curr(&_G(flags)[V111], 1);
				conv_export_value_curr(_G(flags)[V112], 2);
				conv_export_value_curr(_G(flags)[V114], 4);
				conv_export_pointer_curr(&_G(flags)[V115], 5);
				conv_export_pointer_curr(&_G(flags)[V135], 7);
				conv_export_pointer_curr(&_G(flags)[V337], 8);
				conv_export_value_curr(_G(flags)[V110], 9);
				conv_export_pointer_curr(&_G(flags)[V314], 10);
				conv_play();
				break;

			case 1112:
				terminateMachineAndNull(_ripEnterLeave);
				terminateMachineAndNull(_safariShadow);
				ws_unhide_walker();
				_G(flags)[V114] = 0;
				_G(flags)[V112] = 0;

				if (_currentNode == 19 || _currentNode == 22 || _currentNode == 23) {
					_val12 = 2002;
					_val13 = 2190;
					kernel_timing_trigger(1, 110);
				} else if (!_G(flags)[V115]) {
					_val12 = 2001;
					_val13 = 2300;
					player_set_commands_allowed(true);
				} else {
					_val6 = 1;

					if (!_sound2.empty()) {
						_val13 = 2142;
						_val11 = 1113;
						digi_play(_sound2.c_str(), 1, 255, 103);
						_sound2.clear();
					}
				}
				break;

			case 1113:
				_val13 = 2150;
				break;

			case 1114:
				frame = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, frame, frame, 102,
					_ripTalker, frame, frame, 0);
				break;

			case 1115:
				sendWSMessage_10000(1, _ripEnterLeave, _ripTalker, 1, 1, 102,
					_ripTalker, 1, 1, 0);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 110:
		switch (_val12) {
		case 2000:
			if (_val5 == -1) {
				kernel_timing_trigger(1, 111);
			} else {
				kernel_timing_trigger(1, _val5);
				_val5 = -1;
			}
			break;

		case 2001:
			if (_val13 == 2300) {
				if (_val5 == -1) {
					kernel_timing_trigger(1, 111);
				} else {
					kernel_timing_trigger(1, _val5);
					_val5 = -1;
				}
			}
			break;

		case 2002:
			if (_val5 == -1) {
				kernel_timing_trigger(1, 111);
			} else {
				kernel_timing_trigger(1, _val5);
				_val5 = -1;
			}
			break;

		default:
			break;
		}
		break;

	case 111:
		switch (_val12) {
		case 2000:
			switch (_val13) {
			case 2100:
				sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 8, 8, 110,
					_wolfClipping, 8, 8, 0);
				_val13 = 2104;
				break;

			case 2101:
				frame = imath_ranged_rand(8, 10);
				sendWSMessage_10000(1, _wolfieMach, _wolfClipping, frame, frame, 110,
					_wolfClipping, frame, frame, 0);
				break;

			case 2104:
				sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 8, 8, 110,
					_wolfClipping, 8, 8, 0);
				break;

			case 2105:
				sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 8, 10, -1,
					_wolfClipping, 10, 10, 0);
				break;

			case 2106:
				sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
					_wolfClipping, 10, 10, 0);
				break;

			case 2107:
				_machine1 = series_stream("402 WOLF TAKE", 7, 0, 111);
				_val13 = 2110;
				break;

			case 2108:
				sendWSMessage_10000(1, _machine1, _machine1Series, 44, 16, 111,
					_machine1Series, 16, 16, 0);
				_val13 = 2110;
				break;

			case 2109:
				kernel_timing_trigger(4, 111);
				_val13 = 2108;
				break;

			case 2110:
				terminateMachineAndNull(_machine1);
				_val13 = 2101;
				kernel_timing_trigger(1, 110);

				if (!_sound2.empty()) {
					_G(kernel).trigger_mode = KT_PARSE;
					digi_play(_sound2.c_str(), 1, 255, 777);
					_G(kernel).trigger_mode = KT_DAEMON;
					_sound2.clear();
				}
				break;

			case 2230:
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 1, 8, 111,
					_wolfClippersDown, 8, 8, 0);
				_val13 = 2231;
				break;

			case 2231:
				digi_play("402_s04", 2);
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 8, 43, 111,
					_wolfClippersDown, 43, 43, 0);
				_val13 = 2232;
				break;

			case 2232:
				if (!_sound2.empty()) {
					digi_play(_sound2.c_str(), 1);
					_sound2.clear();
				}

				_val13 = 2233;
				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, 111,
					_wolfWantsMoney, 16, 16, 0);
				break;

			case 2233:
				if (_G(flags)[V111] > 0) {
					_val13 = 2234;
					series_stream("Rip popup", 7, 0, 111);
				} else {
					player_set_commands_allowed(true);
				}
				break;

			case 2234:
				player_set_commands_allowed(true);
				break;

			case 2240:
				_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");
				player_update_info();

				_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x200, 0,
					triggerMachineByHashCallback, "rip talks wolf SHADOW");
				sendWSMessage_10000(1, _safariShadow, _shadow3, 1, 1, -1,
					_shadow3, 1, 1, 0);
				ws_hide_walker();

				sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 1, 7, 111,
					_ripTalkWolf, 7, 7, 0);
				_val13 = 2242;
				break;

			case 2242:
				sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 1, 16, 214,
					_ripPaysWolfie, 16, 16, 0);
				_flags111 = _G(flags)[V111];
				break;

			case 2250:
				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 1, 111,
					_wolfWantsMoney, 1, 1, 0);
				_val13 = 2252;
				break;

			case 2252:
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 1, 111,
					_wolfClipping, 1, 1, 0);
				_val13 = 2253;
				break;

			case 2253:
				_val12 = 2001;
				_val13 = 2300;
				kernel_timing_trigger(1, 110);

				if (!player_said("enter", "CASTLE DOOR") && !_G(flags)[V052]) {
					player_set_commands_allowed(true);
				}
				break;

			default:
				break;
			}
			break;

		case 2001:
			if (_val13 == 2300) {
				sendWSMessage_10000(1, _wolfieMach, _wolfClipping, 1, 10, 110,
					_wolfClipping, 10, 10, 0);
				kernel_timing_trigger(35, 10000);
			}
			break;

		case 2002:
			switch (_val13) {
			case 2141:
				sendWSMessage_10000(1, _wolfieMach, _wolfShakesHead, 1, 1, 110,
					_wolfShakesHead, 1, 1, 0);
				_val13 = 2143;
				break;

			case 2142:
				frame = imath_ranged_rand(1, 5);
				sendWSMessage_10000(1, _wolfieMach, _wolfShakesHead, frame, frame, 110,
					_wolfShakesHead, frame, frame, 0);
				break;

			case 2143:
				sendWSMessage_10000(1, _wolfieMach, _wolfShakesHead, 1, 1, 110,
					_wolfShakesHead, 1, 1, 0);
				break;

			case 2150:
				_val13 = 2152;
				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, 111,
					_wolfWantsMoney, 16, 16, 0);

				if (!_sound2.empty()) {
					digi_play(_sound2.c_str(), 1);
					_sound2.clear();
				}
				break;

			case 2152:
				_val13 = 2153;

				if (_G(flags)[V111] > 0) {
					_machine1 = series_stream("Rip popup", 7, 0, 111);
				} else {
					kernel_timing_trigger(1, 111);
				}
				break;

			case 2153:
				player_set_commands_allowed(true);
				break;

			case 2160:
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 1, 111,
					_wolfClipping, 1, 1, 0);
				_val13 = 2162;
				break;

			case 2162:
				_val12 = 2000;
				_val13 = 2104;
				kernel_timing_trigger(1, 110);
				conv_resume();
				break;

			case 2170:
				_val13 = 2272;
				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, -1,
					_wolfWantsMoney, 16, 16, 0);

				if (!_sound2.empty()) {
					digi_play(_sound2.c_str(), 1, 255, 111);
					_sound2.clear();
				}
				break;

			case 2180:
				_val13 = 2181;
				digi_play(_sound2.c_str(), 1, 255, 111);
				break;

			case 2181:
				_val13 = 2182;
				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 1, 111,
					_wolfWantsMoney, 1, 1, 0);
				break;

			case 2182:
				_val12 = 2002;
				_val13 = 2143;
				kernel_timing_trigger(1, 110);
				conv_resume();
				break;

			case 2190:
				if (!_sound2.empty()) {
					digi_play(_sound2.c_str(), 1);
					_sound2.clear();
				}

				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 1, 111,
					_wolfWantsMoney, 1, 1, 0);
				_val13 = 2192;
				break;

			case 2192:
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 1, 111,
					_wolfClipping, 1, 1, 0);
				_val13 = 2193;
				break;

			case 2193:
				_val12 = 2001;
				_val13 = 2300;
				kernel_timing_trigger(1, 110);
				player_set_commands_allowed(true);
				break;

			case 2260:
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 1, 8, 111,
					_wolfClippersDown, 8, 8, 0);
				_val13 = 2261;
				break;

			case 2261:
				digi_play("402_s04", 2);
				sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 8, 43, 111,
					_wolfClippersDown, 43, 43, 0);
				_val13 = 2262;
				break;

			case 2262:
				if (_G(flags)[V111] > 0 || _currentNode == 16 ||
						_currentNode == 14 || _currentNode == 26) {
					sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, -1,
						_wolfWantsMoney, 16, 16, 0);

					if (!_sound2.empty()) {
						_G(kernel).trigger_mode = KT_PARSE;
						digi_play(_sound2.c_str(), 1, 255, 777);
						_G(kernel).trigger_mode = KT_DAEMON;
						_sound2.clear();
					}
				} else {
					_val13 = 2263;

					if (!_sound2.empty()) {
						digi_play(_sound2.c_str(), 1, 255, 111);
					} else {
						kernel_timing_trigger(1, 111);
					}
				}

				if (inv_player_has("POMERANIAN MARKS")) {
					player_set_commands_allowed(true);
				}
				break;

			case 2263:
				sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, 110,
					_wolfWantsMoney, 16, 16, 0);
				_val13 = 2264;
				break;

			case 2264:
				_machine1 = series_stream("Rip popup", 7, 0, 111);
				_val13 = 2265;
				break;

			case 2265:
				if (!_sound2.empty()) {
					_sound2.clear();
					_G(kernel).trigger_mode = KT_PARSE;
					kernel_timing_trigger(1, 777);
					_G(kernel).trigger_mode = KT_PARSE;

				} else {
					player_set_commands_allowed(true);
				}
				break;

			case 2272:
				conv_resume();
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 200:
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");

		player_update_info();
		_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x200, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 1, 7, -1,
			_ripTalkWolf, 7, 7, 0);
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 1, 8, 201,
			_wolfClippersDown, 8, 8, 0);
		break;

	case 201:
		digi_play("402_s04", 2);
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 8, 43, 202,
			_wolfClippersDown, 43, 43, 0);
		break;

	case 202:
		sendWSMessage_10000(1, _wolfieMach, _ripPaysWolfie, 1, 24, 203,
			_ripPaysWolfie, 24, 24, 0);
		break;

	case 203:
		sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, 204,
			_wolfWantsMoney, 16, 16, 0);

		if (!_G(flags)[V115])
			digi_play("402w07", 1);
		break;

	case 204:
		digi_play("950_s23", 2);
		kernel_timing_trigger(30, 205);
		break;

	case 205:
		sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 1, 208,
			_wolfWantsMoney, 1, 1, 0);
		sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 24, 1, 206,
			_ripPaysWolfie, 1, 1, 0);
		break;

	case 206:
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 7, 1, 207,
			_ripTalkWolf, 1, 1, 0);
		break;

	case 207:
	case 236:
		terminateMachineAndNull(_ripEnterLeave);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		break;

	case 208:
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 1, 209,
			_wolfClipping, 1, 1, 0);
		break;

	case 209:
		kernel_timing_trigger(1, 110);
		player_set_commands_allowed(true);
		break;

	case 210:
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");
		player_update_info();
		_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x200, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _safariShadow, _shadow3, 1, 1, -1,
			_shadow3, 1, 1, 0);
		ws_hide_walker();
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 1, 7, 211,
			_ripTalkWolf, 7, 7, 0);
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 1, 8, 213,
			_wolfClippersDown, 8, 8, 0);
		break;

	case 211:
		sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 1, 16, -1,
			_ripPaysWolfie, 16, 16, 0);
		break;

	case 212:
		sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 1, 16, 214,
			_wolfWantsMoney, 16, 16, 0);
		_flags111 = _G(flags)[V111];
		break;

	case 213:
		digi_play("402_s04", 2);
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 8, 43, 212,
			_wolfClippersDown, 43, 43, 0);
		break;

	case 214:
		sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 17, 24, 215,
			_ripPaysWolfie, 24, 24, 0);
		break;

	case 215:
		digi_play("950_s23", 2);
		kernel_timing_trigger(30, 216);
		break;

	case 216:
		if (_G(flags)[V111] > 1) {
			_flags111--;
			sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 24, 17, 214,
				_ripPaysWolfie, 17, 17, 0);
		} else {
			sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 1, -1,
				_wolfWantsMoney, 1, 1, 0);
			sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 24, 1, 217,
				_ripPaysWolfie, 1, 1, 0);
		}
		break;

	case 217:
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 7, 1, 218,
			_ripTalkWolf, 1, 1, 0);
		break;

	case 218:
		terminateMachineAndNull(_ripEnterLeave);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		_val5 = -1;
		_val10 = 1001;
		_val11 = 1110;
		kernel_timing_trigger(1, 102);
		break;

	case 221:
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");
		player_update_info();
		_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x200, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _safariShadow, _shadow3, 1, 1, -1,
			_shadow3, 1, 1, 0);
		ws_hide_walker();
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 1, 7, -1,
			_ripTalkWolf, 7, 7, 0);
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 1, 8, 222,
			_wolfClippersDown, 8, 8, 0);
		break;

	case 222:
		digi_play("402_s04", 2);
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 8, 43, 223,
			_wolfClippersDown, 43, 43, 0);
		break;

	case 223:
		sendWSMessage_10000(1, _wolfieMach, _ripPaysWolfie, 1, 24, 224,
			_ripPaysWolfie, 24, 24, 0);
		break;

	case 224:
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 1, 227,
			_wolfClipping, 1, 1, 0);
		digi_play("402w08", 1);
		kernel_timing_trigger(60, 225);
		break;

	case 225:
		sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 24, 1, 226,
			_ripPaysWolfie, 1, 1, 0);
		break;

	case 226:
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 7, 1, -1,
			_ripTalkWolf, 1, 1, 0);
		break;

	case 227:
		terminateMachineAndNull(_ripEnterLeave);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		kernel_timing_trigger(1, 110);
		player_set_commands_allowed(true);
		break;

	case 230:
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "trip talks wolf");
		player_update_info();
		_safariShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x200, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _safariShadow, _shadow3, 1, 1, -1,
			_shadow3, 1, 1, 0);
		ws_unhide_walker();
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 1, 7, 232,
			_ripTalkWolf, 7, 7, 0);
		break;

	case 232:
		sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 1, 24, 233,
			_ripPaysWolfie, 24, 24, 0);
		break;

	case 233:
		sendWSMessage_10000(1, _wolfieMach, _wolfWantsMoney, 16, 1, 234,
			_wolfWantsMoney, 1, 1, 0);
		break;

	case 234:
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 1, 237,
			_wolfClipping, 1, 1, 0);
		sendWSMessage_10000(1, _ripEnterLeave, _ripPaysWolfie, 24, 1, 235,
			_ripPaysWolfie, 1, 1, 0);
		break;

	case 235:
		sendWSMessage_10000(1, _ripEnterLeave, _ripTalkWolf, 7, 1, 236,
			_ripTalkWolf, 1, 1, 0);
		break;

	case 237:
		_val12 = 2001;
		_val13 = 2300;
		player_set_commands_allowed(true);
		kernel_timing_trigger(1, 110);
		break;

	case 300:
		sendWSMessage_10000(1, _wolfieMach, _wolfClippersDown, 43, 18, 302,
			_wolfClippersDown, 18, 18, 0);
		midi_play("DANZIG1", 1, 255, -1, 949);
		break;

	case 302:
		terminateMachineAndNull(_wolfieMach);
		_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS,
			484, 315, 11, triggerMachineByHashCallback3000, "wolf_walker");
		sendWSMessage_10000(_wolfWalker, 517, 239, 9, -1, 0);

		digi_play(_sound2.c_str(), 1);
		kernel_timing_trigger(80, 303);
		break;

	case 303:
		terminateMachineAndNull(_ripEnterLeave);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		_G(flags)[V114] = 0;
		ws_walk(517, 239, nullptr, -1, 9);
		kernel_timing_trigger(60, 304);
		break;

	case 304:
		disable_player_commands_and_fade_init(305);
		break;

	case 305:
		_G(flags)[V112] = 1;
		_G(game).setRoom(408);
		break;

	case 350:
		ws_hide_walker();
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip leans against wall");
		sendWSMessage_10000(1, _ripEnterLeave, _ripLeanWall, 1, 23, 351,
			_ripLeanWall, 23, 23, 0);

		_turtlePopupMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, 0,
			triggerMachineByHashCallback, "TURTLE POPUP");
		_val14 = 0;
		digi_preload("950_s27", 950);
		break;

	case 351:
		if (_val14 == 1) {
			digi_play("950_s27", 3, 255, 353, 950);
			_val5 = 354;
		} else {
			digi_play("950_s27", 3, 255, 352, 950);
			++_val14;
		}
		break;

	case 352:
		kernel_timing_trigger(5, 351);
		break;

	case 353:
		sendWSMessage_10000(1, _turtlePopupMach, _turtlePopup, 15, 41, 353,
			_turtlePopup, 41, 41, 0);
		digi_play("950_s27", 2, 255, -1, 950);
		break;

	case 354:
		sendWSMessage_10000(1, _wolfieMach, _wolfTurnsClipping, 1, 38, 355,
			_wolfTurnsClipping, 38, 38, 0);
		break;

	case 355:
		digi_play("402w10", 1, 255, 356);
		break;

	case 356:
		digi_play("402r32", 1, 255, 357);
		break;

	case 357:
		digi_play("402w11", 1);
		terminateMachineAndNull(_wolfieMach);
		_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS,
			484, 315, 11, triggerMachineByHashCallback3000, "wolf_walker");
		sendWSMessage_10000(_wolfWalker, 517, 239, 9, -1, 0);
		kernel_timing_trigger(150, 358);
		break;

	case 358:
		disable_player_commands_and_fade_init(359);
		break;

	case 359:
		midi_stop();
		_G(game).setRoom(409);
		break;

	case 370:
		digi_play("402w06", 1, 255, 372);
		break;

	case 372:
		digi_play("402r07", 1);
		ws_walk(517, 239, nullptr, -1, 9);
		kernel_timing_trigger(90, 373);
		break;

	case 373:
		disable_player_commands_and_fade_init(374);
		break;

	case 374:
		if (_G(flags)[V139] == 1 || _G(flags)[V139] == 3)
			_G(game).setRoom(408);
		else
			_G(game).setRoom(403);
		break;

	case 10000:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("950_s19", 2);
			break;
		case 2:
			digi_play("950_s20", 2);
			break;
		case 3:
			digi_play("950_s21", 2);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

void Room402::pre_parser() {
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool lookFlag = player_said_any("look", "look at");

	if (lookFlag && player_said(" "))
		_G(player).resetWalk();

	if (player_said("journal") && !takeFlag && !lookFlag && _G(kernel).trigger == -1)
		_G(player).resetWalk();

	if (player_said("DANZIG") && !player_said("ENTER", "DANZIG"))
		_G(player).resetWalk();

	if (!_G(flags)[V112] && !player_said("WALK TO") &&
		(!talkFlag || !player_said("WOLF")) &&
		!player_said("POMERANIAN MARKS", "WOLF"))
		_G(flags)[V112] = 1;

	if (_val6 == 0)
		return;

	if (player_said("POMERANIAN MARKS", "WOLF") && inv_player_has("POMERANIAN MARKS")) {
		player_set_commands_allowed(false);
		intr_cancel_sentence();
		_G(flags)[V114] = 1;
		_val12 = 2000;
		_val13 = 2240;

		_G(flags)[V111]++;
		kernel_timing_trigger(1, 110, KT_DAEMON, KT_PREPARSE);

	} else if (!_G(flags)[V116] && player_said("WOLF") && (
			HAS("US DOLLARS") || HAS("CHINESE YUAN") ||
			HAS("PERUVIAN INTI") || HAS("SIKKIMESE RUPEE"))) {
		player_set_commands_allowed(false);
		_G(flags)[V116] = 1;
		intr_cancel_sentence();
		kernel_timing_trigger(1, 230, KT_DAEMON, KT_PREPARSE);
	} else {
		if (talkFlag && player_said("WOLF"))
			intr_cancel_sentence();

		player_set_commands_allowed(false);
		_val12 = 2000;
		_val13 = 2250;
		kernel_timing_trigger(1, 110, KT_DAEMON, KT_PREPARSE);
	}
}

void Room402::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool enterFlag = player_said("enter");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv402a")) {
		if (_G(kernel).trigger) {
			conv402a777();
		} else {
			conv402a();
		}
	} else if (talkFlag && player_said("WOLF")) {
		player_set_commands_allowed(false);
		_val5 = -1;
		_val10 = 1000;
		_val11 = 1100;

		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 102);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (lookFlag && player_said("WOLF")) {
		digi_play(_G(flags)[V111] ? "402r13" : "402r12", 1);
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (lookFlag && player_said_any("TOPIARY", "TOPIARY ")) {
		digi_play("408r02", 1);
	} else if (takeFlag && player_said("WHEELBARROW")) {
		digi_play("402r17", 1);
	} else if (takeFlag && player_said_any("TOPIARY", "TOPIARY ")) {
		digi_play("402r18", 1);
	} else if (useFlag && player_said("TOPIARY")) {
		digi_play("402r16", 1);
	} else if (useFlag && player_said("TOPIARY ")) {
		useTopiary();
	} else if (player_said("POMERANIAN MARKS", "WOLF") && inv_player_has("POMERANIAN MARKS")) {
		if (_G(flags)[V115] && !_G(flags)[V114]) {
			_G(flags)[V114] = 1;
			_G(flags)[V111]++;
			player_set_commands_allowed(false);
			_val5 = 210;
		} else {
			player_set_commands_allowed(false);
			_val5 = 200;
		}
	}
#define MONEY(ITEM) (player_said("WOLF") && player_said(ITEM) && inv_player_has(ITEM))
	else if (MONEY("US DOLLARS") || MONEY("CHINESE YUAN") ||
			MONEY("PERUVIAN INTI") || MONEY("SIKKIMESE RUPEE")) {
		if (!_G(flags)[V116]) {
			_G(flags)[V116] = 1;
			player_set_commands_allowed(false);
			_val5 = 221;
		}
	}
#undef MONEY
	else if (enterFlag && player_said("GATE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(517, 239, nullptr, 2, 11);
			break;
		case 2:
			disable_player_commands_and_fade_init(3);
			break;
		case 3:
			_G(flags)[V112] = 1;
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s22", 3);
			_G(game).setRoom(408);
			break;
		default:
			break;
		}
	} else if (enterFlag && player_said("DANZIG")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			_G(flags)[V112] = 1;
			_G(game).setRoom(401);
			break;
		default:
			break;
		}
	} else if (player_said("DANZIG")) {
		// No implementation
	} else if (enterFlag && player_said("CASTLE DOOR")) {
		enterCastle();
	} else if (player_said("journal") && !talkFlag && !lookFlag &&
			!inv_player_has(_G(player).noun)) {
		if (_G(flags)[kCastleCartoon])
			digi_play("com016", 1);
		else if (_G(kernel).trigger == -1)
			ws_walk(190, 333, nullptr, 8, 2);
		else {
			if (_G(kernel).trigger == 6) {
				_G(flags)[kCastleCartoon] = 1;
				_G(flags)[V089] = 1;
			}
			sketchInJournal("com015");
		}
	} else if (lookFlag && player_said(" ")) {
		digi_play("402r08", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room402::conv402a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	_currentNode = node;

	if (!sound) {
		conv_resume();
	} else if (who <= 0) {
		switch (node) {
		case 4:
		case 9:
			_sound2 = sound;
			_val12 = 2000;
			_val13 = 2107;
			break;

		case 5:
		case 10:
			_sound2 = sound;

			if (inv_player_has("POMERANIAN MARKS")) {
				conv_resume();
			} else {
				_val12 = 2002;
				_val13 = 2260;
			}
			break;

		case 7:
		case 25:
			switch (entry) {
			case 1:
				digi_play(sound, 1);
				_val12 = 2002;
				_val13 = 2160;
				break;

			default:
				_val13 = 2142;
				digi_play(sound, 1, 255, 777);
				break;
			}
			break;

		case 14:
			switch (entry) {
			case 0:
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2260;
				break;
			case 1:
				_sound2 = sound;

				if (inv_player_has("POMERANIAN MARKS")) {
					_val12 = 2002;
					_val13 = 2143;
					conv_resume();
				} else {
					_val12 = 2002;
					_val13 = 2170;
				}
				break;
			default:
				break;
			}
			break;

		case 16:
			switch (entry) {
			case 0:
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2260;
				break;
			case 1:
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2143;
				break;
			default:
				break;
			}
			break;

		case 17:
		case 18:
			_G(flags)[V132] = 1;
			_sound2 = sound;
			conv_resume();
			break;

		case 21:
			if (entry == 1) {
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2260;
			} else {
				_val13 = 201;
				digi_play(sound, 1, 255, 777);
			}
			break;

		case 19:
		case 22:
		case 23:
			_val10 = 1001;
			_sound2 = sound;
			conv_resume();
			break;

		case 26:
			switch (entry) {
			case 0:
				_val13 = 2101;
				digi_play(sound, 1, 255, 777);
				break;

			case 1:
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2260;
				break;

			case 2:
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2143;
				conv_resume();
				break;

			default:
				break;
			}
			break;

		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
			_val8 = 1;
			_val13 = 201;
			digi_play(sound, 1, 255, 777);
			break;

		default:
			_val13 = (_val12 == 2002) ? 2142 : 2101;
			digi_play(sound, 1, 255, 777);
			break;
		}
	} else if (who == 1) {
		switch (node) {
		case 1:
			if (entry == 3) {
				_sound1 = sound;
				_val10 = 1000;
				_val11 = 1120;
			} else if (_val10 == 1001) {
				_val11 = 1114;
				digi_play(sound, 1, 255, 777);
			} else {
				_val11 = 1102;
				conv_resume();
			}
			break;

		case 6:
		case 11:
			_val10 = 1000;
			if (entry == 1) {
				_sound1 = sound;
				conv_resume();
			} else {
				_val11 = 1102;
				digi_play(sound, 1, 255, 777);
			}
			break;

		case 14:
		case 16:
			if (entry == 1) {
				_val10 = 1001;
				_val11 = 1115;
				_sound2 = sound;
				_val12 = 2002;
				_val13 = 2180;

				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 110);
				_G(kernel).trigger_mode = KT_PARSE;
			} else {
				_val11 = (_val11 == 1001) ? 1114 : 2142;
				digi_play(sound, 1, 255, 777);
			}
			break;

		default:
			_val11 = (_val10 == 1001) ? 1114 : 2142;
			digi_play(sound, 1, 255, 777);
			break;
		}
	}
}

void Room402::conv402a777() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (who <= 0) {
		if (node == 26 && entry == 0) {
			_val10 = 1000;
			_val11 = 1123;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 102);
			_G(kernel).trigger_mode = KT_PARSE;
		} else {
			_val13 = (_val12 == 2002) ? 2143 : 2104;
			conv_resume();
		}
	} else if (who == 1) {
		_val11 = (_val10 == 1001) ? 1115 : 1103;
		conv_resume();
	}
}

void Room402::useTopiary() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripMessesBush = series_load("RIP MESSES UP THE BUSH");
		player_update_info();
		_safariShadow = series_place_sprite("SAFARI SHADOW 3", 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00);
		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x200, 0,
			triggerMachineByHashCallback, "rip entering castle");
		sendWSMessage_10000(1, _ripEnterLeave, _ripMessesBush, 1, 10, 1,
			_ripMessesBush, 10, 10, 0);
		break;

	case 1:
		sendWSMessage_10000(1, _ripEnterLeave, _ripMessesBush, 10, 34, 2,
			_ripMessesBush, 34, 34, 0);
		digi_play("402_s02", 1, 255, 2);
		break;

	case 2:
		if (inv_player_has("TURTLE") && !_G(flags)[V117]) {
			_G(flags)[V117] = 1;
			_G(flags)[V118]++;
		}

		_branch = series_place_sprite("sprite of the pulled out branch", 0, 0, -53, 100, 0x300);
		series_unload(_ripMessesBush);
		terminateMachineAndNull(_safariShadow);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}

}

void Room402::enterCastle() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripClimbKnock = series_load("RIP CLIMBS AND KNOCKS");
		_doorOpens = series_load("DOOR OPENS");
		ws_hide_walker();

		_ripEnterLeave = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x600, 0,
			triggerMachineByHashCallback, "rip entering castle");
		sendWSMessage_10000(1, _ripEnterLeave, _ripClimbKnock, 1,
			69, 1, _ripClimbKnock, 69, 69, 0);
		break;

	case 1:
		digi_play("402_s01", 1, 255, 2);
		sendWSMessage_10000(1, _ripEnterLeave, _ripClimbKnock, 69, 58, -1,
			_ripClimbKnock, 58, 58, 0);
		break;

	case 2:
		kernel_timing_trigger(15, 3);
		break;

	case 3:
		_castleDoor = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallback, "castle door");
		sendWSMessage_10000(1, _castleDoor, _doorOpens, 1, 2, -1,
			_doorOpens, 2, 2, 0);
		digi_play("402_S05", 1, 255, 4);
		break;

	case 4:
		if (player_been_here(404)) {
			kernel_timing_trigger(1, 5);
		} else {
			digi_play("402r28", 1, 255, 5);
		}
		break;

	case 5:
		disable_player_commands_and_fade_init(6);
		break;

	case 6:
		_G(flags)[V112] = 1;
		_G(game).setRoom(404);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
