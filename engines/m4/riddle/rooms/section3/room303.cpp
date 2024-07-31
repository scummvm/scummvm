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

#include "m4/riddle/rooms/section3/room303.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"
#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *SAID[][2] = {
	{ "IRON MAIDEN",     "303R29" },
	{ "MUMMY WITH HAIR", "303R30" },
	{ "MAN WITH HAMMER", "303R33" },
	{ "TICKET WINDOW",   "303R34" },
	{ "PICTURE",         "303R35" },
	{ nullptr, nullptr }
};

void Room303::preload() {
	LoadWSAssets("OTHER SCRIPT");

	if (_G(flags)[V000]) {
		_G(art_base_override) = player_been_here(201) ?
			"EXHIBIT HALL-TREK" : "EH TREK NO SNAKE";
		_G(use_alternate_attribute_file) = true;
		_G(player).walker_type = 1;
		_G(player).shadow_type = 1;
		_G(player).walker_in_this_scene = true;

	} else {
		_G(player).walker_type = 0;
		_G(player).shadow_type = 0;
		_G(player).walker_in_this_scene = true;

		if (_G(game).room_id == 352) {
			_G(player).walker_in_this_scene = false;
			_G(player).disable_hyperwalk = true;
		}
	}
}

void Room303::init() {
	static const int16 NORMAL_DIRS[] = { 200, 201, 202, -1 };
	static const char *NORMAL_NAMES[] = {
		"Feng Li walk pos3",
		"Feng Li walk pos4",
		"Feng Li walk pos5"
	};
	static const int16 SHADOW_DIRS[] = { 210, 211, 212, -1 };
	static const char *SHADOW_NAMES[] = {
		"candleman shadow3",
		"candleman shadow4",
		"candleman shadow5"
	};

	static const int16 NORMAL_DIRS2[] = { 220, 221, 222, -1 };
	static const char *NORMAL_NAMES2[] = {
		"mei chen ny walker pos3",
		"mei chen ny walker pos4",
		"mei chen ny walker pos5"
	};
	static const int16 SHADOW_DIRS2[] = { 230, 231, 232, -1 };
	static const char *SHADOW_NAMES2[] = {
		"candleman shadow3",
		"candleman shadow4",
		"candleman shadow5"
	};

	_val1 = _lonelyFlag = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val3 = 0;
		_val4 = -1;
		_triggerMode1 = _triggerMode2 = KT_DAEMON;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
	}

	if (player_been_here(301)) {
		hotspot_set_active("MEI CHEN", false);
		hotspot_set_active("FENG LI", false);
		hotspot_set_active("COVER", false);
	}

	if (_G(game).previous_room != 304) {
		_door = series_show_sprite("DOOR", 0, 0xf05);
	}

	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	assert(game_buff_ptr);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (!player_been_here(301)) {
			kernel_load_variant(_val13 ? "303lock1" : "303lock2");
			setFengActive(_val13);
			loadHands();
			setShadow4(true);

			ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
			ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
			loadClasped();

			if (_val13) {
				_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
					triggerMachineByHashCallbackNegative, "fl");
				setShadow5(true);
			} else {
				_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 706, 256, 86, 0xc00, 0,
					triggerMachineByHashCallbackNegative, "fl state machine");
				setShadow5(false);
			}

			sendWSMessage_10000(1, _machine1, _clasped4, 1, 1, 400,
				_clasped4, 1, 6, 0);
			_val14 = _val15 = 1;
		}

		playSeries();
		break;

	case 301:
		MoveScreenDelta(game_buff_ptr, -641, 0);
		playSeries();

		kernel_timing_trigger(1, 9);
		break;

	case 304:
		if (_G(flags)[V001]) {
			_G(player).disable_hyperwalk = true;
			LoadWSAssets("303 FL SCRIPT");

			_G(camera_reacts_to_player) = true;
			MoveScreenDelta(game_buff_ptr, -110, 0);
			_val12 = KT_DAEMON;

			player_set_commands_allowed(false);
			playSeries(false);

			_door = series_show_sprite("DOOR", 7, 0xf05);
			ws_demand_location(393, 260, 5);

			ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
			ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
			_machine1 = triggerMachineByHash_3000(8, 2, NORMAL_DIRS, SHADOW_DIRS,
				470, 226, 9, triggerMachineByHashCallback3000, "fl");
			_val13 = 1;

			kernel_load_variant("303lock1");
			setFengActive(true);
			loadClasped();

			ws_walk_load_walker_series(NORMAL_DIRS2, NORMAL_NAMES2);
			ws_walk_load_shadow_series(SHADOW_DIRS2, SHADOW_NAMES2);

			_hands4 = triggerMachineByHash_3000(8, 3, NORMAL_DIRS2, SHADOW_DIRS2,
				445, 215, 7, triggerMachineByHashCallback3000, "mc");
			_gestTalk4 = series_load("mei ny lft hand gest talk pos4");
			kernel_timing_trigger(1, 107);

		} else {
			player_set_commands_allowed(false);
			kernel_timing_trigger(1, 10);
			_door = series_show_sprite("DOOR", 7, 0xf05);

			if (!player_been_here(301)) {
				kernel_load_variant("303lock1");
				setFengActive(true);
				loadHands();
				setShadow4(true);

				ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
				ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
				loadClasped();

				_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
					triggerMachineByHashCallbackNegative, "fl");
				sendWSMessage_10000(1, _machine1, _clasped4, 1, 1, 400,
					_clasped4, 1, 6, 0);

				_val14 = _val15 = _val13 = 1;
				setShadow5(true);
			}

			playSeries();
		}
		break;

	case 305:
		player_set_commands_allowed(false);
		ws_demand_location(28, 267, 3);
		playSeries();

		if (!player_been_here(301)) {
			kernel_timing_trigger(1, 6);
			loadHands();
			setShadow4(true);

			ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
			ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
			loadClasped();

			_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
				triggerMachineByHashCallbackNegative, "fl");
			sendWSMessage_10000(1, _machine1, _clasped1, 1, 16, 400,
				_clasped1, 1, 6, 0);
			_val14 = _val15 = 1;
			setShadow5(true);
			_val13 = 1;

			kernel_load_variant("303lock1");
			setFengActive(true);
		} else {
			kernel_timing_trigger(1, 6);
		}
		break;

	case 309:
		interface_show();
		ws_demand_location(230, 258, 10);
		player_set_commands_allowed(false);

		if (!player_been_here(301)) {
			loadHands();
			setShadow4(true);
			ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
			ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
			_val13 = 1;
			loadClasped();

			_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
				triggerMachineByHashCallbackNegative, "fl");
			sendWSMessage_10000(1, _machine1, _clasped4, 1, 16, 400,
				_clasped4, 1, 6, 0);

			_val14 = _val15 = 1;
			setShadow5(true);
			kernel_load_variant("303lock1");
			setFengActive(true);
		}

		playSeries();

		if (player_been_here(301)) {
			_ripBends = series_load("rip trek bends to viewer");
			setGlobals1(_ripBends, 17, 17, 17, 17, 1, 17, 1, 1, 1, 1);
			sendWSMessage_110000(3);

		} else {
			_ripBends = series_load("RIP BENDS TO SEE CREATURE");
			setGlobals1(_ripBends, 26, 1, 1, 1);
		}
		break;

	case 352:
		player_set_commands_allowed(false);
		interface_hide();
		digi_preload("303r02");
		digi_preload("303m02");
		digi_preload("303f01");

		AddSystemHotkey(KEY_ESCAPE, escapePressed);
		_G(kernel).call_daemon_every_loop = true;
		digi_stop(3);

		series_plain_play("303cow1", -1, 0, 100, 0, 9);
		series_show_sprite("doorknob relocation", 0, 0xf04);
		series_load("303 final frame");
		_machine2 = series_stream("EVERYTHING IN 303", 5, 17, 21);
		series_stream_break_on_frame(_machine2, 3, 18);

		MoveScreenDelta(game_buff_ptr, -320, 0);
		break;

	default:
		_val12 = KT_PARSE;
		kernel_load_variant("303lock1");
		setFengActive(true);
		ws_demand_location(145, 289, 3);
		loadHands();
		setShadow4(true);

		ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);

		_val13 = 1;
		loadClasped();
		_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
			triggerMachineByHashCallbackNegative, "fl");
		sendWSMessage_10000(1, _machine1, _clasped4, 1, 1, 400,
			_clasped4, 1, 6, 0);
		_val14 = _val15 = 1;
		setShadow5(true);

		playSeries();
		break;
	}
}

void Room303::daemon() {
	switch (_G(kernel).trigger) {
	case 3:
		sendWSMessage_120000(4);
		break;

	case 4:
		sendWSMessage_150000(61);
		break;

	case 5:
	case 61:
		ws_demand_location(230, 258);
		player_set_commands_allowed(true);
		break;

	case 6:
		ws_walk(128, 267, nullptr, 75, 3);
		break;

	// TODO
	default:
		break;
	}
}

void Room303::pre_parser() {
	if (player_said("open") && player_been_here(301)) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room303::parser() {
	bool lookFlag = player_said_any("look", "look at");

	if (player_said("conv303b")) {
		conv303b();
	} else if (player_said("conv303a")) {
		conv303a();
	} else if (lookFlag && _G(flags)[V000] == 0 && _val13 &&
			player_said_any("cow", "giant matchstick")) {
		switch (_G(kernel).trigger) {
		case -1:
			_val15 = 5;
			player_set_commands_allowed(false);
			break;

		case 1:
			setShadow5(false);
			if (player_said("giant matchstick"))
				ws_demand_location(610, 256);

			sendWSMessage_10000(_machine1, 706, 256, 5, 2, 1);
			break;

		case 2:
			setShadow5Alt(true);
			_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 706, 256, 86, 0xc00, 0,
				triggerMachineByHashCallbackNegative, "fl state machine");

			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _machine1, _clasped1, 1, 16, 400,
				_clasped4, 1, 6, 0);
			_val14 = _val15 = 1;
			setFengActive(false);
			_val13 = 0;

			kernel_load_variant("303lock2");
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(1, 666);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("torso tomato") &&
			!_G(flags)[V081] && !_G(flags)[V000] && !_val13) {
		switch (_G(kernel).trigger) {
		case 1:
			setShadow5Alt(false);
			sendWSMessage_10000(_machine1, 480, 256, 7, 2, 1);
			break;

		case 2:
			setShadow5(true);
			sendWSMessage_60000(_machine1);
			_machine1 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 480, 256, 86, 0xc00, 1,
				triggerMachineByHashCallbackNegative, "fl state machine");

			_G(kernel).trigger_mode = KT_DAEMON;
			sendWSMessage_10000(1, _machine1, _clasped1, 1, 16, 400,
				_clasped4, 1, 6, 0);
			_val14 = _val15 = 1;
			setFengActive(true);
			_val13 = 1;

			kernel_load_variant("303lock1");
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(1, 666);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("puffball")) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r37", 1);
		} else if (_G(flags)[V080]) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("suit chin in hand pos4");
				setGlobals4(_chinTalk4, 15, 15, 15);
				sendWSMessage_C0000(0);
				digi_play("303r37", 1, 255, 2);
				break;

			case 2:
				sendWSMessage_B0000(3);
				break;

			case 3:
				series_unload(_chinTalk4);
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("rip hand to chin talk pos4");
				setGlobals4(_chinTalk4, 15, 16, 19);
				sendWSMessage_C0000(0);
				digi_play("303F13", 1, 255, 1);
				_val15 = 2;
				break;

			case 1:
				_val15 = 1;
				kernel_timing_trigger(30, 2);
				break;

			case 2:
				sendWSMessage_D0000();
				digi_play("303r26", 1, 255, 3);
				break;

			case 3:
				sendWSMessage_E0000();
				sendWSMessage(0xe0000, 0, _G(my_walker), 0, nullptr, 1);
				kernel_timing_trigger(30, 4);
				break;

			case 4:
				digi_play("303F14", 1, 255, 5);
				_val15 = 2;
				break;

			case 5:
				_val15 = 1;
				kernel_timing_trigger(30, 6);
				break;

			case 6:
				sendWSMessage_D0000();
				digi_play("303r27", 1, 255, 7);
				break;

			case 7:
				sendWSMessage_B0000(8);
				break;

			case 8:
				series_unload(_chinTalk4);
				player_set_commands_allowed(true);
				_G(flags)[V080] = 1;
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("copper tank") &&
			(_G(flags)[V000] == 1 || _val13)) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r31", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("suit hand in pocket pos2");
				setGlobals4(_chinTalk4, 19, 19, 19);
				sendWSMessage_C0000(0);
				digi_play("303r31", 1, 255, 2);
				break;

			case 2:
				sendWSMessage_B0000(3);
				break;

			case 3:
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("torso tomato")) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r28a", 1);
		} else if (_G(flags)[V081]) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("suit chin in hand pos4");
				setGlobals4(_chinTalk4, 15, 15, 15);
				sendWSMessage_C0000(-2);
				digi_play("303r28a", 1, 255, 1);
				break;

			case 1:
				sendWSMessage_B0000(3);
				break;

			case 3:
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("rip suit talker pos4");
				setGlobals4(_chinTalk4, 1, 1, 5);
				sendWSMessage_C0000(-1);
				digi_play("303F15", 1, 255, 1);
				_val15 = 2;
				break;

			case 2:
				sendWSMessage_D0000();
				digi_play("303r28", 1, 255, 3);
				break;

			case 3:
				sendWSMessage_B0000(4);
				break;

			case 4:
				series_unload(_chinTalk4);
				player_set_commands_allowed(true);
				_G(flags)[V081] = 1;
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("cow")) {
		if (_G(flags)[V086] == 2) {
			_G(flags)[V086] = 1;
		} else if (_G(flags)[V086] == 3) {
			_G(flags)[V086] = 0;
		}

		if (_G(flags)[V000] == 1) {
			digi_play("303r23", 1);
		} else if (_G(flags)[V082]) {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				digi_play("303F09", 1, 255, 1);
				_val15 = 2;
				break;

			case 1:
				_val15 = 1;
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("rip hand to chin talk pos4");
				setGlobals4(_chinTalk4, 15, 16, 19);
				sendWSMessage_C0000(0);
				digi_play("303F10", 1, 255, 6);
				_val15 = 2;
				break;

			case 1:
				digi_play("303r24", 1, 255, 3);
				sendWSMessage_D0000();
				_val15 = 0;
				break;

			case 3:
				kernel_timing_trigger(30, 4);
				break;

			case 4:
				sendWSMessage_B0000(7);
				_val15 = 3;
				digi_play("303f11", 1, 255, 5);
				break;

			case 5:
				_val15 = 1;
				player_set_commands_allowed(true);
				_G(flags)[V082] = 1;
				break;

			case 6:
				_val15 = 1;
				kernel_timing_trigger(30, 1);
				break;

			case 7:
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("giant matchstick")) {
		if (_G(flags)[V000] == 1) {
			digi_play("303r36", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
			case 666:
				player_set_commands_allowed(false);
				_chinTalk4 = series_load("rip suit talker pos4");
				setGlobals4(_chinTalk4, 1, 1, 5);
				sendWSMessage_C0000(1);
				break;

			case 2:
				sendWSMessage_D0000();
				digi_play("303r25", 1, 255, 3);
				break;

			case 3:
				digi_play("303F12", 1, 255, 4);
				_val15 = 2;
				sendWSMessage_B0000(0);
				break;

			case 4:
				_val15 = 1;
				series_unload(_chinTalk4);
				break;

			default:
				break;
			}
		}
	} else if (player_said("open")) {
		if (player_been_here(301)) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(409, 266, nullptr, 1, 1);
				break;

			case 1:
				player_set_commands_allowed(false);
				_med1 = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(_med1, 1, 10, 10, 10, 1);
				sendWSMessage_110000(2);
				break;

			case 2:
				sendWSMessage_140000(4);
				terminateMachineAndNull(_door);
				series_plain_play("DOOR", 1, 0, 100, 0xf05, 7, 3);
				digi_play("303_s01", 1);
				break;

			case 3:
				series_show_sprite("DOOR", 7, 0xf05);
				break;

			case 4:
				ws_walk(417, 232, nullptr, -1, 2);
				break;

			case 5:
				_G(game).setRoom(304);
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info();
				ws_hide_walker();
				_ripsh1 = series_place_sprite("ripsh1", 0,
					_G(player_info).x, _G(player_info).y, _G(player_info).scale,
					_G(player_info).depth + 1);
				_med1 = series_load("SUIT DOOR OPENER POS1");

				series_play_xy("SUIT DOOR OPENER POS1", 1, 0, 405, 266,
					_G(player_info).scale, _G(player_info).depth, 7, 1);
				break;

			case 2:
				series_show_sprite("DOOR", 7, 0xf05);
				break;

			case 3:
				ws_unhide_walker();
				terminateMachineAndNull(_ripsh1);
				kernel_timing_trigger(10, 4);
				break;

			case 4:
				ws_walk(417, 232, nullptr, -1, 2);
				disable_player_commands_and_fade_init(6);
				break;

			case 6:
				digi_stop(3);
				_G(game).setRoom(304);
				break;

			default:
				break;
			}
		}
	} else if (lookFlag && player_said("cobra case")) {
		digi_play(player_been_here(201) ? "203r38" : "303r32", 1);
	} else if ((lookFlag || player_said("peer into")) &&
			player_said("copper tank viewer")) {
		if (player_been_here(301)) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripBends = series_load("rip trek bends to viewer");
				setGlobals1(_ripBends, 1, 17, 17, 17, 1);
				sendWSMessage_110000(-1);
				disable_player_commands_and_fade_init(1);
				break;

			case 1:
				_G(game).setRoom(309);
				interface_hide();
				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripBends = series_load("RIP BENDS TO SEE CREATURE");
				setGlobals1(_ripBends, 1, 26, 26, 26);
				sendWSMessage_110000(1);
				break;

			case 1:
				disable_player_commands_and_fade_init(2);
				break;

			case 2:
				_G(game).setRoom(309);
				interface_hide();
				break;
			default:
				break;
			}
		}
	} else if (player_said("TALK MEI CHEN")) {
		player_set_commands_allowed(false);
		_suit1 = series_load("SUIT CHIN IN HAND POS2");
		_suit2 = series_load("RIP SUIT RT HAND GEST TALK POS2");
		_meiLips = series_load("MEI PUTS FINGER TO LIPS");
		_ripGesture = series_load("RIP HNDS HIPS GEST TALK");
		player_update_info();
		ws_hide_walker();

		_ripsh2 = series_show("ripsh2", 0xf00, 128, -1, 0,
			_G(player_info).scale, _G(player_info).x, _G(player_info).y);
		_machine3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
			0x500, 1, triggerMachineByHashCallbackNegative, "rip");

		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _machine3, 1, 1, 1, 300, 1, 1, 1, 0);

		_val17 = _val16 = 0;
		_G(kernel).trigger_mode = KT_PARSE;

		conv_load("conv303b", 10, 10, 747);
		conv_play(conv_get_handle());
		_val11 = 4;
		_val16 = 0;

	} else if (_G(kernel).trigger == 747) {
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 749);
		_lonelyFlag = false;
		_val7 = 1;
		_val11 = 0;
		_val16 = 5;

	} else if (_G(kernel).trigger == 749) {
		midi_stop();

	} else if (player_said("TALK FL")) {
		_suit1 = series_load("SUIT CHIN IN HAND POS2");
		_suit2 = series_load("RIP SUIT RT HAND GEST TALK POS2");
		_ripGesture = series_load("RIP HNDS HIPS GEST TALK");
		player_update_info();
		ws_hide_walker();

		if (_val13) {
			_ripsh2 = series_show("ripsh2", 0xf00, 0, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			_machine3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
				0x500, 0, triggerMachineByHashCallbackNegative, "rip");

		} else {
			_ripsh2 = series_show("ripsh2", 0xf00, 128, -1, -1, 0,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y);
			_machine3 = TriggerMachineByHash(1, 1, 0, 0, 0, 0,
				_G(player_info).x, _G(player_info).y, _G(player_info).scale + 1,
				0x500, 1, triggerMachineByHashCallbackNegative, "rip");
		}

		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _machine3, 1, 1, 1, 300, 1, 1, 1, 0);
		_val16 = _val17 = 0;

		_G(kernel).trigger_mode = KT_PARSE;
		player_set_commands_allowed(false);
		conv_load("conv303a", 10, 10, 748);

		conv_export_value(conv_get_handle(), _G(flags)[V086], 0);
		conv_play();
		_val15 = 1;
		_val16 = 0;

	} else if (_G(kernel).trigger == 748) {
		_G(flags)[V082] = 1;
		_G(flags)[V083] = 1;
		midi_fade_volume(0, 120);
		kernel_timing_trigger(120, 749);
		_lonelyFlag = false;
		_val15 = 4;
		_val16 = 5;

	} else if (player_said("exit left")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			digi_stop(3);
			_G(game).setRoom(305);
			break;

		default:
			break;
		}
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// Already handled
	} else if (player_said("exit right")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			digi_stop(3);
			_G(game).setRoom(301);
			break;

		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room303::loadHands() {
	_hands1 = series_load("MC NY hands behind back pos4");
	_hands2 = series_load("MC NY hand on hip pos4");
	_hands3 = series_load("MC NY hand out talk pos4");
	_hands4 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 357, 255, 86, 0xf00, 0,
		triggerMachineByHashCallbackNegative, "mc");

	_G(kernel).trigger_mode = KT_DAEMON;
	sendWSMessage_10000(1, _hands4, _hands1, 1, 1, 200,
		_hands1, 1, 1, 0);
	_val10 = _val11 = 0;
}

void Room303::loadClasped() {
	_clasped1 = series_load("hands clasped pos5");
	_clasped2 = series_load("shrug head shake pos5");
	_clasped3 = series_load("hands clasped bow pos5");
	_clasped4 = series_load("hands clasped flame loop pos5");
}

void Room303::setFengActive(bool flag) {
	for (HotSpotRec *hotspot = _G(currentSceneDef).hotspots;
		hotspot; hotspot = hotspot->next) {
		if (!strcmp(hotspot->vocab, "FENG LI")) {
			if (flag) {
				hotspot->active = hotspot->lr_x < 600;
			} else {
				hotspot->active = hotspot->lr_x > 600;
			}
		}
	}
}

void Room303::setShadow4(bool active) {
	if (active) {
		_shadow4 = series_place_sprite("candleman shadow4", 0, 360, 252, -86, 0xe06);
	} else {
		terminateMachineAndNull(_shadow4);
	}
}

void Room303::setShadow5(bool active) {
	if (active) {
		_shadow5 = series_place_sprite("candleman shadow5", 0, 480, 256, -84, 0xe06);
	} else {
		terminateMachineAndNull(_shadow5);
	}
}

void Room303::setShadow5Alt(bool active) {
	if (active) {
		_shadow5 = series_place_sprite("candleman shadow5", 0, 706, 256, 84, 0xe06);
	} else {
		terminateMachineAndNull(_shadow5);
	}
}

void Room303::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

void Room303::playSeries(bool cow) {
	series_plain_play("SPINNING TOMATO MAN", -1, 0, 100, 0, 7);
	series_plain_play("PUFFBALL", -1, 0, 100, 0, 8);
	series_plain_play("CREATURE FEATURE LONG VIEW", 1, 0, 100, 0xf05, 7, 70);
}

void Room303::conv303a() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	if (_G(kernel).trigger == 1) {
		if (who <= 0) {
			_val15 = node != 3 || entry ? 1 : 0;

		} else if (who == 1) {
			if ((node == 0 && entry != 0) || (node == 0 && entry == 1)) {
				if (!_lonelyFlag) {
					midi_play("ppstreet", 140, 1, -1, 949);
				}
			}

			if (node != 1 || entry != 1) {
				if (node != 3 || entry != 1)
					_val16 = 0;
			} else {
				_val16 = 3;
			}
		}

		conv_resume();

	} else if (!conv_sound_to_play()) {
		conv_resume();

	} else {
		if (who <= 0) {
			if (node == 3 && !entry)
				_val16 = 2;
			if ((node != 3 || entry != 1) && (node != 3 || entry))
				_val15 = 2;
			else
				_val15 = 3;

		} else if (who == 1) {
			if (node != 1 || entry != 1) {
				if (node != 5 || entry) {
					if (node != 3 || entry != 1)
						_val16 = 1;
				} else {
					_val16 = 0;
				}
			} else {
				_val16 = 4;
			}
		}

		digi_play(sound, 1, 255, 1);
	}
}

void Room303::conv303b() {
	int who = conv_whos_talking();
	int node = conv_current_node();
	int entry = conv_current_entry();
	const char *sound = conv_sound_to_play();

	switch (_G(kernel).trigger) {
	case 1:
		if (who <= 0) {
			_val11 = 4;

			if (node == 1 && entry == 0) {
				digi_unload("08_01n01");
				digi_unload("08_02n01");
			} else if (node == 2 && entry == 2) {
				_val16 = 0;
			} else if (node == 1 && entry == 2) {
				digi_preload("com119");
				_ripPonders = series_stream("303 rip reacts", 4, 0, 667);
				series_stream_break_on_frame(_ripPonders, 5, 7);
				return;
			}

		} else if (who == 1) {
			if ((node == 0 && entry == 0) || (node == 0 && entry == 1)) {
				if (!_lonelyFlag) {
					midi_play("lonelyme", 140, 1, -1, 949);
					_lonelyFlag = true;
				}
			}

			if (node == 0 && entry == 0) {
				series_unload(2);
				series_unload(3);
				series_unload(4);
				_ripPonders = series_stream("303pu01", 4, 0x100, 666);
				series_stream_break_on_frame(_ripPonders, 5, 700);
			} else if (node == 1 && entry == 2) {
				// No implementation
			} else if ((node == 2 && entry == 0) || (node == 2 && entry == 2)) {
				_val16 = 3;
			} else {
				_val16 = 0;
			}
		}
		break;

	case 2:
		series_stream_check_series(_ripPonders, 20);
		series_stream_break_on_frame(_ripPonders, 27, 3);
		return;

	case 3:
		series_stream_check_series(_ripPonders, 5);
		return;

	case 4:
		_val11 = 5;
		return;

	case 5:
		digi_play("com119", 1, 255, 6);
		return;

	case 6:
		digi_unload("com119");
		return;

	case 7:
		_val16 = 0;
		series_stream_break_on_frame(_ripPonders, 22, 5);
		return;

	case 666:
		kernel_timing_trigger(1, 668);
		break;

	case 667:
		kernel_timing_trigger(1, 670);
		break;

	case 668:
		conv_resume();
		digi_preload("08_01n01");
		digi_preload("08_02n01");
		_ripPonders = series_stream("303 rip ponders", 5, 0, -1);
		series_stream_break_on_frame(_ripPonders, 5, 2);
		return;

	case 670:
		series_load("test1");
		series_load("test3");
		series_load("test4");
		series_load("test5");
		break;

	case 700:
		_val16 = 0;
		break;

	default:
		if (sound) {
			if (who <= 0) {
				if (node != 2 || entry != 1)
					_val11 = 5;
			} else if (who == 1) {
				if (node == 1 && entry == 2) {
					_val16 = 2;
				} else if (node == 2 && entry == 1) {
					_val11 = 6;
					kernel_timing_trigger(150, 4);
				} else if ((node == 2 && entry == 0) ||
						(node == 2 && entry == 2)) {
					_val16 = 4;
				} else {
					_val16 = 1;
				}
			}

			digi_play(sound, 1, 255, 1);
			return;
		}
		break;
	}

	conv_resume();
}

void Room303::priestTalkCallback(frac16 myMessage, machine *sender) {
	Room303 *room = (Room303 *)g_engine->_activeRoom;
	auto oldMode = _G(kernel).trigger_mode;
	int trigger = myMessage >> 16;

	if (trigger > 0) {
		_G(kernel).trigger_mode = room->_val12;
		kernel_timing_trigger(1, trigger);
		_G(kernel).trigger_mode = oldMode;
	}
}

void Room303::priestTalk(bool flag, int trigger) {
	ws_hide_walker(_machine1);
	_G(globals)[GLB_TEMP_1] = _clasped1 << 24;
	_G(globals)[GLB_TEMP_2] = 0xD << 24;
	_G(globals)[GLB_TEMP_3] = _clasped3 << 24;
	_G(globals)[GLB_TEMP_4] = 0xD << 24;
	_G(globals)[GLB_TEMP_9] = _clasped4 << 24;
	_G(globals)[GLB_TEMP_10] = 0xD << 24;
	_G(globals)[GLB_TEMP_5] = (flag ? 480 : 705) << 16;
	_G(globals)[GLB_TEMP_6] = 1 << 24;
	_G(globals)[V007] = _G(globals)[V006] *
		((1 << 24) - _G(globals)[V002]) + _G(globals)[V004];
	_G(globals)[GLB_TEMP_8] = (flag ? 1 : 0) << 16;
	_G(globals)[V008] = trigger << 16;
	_G(globals)[GLB_TEMP_12] = 0xdc28;

	_priestTalk = TriggerMachineByHash(32, nullptr, 0, -1,
		priestTalkCallback, false, "fl priest/talk");
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
