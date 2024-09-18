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

#include "m4/riddle/rooms/section6/room603.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS1[] = { 200, -1 };
static const char *NORMAL_NAMES1[] = { "shen guo walker 3" };
static const int16 SHADOW_DIRS1[] = { 210, -1 };
static const char *SHADOW_NAMES1[] = { "shen guo shadow 3" };
static const int16 NORMAL_DIRS2[] = { 220, -1 };
static const char *NORMAL_NAMES2[] = { "kuang walker 2" };
static const int16 SHADOW_DIRS2[] = { 230, -1 };
static const char *SHADOW_NAMES2[] = { "kuangs shadow 2" };

void Room603::preload() {
	if (_G(flags)[V191]) {
		_G(art_base_override) = "EXPLOSION BACKGROUND";
		_G(use_alternate_attribute_file) = false;
		_G(player).walker_in_this_scene = false;
	}

	Room::preload();
}

void Room603::init() {
	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_trigger1 = 0;
		_val2 = 0;
		_val3 = 0;
		_val4 = 0;
		_val5 = 0;
	}

	_val6 = 0;

	if (_G(game).previous_room != 604 && _G(game).previous_room != KERNEL_RESTORING_GAME &&
			inv_player_has("PULL CORD") && !inv_object_is_here("POLE") &&
			_G(flags)[V203] == 4) {
		_G(flags)[V203] = 5;
		_G(flags)[GLB_TEMP_5] = 0;
	}

	if (_G(flags)[V191]) {
		interface_hide();
		player_set_commands_allowed(false);
		digi_preload("explode");
		digi_preload("950_s28a");
		digi_play_loop("950_s28a", 3, 90);
		series_play("653 RIP HOPS ROCK", 0xf00, 16, -1, 5);
		series_load("653 FIRST FRAME SHED");
		_shed = series_show("653 FIRST FRAME SHED", 0xf00, 16);

		ws_walk_load_walker_series(NORMAL_DIRS1, NORMAL_NAMES1);
		ws_walk_load_shadow_series(SHADOW_DIRS1, SHADOW_NAMES1);

		_shenWalker = triggerMachineByHash_3000(8, 12, NORMAL_DIRS1, SHADOW_DIRS1, 525, 238, 3,
			triggerMachineByHashCallback3000, "shen suit gun walker");
		sendWSMessage_10000(_shenWalker, 535, 235, 3, 10, 1);

		ws_walk_load_walker_series(NORMAL_DIRS2, NORMAL_NAMES2);
		ws_walk_load_shadow_series(SHADOW_DIRS2, SHADOW_NAMES2);

		_kuangWalker = triggerMachineByHash_3000(8, 13, NORMAL_DIRS2, SHADOW_DIRS2, 554, 245, 2,
			triggerMachineByHashCallback3000, "kuang suit walker");
		sendWSMessage_10000(_kuangWalker, 563, 244, 2, -1, 1);
		return;
	}

	if (_G(flags)[V203] != 9) {
		digi_preload("603_S02");
		digi_preload("603_S02a");
		digi_preload("603_S02b");
		digi_preload("603_S02c");

		if (_G(flags)[V203] == 0) {
			digi_preload("603_S01");
			digi_preload("603_S01A");
			digi_preload("603_S01B");
			_rp01 = series_load("603RP01");
			_rp02 = series_load("603RP02");
		}

		_tt03 = series_load("603TT03");
		_ttD01 = series_load("603TTD01");
		_rp04 = series_load("603RP04");
		_ripHandsBehindBack = series_load("RIP TREK HANDS BEHIND BACK POS2");
		_ripTalk = series_load("RIP TREK TALK");
		_ripHandsHips = series_load("RIP TREK HANDS HIPS POS2");
		_ripHandChin = series_load("RIP TREK HAND ON CHIN POS2");
		_ripTwoHandTalk = series_load("RIP TREK TWO HAND TALK POS2");
	}

	if (_G(flags)[V203] == 9) {
		_note = series_show("603NOTE", 0x700, 16);
		hotspot_set_active("NOTE", false);
		hotspot_set_active("twelvetrees", false);
		hotspot_set_active("twelvetrees ", false);
		hotspot_set_active("person in pit", false);
	} else {
		hotspot_set_active("NOTE ", false);

		if (!inv_object_is_here("TWELVETREES' NOTE")) {
			hotspot_set_active("NOTE", false);
		} else if (_G(flags)[V203] == 4) {
			_note = series_show("603NOTE", 0x800, 16);
		} else {
			hotspot_set_active("NOTE", false);
		}
	}

	if (inv_object_is_here("POLE")) {
		_pole = series_show("603POLE", 0x400, 16);
	} else {
		hotspot_set_active("POLE", false);
	}

	if (_G(flags)[V203] < 1) {
		_shirt = series_show("603SHIRT", 0x400, 16);
		hotspot_set_active("SLEEVE", false);
	} else {
		hotspot_set_active("SHIRT", false);

		if (inv_object_is_here("SLEEVE"))
			_sleeve = series_show("603SLEEV", 0x400, 16);
		else
			hotspot_set_active("SLEEVE", false);
	}

	if (_G(flags)[V203] == 9) {
		hotspot_set_active("POWER LINE", false);
		hotspot_set_active("shed", false);
		hotspot_set_active("wire", false);
		hotspot_set_active("  ", false);
		series_show("603ROCKS", 0xf00, 16);
	} else {
		series_load("603 POWER LINE");
		series_show("603 POWER LINE", 0, 16);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload(_G(globals)[GLB_TEMP_5] ? "genrun" : "950_s28a");
		break;

	case 604:
		player_set_commands_allowed(false);
		digi_preload("950_s28a");
		ws_demand_location(542, 230, 2);
		ws_walk(534, 240, nullptr, 5, 7);
		break;

	case 605:
		if (_G(globals)[GLB_TEMP_5]) {
			digi_preload("genrun");
		} else {
			digi_preload("950_s28a");
			digi_stop(3);
			digi_unload("950_s28");
		}

		player_set_commands_allowed(false);
		ws_demand_location(670, 232, 2);
		ws_walk(497, 245, nullptr, 1, 8);
		break;

	default:
		player_set_commands_allowed(false);
		digi_preload("950_s28a");
		ws_demand_location(67, 391, 2);
		ws_walk(135, 356, nullptr, 1, 2);
		break;
	}

	switch (_G(flags)[V203]) {
	case 0:
		digi_preload("17_04p01");
		digi_preload("17_04n06");
		_G(player).disable_hyperwalk = true;
		_G(kernel).call_daemon_every_loop = true;

		_door = series_show("603DOOR", 0xf00, 16, -1, -1, 23, 100, 0, 0);
		_ttDigShirtOff = series_load("TT DIG LOOP NO SHIRT");
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
			triggerMachineByHashCallbackNegative, "tt");
		sendWSMessage_10000(1, _tt, _ttDigShirtOff, 2, 2, 200, _ttDigShirtOff, 2, 2, 0);

		_trigger1 = 200;
		_val7 = 1;
		_val8 = 0;
		_val9 = 0;
		_val10 = 0;

		hotspot_set_active("twelvetrees ", false);
		hotspot_set_active("twelvetrees", false);
		break;

	case 1:
	case 3:
	case 5:
	case 7:
		_door = series_show("603DOOR", 0xf00, 16);
		hotspot_set_active("wire", false);
		_ttDigShirtOn = series_load("TT DIG LOOP SHIRT ON");
		_val7 = 6;
		_val8 = 17;
		_val9 = 0;
		_val10 = 0;
		hotspot_set_active("person in pit", false);

		if (_val5) {
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
				triggerMachineByHashCallbackNegative, "tt");
			sendWSMessage_10000(1, _tt, _ttD01, 1, 1, 400, _ttD01, 1, 1, 0);
			_trigger1 = 400;
			hotspot_set_active("twelvetrees ", false);
			_ttShadow = series_show("tt walker shadow 4", 0xf00, 0, -1, -1, 0, 53, 291, 293);

		} else {
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
				triggerMachineByHashCallbackNegative, "tt");
			sendWSMessage_10000(1, _tt, _ttDigShirtOn, 1, 1, 500,
				_ttDigShirtOn, 1, 1, 0);
			_trigger1 = 500;
			hotspot_set_active("twelvetrees", false);
		}
		break;

	default:
		if (_G(game).previous_room == 604)
			_door = series_show("603DOOR", 0xf00, 16, -1, -1, 14, 100, 0, 0);
		else
			_door = series_show("603DOOR", 0xf00, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active("wire", false);
		hotspot_set_active("twelvetrees", false);
		hotspot_set_active("twelvetrees ", false);
		hotspot_set_active("person in pit", false);
		break;
	}

	if (_G(flags)[GLB_TEMP_5])
		digi_play_loop("genrun", 3, 140, -1, 604);
	else
		digi_play_loop("950_s28a", 3, 90, -1, -1);
}

void Room603::daemon() {
	// TODO
}

void Room603::pre_parser() {
	if (_G(flags)[V202] && !player_said("talk to", "twelvetrees"))
		_G(flags)[V204] = 1;

	if (_val6) {
		Common::strcpy_s(_G(player).noun, "remove note");
		_G(player).resetWalk();
	}

	if (player_said("take", "sleeve") && inv_object_is_here("SLEEVE")) {
		auto &v203 = _G(flags)[V203];
		if (v203 == 1 || v203 == 3 || v203 == 5 || v203 == 7)
			_G(player).resetWalk();
	}
}

void Room603::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");

	if (player_said("conv603a")) {
		conv603a();
	} else if (player_said("conv603b")) {
		conv603b();
	} else if (_G(kernel).trigger == 747) {
		if (_val8 != 20) {
			unloadSeries();
			_val9 = 1000;
			_num1 = 11;
			_val8 = 17;
		}
	} else if (_G(kernel).trigger == 748) {
		if (_val8 == 21) {
			disable_player_commands_and_fade_init(749);
		} else {
			unloadSeries();
			_val9 = 1000;
			_num1 = 11;
			_val8 = 17;
		}
	} else if (_G(kernel).trigger == 749) {
		digi_unload("603_S02");
		digi_unload("603_S02a");
		digi_unload("603_S02b");
		digi_unload("603_S02c");
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28a", 3, 90);
		_G(game).setRoom(608);
		_G(flags)[V203] = 6;
	} else if (player_said("talk to", "person it pit")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			player_set_commands_allowed(false);
			player_update_info();
			_G(player).disable_hyperwalk = true;
			_G(flags)[V202] = 1;
			setGlobals1(_ripHandsBehindBack, 1, 15, 15, 15);
			sendWSMessage_110000(2);
			break;
		case 2:
			sendWSMessage_150000(-1);
			ws_hide_walker();
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 311, 308, -59, 0x100, 0,
				triggerMachineByHashCallbackNegative, "rip");
			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _ripley, _rp01, 1, 15, 302, _rp01, 15, 15, 0);
			sendWSMessage_1e0000(0, 0);
			sendWSMessage_200000(_ripley, 100);
			_G(kernel).trigger_mode = KT_PARSE;

			_num2 = 0;
			_num1 = 0;
			_G(flags)[V203] = 1;
			_val5 = 1;
			hotspot_set_active("person in pit", false);
			hotspot_set_active("twelvetrees", true);
			hotspot_set_active("twelvetrees ", false);
			break;
		default:
			break;
		}
	} else if (player_said_any("lighter", "lit lighter") &&
			player_said_any("twelvetrees", "twelvetrees ")) {
		digi_play("com140", 1, 255, -1, 997);
	} else if (player_said("talk to", "twelvetrees")) {
		player_set_commands_allowed(false);
		ws_hide_walker();
		_val2 = 1;
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 345, 300, -56, 0x100, 1,
			triggerMachineByHashCallbackNegative, "rip");
		_shadow = series_show("safari shadow 2", 0xf00, 128, -1, -1, 0, 56, 345, 300);
		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _ripley, _ripTalk, 1, 1, 300, _ripTalk, 1, 1, 0);
		sendWSMessage_190000(_ripley, 4);
		_G(kernel).trigger_mode = KT_PARSE;
		_num2 = 0;
		_num1 = 0;

		conv_load("conv603a", 10, 10, 747);
		conv_set_shading(65);
		conv_export_value_curr(_G(flags)[V204], 0);
		conv_export_value_curr(_G(flags)[V205], 1);
		conv_play();
	} else if (player_said("talk to", "twelvetrees ")) {
		player_set_commands_allowed(false);
		ws_hide_walker();
		_val2 = 1;
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 311, 308, -59, 0x100, 1,
			triggerMachineByHashCallbackNegative, "rip");
		_shadow = series_show("safari shadow 2", 0xf00, 128, -1, -1, 0, 59, 311, 308);
		_G(kernel).trigger_mode = KT_DAEMON;

		sendWSMessage_10000(1, _ripley, _ripTalk, 1, 1, 300, _ripTalk, 1, 1, 0);
		sendWSMessage_190000(_ripley, 4);
		_G(kernel).trigger_mode = KT_PARSE;
		_num2 = 0;
		_num1 = 0;
		_val8 = 7;

		if (_G(flags)[V203] <= 1) {
			conv_load("conv603a", 10, 10, 747);
			conv_export_value_curr(_G(flags)[V204], 0);
			conv_export_value_curr(_G(flags)[V205], 1);
			conv_play();
		} else {
			conv_load("conv603b", 10, 10, 748);

			if (_G(flags)[V203] == 7) {
				conv_export_value_curr(4, 0);
			} else {
				switch (_G(flags)[V186]) {
				case 0:
					conv_export_value_curr(1, 0);
					break;
				case 1:
					conv_export_value_curr(2, 0);
					break;
				case 2:
					conv_export_value_curr(3, 0);
					break;
				default:
					break;
				}
			}

			conv_export_value_curr(0, 1);
			conv_export_value_curr(_G(flags)[V039], 6);
			conv_play();
		}
	} else if (takeFlag && player_said("sleeve")) {
		// TODO
	} else if (lookFlag) {
		// TODO
	}
	// TODO
	else {
		return;
	}

	_G(player).command_ready = false;
}

void Room603::conv603a() {
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	switch (_G(kernel).trigger) {
	case 1:
		if (who <= 0) {
			if (node == 23) {
				if (_val5) {
					_pu01 = series_stream("603pu03", 5, 0, 30);
					_val8 = 6;

					if (_val5)
						kernel_timing_trigger(1, _trigger1, KT_DAEMON, KT_PARSE);
				} else {
					_pu01 = series_stream("603pu03a", 5, 0, 30);
					_val8 = 7;
					kernel_timing_trigger(1, 500, KT_DAEMON, KT_PARSE);
				}
			} else if (_val5) {
				if (node == 3 && (entry == 1 || entry == 2)) {
					// No implementation
				} else if ((node == 6 || node == 27) && entry == 0) {
					_val8 = 12;
					kernel_timing_trigger(1, 400, KT_DAEMON, KT_PARSE);
					conv_resume();
				} else if ((node == 3 && entry == 7) ||
						(node == 3 && entry == 8)) {
					// No implementation
				} else if (node == 25 && entry == 3) {
					_val8 = 20;
					kernel_timing_trigger(1, 400, KT_DAEMON, KT_PARSE);
					conv_resume();
				} else if (node == 3 && entry == 4) {
					_val8 = 16;
					kernel_timing_trigger(1, 400, KT_DAEMON, KT_PARSE);
					conv_resume();
				} else if (node = 0 || node == 1 || node == 2) {
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 43, 43, -1,
						_ttDigShirtOff, 43, 43, 0);
					conv_resume();
				} else {
					_val8 = 6;

					if (_trigger1)
						kernel_timing_trigger(1, _trigger1, KT_DAEMON, KT_PARSE);
					conv_resume();
				}
			} else {
				_val8 = (node == 25 && entry == 3) ? 20 : 7;
				kernel_timing_trigger(1, 500, KT_DAEMON, KT_PARSE);
				conv_resume();
			}
		} else if (who == 1) {
			if (node == 3 && entry == 0) {
				// No implementation
			} else if (node == 0 || node == 1 || node == 2) {
				_num1 = 14;
				kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
				conv_resume();
			} else if (node == 3 && entry == 6) {
				// No implementation
			} else if ((node == 5 && entry == 0) ||
					(node == 25 && entry == 0) ||
					(node == 14 && entry == 0) ||
					(node == 27 && entry == 0) ||
					(node == 11 && entry == 0) ||
					(node == 4 && entry == 0) ||
					(node == 4 && entry == 1) ||
					(node == 7)) {
				conv_resume();
			} else if (node == 12 && entry == 1) {
				_num1 = 0;
				conv_resume();
			} else if (_val2) {
				if (_num1 == 5)
					kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
				conv_resume();
			} else {
				conv_resume();
			}
		} else {
			conv_resume();
		}
		break;

	case 2:
		series_stream_break_on_frame(_pu01, 16, 40);
		digi_play(sound, 1);
		break;

	case 3:
		ws_hide_walker();
		series_play("603tt07", 0, 0, 4, 6);
		break;

	case 4:
		terminateMachineAndNull(_door);
		series_play("603DOOR", 0x300, 2, 6, 6, 0, 100, 0, 0, 1, 23);
		series_play("603tt07", 0, 2, 5, 6, 1, 100, 0, 0, 0, 9);
		break;

	case 5:
		ws_unhide_walker();
		sendWSMessage_10000(_tt, 501, 236, 8, 7, 1);
		break;

	case 6:
		_door = series_show("603DOOR", 0xf00, 16);
		hotspot_set_active("wire", false);
		digi_play("603_s03", 2);
		break;

	case 7:
	case 33:
		conv_resume();
		break;

	case 10:
		kernel_timing_trigger(40, 11);
		break;

	case 11:
		_pu01 = series_stream("603pu02", 5, 0, 12);
		break;

	case 12:
		digi_unload("17_04n08");
		digi_unload("17_04n99");
		sendWSMessage_10000(_tt, 357, 283, 1, 13, 0);
		kernel_timing_trigger(200, 15);
		break;

	case 13:
		sendWSMessage_10000(_tt, 291, 293, 4, 14, 1);
		break;

	case 14:
		conv_resume();
		sendWSMessage_60000(_tt);
		_ttShadow = series_show("tt walker shadow 4", 0xf00, 0, -1, -1, 0, 53, 291, 293);
		_trigger1 = 400;
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
			triggerMachineByHashCallbackNegative, "tt");
		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _tt, _tt03, 110, 122, 400, _ttD01, 1, 1, 0);
		_G(kernel).trigger_mode = KT_PARSE;
		break;

	case 15:
		ws_walk(345, 300, nullptr, 16, 10);
		break;

	case 16:
		ws_hide_walker();
		_val2 = 1;
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 345, 300, -56, 0x100, 1,
			triggerMachineByHashCallbackNegative, "rip");
		_shadow = series_show("safari shadow 2", 0xf00, 128, -1, -1, 0, 56, 345, 300);
		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 1, 15, 300,
			_ripHandsBehindBack, 15, 15, 0);
		sendWSMessage_190000(_ripley, 4);
		_G(kernel).trigger_mode = KT_PARSE;
		_num2 = 8;
		_num1 = 8;
		break;

	case 20:
		_num1 = 10;
		break;

	case 21:
		_val8 = 14;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, _trigger1);
		break;

	case 22:
		_val8 = 13;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, _trigger1);
		break;

	case 30:
		kernel_timing_trigger(1, 31);
		break;

	case 31:
		_pu01 = series_stream(_val5 ? "603pu04" : "603pu04a", 5, 0, 33);
		break;

	case 40:
		conv_resume();
		digi_play(sound, 1);
		break;

	default:
		if (sound) {
			if (who <= 0) {
				if (!_val5) {
					_val8 = 8;
				} else if (node == 0 || node == 1 || node == 2) {
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 41, 43, -1,
						_ttDigShirtOff, 41, 43, 0);
					sendWSMessage_190000(_tt, 13);
					sendWSMessage_1a0000(_tt, 13);
				} else if (node == 12 && entry == 1) {
					_val8 = 8;
					kernel_timing_trigger(200, 20);
					kernel_timing_trigger(760, 21);
				} else if (node == 12 && entry == 2) {
					_val8 = 8;
					kernel_timing_trigger(210, 22);
				} else if (node == 11 && entry == 0) {
					_val8 = 14;
				} else if ((node == 27 && entry == 0) ||
						(node == 6 && entry == 0)) {
					_val8 = 11;
				} else if (node == 3 && entry == 7) {
					digi_preload("17_04n08");
					digi_preload("17_04n99");
					_pu01 = series_stream("603pu01", 5, 0, 10);
					series_stream_break_on_frame(_pu01, 6, 2);
				} else {
					_val8 = 8;
				}
			} else if (who == 1) {
				if (node == 3 && entry == 0) {
					_num1 = 1;
				} else if (node == 3 && entry == 6) {
					sendWSMessage_10000(_tt, 501, 234, 1, 3, 1);
				} else if ((node == 4 && entry == 0) || (node == 7)) {
					_num1 = 8;
				} else if (node == 0 || node == 1 || node == 2) {
					_num1 = 13;
				} else if (node == 6 && entry == 0) {
					_num1 = 2;
				} else if (node == 6 && entry == 1) {
					_num1 = 6;
				} else if (node == 15 && entry == 0) {
					_num1 = 6;
				} else if (node == 14 && entry == 0) {
					_num1 = 8;
				} else if (node == 27 && entry == 0) {
					_num1 = 10;
				} else if (node == 8 && entry == 0) {
					_num1 = 9;
				} else if ((node == 11 && entry == 0) ||
						(node == 25 && entry == 0)) {
					_num1 = 10;
				} else if (node == 25 && entry == 2) {
					_num1 = 7;
				} else if ((node == 12 && entry == 1) ||
						(node == 7 && entry == 0)) {
					_num1 = 6;
				} else if (node == 5 && entry == 0) {
					_num1 = 10;
				} else if (_val2) {
					_num1 = 5;
				}
			}

			digi_play(sound, 1);

		} else {
			conv_resume();
		}
		break;
	}
}

void Room603::conv603b() {
	// TODO
}

void Room603::unloadSeries() {
	series_unload(200);
	series_unload(201);
	series_unload(202);
	series_unload(203);
	series_unload(204);
	series_unload(210);
	series_unload(211);
	series_unload(212);
	series_unload(214);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
