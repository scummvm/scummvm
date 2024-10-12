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
#include "m4/riddle/rooms/section6/section6.h"
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


static const char *SAID[][2] = {
	{ "shed",          "603r03" },
	{ "wire",          "603r04" },
	{ "power line",    "603r05" },
	{ "altar",         "603r06" },
	{ "shirt",         "603r07" },
	{ "cliff",         "603r10" },
	{ "person in pit", "603r11" },
	{ nullptr, nullptr }
};

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

		_shenWalker = triggerMachineByHash_3000(8, 12, *NORMAL_DIRS1, *SHADOW_DIRS1, 525, 238, 3,
			triggerMachineByHashCallback3000, "shen suit gun walker");
		sendWSMessage_10000(_shenWalker, 535, 235, 3, 10, 1);

		ws_walk_load_walker_series(NORMAL_DIRS2, NORMAL_NAMES2);
		ws_walk_load_shadow_series(SHADOW_DIRS2, SHADOW_NAMES2);

		_kuangWalker = triggerMachineByHash_3000(8, 13, *NORMAL_DIRS2, *SHADOW_DIRS2, 554, 245, 2,
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
			triggerMachineByHashCallback, "tt");
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
				triggerMachineByHashCallback, "tt");
			sendWSMessage_10000(1, _tt, _ttD01, 1, 1, 400, _ttD01, 1, 1, 0);
			_trigger1 = 400;
			hotspot_set_active("twelvetrees ", false);
			_ttShadow = series_show("tt walker shadow 4", 0xf00, 0, -1, -1, 0, 53, 291, 293);

		} else {
			_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
				triggerMachineByHashCallback, "tt");
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
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 2:
		player_set_commands_allowed(true);
		digi_play("603_s03", 2);
		break;

	case 5:
		terminateMachineAndNull(_door);
		_door = series_play("603door", 0xf00, 18, 2, 8, 0, 100, 0, 0, 0, 14);
		break;

	case 10:
		sendWSMessage_60000(_shenWalker);
		sendWSMessage_60000(_kuangWalker);
		_explosion = series_stream("603 EXPLOSION", 5, 0x100, -1);
		series_stream_break_on_frame(_explosion, 40, 24);
		break;

	case 20:
		disable_player_commands_and_fade_init(22);
		break;

	case 22:
		kernel_timing_trigger(180, 23);
		break;

	case 23:
		digi_stop(1);
		digi_unload("explode");
		digi_play_loop("950_s28a", 3, 90);
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(615);
		break;

	case 24:
		midi_stop();
		series_stream_break_on_frame(_explosion, 105, 20);
		terminateMachineAndNull(_shed);
		digi_play("explode", 1);
		break;

	case 200:
		kernel_trigger_dispatchx(kernel_trigger_create(201));
		break;

	case 201:
		switch (_val7) {
		case 1:
			switch (_val8) {
			case 0:
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 2, 2, 200,
						_ttDigShirtOff, 2, 2, 0);
					sendWSMessage_190000(_tt, imath_ranged_rand(10, 60));
					break;
				case 2:
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 1, 1, 200,
						_ttDigShirtOff, 1, 1, 0);
					_val7 = 2;
					sendWSMessage_190000(_tt, imath_ranged_rand(10, 60));
					playRandomDigi(8);
					break;
				default:
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 1, 14, 200,
						_ttDigShirtOff, 15, 15, 0);
					_val7 = 3;
					sendWSMessage_190000(_tt, imath_ranged_rand(7, 10));
					playRandomDigi(8);
					break;
				}
				break;

			case 5:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 25, 43, 202,
					_ttDigShirtOff, 43, 43, 0);
				break;

			case 22:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 25, 34, 210,
					_ttDigShirtOff, 34, 34, 0);
				break;

			default:
				break;
			}
			break;

		case 2:
			switch (_val8) {
			case 0:
				if (imath_ranged_rand(1, 2) == 1) {
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 1, 1, 200,
						_ttDigShirtOff, 1, 1, 0);
					_val7 = 1;
				} else {
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 1, 1, 200,
						_ttDigShirtOff, 1, 1, 0);
					sendWSMessage_190000(_tt, imath_ranged_rand(10, 60));
				}
				break;

			case 5:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 1, 1, 200,
					_ttDigShirtOff, 1, 1, 0);
				_val7 = 1;
				break;

			default:
				break;
			}
			break;

		case 3:
			switch (_val8) {
			case 0:
				switch (imath_ranged_rand(1, 3)) {
				case 1:
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 15, 15, 200,
						_ttDigShirtOff, 15, 15, 0);
					sendWSMessage_190000(_tt, imath_ranged_rand(10, 60));
					break;
				case 2:
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 14, 1, 200,
						_ttDigShirtOff, 2, 2, 0);
					_val7 = 1;
					sendWSMessage_190000(_tt, imath_ranged_rand(7, 10));
					break;
				case 3:
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 16, 24, 200,
						_ttDigShirtOff, 24, 24, 0);
					_val7 = 4;
					sendWSMessage_190000(_tt, imath_ranged_rand(7, 10));
					break;
				default:
					break;
				}
				break;

			case 5:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 14, 1, 200,
					_ttDigShirtOff, 2, 2, 0);
				_val7 = 1;
				break;

			case 22:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 14, 1, 200,
					_ttDigShirtOff, 2, 2, 0);
				_val7 = 1;
				break;

			default:
				break;
			}
			break;

		case 4:
			switch (_val8) {
			case 0:
				if (imath_ranged_rand(1, 2) == 1) {
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 24, 16, 200,
						_ttDigShirtOff, 15, 15, 0);
					_val7 = 3;
					sendWSMessage_190000(_tt, imath_ranged_rand(7, 10));
					playRandomDigi(8);
				} else {
					sendWSMessage_10000(1, _tt, _ttDigShirtOff, 24, 24, 200,
						_ttDigShirtOff, 24, 24, 0);
					_val7 = 1;
					sendWSMessage_190000(_tt, imath_ranged_rand(10, 60));
				}
				break;

			case 5:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 24, 16, 200,
					_ttDigShirtOff, 15, 15, 0);
				_val7 = 3;
				break;

			case 22:
				sendWSMessage_10000(1, _tt, _ttDigShirtOff, 14, 16, 200,
					_ttDigShirtOff, 15, 15, 0);
				_val7 = 3;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 202:
		sendWSMessage_10000(1, _tt, _ttDigShirtOff, 41, 43, -1,
			_ttDigShirtOff, 41, 43, 1);
		sendWSMessage_190000(_tt, 12);
		sendWSMessage_1a0000(_tt, 12);
		digi_play("603t02", 1, 255, 203);
		break;

	case 203:
		sendWSMessage_10000(1, _tt, _ttDigShirtOff, 43, 43, -1,
			_ttDigShirtOff, 43, 43, 1);
		_num1 = 13;
		digi_play("603r13", 1, 255, 204);
		break;

	case 204:
		_num1 = 14;
		kernel_timing_trigger(1, 300);
		_G(kernel).trigger_mode = KT_PARSE;
		conv_load("conv603a", 10, 10, 747);
		conv_set_shading(65);
		conv_export_value_curr(0, 0);
		conv_export_value_curr(0, 1);
		conv_play();
		_G(kernel).trigger_mode = KT_DAEMON;
		break;

	case 210:
		sendWSMessage_10000(1, _tt, _ttDigShirtOff, 35, 43, -1,
			_ttDigShirtOff, 43, 43, 1);
		digi_play("603t01", 1);
		kernel_timing_trigger(60, 212);
		break;

	case 212:
		if (player_said("take", "shirt") || player_said("take", "pole")) {
			sendWSMessage_140000(213);
		} else {
			kernel_timing_trigger(1, 213);
		}
		break;

	case 213:
		if (player_said("take", "shirt") || player_said("take", "pole"))
			series_unload(_ripAction);

		Common::strcpy_s(_G(player).verb, "talk to");
		Common::strcpy_s(_G(player).noun, "person in pit");
		_G(kernel).trigger_mode = KT_PARSE;
		ws_walk(311, 306, nullptr, 666, 10);
		_G(kernel).trigger_mode = KT_DAEMON;
		_G(player).disable_hyperwalk = false;
		break;

	case 300:
		kernel_trigger_dispatchx(kernel_trigger_create(301));
		break;

	case 301:
		switch (_num2) {
		case 0:
			switch (_num1) {
			case 0:
				sendWSMessage_10000(1, _ripley, _ripTalk, 1, 1, 300, _ripTalk, 1, 1, 0);
				break;
			case 2:
			case 8:
			case 14:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 1, 15, 300,
					_ripHandsBehindBack, 15, 15, 0);
				_num2 = 8;
				sendWSMessage_190000(_ripley, 4);
				break;

			case 5:
				sendWSMessage_10000(1, _ripley, _ripTalk, 1, 1, -1, _ripTalk, 1, 7, 4);
				sendWSMessage_1a0000(_ripley, 13);
				break;

			case 6:
				sendWSMessage_10000(1, _ripley, _ripTwoHandTalk, 1, 6, 300,
					_ripTwoHandTalk, 6, 6, 0);
				_num2 = 6;
				break;

			case 9:
				sendWSMessage_10000(1, _ripley, _ripHandChin, 1, 16, 300,
					_ripHandChin, 16, 16, 0);
				_num2 = 9;
				break;

			case 10:
				sendWSMessage_10000(1, _ripley, _ripHandsHips, 1, 12, 300,
					_ripHandsHips, 12, 12, 0);
				_num2 = 10;
				break;

			case 11:
				terminateMachineAndNull(_ripley);
				player_set_commands_allowed(true);
				ws_unhide_walker();
				break;

			case 12:
				terminateMachineAndNull(_ripley);
				ws_unhide_walker();
				terminateMachineAndNull(_shadow);
				ws_walk(670, 232, nullptr, -1, 3);
				break;

			default:
				break;
			}
			break;

		case 6:
			if (_num1 == 6) {
				sendWSMessage_10000(1, _ripley, _ripTwoHandTalk, 6, 6, 300,
					_ripTwoHandTalk, 6, 6, 0);
			} else {
				sendWSMessage_10000(1, _ripley, _ripTwoHandTalk, 6, 1, 300,
					_ripTwoHandTalk, 1, 1, 0);
				_num2 = 0;
			}
			break;

		case 8:
			switch (_num1) {
			case 1:
				terminateMachineAndNull(_tt);
				terminateMachineAndNull(_shirt);
				terminateMachineAndNull(_shadow);
				_sleeve = series_show("603SLEEV", 0x400, 16);
				hotspot_set_active("SLEEVE", true);
				hotspot_set_active("SHIRT", false);
				sendWSMessage_10000(1, _ripley, _rp02, 1, 28, 305, _rp02, 28, 28, 0);
				_num1 = 0;
				sendWSMessage_1e0000(_ripley, 0, 0);
				sendWSMessage_1f0000(_ripley, 0, 0);
				sendWSMessage_200000(_ripley, 100);
				sendWSMessage_210000(_ripley, 100);
				break;

			case 2:
				sendWSMessage_10000(1, _ripley, _rp04, 1, 31, 330, _rp04, 31, 31, 0);
				_num1 = 0;
				break;

			case 8:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 15, 300,
					_ripHandsBehindBack, 15, 15, 0);
				break;

			case 13:
			case 14:
				sendWSMessage_10000(1, _ripley, _rp01, 34, 35, 300, _rp01, 35, 35, 1);
				sendWSMessage_1e0000(_ripley, 0, 0);
				sendWSMessage_200000(_ripley, 100);
				sendWSMessage_1f0000(_ripley, 0, 0);
				sendWSMessage_210000(_ripley, 100);
				_num2 = 13;
				break;

			default:
				sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 1, 300,
					_rp04, 1, 1, 0);
				sendWSMessage_190000(_ripley, 4);
				_num2 = 0;
				break;
			}
			break;

		case 9:
			if (_num1 == 9) {
				sendWSMessage_10000(1, _ripley, _ripHandChin, 16, 16, 300,
					_ripHandChin, 16, 16, 0);
			} else {
				sendWSMessage_10000(1, _ripley, _ripHandChin, 16, 1, 300,
					_ripHandChin, 1, 1, 0);
				_num2 = 0;
			}
			break;

		case 10:
			if (_num1 == 9) {
				sendWSMessage_10000(1, _ripley, _ripHandsHips, 12, 12, 300,
					_ripHandChin, 12, 12, 0);
			} else {
				sendWSMessage_10000(1, _ripley, _ripHandsHips, 12, 1, 300,
					_ripHandsHips, 1, 1, 0);
				_num2 = 0;
			}
			break;

		case 13:
			switch (_num1) {
			case 5:
			case 13:
				sendWSMessage_10000(1, _ripley, _rp01, 36, 36, -1, _rp01, 36, 38, 1);
				sendWSMessage_1e0000(_ripley, 0, 0);
				sendWSMessage_200000(_ripley, 100);
				sendWSMessage_1f0000(_ripley, 0, 0);
				sendWSMessage_210000(_ripley, 100);
				sendWSMessage_1a0000(_ripley, 13);
				break;

			case 14:
				sendWSMessage_10000(1, _ripley, _rp01, 36, 36, 300, _rp01, 36, 36, 1);
				sendWSMessage_1e0000(_ripley, 0, 0);
				sendWSMessage_200000(_ripley, 100);
				sendWSMessage_1f0000(_ripley, 0, 0);
				sendWSMessage_210000(_ripley, 100);
				sendWSMessage_1a0000(_ripley, 30);
				break;
			default:
				sendWSMessage_10000(1, _ripley, _rp01, 35, 34, 300,
					_ripHandsBehindBack, 15, 15, 300);
				sendWSMessage_1e0000(_ripley, 0, 0);
				sendWSMessage_200000(_ripley, 100);
				_num2 = 8;
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 302:
		sendWSMessage_10000(1, _ripley, _rp01, 16, 36, 300,
			_ripHandsBehindBack, 15, 15, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		_num2 = 13;
		_num1 = 14;
		digi_play("603r12", 1, 255, 203);
		break;

	case 303:
		if (_val8 == 22)
			kernel_timing_trigger(1, 22);
		else
			_val8 = 5;
		break;

	case 304:
		sendWSMessage_10000(1, _ripley, _ripHandsBehindBack, 15, 15, 300,
			_ripHandsBehindBack, 15, 15, 0);
		_shadow = series_show("safari shadow 2", 0xf00, 128, -1, -1, 0, 59, 311, 308);
		break;

	case 305:
		sendWSMessage_10000(1, _ripley, _ripHandChin, 29, 36, 309,
			_rp02, 36, 36, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		digi_play("603_s01", 2, 255, 306);
		break;

	case 306:
		conv_resume();
		break;

	case 309:
		sendWSMessage_10000(1, _ripley, _rp02, 36, 30, 310, _rp02, 30, 30, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		break;

	case 310:
		sendWSMessage_10000(1, _ripley, _rp02, 30, 37, 311, _rp02, 37, 37, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		break;

	case 311:
		sendWSMessage_10000(1, _ripley, _rp02, 36, 31, 312, _rp02, 31, 31, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		break;

	case 312:
		sendWSMessage_10000(1, _ripley, _rp02, 32, 38, 313, _rp02, 38, 38, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		digi_play("603_s01a", 2);
		break;

	case 313:
		sendWSMessage_10000(1, _ripley, _rp02, 39, 124, 320, _rp02, 124, 124, 0);
		sendWSMessage_1e0000(_ripley, 0, 0);
		sendWSMessage_200000(_ripley, 100);
		digi_stop(2);
		break;

	case 320:
		terminateMachineAndNull(_ripley);
		_treesGoneHome = series_show("603rp02a", 0x100, 16);
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
			triggerMachineByHashCallback, "tt");
		sendWSMessage_10000(1, _tt, _tt03, 1, 39, 322, _tt03, 39, 39, 0);
		break;

	case 322:
		sendWSMessage_10000(1, _tt, _tt03, 40, 59, 324, _tt03, 59, 59, 0);
		conv_resume();
		break;

	case 324:
		sendWSMessage_10000(1, _tt, _tt03, 59, 82, 325, _tt03, 82, 82, 0);
		digi_play("603_s01b", 2);
		break;

	case 325:
		sendWSMessage_10000(1, _tt, _tt03, 83, 122, 326, _tt03, 122, 122, 0);
		terminateMachineAndNull(_treesGoneHome);
		terminateMachineAndNull(_shadow);
		ws_unhide_walker();
		ws_demand_facing(4);
		ws_walk(345, 300, nullptr, -1, 10);
		break;

	case 326:
		terminateMachineAndNull(_tt);
		_tt = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
			triggerMachineByHashCallback, "tt");
		sendWSMessage_10000(1, _tt, _ttD01, 1, 1, 400, _ttD01, 1, 1, 0);
		_val8 = 6;
		_val7 = 6;
		_ttShadow = series_show("tt walker shadow 4", 0xf00, 0, -1, -1, 0, 53, 291, 293);
		series_unload(_rp01);
		series_unload(_rp02);
		_trigger1 = 400;
		conv_resume();
		break;

	case 330:
		sendWSMessage_10000(1, _ripley, _rp04, 31, 31, -1, _rp04, 31, 31, 0);
		kernel_timing_trigger(100, 331);
		break;

	case 331:
		sendWSMessage_10000(1, _ripley, _rp04, 32, 41, 300, _rp04, 15, 15, 0);
		break;

	case 400:
		kernel_trigger_dispatchx(kernel_trigger_create(401));
		break;

	case 401:
		switch (_val7) {
		case 6:
			switch (_val8) {
			case 6:
				sendWSMessage_10000(1, _tt, _ttD01, 1, 1, 400, _ttD01, 1, 1, 0);
				break;

			case 8:
				if (imath_ranged_rand(1, 2) == 1) {
					sendWSMessage_10000(1, _tt, _ttD01, 2, 2, -1, _ttD01, 2, 24, 4);
				} else {
					sendWSMessage_10000(1, _tt, _ttD01, 25, 25, -1, _ttD01, 25, 60, 4);
				}

				sendWSMessage_1a0000(_tt, 13);
				break;

			case 11:
			case 12:
				sendWSMessage_10000(1, _tt, _ttD01, 61, 76, 400, _ttD01, 77, 77, 1);
				_val7 = 12;
				break;

			case 13:
				sendWSMessage_10000(1, _tt, _ttD01, 96, 121, 400, _ttD01, 1, 1, 1);
				_val8 = 6;
				break;

			case 14:
			case 15:
				sendWSMessage_10000(1, _tt, _ttD01, 85, 93, 400, _ttD01, 93, 93, 1);
				_val7 = 15;
				break;

			case 16:
				ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
				ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);
				sendWSMessage_10000(1, _tt, _tt03, 122, 112, 402, _tt03, 112, 112, 0);
				break;

			case 17:
				if (imath_ranged_rand(50, 150) < ++_val9) {
					_val9 = 0;

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _tt, _ttD01, 61, 76, 400, _ttD01, 77, 77, 1);
						_val7 = 12;
					} else {
						sendWSMessage_10000(1, _tt, _ttD01, 1, 1, 400, _ttD01, 1, 1, 0);
					}
				} else {
					kernel_timing_trigger(13, 400);
				}
				break;

			case 20:
				ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
				ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);
				sendWSMessage_10000(1, _tt, _tt03, 122, 112, 410, _tt03, 112, 112, 0);
				break;

			default:
				break;
			}
			break;

		case 12:
			switch (_val8) {
			case 11:
				sendWSMessage_10000(1, _tt, _ttD01, 77, 77, -1, _ttD01, 77, 84, 4);
				sendWSMessage_1a0000(_tt, 13);
				break;

			case 12:
				sendWSMessage_10000(1, _tt, _ttD01, 77, 77, 400, _ttD01, 77, 77, 0);
				break;

			case 17:
				if (imath_ranged_rand(50, 150) < ++_val9) {
					_val9 = 0;

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _tt, _ttD01, 76, 61, 400, _ttD01, 1, 1, 0);
						_val7 = 6;
					} else {
						sendWSMessage_10000(1, _tt, _ttD01, 77, 77, 400, _ttD01, 77, 77, 0);
					}
				} else {
					kernel_timing_trigger(13, 400);
				}
				break;

			default:
				sendWSMessage_10000(1, _tt, _ttD01, 76, 61, 400, _ttD01, 1, 1, 0);
				break;
			}
			break;

		case 15:
			switch (_val8) {
			case 15:
				sendWSMessage_10000(1, _tt, _ttD01, 93, 93, 400, _ttD01, 93, 93, 0);
				break;

			default:
				sendWSMessage_10000(1, _tt, _ttD01, 93, 85, 400, _ttD01, 1, 1, 0);
				_val7 = 6;

				if (_val8 == 14)
					_val8 = 6;
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 402:
		terminateMachineAndNull(_tt);
		digi_unload("603_S01");
		digi_unload("603_S01B");
		_trigger1 = 0;
		terminateMachineAndNull(_ttShadow);

		_tt = triggerMachineByHash_3000(8, 9, *SECTION6_NORMAL_DIRS, *SECTION6_SHADOW_DIRS,
			291, 293, 4, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 357, 283, 1, 405, 0);
		conv_resume();
		kernel_timing_trigger(180, 406);
		break;

	case 404:
		// Switch case not found :)
		break;

	case 405:
		sendWSMessage_10000(_tt, 501, 235, 8, -1, 1);
		break;

	case 406:
		ws_walk(459, 236, nullptr, 404, 2);
		break;

	case 410:
		terminateMachineAndNull(_tt);
		terminateMachineAndNull(_ttShadow);
		_tt = triggerMachineByHash_3000(8, 9, *SECTION6_NORMAL_DIRS, *SECTION6_SHADOW_DIRS,
			291, 293, 4, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 397, 286, 2, 415, 0);
		kernel_timing_trigger(200, 411);
		_G(player).disable_hyperwalk = true;
		break;

	case 411:
	case 511:
		_num1 = 12;
		kernel_timing_trigger(1, 300);
		break;

	case 412:
		sendWSMessage_10000(_tt, 670, 229, 3, -1, 1);
		disable_player_commands_and_fade_init(416);
		break;

	case 415:
		sendWSMessage_10000(_tt, 519, 244, 3, 412, 0);
		break;

	case 416:
		_G(game).setRoom(605);
		_G(flags)[V203] = 2;
		adv_kill_digi_between_rooms(false);
		digi_play("950_s28a", 3);
		break;

	case 500:
		kernel_trigger_dispatchx(kernel_trigger_create(501));
		break;

	case 501:
		switch (_val7) {
		case 6:
			switch (_val8) {
			case 7:
			case 8:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 11, 13, 500,
					_ttDigShirtOn, 13, 13, 0);
				_val7 = 18;
				break;

			case 17:
				if (imath_ranged_rand(1, 5) < ++_val9) {
					_val9 = 0;

					switch (imath_ranged_rand(1, 3)) {
					case 1:
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 1, 1, 500,
							_ttDigShirtOn, 1, 1, 0);
						break;
					case 2:
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 1, 10, 500,
							_ttDigShirtOn, 1, 1, 0);
						playRandomDigi(4);
						break;
					case 3:
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 11, 13, 500,
							_ttDigShirtOn, 13, 13, 0);
						playRandomDigi(4);
						break;
					default:
						break;
					}
				} else {
					kernel_timing_trigger(13, 500);
				}
				break;

			case 21:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 11, 13, 500,
					_ttDigShirtOn, 13, 13, 0);
				_val7 = 18;
				break;

			default:
				break;
			}
			break;

		case 7:
			switch (_val8) {
			case 7:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 38, 38, 500,
					_ttDigShirtOn, 38, 38, 0);
				break;

			case 8:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 39, 39, -1,
					_ttDigShirtOn, 39, 65, 4);
				sendWSMessage_1a0000(_tt, 13);
				break;

			case 17:
				if (imath_ranged_rand(3, 9) < ++_val9) {
					_val9 = 0;

					if (imath_ranged_rand(1, 2) == 1) {
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 38, 38, 500,
							_ttDigShirtOn, 38, 38, 0);
					} else {
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 66, 85, 500,
							_ttDigShirtOn, 1, 1, 0);
						_val7 = 6;
					}
				} else {
					kernel_timing_trigger(13, 500);
				}
				break;

			case 20:
				ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
				ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);
				sendWSMessage_10000(1, _tt, _tt03, 83, 111, 510, _tt03, 111, 111, 0);
				break;

			case 21:
				ws_walk_load_walker_series(SECTION6_NORMAL_DIRS, SECTION6_NORMAL_NAMES);
				ws_walk_load_shadow_series(SECTION6_SHADOW_DIRS, SECTION6_SHADOW_NAMES);
				sendWSMessage_10000(1, _tt, _tt03, 83, 112, 530, _tt03, 112, 112, 1);
				break;


			default:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 66, 85, 500,
					_ttDigShirtOn, 1, 1, 0);
				_val7 = 6;
				break;
			}
			break;

		case 12:
			switch (_val8) {
			case 7:
			case 8:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 17, 38, 500,
					_ttDigShirtOn, 38, 38, 0);
				_val7 = 7;
				break;

			case 17:
				if (imath_ranged_rand(1, 5) < ++_val9) {
					_val9 = 0;

					switch (imath_ranged_rand(1, 3)) {
					case 1:
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 13, 13, 500,
							_ttDigShirtOn, 13, 13, 0);
						break;
					case 2:
						sendWSMessage_10000(1, _tt, _ttDigShirtOn, 13, 11, 500,
							_ttDigShirtOn, 1, 1, 0);
						_val7 = 6;
						playRandomDigi(4);
						break;
					case 3:
						if (imath_ranged_rand(4, 10) < ++_val10) {
							_val10 = 0;
							sendWSMessage_10000(1, _tt, _ttDigShirtOn, 17, 38, 500,
								_ttDigShirtOn, 38, 38, 0);
							_val7 = 7;
						} else {
							sendWSMessage_10000(1, _tt, _ttDigShirtOn, 13, 13, 500,
								_ttDigShirtOn, 13, 13, 0);
						}
						break;
					default:
						break;
					}
				} else {
					kernel_timing_trigger(13, 500);
				}
				break;

			case 21:
				sendWSMessage_10000(1, _tt, _ttDigShirtOn, 17, 38, 500,
					_ttDigShirtOn, 38, 38, 0);
				_val7 = 7;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case 510:
		terminateMachineAndNull(_tt);
		_trigger1 = 0;
		_tt = triggerMachineByHash_3000(8, 9, *SECTION6_NORMAL_DIRS, *SECTION6_SHADOW_DIRS,
			291, 293, 4, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 357, 283, 3, 0x200, 0);
		kernel_timing_trigger(100, 511);
		_G(player).disable_hyperwalk = true;
		break;

	case 512:
		sendWSMessage_10000(_tt, 519, 244, 3, 513, 0);
		break;

	case 513:
		sendWSMessage_10000(_tt, 670, 229, 3, -1, 1);
		disable_player_commands_and_fade_init(516);
		break;

	case 516:
		_G(game).setRoom(605);
		_G(flags)[V203] = 2;
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28a", 3);
		break;

	case 530:
		terminateMachineAndNull(_tt);
		_trigger1 = 0;
		_tt = triggerMachineByHash_3000(8, 9, *SECTION6_NORMAL_DIRS, *SECTION6_SHADOW_DIRS,
			291, 293, 4, triggerMachineByHashCallback3000, "tt walker");
		sendWSMessage_10000(_tt, 397, 286, 2, 533, 0);
		kernel_timing_trigger(70, 531);
		_G(player).disable_hyperwalk = true;
		conv_resume();
		break;

	case 531:
		_num1 = 12;
		break;

	case 532:
		sendWSMessage_10000(_tt, 670, 229, 3, 534, 1);
		break;

	case 533:
		sendWSMessage_10000(_tt, 519, 244, 3, 532, 0);
		break;

	// TODO: Various
	default:
		break;
	}

	if (!_G(flags)[V191]) {
		player_update_info();

		if (_G(player_info).y < 284 && !_val3 && !_G(flags)[V203]) {
			_val3 = 1;
			_val8 = 22;
			_G(kernel).call_daemon_every_loop = false;
			_val7 = 1;
			kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
			player_set_commands_allowed(false);
		}
	}
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
				triggerMachineByHashCallback, "rip");
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
			triggerMachineByHashCallback, "rip");
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
			triggerMachineByHashCallback, "rip");
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
	} else if (takeFlag && player_said("sleeve") && takeSleeve()) {
		// No implementation
	} else if (takeFlag && player_said("NOTE ")) {
		digi_play("603r45", 1);
	} else if (takeFlag && player_said("NOTE") && takeNote()) {
		// No implementation
	} else if (takeFlag && player_said_any("shirt", "pole") && !_G(flags)[V203]) {
		player_set_commands_allowed(false);
		_val8 = 22;
		_G(kernel).call_daemon_every_loop = false;
		_val7 = 1;
		kernel_timing_trigger(1, 200, KT_DAEMON, KT_PARSE);
		_ripAction = series_load("RIP MED REACH 1HAND POS2");
		setGlobals1(_ripAction, 1, 15, 15, 15);
		sendWSMessage_110000(-1);
	} else if (takeFlag && player_said("pole") && takePole()) {
		// No implementation
	} else if (player_said("enter hut")) {
		if (_G(flags)[V203] == 4 || _G(flags)[V203] == 9) {
			digi_play("603r32", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_val8 = _val5 ? 12 : 7;
				_ripAction = series_load("RIP TREK HAT TIP POS4");
				kernel_timing_trigger(60, 2);
				break;

			case 2:
				_val8 = _val5 ? 11 : 8;
				digi_play("603t06", 1, 255, 3);
				break;

			case 3:
				_val8 = 17;
				_val9 = 5000;
				kernel_timing_trigger(1, _val5 ? 400 : 500, KT_DAEMON, KT_PARSE);
				setGlobals1(_ripAction, 1, 8, 8, 8);
				sendWSMessage_110000(4);
				break;

			case 4:
				digi_play("603r23", 1, 255, 5);
				sendWSMessage_140000(5);
				break;

			case 5:
				series_unload(_ripAction);
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		}
	} else if (player_said("enter")) {
		enter();
	} else if (lookFlag && player_said(" ")) {
		if (_G(flags)[V200]) {
			digi_play("603r01a", 1);
		} else {
			_G(flags)[V200] = 1;
			digi_play("603r01", 1);
		}
	} else if (lookFlag && player_said("hut")) {
		if (_G(flags)[V203] == 4 || _G(flags)[V203] == 9) {
			if (inv_object_is_here("TWELVETREES' NOTE") || _G(flags)[V203] == 9)
				digi_play("603r26", 1);
			else
				digi_play("603r02", 1);
		} else {
			digi_play("603r02", 1);
		}
	} else if (lookFlag && player_said("note ")) {
		_val6 = 1;
		hotspot_hide_all();
		_treesGoneHome = series_show("603 12TREES GONE HOME NOTE", 0, 16);
		hotspot_add_dynamic("LOOK AT", " ", 0, 0, 640, 480, 0);
		digi_play("603r44", 1);
		interface_hide();
	} else if (lookFlag && player_said("note") &&
			inv_object_is_here("TWELVETREES' NOTE")) {
		_val6 = 1;
		hotspot_hide_all();
		_treesGoneHome = series_show("603 tt map popup", 0, 16);
		hotspot_add_dynamic("LOOK AT", " ", 0, 0, 640, 480, 0);
		_G(flags)[V046] = 1;
		digi_play("603r28", 1);
		interface_hide();
	} else if (player_said("remote note")) {
		_val6 = 0;
		hotspot_restore_all();
		terminateMachineAndNull(_treesGoneHome);
		interface_show();
	} else if (_G(kernel).trigger == 555) {
		if (_G(flags)[V038])
			_G(flags)[V039] = 1;

		_G(flags)[GLB_TEMP_5] = 0;
		_G(game).setRoom(495);
		_G(flags)[V129] = 4;
	} else if (_G(kernel).trigger == 556) {
		digi_stop(1);
		digi_stop(2);
		digi_unload("603_S02");
		digi_unload("603_S02a");
		digi_unload("603_S02b");
		digi_unload("603_S02c");

		if (_G(flags)[GLB_TEMP_5]) {
			digi_stop(3);
			digi_unload("genrun");
			digi_preload("950_s28a");
		}

		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s28a", 3, 90);

		if (_G(flags)[V038])
			_G(flags)[V039] = 1;
		_G(game).setRoom(605);
	} else if (player_said("down")) {
		if (_G(flags)[V202])
			_G(flags)[V205] = 1;

		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(555);
	} else if (player_said("right")) {
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(556);
	} else if (player_said_any("TWELVETREES", "TWELVETREES ") &&
			player_said("POLE")) {
		digi_play("603r46", 1);
	} else if (player_said("go into")) {
		if (_G(flags)[V203] == 4 || _G(flags)[V203] == 9)
			digi_play("603r34", 1);
		else if (_G(flags)[V202])
			digi_play("603r25", 1);
		else
			digi_play("603r24", 1);
	} else if (lookFlag && player_said_any("twelvetrees", "twelvetrees ")) {
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("603r36", 1);
			break;
		case 2:
			digi_play("603r37", 1);
			break;
		case 3:
			digi_play("603r38", 1);
			break;
		case 4:
			digi_play("603r39", 1);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("pit")) {
		digi_play(_G(flags)[V202] ? "603r27" : "603r11", 1);
	} else if (lookFlag && player_said("sleeve") && inv_object_is_here("SLEEVE")) {
		digi_play("603r08", 1);
	} else if (lookFlag && player_said("pole") && inv_object_is_here("POLE")) {
		digi_play("603r09", 1);
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else {
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
				} else if (node == 0 || node == 1 || node == 2) {
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
			triggerMachineByHashCallback, "tt");
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
			triggerMachineByHashCallback, "rip");
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
	const char *sound = conv_sound_to_play();
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();

	if (_G(kernel).trigger == 1) {
		if (!(node == 13 && entry == 16)) {
			if (who <= 0) {
				if (node == 2 || node == 5 || node == 7)
					_G(flags)[V038] = 1;

				if (node == 10 || node == 12) {
					_val8 = 21;
					kernel_timing_trigger(1, 500, KT_DAEMON, KT_PARSE);
					return;
				} else {
					_val8 = 7;
					kernel_timing_trigger(1, 500, KT_DAEMON, KT_PARSE);
				}
			} else if (who == 1) {
				if (node == 11 && entry == 1) {
					_val8 = 21;
					kernel_timing_trigger(1, 500, KT_DAEMON, KT_PARSE);
					_num1 = 0;
					kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
					return;
				} else {
					_num1 = 0;
					kernel_timing_trigger(1, 300, KT_DAEMON, KT_PARSE);
				}
			}
		}

		conv_resume();
	} else if (sound) {
		if (node != 13 && node != 16) {
			if (who <= 0)
				_val8 = 8;
			else if (who == 1)
				_num1 = 5;
		}

		digi_play(sound, 1);
	} else {
		conv_resume();
	}
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

bool Room603::takeSleeve() {
	if (_G(flags)[V203] == 4 || _val4) {
		switch (_G(kernel).trigger) {
		case -1:
		case 666:
			if (inv_object_is_here("sleeve")) {
				player_set_commands_allowed(false);
				_ripAction = series_load("RIP MED REACH 1HAND POS2");
				setGlobals1(_ripAction, 1, 15, 15, 15);
				sendWSMessage_110000(2);
				return true;
			}
			break;

		case 2:
			hotspot_set_active("SLEEVE", false);
			inv_give_to_player("SLEEVE");
			kernel_examine_inventory_object("ping sleeve", 5, 1, 280, 220, 3, 0, -1);
			terminateMachineAndNull(_sleeve);
			return true;

		case 3:
			sendWSMessage_140000(5);
			return true;

		case 5:
			series_unload(_ripAction);
			player_set_commands_allowed(true);
			_val4 = 0;
			return true;

		default:
			break;
		}
	} else {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("sleeve")) {
				if (_val5)
					ws_walk(345, 300, nullptr, 1, 10);
				else
					ws_walk(311, 308, nullptr, 1, 10);
				return true;
			}
			break;

		case 1:
			player_set_commands_allowed(false);
			_val8 = _val5 ? 12 : 7;
			setGlobals1(_ripTalk, 1, 7, 1, 7, 1, 1, 1, 1, 1);
			sendWSMessage_110000(-1);
			digi_play("603r14", 1, 255, 2);
			return true;

		case 2:
			sendWSMessage_120000(-1);
			_val8 = _val5 ? 11 : 8;
			digi_play("603t04", 1, 255, 3);
			return true;

		case 3:
			_val8 = _val5 ? 12 : 7;
			kernel_timing_trigger(1, _val5 ? 400 : 500, KT_DAEMON, KT_PARSE);
			sendWSMessage_110000(-1);
			digi_play("603r14a", 1, 255, 5);
			return true;

		case 5:
			sendWSMessage_120000(-1);
			_val8 = _val5 ? 11 : 8;
			digi_play("603t05", 1, 255, 6);
			return true;

		case 6:
			_val8 = 17;
			kernel_timing_trigger(1, _val5 ? 400 : 500, KT_DAEMON, KT_PARSE);
			sendWSMessage_150000(-1);
			ws_walk(365, 298, nullptr, 666, 10);
			_val4 = 1;
			return true;

		default:
			break;
		}
	}

	return false;
}

bool Room603::takeNote() {
	switch (_G(kernel).trigger) {
	case -1:
		if (!inv_object_is_here("TWELVETREES' NOTE"))
			return false;

		player_set_commands_allowed(false);
		_ripAction = series_load("RIP MED HIGH REACHER POS2");
		setGlobals1(_ripAction, 1, 12, 12, 12);
		sendWSMessage_110000(2);
		break;

	case 2:
		hotspot_set_active("NOTE", false);
		inv_move_object("TWELVETREES' NOTE", NOWHERE);
		inv_move_object("TWELVETREES' MAP", NOWHERE);
		kernel_examine_inventory_object("PING TWELVETREES' NOTE", 5, 1, 205, 190, 3,
			_G(flags)[V046] ? nullptr : "603R28");
		_G(flags)[V046] = 1;
		terminateMachineAndNull(_note);
		break;

	case 3:
		kernel_timing_trigger(1, 4);
		break;

	case 4:
		kernel_examine_inventory_object("PING OBJ136", 5, 1, 205, 160, 5,
			player_been_here(623) ? "603R30" : "603R31");
		break;

	case 5:
		sendWSMessage_140000(6);
		break;

	case 6:
		series_unload(_ripAction);
		player_update_info();
		_ripAction = series_load("RIP SKETCHING IN NOTEBOOK POS 2");
		setGlobals1(_ripAction, 1, 17, 17, 17);
		sendWSMessage_110000(7);
		break;

	case 7:
		kernel_timing_trigger(60, 8);
		break;

	case 8:
		sendWSMessage_140000(9);
		break;

	case 9:
		series_unload(_ripAction);
		_G(flags)[V047] = 1;
		player_set_commands_allowed(true);
		break;

	default:
		return false;
	}

	return true;
}

bool Room603::takePole() {
	if (_G(flags)[V203] == 4) {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("pole")) {
				player_set_commands_allowed(false);
				_ripAction = series_load("RIP MED REACH 1HAND POS2");
				digi_play("603r29", 1, 255, 1);
				return true;
			}
			break;

		case 1:
			setGlobals1(_ripAction, 1, 15, 15, 15);
			sendWSMessage_110000(2);
			return true;

		case 2:
			hotspot_set_active("pole", false);
			inv_give_to_player("pole");
			kernel_examine_inventory_object("ping pole", 5, 1, 280, 220, 3);
			terminateMachineAndNull(_pole);

			if (inv_object_is_here("SLEEVE"))
				terminateMachineAndNull(_sleeve);
			return true;

		case 3:
			kernel_timing_trigger(1, 6);
			return true;

		case 4:
			sendWSMessage_140000(5);
			return true;

		case 5:
			series_unload(_ripAction);
			player_set_commands_allowed(true);
			return true;

		case 6:
			if (inv_object_is_here("SLEEVE")) {
				hotspot_set_active("SLEEVE", false);
				inv_give_to_player("SLEEVE");
				kernel_examine_inventory_object("ping sleeve", 5, 1, 280, 220, 4);
			} else {
				kernel_timing_trigger(1, 4);
			}
			return true;

		default:
			break;
		}
	} else {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("pole")) {
				player_set_commands_allowed(false);
				_val8 = _val5 ? 12 : 7;
				_ripAction = series_load("RIP MED REACH 1HAND POS2");
				return true;
			}
			break;

		case 2:
			_val8 = _val5 ? 11 : 8;
			digi_play("603t03", 1, 255, 3);
			return true;

		case 3:
			_val8 = 17;
			_val9 = 5000;
			_val5 = 0;
			kernel_timing_trigger(1, _val5 ? 400 : 500, KT_DAEMON, KT_PARSE);
			sendWSMessage_140000(5);
			return true;

		case 5:
			series_unload(_ripAction);
			player_set_commands_allowed(true);
			return true;

		default:
			break;
		}
	}

	return false;
}

void Room603::enter() {
	if (_G(flags)[V203] == 4) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripAction = series_load("rip trek med reach hand pos1");
			setGlobals1(_ripAction, 1, 10, 10, 10);
			sendWSMessage_110000(4);
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			_G(game).setRoom(604);
			digi_stop(1);

			if (_G(flags)[GLB_TEMP_5]) {
				adv_kill_digi_between_rooms(false);
				digi_play_loop("genrun", 3, 140, -1, 604);
			}
			break;

		default:
			break;
		}
	} else {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_val8 = _val5 ? 12 : 7;
			_ripAction = series_load("rip trek med reach hand pos1");
			setGlobals1(_ripAction, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			break;

		case 2:
			_val8 = _val5 ? 11 : 8;
			digi_play("603t07", 1, 255, 3);
			break;

		case 3:
			_val8 = 17;
			_val9 = 5000;
			_val5 = 0;
			kernel_timing_trigger(1, _val5 ? 400 : 500, KT_DAEMON, KT_PARSE);
			sendWSMessage_140000(5);
			break;

		case 5:
			series_unload(_ripAction);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	}
}

void Room603::playRandomDigi(int max) {
	static const char *SOUNDS[4] = {
		"603_s02", "603_s02a", "603_s02b", "603_s02c"
	};
	int num = imath_ranged_rand(1, max) - 1;
	if (num < 4)
		digi_play(SOUNDS[num], 2);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
