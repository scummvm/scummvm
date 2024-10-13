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

#include "m4/riddle/rooms/section9/room902.h"
#include "m4/riddle/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room902::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room902::init() {
	player_set_commands_allowed(false);
	digi_preload("gunshot1", 902);
	digi_preload("gunshot2", 902);
	digi_preload("introcu-", 901);
	digi_play("introcu-", 1, 255, -1, 901);
	series_load("902 one frame hold");
	kernel_timing_trigger(1, 1);
	interface_hide();

	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
}

void Room902::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		kernel_timing_trigger(120, 2);
		kernel_timing_trigger(100, 789);
		break;

	case 2:
		series_ranged_play("902 TITLE", 1, 0, 0, 9, 100, 0xa00, 5, 3, 0);
		break;

	case 3:
		_title = series_ranged_play("902 TITLE", -1, 0, 9, 9, 100, 0xa00, 500, -1, 0);
		kernel_timing_trigger(180, 4);
		break;

	case 4:
		terminateMachineAndNull(_title);
		series_ranged_play("902 TITLE", 1, 2, 0, 9, 100, 0xa00, 5, 5, 0);
		break;

	case 5:
		kernel_timing_trigger(120, 6);
		break;

	case 6:
		_everything = series_stream("EVERYTHING MINUS TITLE FADE", 6, 0x100, 131);
		series_stream_break_on_frame(_everything, 121, 101);
		break;

	case 55:
		_G(game).setRoom(304);
		break;

	case 56:
		_G(game).setRoom(494);
		break;

	case 101:
		digi_play("gunshot1", 3);
		series_stream_break_on_frame(_everything, 144, 102);
		break;

	case 102:
		digi_play("gunshot2", 3, 255, -1, 902);
		break;

	case 131:
		series_plain_play("902 one frame hold", -1, 0, 100, 256, 3000);
		disable_player_commands_and_fade_init(132);
		break;

	case 132:
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(903);
		break;

	case 789:
		digi_stop(2);
		break;

	default:
		break;
	}

	bool flag = false;
	if (_G(MouseState).ButtonState) {
		_buttonDown = true;
	} else if (_buttonDown) {
		_buttonDown = false;
		flag = true;
	}

	if (flag)
		disable_player_commands_and_fade_init(55);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
