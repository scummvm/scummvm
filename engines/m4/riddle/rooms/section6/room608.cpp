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

#include "m4/riddle/rooms/section6/room608.h"
#include "m4/riddle/rooms/section6/section6.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS1[] = { 220, 221, -1 };
static const char *NORMAL_NAMES1[] = { "oldwk1", "oldwk3" };
static const int16 SHADOW_DIRS1[] = { 230, 231, -1 };
static const char *SHADOW_NAMES1[] = { "tt walker shadow 1", "oldwk3s" };


void Room608::init() {
	if (_G(flags)[V186] != 2) {
		_rp09 = series_load("608rp09");

		if (!inv_object_is_here("POLE") || _G(flags)[V186])
			kernel_load_variant("608lock1");
		else
			kernel_load_variant("608lock2");

		_old1 = series_load("608old01");
		_old2 = series_load("608old02");
		_old5 = series_load("608old05");
		_old8 = series_load("608old08");
		_old5a = series_load("608old5a");
		_old5b = series_load("608old5b");
		_old5c = series_load("608old5c");
		_old5d = series_load("608old5d");
		_old5f = series_load("608old5f");

		ws_walk_load_walker_series(NORMAL_DIRS1, NORMAL_NAMES1);
		ws_walk_load_shadow_series(SHADOW_DIRS1, SHADOW_NAMES1);
		_ol = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
			triggerMachineByHashCallbackNegative, "ol");
		sendWSMessage_10000(1, _ol, _old1, 1, 1, 200, _old1, 1, 1, 0);
		_val1 = 0;
		_val2 = 0;
		_val3 = 0;
	}

	if (inv_object_is_here("DRIFTWOOD STUMP")) {
		_stump = series_show("608ST_DN", 0xf00, 16);
		hotspot_set_active("DRIFTWOOD STUMP ", false);
	} else {
		hotspot_set_active("DRIFTWOOD STUMP", false);
	}

	if (inv_object_is_here("LIGHTER") && _G(flags)[V203] >= 7) {
		_lighter = series_show("608LITER", 0xf00, 16);
	} else {
		hotspot_set_active("LIGHTER", false);
	}

	if (!inv_object_is_here("DRIFTWOOD STUMP")) {
		hotspot_set_active("DRIFTWOOD STUMP ", false);
		hotspot_set_active("DRIFTWOOD STUMP  ", false);
	} else if (_G(flags)[V186]) {
		hotspot_set_active("DRIFTWOOD STUMP ", false);
	} else {
		_stump = series_show("608ST_UP", 0x400, 16);
		hotspot_set_active("DRIFTWOOD STUMP  ", false);
	}

	if (!inv_object_is_here("POLE")) {
		hotspot_set_active("POLE", false);
		hotspot_set_active("POLE ", false);
	} else if (_G(flags)[V186]) {
		hotspot_set_active("POLE", false);
	} else {
		_pole = series_show("608POLE", 0x400, 16);
		hotspot_set_active("POLE ", false);
	}

	if (inv_object_is_here("DRIFTWOOD PUFFIN")) {
		_puffin = series_show("608PUFIN", 0x400, 16);
	} else {
		hotspot_set_active("DRIFTWOOD PUFFIN", false);
	}

	switch (_G(flags)[V186]) {
	case 0:
		hotspot_set_active("hole in hillside", false);
		_end1 = series_show("608START", 0xf00, 16);
		hotspot_set_active("STATUE ", false);
		digi_preload("608_s01");
		digi_preload("608_s01a");
		break;
	case 1:
		hotspot_set_active("hole in hillside", false);
		_end1 = series_show("608END", 0xf00, 16);
		hotspot_set_active("STATUE", false);
		hotspot_set_active("stone", false);
		digi_preload("950_s37");
		break;
	case 2:
		hotspot_set_active("hole in hillside", true);
		_end1 = series_show("608END", 0xf00, 16);
		_end2 = series_show("608END2", 0xe00, 16);
		hotspot_set_active("STATUE", false);
		hotspot_set_active("stone", false);
		hotspot_set_active("old woman", false);
	default:
		break;
	}

	if (_G(flags)[V203] == 2) {
		_tt01 = series_load("608TT01");
		_rp01 = series_load("608RP01");

		player_set_commands_allowed(false);
		_G(player).disable_hyperwalk = true;
		ws_demand_location(-30, 345, 1);
		ws_walk(43, 345, nullptr, 18, 3);
	
		ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
		ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);

		_tt = triggerMachineByHash_3000(8, 9, SECTION6_NORMAL_DIRS, SECTION6_SHADOW_DIRS,
			-30, 324, 3, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 105, 324, 5, 20, 1);
	} else if (_G(flags)[V203] == 6) {
		_tt02 = series_load("606TT02");
		_tt03 = series_load("608TT03");
		_tt05 = series_load("608TT05");
		_ripHandChin = series_load("RIP TREK HAND CHIN POS3");
		_ripTalker = series_load("RIP TREK TALKER POS3");
		_all5a = series_load("608all5a");
		_loop0 = series_load("608loop0");
		_loop1 = series_load("608loop1");

		player_set_commands_allowed(false);
		_G(player).disable_hyperwalk = true;
		ws_demand_location(-30, 345, 1);
		kernel_timing_trigger(1, 698);

		ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
		ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);

		_tt = triggerMachineByHash_3000(8, 9, SECTION6_NORMAL_DIRS, SECTION6_SHADOW_DIRS,
			-30, 324, 3, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 103, 318, 3, 700, 1);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28b");
		break;

	case 603:
		digi_preload("950_s28b");
		digi_stop(3);
		digi_unload("950_s28a");
		break;

	case 605:
		digi_preload("950_s28b");
		digi_stop(3);
		digi_unload("950_s28");

		if (_G(flags)[V203] != 2) {
			player_set_commands_allowed(false);
			ws_demand_location(-30, 345, 3);
			ws_walk(43, 345, nullptr, 1, 3);
		}
		break;

	default:
		digi_preload("950_s28b");
		break;
	}

	series_play("608surf", 0xf00, 0, -1, 15, -1, 100, 0, 0, 0, -1);
	digi_play_loop("950_s28b", 3, 90);
}

void Room608::daemon() {
	int frame;

	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 18:
		player_update_info();
		ws_hide_walker();

		_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip talker");
		sendWSMessage_10000(1, _ripley, _rp01, 1, 13, -1, _rp01, 20, 20, 0);
		break;

	case 20:
		player_update_info(_tt, &_G(player_info));
		ws_hide_walker();

		_ttShadow = series_show("tt walker shadow 5", 0xf00, 0, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_ttTalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "tt talker");
		sendWSMessage_10000(1, _ttTalker, _tt01, 63, 54, 22, _tt01, 54, 54, 0);
		break;

	case 22:
		sendWSMessage_10000(1, _ttTalker, _tt01, 40, 45, -1, _tt01, 46, 53, 4);
		sendWSMessage_1a0000(_ttTalker, 11);
		digi_play("608t01", 1, 255, 23);
		break;

	case 23:
		sendWSMessage_10000(1, _ttTalker, _tt01, 45, 40, 26, _tt01, 40, 40, 0);
		break;

	case 26:
		sendWSMessage_10000(1, _ttTalker, _tt01, 9, 39, -1, _tt01, 39, 39, 0);
		sendWSMessage_190000(_ttTalker, 7);
		digi_play("608t01a", 1, 255, 30);
		break;

	case 30:
		sendWSMessage_10000(1, _ripley, _rp01, 20, 20, -1, _rp01, 20, 23, 1);
		sendWSMessage_1a0000(_ripley, 11);
		digi_play("608r01", 1, 255, 34);
		break;

	case 34:
		sendWSMessage_10000(1, _ripley, _rp01, 20, 20, -1, _rp01, 20, 20, 0);
		sendWSMessage_10000(1, _ttTalker, _tt01, 40, 45, -1, _tt01, 46, 53, 4);
		sendWSMessage_1a0000(_ttTalker, 11);
		digi_play("608t02", 1, 255, 39);
		kernel_timing_trigger(60, 35);
		break;

	case 35:
		sendWSMessage_10000(1, _ripley, _rp01, 14, 19, 36, _rp01, 20, 20, 0);
		break;

	case 36:
		sendWSMessage_10000(1, _ripley, _rp01, 14, 19, 37, _rp01, 20, 20, 0);
		break;

	case 37:
		kernel_timing_trigger(60, 38);
		break;

	case 38:
		sendWSMessage_10000(1, _ripley, _rp01, 24, 61, 40, _rp01, 20, 20, 0);
		sendWSMessage_190000(_ripley, 6);
		break;

	case 39:
		sendWSMessage_10000(1, _ripley, _rp01, 20, 20, -1, _rp01, 20, 23, 1);
		sendWSMessage_1a0000(_ripley, 11);
		sendWSMessage_10000(1, _ttTalker, _tt01, 46, 46, -1, _tt01, 46, 46, 0);
		digi_play("608r02", 1, 255, 42);
		break;

	case 40:
		kernel_timing_trigger(120, 41);
		break;

	case 41:
		sendWSMessage_10000(1, _ripley, _rp01, 61, 24, -1, _rp01, 20, 20, 0);
		sendWSMessage_190000(_ripley, 6);
		break;

	case 42:
		sendWSMessage_10000(1, _ripley, _rp01, 20, 20, -1, _rp01, 20, 20, 0);
		sendWSMessage_10000(1, _ttTalker, _tt01, 46, 46, -1, _tt01, 46, 53, 4);
		sendWSMessage_1a0000(_ttTalker, 11);
		digi_play("608t03", 1, 255, 49);
		kernel_timing_trigger(60, 43);
		break;

	case 43:
		sendWSMessage_10000(1, _ripley, _rp01, 24, 39, 44, _rp01, 20, 20, 0);
		sendWSMessage_190000(_ripley, 6);
		break;

	case 44:
		sendWSMessage_10000(1, _ripley, _rp01, 24, 39, 45, _rp01, 20, 20, 0);
		sendWSMessage_190000(_ripley, 6);
		break;

	case 45:
		kernel_timing_trigger(60, 46);
		break;

	case 46:
		sendWSMessage_10000(1, _ripley, _rp01, 14, 14, -1, _rp01, 14, 19, 4);
		sendWSMessage_1a0000(_ripley, 11);
		kernel_timing_trigger(180, 47);
		break;

	case 47:
		sendWSMessage_10000(1, _ripley, _rp01, 20, 20, -1, _rp01, 20, 20, 4);
		break;

	case 49:
		sendWSMessage_10000(1, _ttTalker, _tt01, 45, 40, 50, _tt01, 40, 40, 0);
		break;

	case 50:
		sendWSMessage_10000(1, _ttTalker, _tt01, 54, 63, 55, _tt01, 63, 63, 0);
		break;

	case 55:
		terminateMachineAndNull(_ttTalker);
		ws_unhide_walker();
		terminateMachineAndNull(_ttShadow);
		sendWSMessage_10000(_tt, -30, 324, 9, 60, 1);
		_G(flags)[V203] = 3;
		break;

	case 57:
		sendWSMessage_10000(1, _ripley, _rp01, 82, 71, 61, _rp01, 71, 71, 0);
		digi_play("608r03", 1);
		_G(flags)[V203] = 4;
		break;

	case 60:
		sendWSMessage_60000(_tt);
		sendWSMessage_10000(1, _ripley, _rp01, 62, 82, 57, _rp01, 82, 82, 0);
		break;

	case 61:
		series_unload(200);
		series_unload(201);
		series_unload(202);
		series_unload(203);
		series_unload(204);
		series_unload(211);
		series_unload(212);
		series_unload(213);
		series_unload(214);
		series_unload(_tt01);
		series_unload(_rp01);
		terminateMachineAndNull(_ripley);
		terminateMachineAndNull(_shadow);
		ws_unhide_walker();
		_G(player).disable_hyperwalk = false;
		player_set_commands_allowed(true);
		break;

	case 200:
		kernel_trigger_dispatchx(kernel_trigger_create(201));
		break;

	case 201:
		switch (_val2) {
		case 0:
			switch (_val3) {
			case 0:
				sendWSMessage_10000(1, _ol, _old1, 1, 1, 200, _old1, 1, 1, 0);
				break;

			case 1:
				if (imath_ranged_rand(10, 30) <= ++_val1) {
					_val1 = 0;

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _ol, _old1, 1, 1, 200, _old1, 1, 1, 0);
						break;
					} else {
						sendWSMessage_10000(1, _ol, _old1, 1, 3, 200, _old1, 3, 3, 0);
						_val2 = 2;
					}
				} else {
					kernel_timing_trigger(10, 200);
				}
				break;

			case 8:
			case 9:
			case 10:
			case 11:
				sendWSMessage_10000(1, _ol, _old2, 1, 38, 200, _old2, 38, 38, 0);
				_val2 = 8;
				break;

			case 13:
				player_update_info();
				ws_hide_walker();
				_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				sendWSMessage_10000(1, _ol, _old5, 1, 40, 250, _old5, 40, 40, 0);
				break;

			case 14:
				player_update_info();
				ws_hide_walker();
				_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				sendWSMessage_10000(1, _ol, _old5, 1, 40, 202, _old5, 40, 40, 0);
				break;

			case 15:
				player_update_info();
				ws_hide_walker();
				_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				sendWSMessage_10000(1, _ol, _old5, 1, 40, 280, _old5, 40, 40, 0);
				break;

			case 16:
				player_update_info();
				ws_hide_walker();
				_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				sendWSMessage_10000(1, _ol, _old5, 1, 40, 270, _old5, 40, 40, 0);
				break;

			default:
				sendWSMessage_10000(1, _ol, _old1, 1, 3, 200, _old1, 3, 3, 0);
				_val2 = 2;
				break;
			}
			break;

		case 2:
			switch (_val3) {
			case 1:
				if (imath_ranged_rand(10, 30) <= ++_val1) {
					_val1 = 0;

					switch (imath_ranged_rand(1, 3)) {
					case 1:
						sendWSMessage_10000(1, _ol, _old1, 3, 3, 200, _old1, 3, 3, 0);
						break;
					case 2:
						sendWSMessage_10000(1, _ol, _old1, 3, 1, 200, _old1, 1, 1, 0);
						sendWSMessage_190000(_ol, 7);
						break;
					case 3:
						sendWSMessage_10000(1, _ol, _old1, 4, 5, 200, _old1, 5, 5, 0);
						sendWSMessage_190000(_ol, 7);
						break;
					default:
						break;
					}
				} else {
					kernel_timing_trigger(10, 200);
				}
				break;

			case 5:
			case 6:
			case 7:
			case 12:
				sendWSMessage_10000(1, _ol, _old1, 4, 5, 200, _old1, 5, 5, 0);
				sendWSMessage_190000(_ol, 7);
				break;

			default:
				sendWSMessage_10000(1, _ol, _old1, 3, 1, 200, _old1, 1, 1, 0);
				sendWSMessage_190000(_ol, 7);
				break;
			}
			break;

		case 3:
			switch (_val3) {
			case 1:
				if (imath_ranged_rand(10, 30) <= ++_val1) {
					_val1 = 0;

					switch (imath_ranged_rand(1, 3)) {
					case 1:
						sendWSMessage_10000(1, _ol, _old1, 5, 5, 200, _old1, 5, 5, 0);
						break;
					case 2:
						sendWSMessage_10000(1, _ol, _old1, 5, 4, 200, _old1, 3, 3, 0);
						_val2 = 2;
						sendWSMessage_190000(_ol, 7);
						break;
					case 3:
						sendWSMessage_10000(1, _ol, _old1, 6, 7, 200, _old1, 7, 7, 0);
						sendWSMessage_190000(_ol, 7);
						break;
					default:
						break;
					}
				} else {
					kernel_timing_trigger(10, 200);
				}
				break;

			case 5:
			case 6:
			case 7:
			case 12:
				sendWSMessage_10000(1, _ol, _old1, 6, 7, 200, _old1, 7, 7, 0);
				sendWSMessage_190000(_ol, 7);
				break;

			default:
				sendWSMessage_10000(1, _ol, _old1, 5, 4, 200, _old1, 3, 3, 0);
				sendWSMessage_190000(_ol, 7);
				break;
			}
			break;

		case 4:
			switch (_val3) {
			case 1:
				if (imath_ranged_rand(10, 30) <= ++_val1) {
					_val1 = 0;

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _ol, _old1, 7, 7, 200, _old1, 7, 7, 0);
					} else {
						sendWSMessage_10000(1, _ol, _old1, 7, 6, 200, _old1, 5, 5, 0);
						sendWSMessage_190000(_ol, 7);
					}
				} else {
					kernel_timing_trigger(10, 200);
				}
				break;

			case 7:
			case 12:
				sendWSMessage_10000(1, _ol, _old1, 8, 14, 200, _old1, 26, 26, 0);
				_val2 = 6;
				break;

			default:
				sendWSMessage_10000(1, _ol, _old1, 7, 6, 200, _old1, 5, 5, 0);
				_val2 = 3;
				break;
			}
			break;

		case 6:
			switch (_val3) {
			case 5:
				sendWSMessage_10000(1, _ol, _old1, 26, 26, -1, _old1, 26, 42, 4);
				sendWSMessage_1a0000(_ol, 11);
				break;

			case 6:
				sendWSMessage_10000(1, _ol, _old1, 26, 26, 200, _old1, 26, 26, 0);
				break;

			case 7:
				sendWSMessage_10000(1, _ol, _old1, 61, 74, 200, _old1, 74, 74, 0);
				_val2 = 7;
				_val3 = 5;
				break;

			case 12:
				sendWSMessage_10000(1, _ol, _old1, 15, 28, 200, _old1, 26, 26, 0);
				_val3 = 5;
				break;

			default:
				sendWSMessage_10000(1, _ol, _old1, 14, 8, 200, _old1, 7, 7, 0);
				_val2 = 4;
				break;
			}
			break;

		case 7:
			switch (_val3) {
			case 7:
				sendWSMessage_10000(1, _ol, _old1, 74, 74, 200, _old1, 74, 74, 0);
				break;

			default:
				sendWSMessage_10000(1, _ol, _old1, 74, 61, 200, _old1, 26, 26, 0);
				_val2 = 6;
				break;
			}
			break;

		case 8:
			switch (_val3) {
			case 8:
				sendWSMessage_10000(1, _ol, _old2, 38, 38, 200, _old2, 38, 38, 0);
				break;

			case 9:
				sendWSMessage_10000(1, _ol, _old2, 39, 39, -1, _old2, 39, 48, 4);
				sendWSMessage_1a0000(_ol, 15);
				break;

			case 10:
			case 11:
				sendWSMessage_10000(1, _ol, _old2, 49, 57, 200, _old2, 57, 57, 0);
				_val2 = 11;
				break;

			default:
				sendWSMessage_10000(1, _ol, _old2, 38, 1, 200, _old1, 1, 1, 0);
				_val2 = 0;
				break;
			}
			break;

		case 11:
			switch (_val3) {
			case 10:
				sendWSMessage_10000(1, _ol, _old2, 57, 49, 200, _old2, 38, 38, 0);
				_val2 = 8;
				_val3 = 8;
				break;

			case 11:
				sendWSMessage_10000(1, _ol, _old2, 57, 57, 200, _old2, 57, 57, 0);
				break;

			default:
				sendWSMessage_10000(1, _ol, _old2, 57, 49, 200, _old2, 38, 38, 0);
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 202:
		sendWSMessage_10000(1, _ol, _old5a, 1, 8, 205, _old5a, 8, 8, 0);
		break;

	case 205:
		sendWSMessage_10000(1, _ol, _old5a, 9, 19, 208, _old5a, 19, 19, 0);
		inv_move_object("OBSIDIAN DISK", 608);
		break;

	case 208:
		sendWSMessage_10000(1, _ol, _old5a, 16, 19, -1, _old5a, 16, 19, 1);
		digi_play("608o06", 1, 255, 211);
		break;

	case 211:
		sendWSMessage_10000(1, _ol, _old5a, 20, 22, -1, _old5a, 20, 22, 1);
		digi_play("6084r43", 1, 255, 213);
		break;

	case 213:
		sendWSMessage_10000(1, _ol, _old5a, 16, 19, -1, _old5a, 16, 19, 1);
		digi_play("608o07", 1, 255, 216);
		break;

	case 216:
		sendWSMessage_10000(1, _ol, _old5a, 23, 54, -1, _old5a, 54, 54, 1);
		sendWSMessage_190000(_ol, 8);
		digi_play("608o08", 1, 255, 220);
		break;

	case 220:
		sendWSMessage_10000(1, _ol, _old5, 40, 1, 225, _old1, 1, 1, 0);
		break;

	case 225:
		sendWSMessage_10000(1, _ol, _old1, 1, 1, 200, _old1, 1, 1, 0);
		_val2 = 0;
		_val3 = 1;
		_ol2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip talker");
		sendWSMessage_10000(1, _ol2, _old5f, 1, 1, -1, _old5f, 1, 4, 1);
		sendWSMessage_1a0000(_ol2, 11);
		digi_play("608r44", 1, 255, 226);
		break;

	case 226:
	case 268:
		terminateMachineAndNull(_ol2);
		terminateMachineAndNull(_shadow);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 250:
		sendWSMessage_10000(1, _ol, _old5b, 1, 11, 254, _old5b, 11, 11, 0);
		break;

	case 254:
		sendWSMessage_10000(1, _ol, _old5b, 12, 49, -1, _old5b, 50, 53, 1);
		sendWSMessage_1a0000(_ol, 11);
		inv_move_object("CLOCK FACING", 608);
		digi_play("608o06", 1, 255, 256);
		break;

	case 256:
		digi_play("608r43", 1, 255, 260);
		break;

	case 260:
		digi_play("608o07", 1, 255, 263);
		break;

	case 263:
		digi_play("608o08", 1, 255, 265);
		break;

	case 265:
		sendWSMessage_10000(1, _ol, _old5, 40, 1, 267, _old1, 1, 1, 0);
		break;

	case 267:
		sendWSMessage_10000(1, _ol, _old1, 1, 1, 200, _old1, 1, 1, 0);
		_val2 = 0;
		_val3 = 1;
		_ol2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip talker");
		sendWSMessage_10000(1, _ol2, _old5f, 1, 1, -1, _old5f, 1, 4, 1);
		sendWSMessage_1a0000(_ol2, 11);
		digi_play("608r44", 1, 255, 268);
		break;

	case 270:
		sendWSMessage_10000(1, _ol, _old5d, 1, 11, 272, _old5d, 11, 11, 1);
		break;

	case 272:
		sendWSMessage_10000(1, _ol, _old5d, 11, 23, -1, _old5d, 24, 30, 4);
		sendWSMessage_1a0000(_ol, 11);
		inv_move_object("CLOCK FACING", 608);
		digi_play("608o09", 1, 255, 275);
		break;

	case 275:
		digi_play("608o09a", 1, 255, 277);
		break;

	case 277:
		sendWSMessage_10000(1, _ol, _old5d, 39, 45, 500, _old5d, 45, 45, 4);
		digi_play("608r46", 1);
		break;

	case 280:
		sendWSMessage_10000(1, _ol, _old5c, 1, 13, 282, _old5c, 13, 13, 1);
		break;

	case 282:
		sendWSMessage_10000(1, _ol, _old5c, 14, 27, -1, _old5c, 28, 32, 4);
		sendWSMessage_1a0000(_ol, 11);
		inv_move_object("OBSIDIAN DISK", 608);
		digi_play("608o09", 1, 255, 285);
		break;

	case 285:
		digi_play("608o09a", 1, 255, 287);
		break;

	case 287:
		sendWSMessage_10000(1, _ol, _old5c, 33, 47, -1, _old5c, 47, 47, 4);
		digi_play("608r46", 1, 255, 500);
		break;

	case 300:
		kernel_trigger_dispatchx(kernel_trigger_create(301));
		break;

	case 301:
		if (!_val5) {
			switch (_val4) {
			case 0:
				sendWSMessage_10000(1, _ripley, _rp09, 23, 23, 300, _rp09, 23, 23, 0);
				break;

			case 1:
				frame = imath_ranged_rand(23, 25);
				sendWSMessage_10000(1, _ripley, _rp09, frame, frame, 300, _rp09, frame, frame, 0);
				sendWSMessage_190000(_ripley, 13);
				sendWSMessage_1a0000(_ripley, 13);
				break;

			case 2:
				sendWSMessage_10000(1, _ripley, _rp09, 23, 1, 302, _rp09, 1, 1, 0);
				break;

			case 3:
				sendWSMessage_10000(1, _ripley, _rp09, 23, 1, 305, _rp09, 1, 1, 0);
				break;

			case 4:
				sendWSMessage_10000(1, _ripley, _rp09, 23, 1, 306, _rp09, 1, 1, 0);
				break;

			case 5:
				sendWSMessage_10000(1, _ripley, _rp09, 23, 1, 307, _rp09, 1, 1, 0);
				break;

			case 6:
				sendWSMessage_10000(1, _ripley, _rp09, 23, 1, 308, _rp09, 1, 1, 0);
				break;

			default:
				break;
			}
		}
		break;

	case 302:
		terminateMachineAndNull(_shadow);
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 305:
		terminateMachineAndNull(_shadow);
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		_val3 = 14;
		break;

	case 306:
		terminateMachineAndNull(_shadow);
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		_val3 = 13;
		break;

	case 307:
		terminateMachineAndNull(_shadow);
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		_val3 = 15;
		break;

	case 308:
		terminateMachineAndNull(_shadow);
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		_val3 = 16;
		break;

	case 500:
		terminateMachineAndNull(_ol);
		_ol = triggerMachineByHash_3000(8, 15, NORMAL_DIRS1, SHADOW_DIRS1,
			499, 318, 9, triggerMachineByHashCallback3000, "ow walker");
		sendWSMessage_10000(_ol, 487, 290, 11, -1, 1);
		kernel_timing_trigger(60, 501);
		ws_unhide_walker();
		break;

	case 501:
		ws_walk(465, 284, nullptr, 503, 1);
		break;

	case 503:
		sendWSMessage_60000(_ol);
		ws_hide_walker();
		ws_hide_walker(_ol);
		digi_preload("608o10");
		digi_preload("608r47");
		digi_preload("608o11");
		digi_preload("608r48");
		digi_preload("608o12");
		digi_preload("608o13");
		digi_preload("608r49");
		digi_preload("608o14");
		digi_preload("608_s04");
		digi_preload("608_s04a");
		digi_preload("608_s05");
		digi_preload("608_s05a");
		digi_preload("608o19");

		kernel_load_variant("608lock0");
		_ol2 = series_stream("608all0", 5, 0x600, 530);
		series_stream_break_on_frame(_ol2, 3, 504);
		break;

	case 504:
		series_stream_break_on_frame(_ol2, 49, 505);
		digi_play("608o10", 1, 255, 507);
		break;

	case 505:
		series_stream_check_series(_ol2, 240);
		series_stream_break_on_frame(_ol2, 50, 506);
		break;

	case 506:
		series_stream_check_series(_ol2, 7);
		series_stream_break_on_frame(_ol2, 53, 508);
		break;

	case 507:
		series_stream_check_series(_ol2, 13);
		digi_play("608r47", 1);
		break;

	case 508:
		series_stream_break_on_frame(_ol2, 137, 509);
		digi_play("608o11", 1);
		break;

	case 509:
		series_stream_break_on_frame(_ol2, 150, 559);
		digi_play("608_s05", 2);
		break;

	case 511:
		series_stream_break_on_frame(_ol2, 256, 512);
		digi_play("608_s04", 2);
		break;

	case 512:
		series_stream_break_on_frame(_ol2, 270, 513);
		digi_play("608_s04a", 2);
		break;

	case 513:
		series_stream_break_on_frame(_ol2, 274, 514);
		break;

	case 514:
		series_stream_check_series(_ol2, 300);
		kernel_timing_trigger(60, 555);
		series_stream_break_on_frame(_ol2, 275, 515);
		break;

	case 515:
		series_stream_check_series(_ol2, 7);
		series_stream_break_on_frame(_ol2, 290, 516);
		break;

	case 516:
		series_stream_break_on_frame(_ol2, 330, 558);
		digi_play("608_s05a", 2);
		break;

	case 517:
		series_stream_break_on_frame(_ol2, 367, 519);
		break;

	case 519:
		series_stream_break_on_frame(_ol2, 409, 521);
		break;

	case 521:
		series_stream_break_on_frame(_ol2, 429, 524);
		digi_play("608r49", 1);
		break;

	case 524:
		series_stream_break_on_frame(_ol2, 483, 525);
		digi_play("608o14", 1);
		break;

	case 525:
		series_stream_break_on_frame(_ol2, 487, 560);
		digi_play("608_s05", 2);
		break;

	case 527:
		digi_play("608_s05a", 2);
		break;

	case 530:
		digi_unload("608o10");
		digi_unload("608r47");
		digi_unload("608o11");
		digi_unload("608r48");
		digi_unload("608o12");
		digi_unload("608o13");
		digi_unload("608r49");
		digi_unload("608o14");
		digi_unload("608_s04");
		digi_unload("608_s04a");
		digi_unload("608_s05");
		digi_unload("608_s05a");
		digi_unload("608o19");
		hotspot_set_active("hole in hillside", true);

		_ripHandTalk3 = series_load("RIP TREK HAND TALK POS3");
		_end2 = series_show("608END2", 0xe00, 16);
		ws_unhide_walker();
		ws_demand_location(494, 276, 3);

		sendWSMessage_10000(_G(my_walker), 524, 296, 9, 532, 1);
		_ol = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
			triggerMachineByHashCallbackNegative, "ol");
		sendWSMessage_10000(1, _ol, _old8, 1, 1, -1, _old8, 1, 1, 1);
		break;

	case 532:
		sendWSMessage_10000(1, _ol, _old8, 1, 11, 534, _old8, 12, 17, 1);
		sendWSMessage_1a0000(_ol, 11);
		break;

	case 534:
		digi_play("608o15", 1, 255, 536);
		break;

	case 536:
		sendWSMessage_10000(1, _ol, _old8, 48, 46, 538, _old8, 47, 50, 1);
		sendWSMessage_1a0000(_ol, 11);
		break;

	case 538:
		digi_play("608o15a", 1, 255, 540);
		break;

	case 540:
		sendWSMessage_10000(1, _ol, _old8, 47, 47, -1, _old8, 47, 47, 1);
		setGlobals1(_ripHandTalk3, 1, 16, 16, 16);
		sendWSMessage_110000(546);
		digi_play("608r50", 1, 255, 544);
		break;

	case 543:
		sendWSMessage_10000(1, _ol, _old8, 26, 45, 545, _old8, 45, 45, 1);
		break;

	case 544:
		sendWSMessage_10000(1, _ol, _old8, 47, 50, -1, _old8, 47, 50, 1);
		digi_play("608o16", 1, 255, 543);
		break;

	case 545:
		sendWSMessage_10000(1, _ol, _old8, 51, 72, 548, _old8, 72, 72, 1);
		break;

	case 546:
		sendWSMessage_150000(-1);
		break;

	case 548:
		terminateMachineAndNull(_ol);
		_ol = triggerMachineByHash_3000(8, 15, NORMAL_DIRS1, SHADOW_DIRS1,
			487, 293, 9, triggerMachineByHashCallback3000, "ow walker");
		sendWSMessage_10000(_ol, 417, 295, 9, 549, 0);
		kernel_timing_trigger(300, 551);
		midi_play("eastiswo", 255, 1, -1, 949);
		_G(player).disable_hyperwalk = true;
		break;

	case 549:
		sendWSMessage_10000(_ol, 150, 330, 9, 550, 0);
		break;

	case 550:
		hotspot_set_active("old woman", false);
		sendWSMessage_10000(_ol, -30, 324, 9, 552, 1);
		player_set_commands_allowed(true);
		break;

	case 551:
		sendWSMessage_10000(_G(my_walker), 503, 296, 9, -1, 1);
		digi_play("608r51", 1);
		_G(flags)[V186] = 2;
		break;

	case 552:
		sendWSMessage_60000(_ol);
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 553);
		_G(player).disable_hyperwalk = false;
		break;

	case 553:
		midi_stop();
		break;

	case 555:
		digi_play("608r48", 1);
		break;

	case 558:
		series_stream_break_on_frame(_ol2, 343, 517);
		digi_play("608o13", 1);
		break;

	case 559:
		series_stream_break_on_frame(_ol2, 216, 511);
		digi_play("950_s37", 1);
		break;

	case 560:
		series_stream_break_on_frame(_ol2, 572, 570);
		digi_play("950_s37", 2);
		break;

	case 570:
		series_stream_break_on_frame(_ol2, 639, 527);
		digi_play("608o19", 1);
		break;

	case 698:
		kernel_timing_trigger(60, 699);
		break;

	case 699:
		ws_walk(43, 345, nullptr, -1, 3);
		break;

	case 700:
		player_update_info(_tt, &_G(player_info));
		ws_hide_walker();
		_ttShadow = series_show("tt walker shadow 3", 0xf00, 0, -1, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_ttTalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "tt walker");
		sendWSMessage_10000(1, _ttTalker, _tt02, 1, 11, 703, _tt02, 11, 11, 0);
		break;

	case 703:
		sendWSMessage_10000(1, _ttTalker, _tt02, 12, 12, -1, _tt02, 12, 34, 4);
		sendWSMessage_1a0000(_ttTalker, 13);
		digi_play("608t04", 1, 255, 704);
		break;

	case 704:
		sendWSMessage_10000(1, _ttTalker, _tt02, 12, 12, -1, _tt02, 12, 12, 4);
		kernel_timing_trigger(60, 706);
		break;

	case 706:
		sendWSMessage_10000(1, _ttTalker, _tt02, 35, 35, -1, _tt02, 35, 51, 4);
		sendWSMessage_1a0000(_ttTalker, 13);
		digi_play("608t05", 1, 255, 708);
		break;

	case 708:
		sendWSMessage_10000(1, _ttTalker, _tt02, 51, 51, -1, _tt02, 51, 51, 0);
		kernel_timing_trigger(20, 709);
		break;

	case 709:
		setGlobals1(_ripHandChin, 1, 14, 14, 14);
		sendWSMessage_110000(713);
		digi_play("608r53", 1);
		break;

	case 713:
		sendWSMessage_10000(1, _ttTalker, _tt02, 55, 64, 714, _tt02, 64, 64, 0);
		digi_play("608r53a", 1);
		kernel_timing_trigger(60, 717);
		break;

	case 714:
		terminateMachineAndNull(_ttTalker);
		ws_unhide_walker(_tt);
		terminateMachineAndNull(_ttShadow);
		sendWSMessage_10000(_tt, 238, 322, 3, 715, 0);
		break;

	case 715:
		sendWSMessage_10000(_tt, 459, 277, 3, -1, 1);
		break;

	case 717:
		sendWSMessage_140000(718);
		break;

	case 718:
		sendWSMessage_10000(_G(my_walker), 248, 328, 3, 719, 0);
		break;

	case 719:
		sendWSMessage_10000(_G(my_walker), 466, 292, 3, 721, 1);
		break;

	case 721:
		ws_hide_walker(_tt);
		series_unload(200);
		series_unload(201);
		series_unload(203);
		series_unload(204);

		_ttTalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xd00, 0,
			triggerMachineByHashCallbackNegative, "tt walker");
		sendWSMessage_10000(1, _ttTalker, _tt03, 1, 60, -1, _tt03, 60, 60, 0);
		digi_play("608t06", 1, 255, 726);
		break;

	case 726:
		sendWSMessage_10000(_G(my_walker), 495, 277, 9, 729, 1);
		digi_play("608r54", 1);
		break;

	case 729:
		terminateMachineAndNull(_ttTalker);
		ws_hide_walker();
		digi_preload("950_s37");
		digi_preload("608r55");
		digi_preload("604_s01");
		digi_preload("608t08");
		digi_preload("608_s11");
		digi_preload("608_s05");
		digi_preload("608_s05a");
		digi_preload("608_s09");
		digi_preload("608_s10");
		digi_preload("608_s07");
		digi_preload("608t07");
		digi_preload("608_s06");
		digi_preload("608r56");

		_ol2 = series_stream("608all5", 5, 0x600, 750);
		series_stream_break_on_frame(_ol2, 21, 783);
		series_stream_check_series(_ol2, 10);
		break;

	case 731:
		series_stream_break_on_frame(_ol2, 45, 787);
		digi_play("608r55", 1);
		break;

	case 733:
		series_stream_break_on_frame(_ol2, 64, 734);
		digi_play("608t08", 1);
		break;

	case 734:
		series_stream_check_series(_ol2, 7);
		series_stream_break_on_frame(_ol2, 107, 792);
		break;

	case 735:
		series_stream_break_on_frame(_ol2, 177, 782);
		break;

	case 736:
		series_stream_break_on_frame(_ol2, 193, 739);
		digi_play("608_s06", 2);
		break;

	case 739:
		series_stream_break_on_frame(_ol2, 290, 747);
		digi_play("608r56", 2);
		break;

	case 747:
		ws_unhide_walker();
		ws_demand_location(461, 293, 3);
		sendWSMessage_10000(_G(my_walker), 432, 297, 3, 738, 1);
		break;

	case 750:
		_lighter = series_show("608LITER", 0xf00, 16);
		hotspot_set_active("LIGHTER", true);
		ws_hide_walker();

		digi_unload("950_s37");
		digi_unload("608r55");
		digi_unload("604_s01");
		digi_unload("608t08");
		digi_unload("608_s11");
		digi_unload("608_s05");
		digi_unload("608_s05a");
		digi_unload("608_s09");
		digi_unload("608_s10");
		digi_unload("608_s07");
		digi_unload("608t07");
		digi_unload("608_s06");
		digi_unload("608r56");

		series_unload(_tt02);
		series_unload(_tt03);
		series_unload(_ripHandChin);
		series_unload(_ripTalker);

		series_load(SECTION6_NORMAL_NAMES[0], SECTION6_NORMAL_DIRS[0]);
		series_load(SECTION6_NORMAL_NAMES[1], SECTION6_NORMAL_DIRS[1]);
		series_load(SECTION6_NORMAL_NAMES[3], SECTION6_NORMAL_DIRS[3]);
		series_load(SECTION6_NORMAL_NAMES[4], SECTION6_NORMAL_DIRS[4]);
		ws_hide_walker(_tt);

		_ttTalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "tt walker");
		sendWSMessage_10000(1, _ttTalker, _all5a, 1, 1, -1, _all5a, 1, 6, 4);
		digi_play("608r58", 2, 255, 752);
		break;

	case 752:
		sendWSMessage_10000(1, _ttTalker, _all5a, 7, 7, -1, _all5a, 7, 12, 4);
		digi_play("608t10", 2, 255, 755);
		break;

	case 755:
		sendWSMessage_10000(1, _ttTalker, _all5a, 1, 1, -1, _all5a, 1, 6, 4);
		digi_play("608r59", 2, 255, 758);
		break;

	case 758:
		sendWSMessage_10000(1, _ttTalker, _all5a, 760, 7, -1, _all5a, 1, 12, 4);
		digi_play("608t11", 2, 255, 760);
		break;

	case 760:
		sendWSMessage_10000(1, _ttTalker, _all5a, 13, 50, 763, _all5a, 50, 50, 4);
		break;

	case 763:
		inv_give_to_player("STICK AND SHELL MAP");
		inv_give_to_player("WHALE BONE HORN");
		kernel_examine_inventory_object("PING STICK AND SHELL MAP",
			5, 1, 350, 224, 764);
		break;

	case 764:
		kernel_timing_trigger(1, 765);
		break;

	case 765:
		kernel_examine_inventory_object("PING WHALE BONE HORN",
			5, 1, 350, 224, 766);
		break;

	case 766:
		terminateMachineAndNull(_ttTalker);
		_ttTalker = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "tt talker");
		sendWSMessage_10000(1, _ttTalker, _loop0, 1, 1, -1, _loop0, 1, 6, 4);

		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallbackNegative, "rip talker");
		sendWSMessage_10000(1, _ripley, _loop1, 1, 1, -1, _loop1, 1, 1, 0);
		digi_play("608t11a", 2, 255, 767);
		break;

	case 767:
		sendWSMessage_10000(1, _ttTalker, _loop0, 1, 1, -1, _loop0, 1, 6, 4);
		digi_play("608t12", 2, 255, 768);
		break;

	case 768:
		sendWSMessage_10000(1, _ttTalker, _loop0, 1, 1, -1, _loop0, 1, 1, 0);
		sendWSMessage_10000(1, _ripley, _loop1, 1, 1, -1, _loop1, 1, 11, 0);
		digi_play("608r60", 2, 255, 769);
		break;

	case 769:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		sendWSMessage_10000(1, _ttTalker, _tt05, 1, 23, 770, _tt05, 23, 23, 1);
		digi_play("608t13", 2, 255, 771);
		break;

	case 770:
		sendWSMessage_10000(1, _ttTalker, _all5a, 23, 1, 773, _all5a, 1, 1, 1);
		break;

	case 771:
		midi_play("thelook", 255, 0, -1, 949);
		break;

	case 773:
		terminateMachineAndNull(_ttTalker);
		ws_unhide_walker(_tt);
		ws_demand_location(_tt, 480, 294, 9);
		sendWSMessage_10000(_tt, 415, 287, 9, 775, 0);
		break;

	case 775:
		sendWSMessage_10000(_tt, 239, 325, 9, 778, 0);
		_G(player).disable_hyperwalk = true;
		player_set_commands_allowed(true);
		_G(flags)[V203] = 7;
		break;

	case 778:
		sendWSMessage_10000(_tt, -30, 325, 9, 779, 1);
		break;

	case 779:
		_G(player).disable_hyperwalk = false;
		break;

	case 780:
		digi_play("608_s09", 2);
		series_stream_break_on_frame(_ol2, 160, 781);
		break;

	case 781:
		digi_play("608_s10", 2);
		series_stream_break_on_frame(_ol2, 171, 735);
		break;

	case 782:
		series_stream_break_on_frame(_ol2, 184, 736);
		digi_play("608_s07", 1);
		break;

	case 783:
		series_stream_break_on_frame(_ol2, 34, 731);
		digi_play("950_s37", 2);
		break;

	case 784:
		series_stream_break_on_frame(_ol2, 135, 785);
		digi_play("608_s05", 2);
		break;

	case 785:
		series_stream_break_on_frame(_ol2, 154, 780);
		digi_play("608_s05a", 2);
		break;

	case 787:
		series_stream_break_on_frame(_ol2, 53, 733);
		digi_play("604_s01", 2);
		break;

	case 792:
		series_stream_break_on_frame(_ol2, 120, 784);
		digi_play("608_s11", 2);
		break;

	default:
		break;
	}
}

void Room608::pre_parser() {
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("HORN/PULL CORD", "WATER") && !_G(flags)[GLB_TEMP_3])
		_G(player).resetWalk();
	if (player_said("POLE", "DRIFTWOOD STUMP ") && inv_player_has("POLE"))
		_G(player).resetWalk();
	if (useFlag && HERE("POLE"))
		_G(player).resetWalk();
	if (player_said("DRIFTWOOD STUMP", "LARGE HOLE") && inv_player_has("DRIFTWOOD STUMP"))
		_G(player).resetWalk();
	if (takeFlag && HAS("POLE"))
		_G(player).resetWalk();
}

void Room608::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv608a")) {
		conv608a();
	} else if (_G(kernel).trigger == 747) {
		_val3 = 1;
		_val4 = 2;
	} else if (talkFlag && player_said("old woman")) {
		if (_G(flags)[GLB_TEMP_7]) {
			digi_play(_G(flags)[V203] > 2 ? "608r04" : "608r35", 1);
		} else {
			if (_G(flags)[V203] >= 2 || !_G(flags)[V034]) {
				player_set_commands_allowed(false);
				_G(flags)[V034] = 1;

				player_update_info();
				_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
					_G(player_info).scale, _G(player_info).x, _G(player_info).y);
				ws_hide_walker();
				_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
					triggerMachineByHashCallbackNegative, "rip");

				_G(kernel).trigger_mode = KT_DAEMON;
				sendWSMessage_10000(1, _ripley, _rp09, 1, 23, 300, _rp09, 23, 23, 0);
				_G(kernel).trigger_mode = KT_PARSE;

				_val5 = 0;
				_val4 = 0;
				_val3 = 6;
				conv_load("conv608a", 10, 10, 747);
				conv_export_value_curr(_G(flags)[V203] >= 3 ? 1 : 0, 0);
				conv_play();
			} else {
				digi_play("608r35", 1);
			}
		}
	} else if (takeFlag && player_said("DRIFTWOOD STUMP") && takeStump1()) {
		// No implementation
	} else if (takeFlag && player_said("DRIFTWOOD PUFFIN") && takePuffin()) {
		// No implementation
	} else if (player_said("DRIFTWOOD STUMP", "LARGE HOLE") && stumpHole()) {
		// No implementation
	} else if (takeFlag && player_said("DRIFTWOOD STUMP ") && takeStump2()) {
		// No implementation
	} else if (takeFlag && player_said("LIGHTER") && takeLighter()) {
		// No implementation
	} else if (player_said("HORN/PULL CORD/WATER", "WATER")) {
		digi_play("608r72", 1);
	} else if (player_said("DRIFTWOOD STUMP") && player_said_any("STATUE", "STATUE ")) {
		digi_play("com019", 1);
	} else if (player_said("HORN/PULL CORD", "WATER") && hornCordWater()) {
		// No implementation
	} else if (player_said("POLE", "DRIFTWOOD STUMP ") && inv_player_has("POLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(453, 311, nullptr, 1, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			kernel_load_variant("608lock2");
			digi_preload("608_s03");

			_shadow5 = series_show("safari shadow 1", _G(player_info).depth,
				16, -1, -1, 0, _G(player_info).scale,
				_G(player_info).x, _G(player_info).y);
			_horn = series_load("608rp03");
			_ol2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallbackNegative, "ol");
			sendWSMessage_10000(1, _ol2, _horn, 1, 24, 2, _horn, 24, 24, 0);
			break;
		case 2:
			sendWSMessage_10000(1, _ol2, _horn, 25, 39, 3, _horn, 39, 39, 0);
			digi_play("608_s03", 2);
			break;
		case 3:
			digi_stop(2);
			digi_unload("608_s03");
			terminateMachineAndNull(_ol2);
			series_unload(_horn);
			terminateMachineAndNull(_shadow5);

			_pole = series_show("608POLE", 0x400, 16);
			hotspot_set_active("POLE", true);
			ws_unhide_walker();
			inv_move_object("POLE", 608);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("POLE") && !inv_player_has("POLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(453, 311, nullptr, 1, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			kernel_load_variant("608lock1");
			terminateMachineAndNull(_pole);

			_shadow5 = series_show("safari shadow 1", _G(player_info).depth,
				16, -1, -1, 0, _G(player_info).scale,
				_G(player_info).x, _G(player_info).y);
			_horn = series_load("608rp03");
			_ol2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallbackNegative, "ol");
			sendWSMessage_10000(1, _ol2, _horn, 39, 1, 2, _horn, 1, 1, 0);
			break;
		case 2:
			terminateMachineAndNull(_ol2);
			series_unload(_horn);
			terminateMachineAndNull(_shadow5);
			hotspot_set_active("POLE", false);
			ws_unhide_walker();
			inv_give_to_player("POLE");
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && HERE("POLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(453, 311, nullptr, 1, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			kernel_load_variant("608lock1");
			digi_preload("950_s37");
			digi_preload("950_s37a");

			_ol2 = series_stream("608rp04", 5, 0x300, 16);
			series_stream_break_on_frame(_ol2, 25, 3);
			break;
		case 3:
			series_stream_break_on_frame(_ol2, 33, 30);
			digi_play("950_s37", 2);
			break;
		case 4:
			series_stream_break_on_frame(_ol2, 76, 5);
			digi_play("950_s37a", 2);
			break;
		case 5:
			series_stream_break_on_frame(_ol2, 77, 7);
			digi_play("950_s37", 2);
			break;
		case 7:
			terminateMachineAndNull(_end1);
			terminateMachineAndNull(_pole);
			terminateMachineAndNull(_stump);
			_val3 = 8;
			digi_play("608_s01a", 1);
			break;
		case 10:
			digi_unload("950_s37");
			digi_unload("950_s37a");
			_G(flags)[V186] = 1;
			_end1 = series_show("608END", 0xf00, 16);

			hotspot_set_active("STATUE", false);
			hotspot_set_active("STATUE ", true);
			hotspot_set_active("DRIFTWOOD STUMP ", false);
			hotspot_set_active("DRIFTWOOD STUMP  ", true);
			hotspot_set_active("POLE", false);
			hotspot_set_active("POLE ", true);
			hotspot_set_active("stone", false);

			ws_unhide_walker();
			player_update_info();
			ws_walk(_G(player_info).x + 1, _G(player_info).y,
				nullptr, 12, 4);
			break;
		case 12:
			_ripLHandTalk = series_load("RIP TREK L HAND TALK POS4");
			setGlobals1(_ripLHandTalk, 2, 6, 6, 7, 1, 6, 1, 1, 1);
			digi_play("608r15", 1, 255, 15);
			break;
		case 15:
			sendWSMessage_120000(2);
			_val3 = 10;
			digi_play("608o01", 1, 255, 18);
			break;
		case 18:
			sendWSMessage_110000(2);
			digi_play("608r16", 1, 255, 20);
			break;
		case 20:
			sendWSMessage_140000(-1);
			_val3 = 9;
			digi_play("608o02", 1, 255, 22);
			break;
		case 22:
			_val3 = 1;
			kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
			kernel_timing_trigger(100, 25);
			break;
		case 25:
			player_set_commands_allowed(true);
			digi_play("608r17", 1);
			break;
		case 30:
			series_stream_break_on_frame(_ol2, 44, 4);
			digi_play("608_s01", 1);
			break;
		default:
			break;
		}
	} else if (player_said_any("lung", "prostate")) {
		switch (_G(kernel).trigger) {
		case 1:
			player_set_commands_allowed(false);
			player_update_info();
			_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);

			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallbackNegative, "rip");

			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _ripley, _rp09, 1, 23, 300, _rp09, 23, 23, 0);
			_G(kernel).trigger_mode = KT_PARSE;

			_val5 = 0;
			_val4 = 1;
			_val3 = 6;
			digi_play("608r70", 1, 255, 2);
			break;
		case 2:
			_val3 = 5;
			_val4 = 0;
			digi_play("608o18", 1, 255, 3);
			break;
		case 3:
			_val3 = 6;
			kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
			_val4 = 1;
			digi_play("608r71", 1, 255, 5);
			break;
		case 5:
			_val4 = player_said("lung") ? 3 : 4;
			break;
		default:
			break;
		}
	} else if (player_said("obsidian disk", "old woman") &&
			!inv_object_is_here("CLOCK FACING")) {
		if (_G(flags)[V186]) {
			Common::strcpy_s(_G(player).verb, "lung");
			kernel_timing_trigger(1, 1);
		} else {
			digi_play(_G(flags)[GLB_TEMP_4] ? "608r04a" : "608r35a", 1);
		}
	} else if (player_said("clock facing", "old woman") &&
			!inv_object_is_here("OBSIDIAN DISK")) {
		if (_G(flags)[V186]) {
			Common::strcpy_s(_G(player).verb, "prostate");
			kernel_timing_trigger(1, 1);
		} else {
			digi_play(_G(flags)[GLB_TEMP_4] ? "608r04a" : "608r35a", 1);
		}
	} else if (player_said_any("bowels", "scrotum")) {
		switch (_G(kernel).trigger) {
		case 1:
			player_set_commands_allowed(false);
			player_update_info();
			_shadow = series_show("safari shadow 3", 0xf00, 0, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);

			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallbackNegative, "rip");

			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _ripley, _rp09, 1, 23, 300, _rp09, 23, 23, 0);
			_G(kernel).trigger_mode = KT_PARSE;

			_val5 = 0;
			_val4 = 1;
			_val3 = 6;
			digi_play("608r67", 1, 255, 2);
			break;
		case 2:
			_val3 = 5;
			_val4 = 0;
			digi_play("608o17", 1, 255, 3);
			break;
		case 3:
			_val3 = 6;
			kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
			_val4 = 1;
			digi_play("608r68", 1, 255, 5);
			break;
		case 5:
			_val4 = player_said("bowels") ? 5 : 6;
			break;
		default:
			break;
		}
	} else if (player_said("obsidian disk", "old woman") &&
			inv_object_is_here("CLOCK FACING")) {
		Common::strcpy_s(_G(player).verb, "bowels");
		kernel_timing_trigger(1, 1);
	} else if (player_said("clock facing", "old woman") &&
			inv_object_is_here("OBSIDIAN DISK")) {
		Common::strcpy_s(_G(player).verb, "bowels");
		kernel_timing_trigger(1, 1);
	} else if (lookFlag && player_said("statue")) {
		digi_play(_G(flags)[V203] > 1 ? "608r05" : "608r36", 1);
	} else if (lookFlag && player_said("statue ")) {
		digi_play(_G(flags)[V203] == 1 ? "608r38" : "608r52", 1);
	} else if (lookFlag && player_said("stone")) {
		digi_play(_G(flags)[V186] ? "608r39" : "608r06", 1);
	} else if (lookFlag && player_said("base")) {
		digi_play("608r07", 1);
	} else if (lookFlag && player_said("small hole")) {
		digi_play("608r09", 1);
	} else if (lookFlag && player_said("lava")) {
		digi_play("608r08", 1);
	} else if (lookFlag && player_said("crevice")) {
		digi_play("608r09", 1);
	} else if (lookFlag && player_said("large hole")) {
		digi_play(inv_object_is_here("DRIFTWOOD STUMP") ?
			"608r40" : "608r10", 1);
	} else if (lookFlag && player_said(" ")) {
		digi_play("608r11", 1);
	} else if (lookFlag && HERE("DRIFTWOOD PUFFIN") && lookPuffin()) {
		// No implementation
	} else if (player_said("left")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;
		case 1:
			digi_stop(1);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s28b", 3, 90);
			_G(game).setRoom(605);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("DRIFTWOOD STUMP") &&
			inv_object_in_scene("DRIFTWOOD STUMP", 600)) {
		digi_play("608r13", 1);
	} else if (lookFlag && player_said("DRIFTWOOD STUMP ")) {
		digi_play("608r76", 1);
	} else if (lookFlag && player_said("DRIFTWOOD STUMP  ")) {
		digi_play("608r41", 1);
	} else if (lookFlag && player_said("water")) {
		digi_play("608r73", 1);
	} else if (player_said("POLE", "STATUE") ||
			player_said("POLE", "STATUE ")) {
		digi_play("608r77", 1);
	} else if (lookFlag && player_said("old woman")) {
		if (_G(flags)[V203] > 2) {
			if (++_G(flags)[V181] > 3)
				_G(flags)[V181] = 3;

			switch (_G(flags)[V181]) {
			case 1:
				digi_play("608r14", 1);
				break;
			case 2:
				digi_play("608r14a", 1);
				break;
			case 3:
				digi_play("608r14b", 1);
				break;
			default:
				break;
			}
		} else {
			digi_play("608r37", 1);
		}
	} else if (lookFlag && HERE("LIGHTER")) {
		digi_play("608r66", 1);
	} else if (lookFlag && player_said("hole in hillside")) {
		digi_play("608r51a", 1);
	} else if (player_said("WHALE BONE HORN", "WATER")) {
		digi_play("608r64", 1);
	} else if (player_said("OBSIDIAN DISK", "STATUE") ||
			player_said("OBSIDIAN DISK", "STATUE ") ||
			player_said("CLOCK FACING", "STATUE") ||
			player_said("CLOCK FACING", "STATUE ")) {
		digi_play("com146", 1, 255, -1, 997);
	} else if (takeFlag && player_said("pole ")) {
		digi_play("608r18", 1);
	} else if (takeFlag && player_said("stone")) {
		digi_play("608r63", 1);
	} else if (lookFlag && HERE("pole")) {
		digi_play("608r75", 1);
	} else if (lookFlag && HERE("pole ")) {
		digi_play("608r42", 1);
	} else if (takeFlag && player_said("driftwood stump  ")) {
		digi_play("608r19", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room608::conv608a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_val3 = 6;
			kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
		} else if (who == 1) {
			_val4 = 0;
		}

		conv_resume();

	} else if (sound) {
		if (who <= 0) {
			if (node == 4 && entry == 0)
				midi_play("eastiswo", 255, 0, -1, 949);
			if ((node == 2 && entry == 1) || (node == 3 && entry == 2))
				_val3 = 7;
			else if ((node == 2 && entry == 0) || (node == 3 && entry == 1))
				_val3 = 12;
			else
				_val3 = 5;
		} else if (who == 1) {
			if (node == 4)
				_G(flags)[GLB_TEMP_7] = 1;
			if (!(node == 5 && entry == 3))
				_val4 = 1;
		}

		digi_play(sound, 1, 255, 1);
	} else {
		conv_resume();
	}
}

bool Room608::takeStump1() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_in_scene("DRIFTWOOD STUMP", 600)) {
			player_set_commands_allowed(false);
			_ripLowReach = series_load("RIP LOW REACH POS1");
			setGlobals1(_ripLowReach, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			return true;
		}
		break;

	case 2:
		hotspot_set_active("DRIFTWOOD STUMP", false);
		inv_give_to_player("DRIFTWOOD STUMP");
		kernel_examine_inventory_object("PING DRIFTWOOD STUMP",
			5, 1, 230, 244, 3);
		return true;

	case 3:
		sendWSMessage_140000(5);
		return true;

	default:
		break;
	}

	return false;
}

bool Room608::takePuffin() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("DRIFTWOOD PUFFIN")) {
			player_set_commands_allowed(false);
			_ripLowReach2 = series_load("RIP LOW REACH POS2");
			setGlobals1(_ripLowReach2, 1, 16, 16, 16);
			sendWSMessage_110000(2);
			return true;
		}
		break;

	case 2:
		hotspot_set_active("DRIFTWOOD PUFFIN", false);
		inv_give_to_player("DRIFTWOOD PUFFIN");
		kernel_examine_inventory_object("PING DRIFTWOOD PUFFIN",
			5, 1, 142, 239, 3);
		terminateMachineAndNull(_puffin);
		return true;

	case 3:
		sendWSMessage_140000(5);
		return true;

	case 5:
		series_unload(_ripLowReach2);
		break;

	default:
		break;
	}

	return false;
}

bool Room608::stumpHole() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_player_has("DRIFTWOOD STUMP")) {
			ws_walk(474, 309, nullptr, 1, 2);
			return true;
		}
		break;

	case 1:
		player_set_commands_allowed(false);
		_ripLowReach = series_load("RIP LOW REACH POS1");
		setGlobals1(_ripLowReach, 1, 10, 10, 10);
		sendWSMessage_110000(2);
		return true;

	case 2:
		hotspot_set_active("DRIFTWOOD STUMP ", true);
		inv_move_object("DRIFTWOOD STUMP", 608);
		_stump = series_show("608ST_UP", 0x400, 16);
		sendWSMessage_140000(5);
		digi_play("608_s02", 2);
		return true;

	case 5:
		series_unload(_ripLowReach);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room608::takeStump2() {
	if (inv_object_is_here("POLE")) {
		digi_play("608r74", 1);
		return true;
	} else {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(474, 309, nullptr, 1, 2);
			return true;

		case 1:
			player_set_commands_allowed(false);
			_ripLowReach = series_load("RIP LOW REACH POS1");
			setGlobals1(_ripLowReach, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			return true;

		case 2:
			hotspot_set_active("DRIFTWOOD STUMP ", false);
			terminateMachineAndNull(_stump);
			inv_give_to_player("DRIFTWOOD STUMP");
			sendWSMessage_140000(5);
			return true;

		case 5:
			series_unload(_ripLowReach);
			player_set_commands_allowed(true);
			return true;

		default:
			break;
		}

		return false;
	}
}

bool Room608::takeLighter() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("LIGHTER")) {
			player_set_commands_allowed(false);
			_ripLowReach = series_load("RIP LOW REACH POS1");
			return true;
		}
		break;

	case 2:
		hotspot_set_active("LIGHTER", false);
		inv_give_to_player("LIGHTER");
		kernel_examine_inventory_object("PING LIGHTER", 5, 1, 455, 250, 3);
		terminateMachineAndNull(_lighter);
		return true;

	case 3:
		sendWSMessage_140000(5);
		return true;

	case 5:
		series_unload(_ripLowReach);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room608::hornCordWater() {
	if (_G(flags)[V193]) {
		digi_play("com118", 1);
		return true;
	} else {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(64, 354, nullptr, 1, 7);
			return true;

		case 1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			digi_preload("608_s08");

			_shadow5 = series_show("safari shadow 5", _G(player_info).depth, 144, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info.y));
			_ol2 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallbackNegative, "ol");
			sendWSMessage_10000(1, _ol2, _horn, 1, 16, 2, _horn, 16, 16, 0);
			return true;

		case 2:
			sendWSMessage_10000(1, _ol2, _horn, 17, 37, 3, _horn, 37, 37, 0);
			digi_play("608_s08", 2);
			return true;

		case 3:
			inv_give_to_player("HORN/PULL CORD/WATER");
			inv_move_object("HORN/PULL CORD", NOWHERE);
			kernel_examine_inventory_object("PING HORN/PULL CORD/WATER",
				5, 1, 42, 239, 4);
			return true;

		case 4:
			digi_unload("608_s08");
			terminateMachineAndNull(_ol2);
			series_unload(_horn);
			terminateMachineAndNull(_shadow5);
			ws_unhide_walker();
			_G(flags)[GLB_TEMP_3] = 1;
			player_set_commands_allowed(true);
			digi_play("608r65", 1);
			return true;

		default:
			break;
		}

		return false;
	}
}

bool Room608::lookPuffin() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_G(flags)[V187]) {
			digi_play("608r12", 1);
		} else {
			player_set_commands_allowed(false);
			digi_play("608r12", 1, 255, 1);
		}
		return true;

	case 1:
		_G(flags)[V187] = 1;
		player_set_commands_allowed(true);
		digi_play("608r12a", 1);
		return true;

	default:
		break;
	}

	return false;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
