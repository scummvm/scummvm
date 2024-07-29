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

namespace M4 {
namespace Riddle {
namespace Rooms {

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
			_val12 = 2;

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
		_val12 = 1;
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
		case 1:
			setShadow5(false);
			if (player_said("giant matchstick"))
				ws_demand_location(610, 256);

			sendWSMessage_10000(_machine1, 706, 256, 5, 2, 1);
			break;

		default:
			break;
		}
	// TODO
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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
