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

#include "m4/riddle/rooms/section6/room605.h"
#include "m4/riddle/rooms/section6/section6.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room605::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_trigger1 = -1;
		_walkerHidden = false;
		_val3 = 0;
		_val4 = 2;
		_val5 = 2;
	}

	if (inv_object_is_here("OBSIDIAN DISK")) {
		_pupil = series_show("605eye", 0x600, 16);
	} else {
		hotspot_set_active("PUPIL", false);
		hotspot_set_active("OBSIDIAN DISK", false);
	}

	if (_G(flags)[V203] == 2) {
		kernel_load_variant("605lock1");
		_ripHandsHips = series_load("RIP TREK HANDS HIPS POS2");
		_ripHandOnChin = series_load("RIP TREK HAND ON CHIN POS2");
		_ripTwoHandTalk = series_load("RIP TREK TWO HAND TALK POS2");
		_ripTalk = series_load("RIP TREK TALK");
		_605tt = series_load("605 TT");

	} else {
		hotspot_set_active("samantha", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[V203] == 2) {
			_G(player).disable_hyperwalk = true;
			ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
			ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x700, 0,
				triggerMachineByHashCallback, "tt");
			sendWSMessage_10000(1, _tt, _605tt, 1, 1, 200, _605tt, 1, 1, 0);
			_ttShadow = series_show("tt walker shadow 5", 0x701, 16, -1, -1, 0, 68, 476, 290);
		}

		digi_preload("950_s28");
		break;

	case 603:
	case 610:
		digi_preload("950_s28");
		digi_stop(3);
		digi_unload("950_s28a");

		if (_G(flags)[V203] == 2) {
			player_set_commands_allowed(false);
			_G(player).disable_hyperwalk = true;
			ws_demand_location(451, 368, 1);
			ws_walk(513, 316, nullptr, -1, 10);

			ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
			ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);
			_tt = triggerMachineByHash_3000(8, 9, *SECTION6_NORMAL_DIRS, *SECTION6_SHADOW_DIRS,
				450, 339, 1, triggerMachineByHashCallback3000, "tt walker");
			sendWSMessage_10000(_tt, 476, 290, 5, 10, 1);

		} else {
			ws_demand_location(451, 353, 10);
		}
		break;

	case 608:
		digi_preload("950_s28");
		digi_stop(3);
		digi_unload("950_s28b");
		ws_demand_location(534, 219, 7);
		ws_walk(520, 263, nullptr, -1, 7);
		break;

	case 623:
		ws_demand_location(-30, 272, 3);
		ws_walk(34, 272, nullptr, 1, 3);
		break;

	default:
		digi_preload("950_s28");
		ws_demand_location(451, 353, 10);
		break;
	}

	digi_play_loop("950_s28", 3, 90);
}

void Room605::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 10:
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x700, 0,
			triggerMachineByHashCallback, "tt");
		sendWSMessage_10000(1, _tt, _605tt, 222, 234, 11, _605tt, 1, 1, 0);
		_ttShadow = series_show("tt walker shadow 5", 0x701, 16, -1,
			-1, 0, 68, 476, 290);
		break;

	case 11:
		kernel_timing_trigger(1, 200);
		_val7 = 0;
		_val6 = 1;
		digi_play("605t01", 1, 255, 12);
		break;

	case 12:
		setGlobals1(_ripHandOnChin, 1, 16, 16, 16);
		sendWSMessage_110000(-1);
		digi_play("605r01", 1);
		break;

	case 15:
		_val6 = 3;
		digi_play("605t02", 1, 255, 17);
		break;

	case 17:
		_val6 = 0;
		kernel_timing_trigger(1, 200);
		sendWSMessage_140000(18);
		break;

	case 18:
		_G(player).disable_hyperwalk = false;
		player_set_commands_allowed(true);
		break;

	case 200:
		if (!_val7 && !_val6 && _trigger1 != -1) {
			kernel_trigger_dispatchx(_trigger1);
			_trigger1 = -1;

			if (_walkerHidden) {
				ws_unhide_walker();
				_walkerHidden = false;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(201));
		break;

	case 201:
		switch (_val7) {
		case 0:
			switch (_val6) {
			case 0:
				sendWSMessage_10000(1, _tt, _605tt, 1, 1, 200, _605tt, 1, 1, 0);
				_val6 = 0;
				break;

			case 1:
				sendWSMessage_10000(1, _tt, _605tt, 2, 12, 202, _605tt, 12, 12, 0);
				sendWSMessage_190000(_tt, 7);
				_val6 = 0;
				break;

			case 3:
				sendWSMessage_10000(1, _tt, _605tt, 13, 13, -1, _605tt, 13, 30, 4);
				break;

			case 5:
				sendWSMessage_10000(1, _tt, _605tt, 31, 37, 200, _605tt, 38, 38, 0);
				_val7 = 4;
				break;

			case 6:
			case 7:
				sendWSMessage_10000(1, _tt, _605tt, 58, 66, 200, _605tt, 67, 67, 0);
				break;

			case 8:
				sendWSMessage_10000(1, _tt, _605tt, 134, 221, 207, _605tt, 221, 221, 0);
				_val6 = 3;
				break;

			case 9:
				sendWSMessage_10000(1, _tt, _605tt, 85, 93, 200, _605tt, 93, 93, 0);
				_val7 = 9;
				break;

			case 10:
				sendWSMessage_10000(1, _tt, _605tt, 234, 222, 203, _605tt, 222, 222, 0);
				break;

			default:
				break;
			}
			break;

		case 4:
			switch (_val6) {
			case 4:
				sendWSMessage_10000(1, _tt, _605tt, 38, 38, -1, _605tt, 38, 43, 4);
				sendWSMessage_1a0000(_tt, 13);
				break;

			case 5:
				sendWSMessage_10000(1, _tt, _605tt, 38, 38, 200, _605tt, 38, 38, 0);
				break;

			default:
				sendWSMessage_10000(1, _tt, _605tt, 44, 49, 200, _605tt, 1, 1, 0);
				_val7 = 0;
				break;
			}
			break;

		case 6:
			switch (_val6) {
			case 6:
				sendWSMessage_10000(1, _tt, _605tt, 67, 67, -1, _605tt, 67, 76, 4);
				sendWSMessage_1a0000(_tt, 13);
				break;

			case 7:
				sendWSMessage_10000(1, _tt, _605tt, 67, 67, 200, _605tt, 67, 67, 0);
				break;

			default:
				sendWSMessage_10000(1, _tt, _605tt, 77, 86, 200, _605tt, 1, 1, 0);
				_val7 = 0;
				break;
			}
			break;

		case 9:
			if (_val6 == 9) {
				sendWSMessage_10000(1, _tt, _605tt, 93, 93, 200, _605tt, 93, 93, 0);
			} else {
				sendWSMessage_10000(1, _tt, _605tt, 93, 85, 200, _605tt, 1, 1, 0);
				_val7 = 0;
			}
			break;

		default:
			break;
		}
		break;

	case 202:
		sendWSMessage_10000(1, _tt, _605tt, 12, 1, 200, _605tt, 1, 1, 0);
		sendWSMessage_190000(_tt, 7);
		break;

	case 203:
		terminateMachineAndNull(_tt);
		terminateMachineAndNull(_ttShadow);
		_tt = triggerMachineByHash_3000(8, 9, *SECTION6_NORMAL_DIRS, *SECTION6_SHADOW_DIRS,
			476, 290, 5, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 485, 199, 2, 208, 0);
		digi_play("19_07n04", 1, 255, 206);
		kernel_timing_trigger(420, 205);
		kernel_timing_trigger(20, 204);
		break;

	case 204:
		_G(player).disable_hyperwalk = true;
		sendWSMessage_10000(_G(my_walker), 490, 187, 2, 210, 0);
		break;

	case 205:
		disable_player_commands_and_fade_init(-1);
		break;

	case 206:
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28", 3, 90);
		_G(game).setRoom(608);
		break;

	case 207:
		sendWSMessage_10000(1, _tt, _605tt, 222, 234, 200, _605tt, 1, 1, 0);
		break;

	case 208:
		sendWSMessage_10000(_tt, 498, 189, 3, 209, 0);
		break;

	case 209:
		sendWSMessage_10000(_tt, 670, 183, 3, -1, 1);
		break;

	case 210:
		sendWSMessage_10000(_G(my_walker), 639, 171, 3, 211, 0);
		break;

	case 211:
		ws_walk(680, 183, nullptr, -1, 3);
		break;

	case 300:
		if (!_val1 && !_val2 && _trigger1 != -1) {
			kernel_trigger_dispatchx(_trigger1);
			_trigger1 = -1;

			if (_walkerHidden) {
				ws_unhide_walker();
				_walkerHidden = false;
			}
		}

		kernel_trigger_dispatchx(kernel_trigger_create(301));		
		break;

	case 301:
		switch (_val1) {
		case 0:
			switch (_val2) {
			case 0:
				sendWSMessage_10000(1, _ripley, 1, 1, 1, 300, 1, 1, 1, 0);
				break;
			case 1:
				sendWSMessage_10000(1, _ripley, _ripTwoHandTalk, 1, 6, 300,
					_ripTwoHandTalk, 6, 6, 0);
				_val1 = 1;
				break;
			case 2:
				sendWSMessage_10000(1, _ripley, _ripHandOnChin, 1, 16, 300,
					_ripHandOnChin, 16, 16, 0);
				_val1 = 2;
				break;
			case 3:
				sendWSMessage_10000(1, _ripley, _ripHandsHips, 1, 12, 300,
					_ripHandsHips, 12, 12, 0);
				_val1 = 3;
				break;
			case 5:
				terminateMachineAndNull(_ripley);
				ws_unhide_walker();
				terminateMachineAndNull(_shadow);
				break;
			case 6:
				sendWSMessage_10000(1, _ripley, _ripTalk, 1, 1, -1, _ripTalk, 1, 7, 4);
				sendWSMessage_1a0000(_ripley, 11);
				break;

			default:
				break;
			}
			break;

		case 1:
			if (_val2 == 1) {
				sendWSMessage_10000(1, _ripley, _ripTwoHandTalk, 6, 6, 300,
					_ripTwoHandTalk, 6, 6, 0);
			} else {
				sendWSMessage_10000(1, _ripley, _ripTwoHandTalk, 6, 1, 300, 1, 1, 1, 0);
				_val1 = 0;
			}
			break;

		case 2:
			if (_val2 == 2) {
				sendWSMessage_10000(1, _ripley, _ripHandOnChin, 16, 16, 300,
					_ripHandOnChin, 16, 16, 0);
			} else {
				sendWSMessage_10000(1, _ripley, _ripHandOnChin, 16, 1, 300, 1, 1, 1, 0);
				_val1 = 0;
			}
			break;

		case 3:
			if (_val2 == 3) {
				sendWSMessage_10000(1, _ripley, _ripHandsHips, 12, 12, 300,
					_ripHandsHips, 12, 12, 0);
			} else {
				sendWSMessage_10000(1, _ripley, _ripHandsHips, 12, 1, 300, 1, 1, 1, 0);
				_val1 = 0;
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

void Room605::pre_parser() {
	if (player_said_any("down", "left", "right") && _G(flags)[V203] == 2) {
		player_set_commands_allowed(false);
		intr_cancel_sentence();

		conv_load("conv605a", 10, 10, 747);
		conv_set_text_colour(12, 16);
		conv_set_shading(75);

		conv_export_value_curr(_G(flags)[V195] ? 1 : 0, 0);
		conv_export_value_curr(2, 1);
		conv_play();
		_val6 = 3;
	}
}

void Room605::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool talkFlag = player_said_any("talk", "talk to");
	bool takeFlag = player_said("take");

	if (player_said("conv605a")) {
		conv605a();
	} else if (talkFlag && player_said("samantha")) {
		player_set_commands_allowed(false);
		player_update_info();
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
			0x100, 1, triggerMachineByHashCallback, "rip");

		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _ripley, 1, 1, 1, 300, 1, 1, 1, 0);
		_G(kernel).trigger_mode = KT_PARSE;

		_val1 = 0;
		_val2 = 0;
		_shadow = series_show("safari shadow 2", 0xd00, 16, -1, -1, 0,
			-_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		conv_load("conv605a", 10, 10, 747);
		conv_play();

	} else if (_G(kernel).trigger == 747) {
		// No implementation
	} else if (takeFlag && player_said_any("pupil", "obsidian disk") &&
			_G(flags)[V203] != 2 && takePupilDisk()) {
		// No implementation
	} else if (player_said("SLEEVE") && player_said_any("pupil", "OBSIDIAN DISK") &&
			_G(flags)[V203] == 2 && sleeveDisk1()) {
		// No implementation
	} else if (player_said("SLEEVE") && player_said_any("pupil", "OBSIDIAN DISK") &&
		_G(flags)[V203] != 2 && sleeveDisk2()) {
		// No implementation
	} else if (player_said("journal", "glyphs")) {
		if (_G(flags)[V196]) {
			digi_play("203r54", 1);
		} else {
			if (_G(kernel).trigger == 6)
				_G(flags)[V196] = 1;
			sketchInJournal(0);
		}
	} else if (lookFlag && player_said("head")) {
		digi_play("605r03", 1);
	} else if (lookFlag && player_said("glyphs")) {
		digi_play("605r35", 1);
	} else if (lookFlag && player_said("SAMANTHA")) {
		static const char *DIGI[4] = { "605r05", "605r06", "605r07", "605r08" };
		digi_play(DIGI[imath_ranged_rand(0, 3)], 1);
	} else if (lookFlag && player_said("topknot")) {
		if (_G(flags)[V203] == 2) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_val6 = 5;
				digi_play("605r04", 1, 255, 1);
				break;
			case 1:
				_val6 = 4;
				digi_play("605t03", 1, 255, 2);
				break;
			case 2:
				_val6 = 0;
				kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
				break;
			default:
				break;
			}
		} else {
			digi_play("605r26", 1);
		}
	} else if (lookFlag && player_said("LEFT")) {
		if (_G(flags)[V000]) {
			if (_G(flags)[V186])
				digi_play("305r53", 1);
			else
				_G(flags)[V185] = 1;
		} else {
			digi_play("305r53", 1);
		}
	} else if (lookFlag && player_said("up")) {
		if (_G(flags)[V000]) {
			if (_G(flags)[V184])
				digi_play("305r53", 1);
			else
				_G(flags)[V184] = 1;
		} else {
			digi_play("305r53", 1);
		}
	} else if (lookFlag && player_said_any("pupil", "OBSIDIAN DISK") &&
			inv_object_is_here("OBSIDIAN DISK")) {
		digi_play(_G(flags)[V203] == 2 ? "605r09" : "605r27", 1);

	} else if (parserMisc()) {
		// No implementation
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room605::conv605a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	switch (_G(kernel).trigger) {
	case 1:
		if (who <= 0) {
			if (node == 1 && entry == 0)
				_val2 = 0;
			if (node == 2 && entry == 0) {
				kernel_timing_trigger(60, 4);
				return;
			}
			if (_val6 == 3)
				kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);

			_val6 = 0;

		} else if (who == 1) {
			if (!(node == 6 && entry == 2) &&
					!(node == 1 && entry == 0) &&
					!(node == 2 && entry == 3)) {
				if (_val2 == 6)
					kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
				_val2 = 0;
			}
		}

		conv_resume();
		break;

	case 2:
		ws_walk(513, 316, nullptr, 3, 10);
		return;

	case 3:
		player_update_info();
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
			0x100, 1, triggerMachineByHashCallback, "rip");

		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _ripley, 1, 1, 1, 300, 1, 1, 1, 0);
		_G(kernel).trigger_mode = KT_PARSE;

		_val1 = 0;
		_val2 = 0;
		_shadow = series_show("safari shadow 2", 0xd00, 16, -1, -1, 0,
			-_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		break;

	case 4:
		conv_resume();
		_val6 = 0;
		break;

	case 5:
		_val2 = 0;
		break;

	default:
		if (sound) {
			if (who <= 0) {
				if (node == 6 && entry == 3) {
					_val6 = 8;
				} else if (node == 6 && entry == 4) {
					_val6 = 10;
					_val2 = 5;
				} else if (node == 3 && entry == 0) {
					kernel_timing_trigger(120, 2);
				} else if (node == 2 && entry == 0) {
					_val6 = 9;
				} else {
					_val6 = 3;
				}
			} else if (who == 1) {
				if (node == 6 && entry == 2) {
					_val2 = 3;
				} else if (node == 6 && entry == 1) {
					_val2 = 1;
				} else if ((node == 1 && entry == 0) ||
						(node == 2 && entry == 3)) {
					_val2 = 2;
				} else if (node == 2 && entry == 1) {
					_val2 = 1;
					kernel_timing_trigger(60, 5);
				} else {
					_val2 = 6;
				}
			}

			digi_play(sound, 1);

		} else {
			conv_resume();
		}
		break;
	}
}

bool Room605::takePupilDisk() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("OBSIDIAN DISK")) {
			player_set_commands_allowed(false);
			_ripHandOnIris = series_load("RIP BURNS HAND ON IRIS");
			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallback, "take pupil");
			sendWSMessage_10000(1, _ripley, _ripHandOnIris, 1, 26, 2,
				_ripHandOnIris, 26, 26, 1);
			return true;
		}
		break;

	case 1:
		_val6 = 7;
		return true;

	case 2:
		sendWSMessage_10000(1, _ripley, _ripHandOnIris, 27, 41, 6,
			_ripHandOnIris, 41, 41, 1);
		digi_play("605r15", 1, 255, 4);
		return true;

	case 4:
		_val6 = 6;
		digi_play("605t06", 1, 255, 8);
		return true;

	case 6:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		return true;

	case 8:
		series_unload(_ripHandOnIris);
		_val6 = 0;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 200);
		player_set_commands_allowed(true);
		_G(flags)[V197] = 1;
		return true;

	default:
		break;
	}

	return false;
}

bool Room605::sleeveDisk1() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("OBSIDIAN DISK")) {
			player_set_commands_allowed(false);
			ws_hide_walker();
			_ripGetsIrisWithCloth = series_load("RIP GETS IRIS WITH CLOTH");
			digi_preload("605_s01");
			digi_preload("605_s02");
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallback, "take pupil");
			sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 1, 41, 1,
				_ripGetsIrisWithCloth, 41, 41, 1);
			return true;
		}
		break;

	case 1:
		sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 42, 43, 3,
			_ripGetsIrisWithCloth, 43, 43, 1);
		_val6 = 7;
		digi_play("605_S01", 2);
		return true;

	case 3:
		terminateMachineAndNull(_pupil);
		inv_give_to_player("OBSIDIAN DISK");
		hotspot_set_active("PUPIL", false);
		hotspot_set_active("OBSIDIAN DISK", false);
		sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 44, 75, 6,
			_ripGetsIrisWithCloth, 75, 75, 1);
		return true;

	case 6:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		digi_play("605r16", 1, 255, 8);
		return true;

	case 8:
		_val6 = 6;
		digi_play("605t07", 1, 255, 9);
		return true;

	case 9:
		_val6 = 7;
		kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
			triggerMachineByHashCallback, "take pupil");
		sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 1, 41, 10,
			_ripGetsIrisWithCloth, 41, 41, 1);
		return true;

	case 10:
		sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 42, 43, 11,
			_ripGetsIrisWithCloth, 43, 43, 1);
		digi_play("605_S02", 2);
		return true;

	default:
		break;
	}

	return false;
}

bool Room605::sleeveDisk2() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("OBSIDIAN DISK")) {
			player_set_commands_allowed(false);
			ws_hide_walker();
			_ripGetsIrisWithCloth = series_load("RIP GETS IRIS WITH CLOTH");
			digi_preload("605_s01");
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, 0,
				triggerMachineByHashCallback, "take pupil");
			sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 1, 41, 1,
				_ripGetsIrisWithCloth, 41, 41, 1);
			return true;
		}
		break;

	case 1:
		sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 42, 43, 2,
			_ripGetsIrisWithCloth, 43, 43, 1);
		digi_play("605_S01", 2);
		return true;

	case 2:
		hotspot_set_active("PUPIL", false);
		hotspot_set_active("OBSIDIAN DISK", false);
		inv_give_to_player("OBSIDIAN DISK");
		kernel_examine_inventory_object("PING OBSIDIAN DISK", 5, 1, 260, 190, 3);
		terminateMachineAndNull(_pupil);
		return true;

	case 3:
		sendWSMessage_10000(1, _ripley, _ripGetsIrisWithCloth, 44, 75, 6,
			_ripGetsIrisWithCloth, 75, 75, 1);
		return true;

	case 6:
		digi_unload("605_s01");
		terminateMachineAndNull(_ripley);
		series_unload(_ripGetsIrisWithCloth);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room605::parserMisc() {
	switch (_G(kernel).trigger) {
	case 555:
		digi_stop(1);
		midi_play("tensions", 255, 1, -1, 949);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28", 3, 90);
		_G(game).setRoom(610);
		break;

	case 556:
		digi_stop(1);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28", 3, 90);
		_G(game).setRoom(603);
		break;

	case 557:
		digi_stop(1);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28", 3, 90);
		_G(game).setRoom(608);
		break;

	case 558:
		digi_stop(1);
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28", 3, 90);
		_G(game).setRoom(623);
		break;

	default:
		if (player_said_any("down", "left", "right")) {
			if (_G(flags)[V203] != 2) {
				if (player_said("down")) {
					player_set_commands_allowed(false);
					disable_player_commands_and_fade_init(
						_G(flags)[V203] == 8 ? 555 : 556);
				}

				if (player_said("right")) {
					player_set_commands_allowed(false);
					disable_player_commands_and_fade_init(557);
				}

				if (player_said("left")) {
					player_set_commands_allowed(false);
					disable_player_commands_and_fade_init(558);
				}
			}
		} else {
			return false;
		}
		break;
	}

	return true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
