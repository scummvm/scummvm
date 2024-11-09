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

#include "m4/riddle/rooms/section4/room403.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_vmng.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *const SAID[][2] = {
	{ "BROKEN HEADSTONE",    "403r11" },
	{ "POMMEE CROSS",        "403r12" },
	{  "GRAVE PLAQUE",       "403r13" },
	{ "GRAVE",               "403r18" },
	{ "1ST MARBLE MONUMENT", "403r15" },
	{ "2ND MARBLE MONUMENT", "403r26" },
	{ "MARBLE COLUMN",       "403r16" },
	{ "SMALL HEADSTONE",     "403r17" },
	{ "MARBLE HEADSTONE",    "403r27" },
	{ "TOMBSTONE",           "403r19" },
	{ "BURIAL TABLET",       "403r20" },
	{ "SMALL GRAVE MARKER",  "403r14" },
	{ "TALL HEADSTONE",      "403r22" },
	{ "GRANITE HEADSTONE",   "403r23" },
	{ "BURIAL PLAQUE",       "403r24" },
	{ "CELTIC CROSS",        "403r25" },
	{ "URN",                 "403r08" },
	{ nullptr, nullptr }
};

void Room403::init() {
	digi_preload("403_s02");
	digi_preload("403_s02a");
	digi_preload("403_s02b");
	digi_preload("950_s23");
	digi_preload("403_s01");

	player_set_commands_allowed(false);
	_bell = series_place_sprite("ONE FRAME BELL", 0, 0, 0, 100, 0xf00);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = 0;
		_val10 = 0;
		_sound1.clear();
		_val12 = 0;

		_G(flags)[V313] = player_been_here(403) && (
			(_G(flags)[V110] && inv_player_has("TURTLE")) ||
			inv_player_has("STEP LADDER") ||
			_G(flags)[GLB_TEMP_12] ||
			!inv_object_is_here("STEP LADDER")) ? 0 : 1;
		_plank = inv_object_in_scene("PLANK", 403) ? 2 : 0;

		_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
	}

	_safariShadow = series_load("SAFARI SHADOW 3");
	hotspot_set_active("WOLF", false);
	hotspot_set_active("STEP LADDER", false);
	hotspot_set_active("STEP LADDER ", false);
	hotspot_set_active("EDGER", false);
	hotspot_set_active("PLANK", false);
	hotspot_set_active("TURTLE TREAT", false);

	if (_G(flags)[V139] == 2) {
		_G(flags)[V139] = 0;

		if (_G(flags)[V133] && _G(flags)[V131] != 403) {
			_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("EDGER", true);
			inv_move_object("EDGER", 403);
		}

		MoveScreenDelta(-640, 0);
		ws_demand_location(1172, 322, 3);
		ws_walk(1172, 322, nullptr, 400, 1);

	} else if (_G(flags)[V139] == 4) {
		_G(flags)[V139] = 0;
		_ladder = series_place_sprite("LADDER LEANS AGAINST WALL", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("STEP LADDER ", true);
		MoveScreenDelta(-640, 0);
		ws_demand_location(1083, 322, 3);
		ws_walk(1201, 321, nullptr, 420, 2);

	} else {
		if (inv_player_has("TURTLE"))
			_G(flags)[V313] = 0;

		switch (_G(flags)[V313]) {
		case 1:
			_ladder = series_place_sprite("LADDER LEANS AGAINST WALL", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("STEP LADDER ", true);
			break;
		case 2:
			_ladder = series_place_sprite("1 sprite of ladder", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("STEP LADDER", true);
			break;
		default:
			break;
		}

		if (_G(flags)[V133] && !_G(flags)[GLB_TEMP_12] && _G(flags)[V131] != 403 &&
				!inv_player_has("TURTLE") && !inv_player_has("EDGER")) {
			_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 0x100, 0xf00);
			hotspot_set_active("EDGER", true);
		}

		if (inv_object_is_here("PLANK") || _plank == 2) {
			_board = series_place_sprite("1 SPRITE OF BOARD", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("PLANK", true);
			_plank = true;
		}

		if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
			if (_G(flags)[V131] == 403) {
				hotspot_set_active("WOLF", true);
				_wolfTurnTalk = series_load("WOLF TURN AND TALK");
				_wolfTurnHand = series_load("WOLF TURN WITH HAND OUT");
				_wolfTalkLeave = series_load("WOLF TALKS AND LEAVES");
				_ripTalkPay = series_load("RIP TALK PAY LOOP");
				_wolfEdger = series_load("WOLF EDGER LOOP");

				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
					triggerMachineByHashCallback, "WOLFIE");

				if (_val12) {
					sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 45, -1,
						_wolfTurnHand, 45, 45, 0);
				} else {
					_val8 = 2001;
					_val9 = 2300;
					sendWSMessage_10000(1, _wolfie, _wolfEdger, 1, 6, 110,
						_wolfEdger, 6, 6, 0);
				}
			}

			if (_val10) {
				ws_demand_facing(11);
				ws_hide_walker();
				_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallback, "RIP on ladder/plank");

				switch (_val10) {
				case 1:
					_ripClimbsLadder = series_load("RIPLEY CLIMBS LADDER");
					sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 52, 52, -1,
						_ripClimbsLadder, 52, 52, 0);
					break;
				case 2:
					_ripClimbsLadder = series_load("RIPLEY CLIMBS LADDER");
					sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 74, 74, -1,
						_ripClimbsLadder, 74, 74, 0);
					break;
				case 3:
					_ripLegUp = series_load("RIP GETS A LEG UP");
					_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
					sendWSMessage_10000(1, _ripOnLadder, _ripLegUp, 44, 44, -1,
						_ripLegUp, 44, 44, 0);
					break;
				case 4:
					_ripLegUp = series_load("RIP GETS A LEG UP");
					_ripTurtle = series_load("RIP TURTLE SERIES");
					_noTreat = series_load("403RP06 NO TREAT");
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 28, 28, -1,
						_noTreat, 28, 28, 0);
					break;
				case 5:
					_ripLegUp = series_load("RIP GETS A LEG UP");
					_ripTurtle = series_load("RIP TURTLE SERIES");
					_noTreat = series_load("403RP06 NO TREAT");
					hotspot_set_active("GRATE", false);
					hotspot_set_active("TURTLE TREAT", true);

					if (_G(flags)[V125] == 2)
						sendWSMessage_10000(1, _ripOnLadder, _noTreat, 52, 52, -1,
						_noTreat, 52, 52, 0);
					else
						sendWSMessage_10000(1, _ripOnLadder, _noTreat, 28, 28, -1,
							_noTreat, 28, 28, 0);
					break;
				default:
					break;
				}
			} else {
				_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
			}
		} else if (_G(flags)[V132]) {
			_G(flags)[V132] = 0;
			_G(camera_reacts_to_player) = false;
			MoveScreenDelta(-640, 0);
			ws_demand_location(620, 326, 3);
			ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);
			ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
			kernel_timing_trigger(1, 310);
		} else {
			if (_G(flags)[V131] == 403) {
				hotspot_set_active("WOLF", true);
				_wolfTurnTalk = series_load("WOLF TURN AND TALK");
				_wolfTurnHand = series_load("WOLF TURN WITH HAND OUT");
				_wolfTalkLeave = series_load("WOLF TALKS AND LEAVES");
				_ripTalkPay = series_load("RIP TALK PAY LOOP");
				_wolfEdger = series_load("WOLF EDGER LOOP");

				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
					triggerMachineByHashCallback, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolfEdger, 1, 6, 110,
					_wolfEdger, 6, 6, 0);
				_val8 = 2001;
				_val9 = 2300;
			}

			if (0) {
				ws_demand_location(4, 296);
				ws_walk(80, 300, nullptr, 300, 3);
			} else {
				MoveScreenDelta(-640, 0);
				ws_demand_location(1110, 322);
			}
		}
	}

	digi_play_loop("403_s01", 3, 180);
}

void Room403::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 10:
		digi_play("graveyard_amb_2", 2, 50);
		kernel_timing_trigger(1050, 10);
		break;

	case 100:
		kernel_timing_trigger(1, 102);
		break;

	case 101:
		_val6 = 1000;
		_val7 = 1105;
		break;

	case 102:
		if (_val2 != -1) {
			kernel_timing_trigger(1, _val2);
			_val2 = -1;
		} else {
			kernel_timing_trigger(1, 103);
		}
		break;

	case 103:
		switch (_val6) {
		case 1000:
			switch (_val7) {
			case 1100:
				_val8 = 2000;
				_val9 = 2100;
				kernel_timing_trigger(1, 110);
				player_update_info();
				ws_hide_walker();
				player_set_commands_allowed(false);

				_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallback, "rip takes wolf");
				_ripTalksWolf = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00, 0,
					triggerMachineByHashCallback, "rip talks wolf SHADOW");

				sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 1, 10, 102,
					_ripTalkPay, 10, 10, 0);
				sendWSMessage_10000(1, _ripTalksWolf, _safariShadow, 1, 1, -1,
					_safariShadow, 1, 1, 0);
				_val7 = 1101;
				break;

			case 1101:
				_val7 = 1003;
				kernel_timing_trigger(1, 102);

				conv_load("conv403a", 10, 10, 101);
				conv_export_value_curr(_G(flags)[V111], 0);
				conv_export_value_curr(_G(flags)[V121], 1);
				conv_export_value_curr(_G(flags)[V122], 2);
				conv_export_value_curr(_G(flags)[V120], 3);

				conv_export_value_curr(inv_player_has("POMERANIAN MARKS") ? 1 : 0, 5);
				conv_export_pointer_curr(&_G(flags)[V114], 5);
				conv_export_pointer_curr(&_G(flags)[V115], 6);
				conv_export_value_curr(_G(flags)[V337], 10);
				conv_play();
				break;

			case 1102:
				frame = imath_ranged_rand(11, 13);
				sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, frame, frame, 102,
					_ripTalkPay, frame, frame, 0);
				sendWSMessage_190000(_ripOnLadder, 11);
				sendWSMessage_1a0000(_ripOnLadder, 11);
				break;

			case 1103:
				sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 10, 102,
					_ripTalkPay, 10, 10, 0);
				break;

			case 1104:
				sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 10, -1,
					_ripTalkPay, 10, 10, 0);
				break;

			case 1105:
				player_set_commands_allowed(false);

				if (_val5) {
					_val5 = 0;
					sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 1, 305,
						_ripTalkPay, 1, 1, 0);
					_val9 = 2251;
					_G(flags)[V124] = 1;
				} else {
					sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 1, 103,
						_ripTalkPay, 1, 1, 0);
					_val7 = 1106;

					if (!_G(flags)[V115])
						_val9 = 2103;
				}
				break;

			case 1106:
				terminateMachineAndNull(_ripOnLadder);
				terminateMachineAndNull(_ripTalksWolf);
				ws_unhide_walker();
				_G(flags)[V114] = 0;

				if (!_G(flags)[V115] || _flag2 ||
						!inv_player_has("POMERANIAN MARKS")) {
					_flag2 = false;
					_val8 = 2001;
					_val9 = 2300;
					kernel_timing_trigger(1, 110);
					player_set_commands_allowed(true);
				} else {
					player_set_commands_allowed(false);
					_val12 = 1;
					_val8 = 2000;
					_val9 = 2230;
					kernel_timing_trigger(1, 110);
				}
				break;

			default:
				break;
			}
			break;

		case 1010:
			switch (_val7) {
			case 1200:
				player_set_commands_allowed(false);
				terminateMachineAndNull(_ventClosed);
				_ripClimbsLadder = series_load("RIPLEY CLIMBS LADDER");
				ws_hide_walker();

				_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallback, "RIP climbs ladder");
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 1, 12, 103,
					_ripClimbsLadder, 12, 12, 0);
				_val7 = 1202;
				break;

			case 1202:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 13, 23, 103,
					_ripClimbsLadder, 23, 23, 0);
				_val7 = 1203;
				break;

			case 1203:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 24, 33, 103,
					_ripClimbsLadder, 33, 33, 0);
				_val7 = 1204;
				break;

			case 1204:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 34, 40, 103,
					_ripClimbsLadder, 40, 40, 0);
				_val7 = 1205;
				break;

			case 1205:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 41, 52, 103,
					_ripClimbsLadder, 52, 52, 0);
				_val7 = 1209;
				break;

			case 1209:
				_val10 = 1;
				player_set_commands_allowed(true);
				break;

			case 1210:
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 52, 36, 103,
					_ripClimbsLadder, 36, 36, 0);
				_val7 = 1212;
				break;

			case 1212:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 35, 26, 103,
					_ripClimbsLadder, 26, 26, 0);
				_val7 = 1213;
				break;

			case 1213:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 25, 14, 103,
					_ripClimbsLadder, 14, 14, 0);
				_val7 = 1214;
				break;

			case 1214:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 13, 4, 103,
					_ripClimbsLadder, 4, 4, 0);
				_val7 = 1215;
				break;

			case 1215:
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 3, 1, 103,
					_ripClimbsLadder, 1, 1, 0);
				_val7 = 1219;
				break;

			case 1219:
				terminateMachineAndNull(_ripOnLadder);
				ws_unhide_walker();
				_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);
				series_unload(_ripClimbsLadder);
				_val10 = 0;
				player_set_commands_allowed(true);
				break;

			case 1220:
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 53, 59, 103,
					_ripClimbsLadder, 59, 59, 0);
				break;

			case 1222:
				digi_play("403_s06", 2);
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 60, 74, 103,
					_ripClimbsLadder, 74, 74, 0);
				_val7 = 1223;
				break;

			case 1230:
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 74, 69, 103,
					_ripClimbsLadder, 69, 69, 0);
				_val7 = 1232;
				break;

			case 1232:
				digi_play("403_s06", 2);
				sendWSMessage_10000(1, _ripOnLadder, _ripClimbsLadder, 68, 53, 103,
					_ripClimbsLadder, 53, 53, 0);
				_val7 = 1210;
				break;

			case 1240:
				player_set_commands_allowed(false);
				digi_preload("403R29");
				series_stream("VENT POPUP LADDER", 10, 0, 103);
				kernel_timing_trigger(390, 350);
				_val7 = 1230;
				break;

			default:
				break;
			}
			break;

		case 1020:
			switch (_val7) {
			case 1300:
				player_set_commands_allowed(false);
				_ripLegUp = series_load("RIP GETS A LEG UP");
				ws_hide_walker();

				_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallback, "RIP climbs plank");
				sendWSMessage_10000(1, _ripOnLadder, _ripLegUp, 1, 10, 103,
					_ripLegUp, 10, 10, 0);
				_val7 = 1302;
				break;

			case 1302:
				digi_play("403_s08", 2);
				sendWSMessage_10000(1, _ripOnLadder, _ripLegUp, 11, 44, 103,
					_ripLegUp, 44, 44, 0);
				_val7 = 1303;
				break;

			case 1303:
				_val10 = 3;
				player_set_commands_allowed(true);
				break;

			case 1310:
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _ripOnLadder, _ripLegUp, 44, 1, 103,
					_ripLegUp, 1, 1, 0);
				digi_play("403_s08", 2);
				_val7 = 1312;
				break;

			case 1312:
				terminateMachineAndNull(_ripOnLadder);
				ws_unhide_walker();
				series_unload(_ripLegUp);
				_val10 = 0;

				if (_G(flags)[V125] == 3)
					kernel_timing_trigger(1, 442);
				break;

			case 1320:
				player_set_commands_allowed(false);
				_ripTurtle = series_load("RIP TURTLE SERIES");
				_noTreat = series_load("403RP06 NO TREAT");
				terminateMachineAndNull(_ventClosed);

				if (_G(flags)[V125] == 1)
					sendWSMessage_10000(1, _ripOnLadder, _ripTurtle, 1, 12, 103,
						_ripTurtle, 12, 12, 0);
				else
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 1, 12, 103,
						_noTreat, 12, 12, 0);

				_val7 = 1322;
				break;

			case 1322:
				digi_play("403_s06", 2);

				if (_G(flags)[V125] == 1)
					sendWSMessage_10000(1, _ripOnLadder, _ripTurtle, 13, 28, 103,
						_ripTurtle, 28, 28, 0);
				else
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 13, 28, 103,
						_noTreat, 28, 28, 0);

				_val7 = 1323;
				break;

			case 1323:
				if (_G(flags)[V125]) {
					hotspot_set_active("GRATE", false);
					hotspot_set_active("TURTLE TREAT", true);
					_val10 = 5;
				} else {
					_val10 = 4;
				}

				player_set_commands_allowed(true);
				break;

			case 1330:
				player_set_commands_allowed(false);

				if (_G(flags)[V125] == 1)
					sendWSMessage_10000(1, _ripOnLadder, _ripTurtle, 28, 21, 103,
						_ripTurtle, 21, 21, 0);
				else
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 28, 21, 103,
						_noTreat, 21, 21, 0);

				_val7 = 1332;
				break;

			case 1332:
				digi_play("403_s10", 2);

				if (_G(flags)[V125] == 1)
					sendWSMessage_10000(1, _ripOnLadder, _ripTurtle, 20, 1, 103,
						_ripTurtle, 44, 44, 0);
				else
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 20, 1, 103,
						_noTreat, 44, 44, 0);

				_val7 = 1333;
				break;

			case 1333:
				series_unload(_noTreat);
				series_unload(_ripTurtle);
				_ventClosed = series_show("SPRITE OF VENT CLOSED", 0x600, 16);

				hotspot_set_active("GRATE", 1);
				hotspot_set_active("TURTLE TREAT", false);
				_val7 = 1310;
				kernel_timing_trigger(1, 103);
				break;

			case 1340:
				player_set_commands_allowed(false);
				digi_preload("403R29");
				series_stream("VENT POPUP PLANK", 10, 0, 103);
				kernel_timing_trigger(390, 350);
				_val7 = 1330;
				break;

			case 1400:
				player_set_commands_allowed(false);

				if (_G(flags)[V125]) {
					digi_play("403r49", 1, 255, 103);
				} else {
					sendWSMessage_10000(1, _ripOnLadder, _ripTurtle, 29, 52, 103,
						_ripTurtle, 52, 52, 0);
					_G(flags)[V125] = 1;
					_val10 = 5;

					hotspot_set_active("GRATE", false);
					hotspot_set_active("TURTLE TREATS", true);
					_val7 = 1402;
				}
				break;

			case 1402:
				player_set_commands_allowed(true);
				break;

			case 1410:
				player_set_commands_allowed(false);

				if (_G(flags)[V125] == 1)
					sendWSMessage_10000(1, _ripOnLadder, _ripTurtle, 53, 81, 103,
						_ripTurtle, 81, 81, 0);
				else
					sendWSMessage_10000(1, _ripOnLadder, _noTreat, 53, 81, 103,
						_noTreat, 81, 81, 0);

				_val7 = 1412;
				break;

			case 1412:
				if (_val10 != 5) {
					digi_play("403r50", 1, 255, 103);
				} else if (_G(flags)[V125] == 1) {
					digi_play("403r51", 1, 255, 103);
					_G(flags)[V125] = 0;
				} else {
					kernel_timing_trigger(1, 440);
				}

				_val7 = 1413;
				break;

			case 1413:
				sendWSMessage_10000(1, _ripOnLadder, _noTreat, 81, 53, 103,
					_noTreat, 28, 28, 0);
				_val7 = 1414;
				break;

			case 1414:
				inv_give_to_player("TURTLE");
				_val10 = 4;
				hotspot_set_active("GRATE", true);
				hotspot_set_active("TURTLE TREAT", false);
				player_set_commands_allowed(true);
				break;

			case 1500:
				player_set_commands_allowed(false);
				_ripPlankEdger = series_load("RIPLEY ON PLANK USES EDGER");
				sendWSMessage_10000(1, _ripOnLadder, _ripPlankEdger, 1, 20, 103,
					_ripPlankEdger, 20, 20, 0);
				_val7 = 1502;
				break;

			case 1502:
				digi_play("403_s10", 2);
				sendWSMessage_10000(1, _ripOnLadder, _ripPlankEdger, 20, 63, 103,
					_noTreat, 52, 52, 0);
				_val7 = 1503;
				break;

			case 1504:
				_G(flags)[V125] = 2;
				series_unload(_ripPlankEdger);
				player_set_commands_allowed(true);
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
		switch (_val8) {
		case 2000:
		case 2001:
			if (_val4 != -1) {
				kernel_timing_trigger(1, _val4);
				_val4 = -1;
			} else {
				kernel_timing_trigger(1, 111);
			}
			break;

		default:
			break;
		}
		break;

	case 111:
		switch (_val8) {
		case 2000:
			switch (_val9) {
			case 2100:
				sendWSMessage_10000(1, _wolfie, _wolfTurnTalk, 1, 7, 110,
					_wolfTurnTalk, 7, 7, 0);
				_val9 = 2102;
				break;

			case 2101:
				frame = imath_ranged_rand(8, 13);
				sendWSMessage_10000(1, _wolfie, _wolfTurnTalk, frame, frame, 110,
					_wolfTurnTalk, frame, frame, 0);
				sendWSMessage_190000(_wolfie, 11);
				sendWSMessage_1a0000(_wolfie, 11);
				break;

			case 2102:
				sendWSMessage_10000(1, _wolfie, _wolfTurnTalk, 7, 7, 110,
					_wolfTurnTalk, 7, 7, 0);
				break;

			case 2103:
				sendWSMessage_10000(1, _wolfie, _wolfTurnTalk, 6, 6, -1,
					_wolfEdger, 6, 6, 0);
				break;

			case 2104:
				sendWSMessage_10000(1, _wolfie, _wolfEdger, 1, 6, 110,
					_wolfEdger, 6, 6, 0);
				break;

			case 2105:
				sendWSMessage_10000(1, _wolfie, _wolfTurnTalk, 7, 1, 110,
					_wolfEdger, 1, 1, 0);
				_val8 = 2001;
				_val9 = 2300;
				break;

			case 2230:
				terminateMachineAndNull(_wolfie);
				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
					triggerMachineByHashCallback, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 11, 45, 111,
					_wolfTurnHand, 45, 45, 0);
				_val9 = 2231;

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1);
					_sound1.clear();
				}
				break;

			case 2231:
				player_set_commands_allowed(true);
				break;

			case 2232:
				_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");
				player_update_info();

				_ripTalksWolf = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00, 0,
					triggerMachineByHashCallback, "rip talks wolf SHADOW");
				sendWSMessage_10000(1, _ripTalksWolf, _safariShadow, 1, 1, -1,
					_safariShadow, 1, 1, 0);

				ws_hide_walker();
				sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 1, 10, 111,
					_ripTalkPay, 10, 10, 0);
				_val9 = 2233;
				break;

			case 2233:
				sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 40, 56, 213,
					_ripTalkPay, 56, 56, 0);
				_val13 = _G(flags)[V111];
				break;

			case 2234:
				player_set_commands_allowed(false);
				sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 11, 111,
					_wolfTurnHand, 11, 11, 0);
				_val9 = 2235;
				break;

			case 2235:
				_val8 = 2001;
				_val9 = 2300;
				kernel_timing_trigger(1, 110);

				if (!_G(flags)[V052])
					player_set_commands_allowed(true);
				break;

			case 2250:
				frame = imath_ranged_rand(19, 24);
				sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, frame, frame, 110,
					_wolfTalkLeave, frame, frame, 0);
				sendWSMessage_190000(_wolfie, 11);
				sendWSMessage_1a0000(_wolfie, 11);
				break;

			case 2251:
				sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, 19, 19, -1,
					_wolfTalkLeave, 19, 19, 0);
				break;

			case 2252:
				sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, 19, 19, 110,
					_wolfTalkLeave, 19, 19, 0);
				break;

			case 2253:
				sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 15, 34, 111,
					_wolfTurnHand, 34, 34, 0);
				_val9 = 2254;
				break;

			case 2254:
				sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, 40, 60, 111,
					_wolfTalkLeave, 60, 60, 0);
				_val9 = 2255;

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1);
					_sound1.clear();
				}
				break;

			case 2255:
				sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, 60, 40, 111,
					_wolfTalkLeave, 40, 40, 0);
				_val9 = 2256;
				break;

			case 2256:
				sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 34, 15, 111,
					_wolfTurnHand, 15, 15, 0);
				_val9 = 2257;
				break;

			case 2257:
				_val9 = 2102;
				kernel_timing_trigger(1, 110);
				conv_resume();
				break;

			case 2258:
				sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, 1, 18, 111,
					_wolfTalkLeave, 18, 18, 0);
				_val9 = 2250;
				break;

			case 2260:
				sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 11, 45, -1,
					_wolfTurnHand, 45, 45, 0);

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1, 255, 111);
					_val9 = 2262;
					_sound1.clear();
				}
				break;

			case 2262:
				conv_resume();
				break;

			case 2270:
				sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 11, 111,
					_wolfTurnHand, 11, 11, 0);

				if (!_sound1.empty()) {
					digi_play(_sound1.c_str(), 1);
					_sound1.clear();
				}
				break;

			case 2272:
				_val8 = 2001;
				_val9 = 2300;
				kernel_timing_trigger(1, 110);
				break;

			default:
				break;
			}
			break;

		case 2001:
			if (_val9 == 2300) {
				if (imath_ranged_rand(1, 3) == 1) {
					frame = imath_ranged_rand(1, 6);
					sendWSMessage_10000(1, _wolfie, _wolfEdger,
						imath_ranged_rand(1, 6), imath_ranged_rand(1, 6), 100,
						_wolfEdger, frame, frame, 0);
					sendWSMessage_190000(_wolfie, 8);
					sendWSMessage_1a0000(_wolfie, 6);

					switch (imath_ranged_rand(1, 6)) {
					case 1:
						digi_play("403_s02", 2);
						break;
					case 2:
						digi_play("403_s02a", 2);
						break;
					case 3:
						digi_play("403_s02b", 2);
						break;
					default:
						break;
					}
				} else {
					kernel_timing_trigger(15, 110);
				}
			}
			break;

		default:
			break;
		}
		break;

	case 200:
		player_set_commands_allowed(false);
		_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");

		player_update_info();
		_ripTalksWolf = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");
		sendWSMessage_10000(1, _ripTalksWolf, _safariShadow, 1, 1, -1,
			_safariShadow, 1, 1, 0);

		ws_hide_walker();
		sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 11, 45, 202,
			_wolfTurnHand, 45, 45, 0);
		break;

	case 202:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 40, 63, 203,
			_ripTalkPay, 63, 63, 0);
		break;

	case 203:
		digi_play("950_s23", 2, 255, -1, 950);
		kernel_timing_trigger(30, 205);
		digi_play("402w07", 1);
		break;

	case 205:
		sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 1, 209,
			_wolfTurnHand, 1, 1, 0);
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 63, 40, 206,
			_ripTalkPay, 40, 40, 0);
		break;

	case 206:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 1, 207,
			_ripTalkPay, 1, 1, 0);
		break;

	case 207:
	case 226:
	case 236:
		terminateMachineAndNull(_ripOnLadder);
		terminateMachineAndNull(_ripTalksWolf);
		ws_unhide_walker();
		break;

	case 209:
	case 227:
		kernel_timing_trigger(1, 110);
		player_set_commands_allowed(true);
		break;

	case 210:
		player_set_commands_allowed(false);
		_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");

		player_update_info();
		_ripTalksWolf = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _ripTalksWolf, _safariShadow, 1, 1, -1,
			_safariShadow, 1, 1, 0);
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 1, 10, 211,
			_ripTalkPay, 10, 110, 0);
		sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 11, 45, 213,
			_wolfTurnHand, 45, 45, 0);
		_val13 = _G(flags)[V111];
		break;

	case 211:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 40, 56, -1,
			_ripTalkPay, 56, 56, 0);
		break;

	case 213:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 57, 63, 214,
			_ripTalkPay, 63, 63, 0);
		break;

	case 214:
		digi_play("950_s23", 2, 255, -1, 950);
		kernel_timing_trigger(30, 215);
		break;

	case 215:
		if (_val13 > 1) {
			--_val13;
			sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 63, 57, 213,
				_ripTalkPay, 57, 57, 0);
		} else {
			sendWSMessage_10000(1, _wolfie, _wolfTalkLeave, 1, 18, 217,
				_wolfTalkLeave, 18, 18, 0);
			sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 56, 40, 216,
				_ripTalkPay, 40, 40, 0);
			_flag3 = false;
		}
		break;

	case 216:
	case 217:
		if (!_flag3) {
			_flag3 = true;
		} else {
			kernel_timing_trigger(1, 218);
		}
		break;

	case 218:
		_val4 = -1;
		_val8 = 2000;
		_val9 = 2252;
		kernel_timing_trigger(1, 110);
		_val6 = 1000;
		_val7 = 1103;
		kernel_timing_trigger(1, 102);

		conv_load("conv403a", 0, 10, 101);
		conv_export_value_curr(_G(flags)[V111], 0);
		conv_export_value_curr(_G(flags)[V121], 1);
		conv_export_value_curr(_G(flags)[V122], 2);
		conv_export_value_curr(_G(flags)[V120], 3);

		conv_export_value_curr(inv_player_has("POMERANIAN MARKS") ? 1 : 0, 5);
		conv_export_pointer_curr(&_G(flags)[V114], 5);
		conv_export_pointer_curr(&_G(flags)[V115], 6);
		conv_export_value_curr(_G(flags)[V337], 10);
		conv_play();
		break;

	case 220:
		player_set_commands_allowed(false);
		_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");

		player_update_info();
		_ripTalksWolf = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _ripTalksWolf, _safariShadow, 1, 1, -1,
			_safariShadow, 1, 1, 0);
		ws_hide_walker();
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 1, 10, -1,
			_ripTalkPay, 10, 10, 0);
		sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 11, 45, 222,
			_wolfTurnHand, 45, 45, 0);
		break;

	case 222:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 40, 63, 223,
			_ripTalkPay, 63, 63, 0);
		break;

	case 223:
		sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 11, 227,
			_wolfTurnHand, 11, 11, 0);
		digi_play("402w08", 1);
		kernel_timing_trigger(60, 224);
		break;

	case 224:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 63, 40, 225,
			_ripTalkPay, 40, 40, 0);
		break;

	case 225:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 1, 226,
			_ripTalkPay, 1, 1, 0);
		break;

	case 230:
		player_set_commands_allowed(false);
		_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "rip talks wolf");

		player_update_info();
		_ripTalksWolf = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0xf00, 0,
			triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _ripTalksWolf, _safariShadow, 1, 1, -1,
			_safariShadow, 1, 1, 0);
		ws_hide_walker();
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 1, 10, 232,
			_ripTalkPay, 10, 10, 0);
		break;

	case 232:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 40, 63, 233,
			_ripTalkPay, 63, 63, 0);
		break;

	case 233:
		sendWSMessage_10000(1, _wolfie, _wolfTurnHand, 45, 11, 237,
			_wolfTurnHand, 11, 11, 0);
		digi_play("402w08", 1);
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 63, 40, 235,
			_ripTalkPay, 40, 40, 0);
		break;

	case 235:
		sendWSMessage_10000(1, _ripOnLadder, _ripTalkPay, 10, 1, 236,
			_ripTalkPay, 1, 1, 0);
		break;

	case 237:
		_val8 = 2001;
		_val9 = 2300;
		kernel_timing_trigger(1, 110);
		player_set_commands_allowed(true);
		break;

	case 300:
		player_set_commands_allowed(true);
		break;

	case 305:
		_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("WOLF", false);
		hotspot_set_active("EDGER", true);

		_G(flags)[V133] = 1;
		_G(flags)[V131] = 402;
		terminateMachineAndNull(_ripOnLadder);
		terminateMachineAndNull(_ripTalksWolf);
		ws_unhide_walker();
		terminateMachineAndNull(_wolfie);

		series_unload(_wolfEdger);
		series_unload(_wolfTurnTalk);
		series_unload(_wolfTurnHand);
		series_unload(_wolfTalkLeave);
		series_unload(_ripTalkPay);

		series_stream("403W08", 6, 0, 307);
		break;

	case 307:
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 308);
		break;

	case 308:
		midi_stop();
		player_set_commands_allowed(true);
		break;

	case 310:
		_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS,
			620, 313, 3, triggerMachineByHashCallback, "wolf_walker");
		DisposePath(_wolfWalker->walkPath);
		_wolfWalker->walkPath = CreateCustomPath(1067, 313, -1);
		ws_custom_walk(_wolfWalker, 3, 312);
		kernel_timing_trigger(480, 311);
		break;

	case 311:
		ws_walk(1120, 328, nullptr, 314, 1);
		break;

	case 312:
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 313);
		break;

	case 313:
	case 325:
		midi_stop();
		break;

	case 314:
		_ripHeadTurn = series_load("RIP TREK HEAD TURN POS1");
		setGlobals1(_ripHeadTurn, 1, 3, 3, 3, 0, 3, 1, 1, 1);
		sendWSMessage_110000(315);
		break;

	case 315:
		sendWSMessage_60000(_wolfWalker);
		_wolfIndicatesTomb = series_load("WOLF INDICATES TOMB");
		_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x900, 0,
			triggerMachineByHashCallback, "WOLFIE");
		sendWSMessage_10000(1, _wolfie, _wolfIndicatesTomb, 1, 93, -1,
			_wolfIndicatesTomb, 93, 93, 0);
		digi_play("403w01", 1, 255, 316);
		break;

	case 316:
		sendWSMessage_120000(-1);
		digi_play("403r01", 1, 255, 317);
		break;

	case 317:
		sendWSMessage_10000(1, _wolfie, _wolfIndicatesTomb, 94, 96, -1,
			_wolfIndicatesTomb, 94, 96, 1);
		digi_play("403w02", 1, 255, 318);
		break;

	case 318:
		digi_play("403r02", 1, 255, 319);
		break;

	case 319:
		sendWSMessage_10000(1, _wolfie, _wolfIndicatesTomb, 94, 96, -1,
			_wolfIndicatesTomb, 94, 96, 1);
		digi_play("403w03", 1, 255, 320);
		break;

	case 320:
		sendWSMessage_10000(1, _wolfie, _wolfIndicatesTomb, 93, 93, -1,
			_wolfIndicatesTomb, 93, 93, 1);
		digi_play("403r03", 1, 255, 321);
		break;

	case 321:
		digi_preload("403w04");
		_wolfJustSo = series_stream("403 WOLF JUST SO", 5, 0, 322);
		series_stream_break_on_frame(_wolfJustSo, 8, 323);
		break;

	case 322:
		midi_play("DANZIG1", 255, 1, -1, 949);
		sendWSMessage_120000(-1);
		terminateMachineAndNull(_wolfie);

		_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS,
			1067, 313, 3, triggerMachineByHashCallback3000, "wolf_walker");
		sendWSMessage_10000(_wolfWalker, 620, 313, 3, 324, 0);
		break;

	case 323:
		digi_play("403w04", 1);
		break;

	case 324:
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 325);
		sendWSMessage_60000(_wolfWalker);
		sendWSMessage_150000(-1);
		_G(camera_reacts_to_player) = 1;
		player_set_commands_allowed(true);
		break;

	case 350:
		digi_play("403r29", 1, 255, 2400);
		break;

	case 400:
		_series1 = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_series1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(402);
		break;

	case 402:
		_ladder = series_place_sprite("LADDER LEANS AGAINST WALL", 0, 0, 0, 100, 0xf00);
		inv_move_object("STEP LADDER", 403);
		hotspot_set_active("STEP LADDER ", true);
		sendWSMessage_120000(403);
		break;

	case 403:
		sendWSMessage_150000(404);
		break;

	case 404:
		series_unload(_series1);
		player_set_commands_allowed(true);
		break;

	case 420:
		_ripMedReach = series_load("RIP MED REACH 1HAND POS2");
		setGlobals1(_ripMedReach, 1, 17, 17, 17, 0, 17, 1, 1, 1);
		sendWSMessage_110000(422);
		break;

	case 422:
		_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("EDGER", true);
		inv_move_object("EDGER", 403);
		sendWSMessage_120000(423);
		break;

	case 423:
		sendWSMessage_150000(424);
		break;

	case 424:
		series_unload(_ripMedReach);
		player_set_commands_allowed(true);
		break;

	case 440:
		player_set_commands_allowed(false);
		_G(flags)[V125] = 3;
		_turtlePopup = series_load("403 turtle popup");
		_wolfJustSo = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "Turtle POPUP");
		sendWSMessage_10000(1, _wolfJustSo, _turtlePopup, 1, 46, -1,
			_turtlePopup, 46, 46, 0);
		midi_play("turtle", 255, 1, -1, 949);
		_val6 = 1020;
		_val7 = 1330;
		kernel_timing_trigger(1, 102);
		break;

	case 442:
		_ripPutBoard = series_load("RIPLEY PUTS BOARD ON POTS");
		terminateMachineAndNull(_board);
		ws_hide_walker();

		_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
			triggerMachineByHashCallback, "RIP plants plank");
		sendWSMessage_10000(1, _ripOnLadder, _ripPutBoard, 57, 1, 443,
			_ripPutBoard, 1, 1, 0);
		break;

	case 443:
		hotspot_set_active("PLANK", false);
		inv_give_to_player("PLANK");
		terminateMachineAndNull(_ripOnLadder);
		ws_unhide_walker();
		series_unload(_ripPutBoard);

		ws_walk(620, 326, nullptr, 1);
		kernel_timing_trigger(60, 444);
		break;

	case 444:
		disable_player_commands_and_fade_init(445);
		break;

	case 445:
		digi_stop(3);
		_G(game).setRoom(408);
		break;

	case 2400:
		digi_stop(1);
		digi_unload("403R29");
		break;

	default:
		break;
	}
}

#define TRIGGER _G(kernel).trigger_mode = KT_DAEMON; \
	kernel_timing_trigger(1, 102); \
	_G(kernel).trigger_mode = KT_PARSE
#define MONEY(ITEM) (player_said(ITEM, "WOLF") && inv_player_has(ITEM))

void Room403::pre_parser() {
	bool talkFlag = player_said_any("talk", "talk to");
	bool lookFlag = player_said_any("look", "look at");
	bool enterFlag = player_said("enter");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (_val10 == 5) {
		intr_cancel_sentence();
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if (player_said("TURTLE", "TURTLE TREAT")) {
			_val6 = 1020;
			_val7 = 1410;
			inv_move_object("TURTLE", 403);
		} else if (player_said("TURTLE TREATS", "TURTLE TREAT")) {
			_val6 = 1020;
			_val7 = 1400;
		} else if (player_said("EDGER", "TURTLE TREAT")) {
			if (_G(flags)[V125] != 2) {
				_val6 = 1020;
				_val7 = 1500;
			}
		} else {
			_val6 = 1020;
			_val7 = 1330;
		}

		TRIGGER;
		return;
	}

	if (_val10 == 4) {
		intr_cancel_sentence();
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if (lookFlag && player_said("GRATE")) {
			_val6 = 1020;
			_val7 = 1340;
		} else if (player_said("TURTLE TREATS", "GRATE")) {
			_val6 = 1020;
			_val7 = 1400;
		} else if (player_said("TURTLE", "GRATE")) {
			_val6 = 1020;
			_val7 = 1410;
			inv_move_object("TURTLE", 403);
		} else {
			_val6 = 1020;
			_val7 = 1330;
		}

		TRIGGER;
		return;
	}

	if (_val10 == 3) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if (lookFlag && player_said("grate")) {
			return;
		}

		intr_cancel_sentence();

		if (useFlag && player_said("GRATE")) {
			_val6 = 1020;
			_val7 = 1320;
		} else {
			_val6 = 1020;
			_val7 = 1310;
		}

		TRIGGER;
		return;
	}

	if (_val10 == 2) {
		intr_cancel_sentence();
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if (lookFlag && player_said("GRATE")) {
			_val6 = 1010;
			_val7 = 1240;
		} else {
			_val6 = 1010;
			_val7 = 1230;
		}

		TRIGGER;
	}

	if (_val10 == 1) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		if (!(lookFlag && player_said("GRATE"))) {
			intr_cancel_sentence();

			if (useFlag && player_said("GRATE")) {
				_val6 = 1010;
				_val7 = 1220;
			} else {
				_val6 = 1010;
				_val7 = 1210;
			}

			TRIGGER;
		}
	}

	if (player_said("PLANK", "URN") || player_said("EDGER", "URN")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 69);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}

	if (!_flag1 && !player_said("WALK TO") &&
			!player_said("POMERANIAN MARKS", "WOLF"))
		_flag1 = true;

	if (_val12) {
		_val12 = 0;

		if (MONEY("POMERANIAN MARKS")) {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			_G(flags)[V114] = 1;
			_val8 = 2000;
			_val9 = 2232;
			_G(flags)[V111]++;

			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 69);
			_G(kernel).trigger_mode = KT_PREPARSE;
		} else if (MONEY("US DOLLARS") || MONEY("CHINESE YUAN") ||
				MONEY("PERUVIAN INTI") || MONEY("SIKKIMESE RUPEE")) {
			_G(flags)[V116] = 1;
			intr_cancel_sentence();
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 230);
			_G(kernel).trigger_mode = KT_PREPARSE;
		} else {
			if (talkFlag && player_said("WOLF"))
				intr_cancel_sentence();

			player_set_commands_allowed(false);
			_val8 = 2000;
			_val9 = 2234;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(1, 110);
			_G(kernel).trigger_mode = KT_PREPARSE;
		}
	}

	if ((lookFlag && player_said(" ")) ||
			(enterFlag && player_said("GRAVEYARD")) ||
			(enterFlag && player_said("CASTLE GROUNDS"))) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room403::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool enterFlag = player_said("enter");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv403a")) {
		if (_G(kernel).trigger == 1) {
			conv403a1();
		} else {
			conv403a();
		}
	} else if (talkFlag && player_said("WOLF")) {
		player_set_commands_allowed(false);
		_val4 = -1;
		_val6 = 1000;
		_val7 = 1100;
		TRIGGER;
	} else if (enterFlag && player_said("CASTLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(408);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("grate")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);

			if (_G(flags)[V123]) {
				digi_play("403R06", 1, 255, 2);
			} else {
				digi_play("403R06", 1, 255, 1);
				_G(flags)[V123] = 1;
			}
			break;
		case 1:
			digi_play("403R06A", 1, 255, 2);
			break;
		case 2:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("WOLF")) {
		digi_play(_G(flags)[V111] > 0 ? "402R13" : "402R12", 1);
	} else if (lookFlag && player_said("wall")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("403r09", 1, 255, 1);
			break;
		case 1:
			player_set_commands_allowed(true);
			digi_play("403r09a", 1);
			break;

		}
	} else if (lookFlag && player_said_any("tomb", "door")) {
		digi_play("403r05", 1);
		_G(flags)[V121] = 1;
	} else if (lookFlag && player_said("bell")) {
		digi_play("403r07", 1);
		_G(flags)[V122] = 1;
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (lookFlag && player_said("edger") && !inv_player_has("EDGER")) {
		digi_play("403r53", 1);
	} else if (lookFlag && player_said_any("STEP LADDER", "STEP LADDER ") &&
			inv_object_is_here("STEP LADDER")) {
		digi_play(_G(flags)[V131] == 403 ? "403R10" : "403R46", 1);
	} else if (useFlag && player_said("PLANK") && inv_object_is_here("PLANK")) {
		_val6 = 1020;
		_val7 = 1300;
		TRIGGER;
	} else if (useFlag && player_said("STEP LADDER") && inv_object_is_here("STEP LADDER")) {
		_val6 = 1010;
		_val7 = 1200;
		TRIGGER;
	} else if (player_said("POMERANIAN MARKS", "WOLF") &&
			inv_player_has("POMERANIAN MARKS")) {
		if (!_G(flags)[V115] || _G(flags)[V114]) {
			player_set_commands_allowed(false);
			_val4 = 200;
		} else {
			_G(flags)[V114] = 1;
			_G(flags)[V111]++;
			player_set_commands_allowed(false);
			_val4 = 210;
		}
	} else if (MONEY("US DOLLARS") || MONEY("CHINESE YUAN") ||
			MONEY("PERUVIAN INTI") || MONEY("SIKKIMESE RUPEE")) {
		if (!_G(flags)[V116]) {
			_G(flags)[V116] = 1;
			player_set_commands_allowed(false);
			_val4 = 220;
		}
	} else if (player_said("EDGER", "BELL") && inv_player_has("EDGER")) {
		edgerBell();
	} else if (player_said("EDGER", "URN")) {
		// No implementation
	} else if ((player_said("STEP LADDER", "TOMB") ||
			player_said("STEP LADDER", "STAIRS")) ||
			stepLadderTomb()) {
		// No implementation
	} else if (player_said("STEP LADDER", "WALL")) {
		digi_play("403R32", 1);
	} else if (player_said("STEP LADDER", "BELL")) {
		digi_play("403R31", 1);
	} else if (takeFlag && player_said("URN")) {
		digi_play("403R28", 1);
	} else if (takeFlag && player_said("BELL")) {
		digi_play("403R30", 1);
	} else if (takeFlag && player_said("PLANK") &&
			takePlank()) {
		// No implementation
	} else if (takeFlag && player_said("EDGER") && takeEdger()) {
		// No implementation
	} else if (takeFlag && player_said("STEP LADDER") && takeStepLadder()) {
		// No implementation
	} else if (takeFlag && player_said("STEP LADDER ")) {
		takeStepLadder_();
	} else if (takeFlag && player_said("EDGER") && inv_object_is_here("EDGER")) {
		digi_play("403R30", 1);
	} else if (takeFlag && (
		player_said_any(
			"broken headstone", "wall", "pommee cross",
			"grave plaque", "1ST MARBLE MONUMENT", "2ND MARBLE MONUMENT",
			"marble column", "small headstone") ||
		player_said_any(
			"marble headstone", "tombstone",
			"burial tablet", "small grave marker", "tall headstone",
			"granite headstone", "burial plaque", "celtic cross")
	)) {
		digi_play("403R33", 1);
	} else if (lookFlag && player_said(" ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("403r04", 1, 255, 2);
			break;
		case 2:
			player_set_commands_allowed(true);
			break;

		}
	} else if (player_said("journal") && (
		player_said_any(
			"broken headstone", "pommee cross", "grave plaque",
			"grave", "1ST MARBLE MONUMENT", "2ND MARBLE MONUMENT",
			"marble column", "small headstone") ||
		player_said_any(
			"marble headstone", "tombstone", "burial tablet",
			"small grave marker", "tall headstone", "granite headstone",
			"burial plaque", "celtic cross")
	)) {
		useJournal();
	} else {
		return;
	}

	_G(player).command_ready = false;
}
#undef MONEY
#undef TRIGGER

void Room403::conv403a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (sound) {
		if (who <= 0) {
			switch (node) {
			case 8:
				_sound1 = sound;

				if (inv_player_has("POMERANIAN MARKS")) {
					conv_resume();
				} else {
					_val9 = 2260;
				}
				break;

			case 10:
				_sound1 = sound;
				_val9 = 2270;
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_timing_trigger(1, 110);
				_G(kernel).trigger_mode = KT_PARSE;
				break;

			case 11:
				if (entry == 0) {
					_sound1 = sound;
					_val9 = 2253;
					return;
				} else {
					_val9 = 2101;
					digi_play(sound, 1, 255, 1);
				}
				break;

			case 17:
				if (entry == 1) {
					_val5 = 1;
					midi_play("DANZIG1", 255, 1, -1, 949);
				}

				_val9 = 2250;
				digi_play(sound, 1, 255, 1);
				break;

			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 24:
			case 28:
			case 29:
			case 30:
			case 31:
			case 32:
			case 33:
				_flag2 = true;
				_val9 = 2101;
				digi_play(sound, 1, 255, 1);
				break;

			default:
				_val9 = 2101;
				digi_play(sound, 1, 255, 1);
				break;
			}
		} else if (who == 1) {
			_val7 = 1102;
			digi_play(sound, 1, 255, 1);
		}
	} else {
		conv_resume();
	}
}

void Room403::conv403a1() {
	int who = conv_whos_talking();

	if (who <= 0) {
		_val9 = (_val9 == 2250) ? 2252 : 2102;
	} else if (who == 1) {
		_val7 = 1103;
	}

	conv_resume();
}

void Room403::edgerBell() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		if (++_G(flags)[V119] < 8) {
			_ripRingsBell = series_load("RIP RINGS BELL");
			player_update_info();
			ws_hide_walker();
			_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallback, "RIP rings bell");

			terminateMachineAndNull(_bell);
			sendWSMessage_10000(1, _ripOnLadder, _ripRingsBell, 1, 19, 1,
				_ripRingsBell, 19, 19, 0);
		}
		break;

	case 1:
		digi_play("403_s12", 2);
		sendWSMessage_10000(1, _ripOnLadder, _ripRingsBell, 19, 19, 2,
			_ripRingsBell, 19, 32, 0);
		break;

	case 2:
		sendWSMessage_10000(1, _ripOnLadder, _ripRingsBell, 32, 1, 3,
			_ripRingsBell, 1, 1, 0);
		break;

	case 3:
		terminateMachineAndNull(_ripOnLadder);
		ws_unhide_walker();
		_bell = series_place_sprite("ONE FRAME BELL", 0, 0, 0, 100, 0xf00);
		series_unload(_ripRingsBell);

		if (_G(flags)[V119] >= 7) {
			ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);
			ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
			_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS, 620, 323, 3,
				triggerMachineByHashCallback3000, "wolf_walker");
			_wolfAdmonish = series_load("WOLF ADMONISHES RIP");
			kernel_timing_trigger(120, 4);
		} else {
			player_set_commands_allowed(true);
		}
		break;

	case 4:
		sendWSMessage_10000(_wolfWalker, 687, 323, 3, 5, 1);
		break;

	case 5:
		sendWSMessage_60000(_wolfWalker);
		_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "WOLFIE");
		sendWSMessage_10000(1, _wolfie, _wolfAdmonish, 1, 18, 6,
			_wolfAdmonish, 18, 18, 0);

		_ripHeadTurn = series_load("RIP TREK HEAD TURN POS1");
		setGlobals1(_ripHeadTurn, 4, 8, 8, 8, 0, 8, 4, 4, 4);
		sendWSMessage_110000(-1);
		break;

	case 6:
		sendWSMessage_10000(1, _wolfie, _wolfAdmonish, 19, 31, 7,
			_wolfAdmonish, 31, 31, 0);
		playNum1(_G(flags)[V119]);
		break;

	case 7:
		sendWSMessage_10000(1, _wolfie, _wolfAdmonish, 31, 1, -1,
			_wolfAdmonish, 1, 1, 0);
		break;

	case 8:
		if (_G(flags)[V119] == 1) {
			digi_play("403r47", 1, 255, 9);
		} else {
			kernel_timing_trigger(60, 9);
		}
		break;

	case 9:
		terminateMachineAndNull(_wolfie);
		_wolfWalker = triggerMachineByHash_3000(8, 8,
			*S4_NORMAL_DIRS, *S4_SHADOW_DIRS, 687, 323, 3,
			triggerMachineByHashCallback3000, "wolf_walker");
		sendWSMessage_10000(_wolfWalker, 620, 323, 3, 10, 0);
		playNum2(_G(flags)[V119]);
		break;

	case 10:
		sendWSMessage_60000(_wolfWalker);
		sendWSMessage_120000(11);
		break;

	case 11:
		series_unload(_wolfAdmonish);
		series_unload(S4_NORMAL_DIRS[1]);
		series_unload(S4_NORMAL_DIRS[0]);
		series_unload(S4_SHADOW_DIRS[1]);
		series_unload(S4_SHADOW_DIRS[0]);
		series_unload(_ripHeadTurn);
		sendWSMessage_150000(-1);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

bool Room403::edgerUrn() {
	switch (_G(kernel).trigger) {
	case 1:
		if (inv_player_has("EDGER")) {
			player_set_commands_allowed(false);
			_ripMedReach = series_load("RIP MED REACH 1HAND POS2");
			setGlobals1(_ripMedReach, 1, 17, 17, 17, 0, 17, 1, 1, 1);
			sendWSMessage_110000(2);
			return true;
		}
		return false;

	case 2:
		_edger = series_place_sprite("ONE FRAME EDGER", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("EDGER", true);
		inv_move_object("EDGER", 403);
		sendWSMessage_120000(3);
		return true;

	case 3:
		sendWSMessage_150000(4);
		return true;

	case 4:
		series_unload(_ripMedReach);
		player_set_commands_allowed(true);
		return true;

	case 69:
		if (inv_player_has("EDGER")) {
			player_set_commands_allowed(false);
			ws_walk(1201, 321, 0, 1, 2);
			return true;
		}
		return false;

	default:
		return true;
	}
}

bool Room403::stepLadderTomb() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_player_has("STEP LADDER")) {
			player_set_commands_allowed(false);
			_series1 = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_series1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			return true;
		}
		return false;

	case 1:
		digi_play("403_s04", 2);
		_ladder = series_place_sprite("1 sprite of ladder", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("STEP LADDER", true);
		inv_move_object("STEP LADDER", 403);
		_G(flags)[V313] = 2;
		sendWSMessage_120000(3);
		return true;

	case 3:
		sendWSMessage_150000(4);
		return true;

	case 4:
		series_unload(_series1);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room403::takePlank() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("PLANK")) {
			_ripPutBoard = series_load("RIPLEY PUTS BOARD ON POTS");
			terminateMachineAndNull(_board);
			ws_hide_walker();

			_ripOnLadder = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x300, 0,
				triggerMachineByHashCallback, "RIP plants plank");
			sendWSMessage_10000(1, _ripOnLadder, _ripPutBoard, 57, 1, 2,
				_ripPutBoard, 1, 1, 0);
			return true;
		}
		return false;

	case 2:
		hotspot_set_active("PLANK", false);
		inv_give_to_player("PLANK");
		_plank = 0;
		kernel_examine_inventory_object("PING PLANK", _G(master_palette),
			5, 1, 396, 223, 3, nullptr, -1);
		return true;

	case 3:
		terminateMachineAndNull(_ripOnLadder);
		ws_unhide_walker();
		series_unload(_ripPutBoard);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room403::takeEdger() {
	switch (_G(kernel).trigger) {
	case -1:
		if (!inv_player_has("EDGER")) {
			player_set_commands_allowed(false);
			_ripMedReach = series_load("RIP MED REACH 1HAND POS2");
			setGlobals1(_ripMedReach, 1, 17, 17, 17, 0, 17, 1, 1, 1);
			sendWSMessage_110000(1);
			return true;
		}
		return false;

	case 1:
		terminateMachineAndNull(_edger);
		hotspot_set_active("EDGER", false);
		inv_give_to_player("EDGER");
		kernel_examine_inventory_object("PING EDGER", _G(master_palette),
			5, 1, 500, 216, 2, 0, -1);
		return true;

	case 2:
		sendWSMessage_120000(3);
		return true;

	case 3:
		sendWSMessage_150000(4);
		return true;

	case 4:
		series_unload(_ripMedReach);
		player_set_commands_allowed(true);
		return true;

	case 5:
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room403::takeStepLadder() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("STEP LADDER")) {
			player_set_commands_allowed(false);
			_series1 = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_series1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			return true;
		}
		return false;

	case 1:
		if (_G(flags)[V131] == 403) {
			digi_play("403w08", 1, 255, 3);
			_val8 = 2000;
			_val9 = 2100;
			kernel_timing_trigger(15, 2);
		} else {
			digi_play("403_s03", 2);
			terminateMachineAndNull(_ladder);
			hotspot_set_active("STEP LADDER", false);
			inv_give_to_player("STEP LADDER");
			_G(flags)[V313] = 0;
			kernel_examine_inventory_object("PING STEP LADDER", _G(master_palette),
				5, 1, 429, 215, 2, nullptr, -1);

		}
		return true;

	case 2:
		sendWSMessage_120000(4);
		return true;

	case 3:
		_val9 = 2105;
		digi_play("403r47", 1);
		return true;

	case 4:
		sendWSMessage_150000(5);
		return true;

	case 5:
		series_unload(_series1);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

void Room403::takeStepLadder_() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("STEP LADDER")) {
			player_set_commands_allowed(false);
			_series1 = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_series1, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
		}
		break;

	case 1:
		if (_G(flags)[V131] == 403) {
			digi_play("403w08", 1, 255, 3);
			_val8 = 2000;
			_val9 = 2100;
			kernel_timing_trigger(15, 2);
		} else {
			digi_play("403_s03", 2);
			hotspot_set_active("STEP LADDER ", false);
			inv_give_to_player("STEP LADDER");
			_G(flags)[V313] = 0;
			kernel_examine_inventory_object("PING STEP LADDER",
				_G(master_palette), 5, 1, 496, 226, 2, nullptr, -1);
			terminateMachineAndNull(_ladder);
		}
		break;

	case 2:
		sendWSMessage_120000(4);
		break;

	case 3:
		_val9 = 2105;
		digi_play("403r47", 1);
		break;

	case 4:
		sendWSMessage_150000(5);
		break;

	case 5:
		series_unload(_series1);
		player_set_commands_allowed(true);
		break;

	case 6:
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room403::useJournal() {
	if (_G(flags)[kGraveyardCartoon]) {
		digi_play("403r42", 1);
	} else if (_G(flags)[kEpitaphCartoon]) {
		if (_G(kernel).trigger == 6)
			_G(flags)[kGraveyardCartoon] = 1;
		sketchInJournal(nullptr);
	} else {
		if (_G(kernel).trigger == 6)
			_G(flags)[kGraveyardCartoon] = 1;
		sketchInJournal("403r41");
	}
}

void Room403::playNum1(int num) {
	static const char *const NAMES[] = {
		"403w11", "403w13", "403w15", "40ew16",
		"403w17", "403w18", "40w1", "40w1"
	};
	digi_play(NAMES[num - 1], 1, 255, 8);
}

void Room403::playNum2(int num) {
	switch (num) {
	case 1:
		digi_play("403w12", 1);
		break;
	case 2:
		digi_play("403w14", 1);
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
