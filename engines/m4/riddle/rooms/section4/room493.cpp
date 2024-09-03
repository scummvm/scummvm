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

#include "m4/riddle/rooms/section4/room493.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room493::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	_G(player).walker_in_this_scene = false;
}

void Room493::init() {
	player_set_commands_allowed(false);
	AddSystemHotkey(KEY_ESCAPE, escape_key_pressed);
	RemoveSystemHotkey(303);
	RemoveSystemHotkey(22);

	_panning = 255;

	if (_G(game).previous_room != 494) {
		digi_preload("lostcity");

		if (_G(game).previous_room != 917) {
			digi_preload("riptheme", 917);
			digi_play("riptheme", 3, 255, -1, 917);
		}
	}

	_credits = series_stream("game credits", 10, 256, 900);
	series_stream_break_on_frame(_credits, 10, 30);
	kernel_timing_trigger(360, 700);
}

void Room493::daemon() {
	static int32 FRAMES[13] = {
		4, 12, 24, 36, 48, 60, 68, 76, 88, 96, 108, 116, 128
	};

	switch (_G(kernel).trigger) {
	case 30:
		series_stream_break_on_frame(_credits, FRAMES[2], 50);
		setTrigger(40);
		break;

	case 50:
		series_stream_break_on_frame(_credits, FRAMES[3], 70);
		setTrigger(60);
		break;

	case 70:
		series_stream_break_on_frame(_credits, FRAMES[4] - 2, 90);
		setTrigger(80);
		break;

	case 90:
		series_stream_break_on_frame(_credits, FRAMES[5] - 2, 100);
		setTrigger(100);
		break;

	case 110:
		series_stream_break_on_frame(_credits, FRAMES[6] - 2, 130);
		setTrigger(120);
		break;

	case 130:
		series_stream_break_on_frame(_credits, FRAMES[7] - 2, 150);
		setTrigger(140);
		break;

	case 150:
		series_stream_break_on_frame(_credits, FRAMES[8] - 2, 170);
		setTrigger(160);
		break;

	case 170:
		series_stream_break_on_frame(_credits, FRAMES[9] - 2, 190);
		setTrigger(180);
		break;

	case 190:
		series_stream_break_on_frame(_credits, FRAMES[10] - 2, 210);
		setTrigger(200);
		break;

	case 210:
		series_stream_break_on_frame(_credits, FRAMES[11] - 2, 230);
		setTrigger(220);
		break;

	case 230:
		series_stream_break_on_frame(_credits, FRAMES[12] - 2, 250);
		setTrigger(240);
		break;

	case 250:
		setTrigger(260);
		break;

	case 666:
		AddSystemHotkey(303, Hotkeys::show_version);
		AddSystemHotkey(22, Hotkeys::show_version);
		_G(game).setRoom(494);
		break;

	case 700:
		if (checkStrings()) {
			digi_stop(3);
			kernel_timing_trigger(10, 800);
		} else {
			kernel_timing_trigger(65, 700);
		}
		break;

	case 800:
		digi_play("lostcity", 3, 255, 810, 917);
		break;

	case 810:
		digi_play_loop("riptheme", 3, 255, -1, 917);
		break;

	case 900:
		if (_G(game).previous_room == 494)
			disable_player_commands_and_fade_init(920);
		else
			disable_player_commands_and_fade_init(910);
		break;

	case 910:
		_panning -= 4;
		if (_panning > 0) {
			digi_change_panning(3, _panning);
			kernel_timing_trigger(2, 910);
		} else {
			AddSystemHotkey(303, Hotkeys::show_version);
			AddSystemHotkey(22, Hotkeys::show_version);
			_G(game).setRoom(494);
		}
		break;

	case 920:
		adv_kill_digi_between_rooms(false);
		AddSystemHotkey(303, Hotkeys::show_version);
		AddSystemHotkey(22, Hotkeys::show_version);
		_G(game).setRoom(494);
		break;

	case 40:
	case 60:
	case 80:
	case 100:
	case 120:
	case 140:
	case 160:
	case 180:
	case 200:
	case 220:
	case 240:
	case 260:
		crunch();
		break;

	default:
		break;
	}
}

void Room493::escape_key_pressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(910);
}

void Room493::setTrigger(int trigger) {
	series_stream_check_series(_credits, 30000);
	kernel_timing_trigger(900, trigger);
}

void Room493::crunch() {
	series_stream_check_series(_credits, 10);
	ws_OverrideCrunchTime(_credits);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
