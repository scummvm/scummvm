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

#include "m4/riddle/rooms/section4/room408.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room408::init() {
	player_set_commands_allowed(false);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_currentNode = -1;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = 0;
	}

	digi_preload("950_s19");
	digi_preload("950_s20");
	digi_preload("950_s21");
	hotspot_set_active("WOLF", false);
	hotspot_set_active("PLANK", false);
	hotspot_set_active("EDGER", false);
	_exit = series_show("RIP EXITS 407", 0xf00, 16);

	if (_G(flags)[V139] == 1) {
		_G(flags)[V139] = 0;
		_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
		hotspot_set_active("EDGER", true);
		inv_move_object("EDGER", 408);
		ws_demand_location(234, 319, 3);
		ws_walk(438, 325, nullptr, 400, 1);
		
	} else if (_G(flags)[V139] == 3) {
		_G(flags)[V139] = 0;

		if (inv_object_is_here("PLANK")) {
			_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("PLANK", true);
		}

		ws_demand_location(-20, 345, 3);
		ws_walk(234, 319, nullptr, 420, 1);

	} else {
		if (inv_player_has("TURTLE") && !inv_player_has("EDGER") &&
				!_G(flags)[GLB_TEMP_12]) {
			_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
			hotspot_set_active("EDGER", true);
		}

		if (inv_player_has("TURTLE") && !inv_player_has("PLANK") &&
				!_G(flags)[GLB_TEMP_12] && inv_object_is_here("PLANK")) {
			_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("PLANK", true);
		}

		switch (_G(game).previous_room) {
		case KERNEL_RESTORING_GAME:
			digi_preload("950_s22");

			if (_G(flags)[V131] == 400) {
				hotspot_set_active("WOLF", true);
				_val8 = 2001;
				_val9 = 2200;
				_wolf = series_load("WOLF CLPNG LOOP LOOKS TO SIDE");
				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
					triggerMachineByHashCallback, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolf, 1, 10, 110, _wolf, 10, 10, 0);
			}

			player_set_commands_allowed(true);
			break;

		case 402:
			if (_G(flags)[V132]) {
				ws_hide_walker();
				ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);
				ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
				kernel_timing_trigger(1, 300);
			} else {
				if (_G(flags)[V131] == 408) {
					hotspot_set_active("WOLF", true);
					_wolf = series_load("WOLF CLPNG LOOP LOOKS TO SIDE");
					_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
						triggerMachineByHashCallback, "WOLFIE");
					sendWSMessage_10000(1, _wolfie, _wolf, 1, 10, 110, _wolf, 10, 10, 0);
					_val8 = 2001;
					_val9 = 2200;
				}

				ws_demand_location(-20, 345, 3);
				ws_walk(35, 345, nullptr, 20, 3);
			}
			break;

		case 407:
			digi_preload("950_s22");
			terminateMachineAndNull(_exit);

			if (_G(flags)[GLB_TEMP_12]) {
				_G(flags)[V131] = 999;
			} else if (inv_player_has("TURTLE")) {
				_G(flags)[V131] = 402;
				_G(flags)[V117] = 1;
			} else if (_G(flags)[V124]) {
				_G(flags)[V131] = 402;
			} else {
				_G(flags)[V131] = 403;
			}

			_ripExits = series_load("RIP EXITS 407");
			ws_demand_location(201, 287, 4);
			ws_hide_walker();

			_exit = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallback, "RIP ENTERS from GIZMO");
			sendWSMessage_10000(1, _exit, _ripExits, 1, 75, 40, _ripExits, 75, 75, 0);
			digi_play("408_s01", 2);
			break;

		default:
			digi_preload("950_s22");

			if (_G(flags)[V117] && _G(flags)[V125] == 3 &&
				!_G(flags)[GLB_TEMP_12] && !inv_player_has("EDGER") &&
				!inv_player_has("PLANK")) {
				_G(flags)[V131] = 408;
				_G(flags)[V117] = 0;
			}

			if (_G(flags)[V131] == 408) {
				hotspot_set_active("WOLF", true);
				_wolf = series_load("WOLF CLPNG LOOP LOOKS TO SIDE");
				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
					triggerMachineByHashCallback, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolf, 1, 10, 110, _wolf, 10, 10, 0);
				_val8 = 2001;
				_val9 = 2200;
			}

			ws_demand_location(660, 345, 9);

			if (_G(flags)[V125] == 3) {
				series_simple_play("408 turtle popup", 0, true);
				ws_walk(438, 325, nullptr, 350, 1);
			} else {
				ws_walk(615, 345, nullptr, 30, 9);
			}
			break;
		}
	}

	digi_play_loop("950_s22", 3, 120, -1, 950);
}

void Room408::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 20:
	case 30:
		player_set_commands_allowed(true);
		break;

	case 40:
		digi_play("408_s03", 2);
		sendWSMessage_10000(1, _exit, _ripExits, 76, 85, 42, _ripExits, 85, 85, 0);
		break;

	case 42:
		ws_unhide_walker();
		DisposePath(_G(my_walker)->walkPath);
		_G(my_walker)->walkPath = CreateCustomPath(250, 235, -1);
		ws_custom_walk(_G(my_walker), 4, -1);
		sendWSMessage_10000(1, _exit, _ripExits, 31, 1, 44, _ripExits, 1, 1, 0);
		digi_play("408r31", 1);
		break;

	case 44:
		terminateMachineAndNull(_exit);
		series_unload(_ripExits);
		_exit = series_show("RIP EXITS 407", 0xf00, 16);
		player_set_commands_allowed(true);
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
		if (_val6 == 1000) {
			switch (_val7) {
			case 1100:
				ws_hide_walker();
				player_set_commands_allowed(false);
				_ripHandsBehindBack = series_load("RIP TREK HANDS BEHIND BACK POS2");

				player_update_info();
				_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf");
				_ripleyShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale, 0x100, 0,
					triggerMachineByHashCallback, "rip talks wolf SHADOW");

				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 1, 15, 102,
					_ripHandsBehindBack, 15, 15, 0);
				sendWSMessage_10000(1, _ripleyShadow, _ripShadowSeries, 1, 1, -1,
					_ripShadowSeries, 1, 1, 0);
				_val7 = 1101;
				_val8 = 2000;
				_val9 = 2100;
				kernel_timing_trigger(1, 110);
				break;

			case 1101:
				_val7 = 1103;
				kernel_timing_trigger(1, 102);
				conv_load("conv408a", 0, 10, 101);
				conv_export_value_curr(_G(flags)[V117], 0);
				conv_export_value_curr(_G(flags)[V118], 1);
				conv_play();
				break;

			case 1102:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 15, 102,
					_ripHandsBehindBack, 15, 15, 0);
				break;

			case 1103:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 15, 102,
					_ripHandsBehindBack, 15, 15, 0);
				break;

			case 1104:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 15, -1,
					_ripHandsBehindBack, 15, 15, 0);
				break;

			case 1105:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 1, 103,
					_ripHandsBehindBack, 1, 1, 0);
				_val7 = 1106;
				_val9 = 2103;
				break;

			case 1106:
				terminateMachineAndNull(_ripley);
				terminateMachineAndNull(_ripleyShadow);
				ws_unhide_walker();
				series_unload(_ripHandsBehindBack);

				if (_currentNode != 8 && _currentNode != 9) {
					_val8 = 2001;
					_val9 = 2200;
					kernel_timing_trigger(1, 110);
					player_set_commands_allowed(true);
				} else {
					_val4 = 320;
					kernel_timing_trigger(1, 110);
				}
				break;

			default:
				break;
			}
		}
		break;

	case 110:
		switch (_val8) {
		case 2000:
			if (_val9 >= 2100 && _val9 <= 2104) {
				if (_val4 != -1) {
					kernel_timing_trigger(1, _val4);
					_val4 = -1;
				} else {
					kernel_timing_trigger(1, 111);
				}
			}
			break;

		case 2001:
			if (_val9 == 2200) {
				if (_val4 != -1) {
					kernel_timing_trigger(1, _val4);
					_val4 = -1;
				} else {
					kernel_timing_trigger(1, 111);
				}
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
				sendWSMessage_10000(1, _wolfie, _wolf, 10, 13, 110, _wolf, 13, 13, 0);
				_val9 = 2102;
				break;
			case 2101:
				frame = imath_ranged_rand(14, 16);
				sendWSMessage_10000(1, _wolfie, _wolf, frame, frame, 110,
					_wolf, frame, frame, 0);
				break;
			case 2102:
				sendWSMessage_10000(1, _wolfie, _wolf, 13, 13, 110, _wolf, 13, 13, 0);
				_val9 = 2102;
				break;
			case 2103:
				sendWSMessage_10000(1, _wolfie, _wolf, 13, 10, -1, _wolf, 10, 10, 0);
				break;
			case 2104:
				sendWSMessage_10000(1, _wolfie, _wolf, 1, 9, 110, _wolf, 9, 9, 0);
				break;
			default:
				break;
			}
			break;

		case 2001:
			if (_val9 == 2200) {
				sendWSMessage_10000(1, _wolfie, _wolf, 1, 9, 110, _wolf, 9, 9, 0);

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
			}
			break;
		default:
			break;
		}
		break;

	case 220:
		_val8 = 2000;
		_val9 = 2100;

		kernel_timing_trigger(1, 110);
		_ripTrekTwoHandTalk = series_load("RIP TREK TWO HAND TALK POS2");

		player_update_info();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale,
			0x100, 0, triggerMachineByHashCallback, "rip talks wolf");
		_ripleyShadow = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale,
			0x100, 0, triggerMachineByHashCallback, "rip talks wolf SHADOW");

		sendWSMessage_10000(1, _ripleyShadow, _ripShadowSeries, 1, 1, -1,
			_ripShadowSeries, 1, 1, 0);
		sendWSMessage_10000(1, _ripley, _ripTrekTwoHandTalk, 1, 6, 222,
			_ripTrekTwoHandTalk, 6, 6, 0);
		break;

	case 222:
		_val9 = 2101;
		kernel_timing_trigger(1, 110);
		digi_play("402w008", 1, 255, 226);
		kernel_timing_trigger(45, 224);
		break;

	case 225:
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_ripleyShadow);
		ws_unhide_walker();
		series_unload(_ripTrekTwoHandTalk);
		break;

	case 226:
		_val9 = 2103;
		kernel_timing_trigger(30, 227);
		break;

	case 227:
		_val8 = 2001;
		_val9 = 2200;
		kernel_timing_trigger(1, 110);
		player_set_commands_allowed(true);
		break;

	case 300:
		_wolfWalker = triggerMachineByHash_3000(8, 8, *S4_NORMAL_DIRS, *S4_SHADOW_DIRS,
			-20, 345, 3, triggerMachineByHashCallback3000, "WOLF_WALKER");
		sendWSMessage_10000(_wolfWalker, 660, 345, 9, -1, 0);
		kernel_timing_trigger(400, 302);
		kernel_timing_trigger(450, 303);
		break;

	case 302:
		ws_unhide_walker();
		player_first_walk(-20, 345, 3, 660, 345, 9, false);
		break;

	case 303:
		disable_player_commands_and_fade_init(304);
		break;

	case 304:
		_G(game).setRoom(403);
		break;

	case 320:
		terminateMachineAndNull(_wolfie);
		_wolfie = series_stream("WOLF STPS DOWN DRPS CLIPPERS", 6, 0xd00, 323);
		series_stream_break_on_frame(_wolfie, 100, 322);
		break;

	case 322:
		ws_walk(414, 336, nullptr, -1, 9);
		break;

	case 323:
		hotspot_set_active("WOLF", false);
		_G(flags)[V131] = 402;
		player_set_commands_allowed(true);
		break;

	case 350:
		_ripLowReacher = series_load("RIP TREK LOW REACHER POS1");
		setGlobals1(_ripLowReacher, 1, 7, 7, 7, 0, 7, 1, 1, 1);
		sendWSMessage_110000(352);
		break;

	case 352:
		_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
		inv_move_object("PLANK", 408);
		hotspot_set_active("PLANK", true);
		sendWSMessage_120000(353);
		break;

	case 353:
		sendWSMessage_150000(354);
		break;

	case 354:
		series_unload(_ripLowReacher);
		ws_walk(234, 319, nullptr, 355, 1);
		_ripLowReacher = series_load("RIP TREK MED REACH HAND POS1");
		break;

	case 355:
		setGlobals1(_ripLowReacher, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(356);
		break;

	case 356:
		_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
		hotspot_set_active("EDGER", true);
		inv_move_object("EDGER", 408);
		sendWSMessage_120000(357);
		break;

	case 357:
		sendWSMessage_150000(358);
		break;

	case 358:
		series_unload(_ripLowReacher);
		ws_walk(-20, 345, nullptr, -1, 9);
		kernel_timing_trigger(60, 359);
		break;

	case 359:
		disable_player_commands_and_fade_init(360);
		break;

	case 360:
		digi_stop(3);
		_G(game).setRoom(402);
		break;

	case 400:
		_ripLowReacher = series_load("RIP TREK LOW REACHER POS1");
		setGlobals1(_ripLowReacher, 1, 7, 7, 7, 0, 7, 1, 1, 1);
		sendWSMessage_110000(402);
		break;

	case 402:
		_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
		inv_move_object("PLANK", 408);
		hotspot_set_active("PLANK", true);
		sendWSMessage_120000(403);
		break;

	case 403:
		sendWSMessage_150000(404);
		break;

	case 404:
	case 424:
		series_unload(_ripLowReacher);
		player_set_commands_allowed(true);
		break;

	case 420:
		_ripLowReacher = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_ripLowReacher, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(422);
		break;

	case 422:
		_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
		hotspot_set_active("EDGER", true);
		inv_move_object("EDGER", 408);
		sendWSMessage_120000(423);
		break;

	case 423:
		sendWSMessage_150000(424);
		break;

	default:
		break;
	}
}

void Room408::pre_parser() {
	bool takeFlag = player_said("take");
	bool lookFlag = player_said_any("look", "look at");
	bool enterFlag = player_said("enter");

	if (lookFlag && player_said(" "))
		_G(player).resetWalk();

	if (enterFlag && player_said("GRAVEYARD"))
		_G(player).resetWalk();

	if (player_said("journal") && !takeFlag && !lookFlag &&
			_G(kernel).trigger == -1)
		_G(player).resetWalk();
}

void Room408::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said("talk", "talk to");
	bool takeFlag = player_said("take");
	bool enterFlag = player_said("enter");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv408a")) {
		if (_G(kernel).trigger == -1) {
			_val7 = 1103;
			_val9 = 2102;
			conv_resume();
		} else {
			conv408a();
		}
	} else if (talkFlag && player_said("WOLF")) {
		player_set_commands_allowed(false);
		_val4 = -1;
		_val6 = 1000;
		_val7 = 1100;
		kernel_timing_trigger(1, 102, KT_DAEMON, KT_PARSE);
	} else if (lookFlag && player_said("WINDOW")) {
		digi_play("408r03", 1);
	} else if (lookFlag && player_said("CASTLE")) {
		digi_play("408r01", 1);
	} else if (lookFlag && player_said("TOPIARY")) {
		digi_play("408r02", 1);
	} else if (lookFlag && player_said("SUNDIAL")) {
		digi_play(player_been_here(408) ? "408r32" : "408r04", 1);
	} else if (lookFlag && player_said_any("BUSH", "BUSH ")) {
		digi_play("408r05", 1);
	} else if (lookFlag && player_said("PLANK") && inv_object_is_here("PLANK")) {
		digi_play("408r20", 1);
	} else if (lookFlag && player_said("EDGER") && inv_object_is_here("EDGER")) {
		digi_play("408r35", 1);
	} else if (lookFlag && player_said("WOLF")) {
		digi_play("408r21", 1);
	} else if (takeFlag && player_said("TOPIARY")) {
		digi_play("408r06", 1);
	} else if (takeFlag && player_said("SUNDIAL")) {
		digi_play("408r08", 1);
	} else if (takeFlag && player_said("PLANK") && takePlank()) {
		// No implementation
	} else if (takeFlag && player_said("EDGER") && takeEdger()) {
		// No implementation
	} else if (player_said("EDGER", "BUSH")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripLowReacher = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripLowReacher, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;

		case 1:
			_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
			hotspot_set_active("EDGER", true);
			inv_move_object("EDGER", 408);
			sendWSMessage_120000(3);
			break;

		case 3:
			sendWSMessage_150000(4);
			break;

		case 4:
			series_unload(_ripLowReacher);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (player_said("PLANK", "TOPIARY")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags)[V131] != 408) {
				player_set_commands_allowed(false);
				_ripLowReacher = series_load("RIP TREK LOW REACHER POS1");
				setGlobals1(_ripLowReacher, 1, 7, 7, 7, 0, 7, 1, 1, 1);
				sendWSMessage_110000(1);
			}
			break;

		case 1:
			_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
			inv_move_object("PLANK", 408);
			hotspot_set_active("PLANK", true);
			sendWSMessage_120000(3);
			break;

		case 2:
			_G(game).setRoom(403);
			break;

		default:
			break;
		}
	} else if (useFlag && player_said("TOPIARY")) {
		digi_play("408r07", 1);
	} else if (useFlag && player_said("SUNDIAL")) {
		digi_play(player_been_here(407) ? "408r34" : "408r09", 1);
	} else if (useFlag && player_said("WINDOW")) {
		digi_play("408r10", 1);
	} else if (player_said("POMERANIAN MARKS", "WOLF") &&
			inv_player_has("POMERANIAN MARKS")) {
		digi_play("408w07", 1);
	} else if (enterFlag && player_said("CASTLE GROUNDS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(-20, 345, nullptr, 1, 9);
			break;
		case 1:
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s22", 3, 255, -1, 950);
			_G(game).setRoom(402);
			break;
		default:
			break;
		}
	} else if (enterFlag && player_said("GRAVEYARD")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(660, 345, nullptr, 1, 3);
			break;
		case 1:
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			_G(game).setRoom(403);
			break;
		default:
			break;
		}
	} else if (player_said("journal") && !takeFlag && !lookFlag &&
			!inv_player_has(_G(player).noun)) {
		if (_G(flags)[kCastleCartoon]) {
			digi_play("com016", 1);
		} else if (_G(kernel).trigger == 6) {
			_G(flags)[kCastleCartoon] = 1;
			sendWSMessage_multi("com015");
		} else {
			sendWSMessage_multi("com015");
		}
	} else if (lookFlag && player_said(" ")) {
		digi_play("408r01", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room408::conv408a() {
	int who = conv_whos_talking();
	_currentNode = conv_current_node();
	const char *sound = conv_sound_to_play();

	if (sound) {
		if (who <= 0) {
			_val9 = 2101;
			digi_play(sound, 1);
		} else if (who == 1) {
			_val7 = 1102;
			digi_play(sound, 1);
		}
	} else {
		conv_resume();
	}
}

bool Room408::takePlank() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_player_has("PLANK")) {
			player_set_commands_allowed(false);

			if (_G(flags)[V131] == 408) {
				digi_play("408r29", 1, 255, 5);
			} else {
				_ripLowReacher = series_load("RIP TREK LOW REACHER POS1");
				setGlobals1(_ripLowReacher, 1, 7, 7, 7, 0, 7, 1, 1, 1);
				sendWSMessage_110000(1);
			}
			break;
		}
		return false;

	case 1:
		terminateMachineAndNull(_plank);
		inv_give_to_player("PLANK");
		hotspot_set_active("PLANK", false);
		kernel_examine_inventory_object("PING PLANK",
			_G(master_palette), 5, 1, 362, 225, 2, "408_s02", -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripLowReacher);
		player_set_commands_allowed(true);
		break;

	case 5:
		player_set_commands_allowed(true);
		break;

	default:
		return false;
	}

	return true;
}

bool Room408::takeEdger() {
	switch (_G(kernel).trigger) {
	case -1:
		if (!inv_player_has("EDGER")) {
			player_set_commands_allowed(false);

			if (_G(flags)[V131] == 408) {
				digi_play("408r30", 1, 255, 5);
			} else {
				_ripLowReacher = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(_ripLowReacher, 1, 10, 10, 10, 0, 10, 1, 1, 1);
				sendWSMessage_110000(1);
			}
			break;
		}
		return false;

	case 1:
		terminateMachineAndNull(_edger);
		hotspot_set_active("EDGER", false);
		inv_give_to_player("EDGER");
		kernel_examine_inventory_object("PING EDGER", _G(master_palette),
			5, 1, 190, 215, 2, nullptr, -1);
		break;

	case 2:
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripLowReacher);
		player_set_commands_allowed(true);
		break;

	case 5:
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}

	return true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
