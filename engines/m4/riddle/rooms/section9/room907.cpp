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

#include "m4/riddle/rooms/section9/room907.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room907::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room907::init() {
	player_set_commands_allowed(false);
	digi_preload("950_s11", 950);
	digi_preload("907r01", -1);


	_roomStates_tt = series_stream("zep flies low over sea", 6, 2560, -1);
	series_stream_break_on_frame(_roomStates_tt, 8, 1);
	interface_hide();
	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
	kernel_timing_trigger(10, 8, nullptr);
}

void Room907::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		series_stream_break_on_frame(_roomStates_tt, 70, 30);
		kernel_timing_trigger(60, 29, nullptr);
		break;

	case 8:
		digi_stop(1);
		digi_unload("INTMOAN");
		digi_play("950_s11", 2, 255, -1, -1);
		kernel_timing_trigger(120, 10, nullptr);
	break;

	case 10:
		digi_stop(1);
		break;

	case 30:
		kernel_timing_trigger(60, 31, nullptr);
		break;

	case 31:
		disable_player_commands_and_fade_init(40);
		break;

	case 29:
		digi_play("907r01", 3, 255, 30, -1);
		break;

	case 40:
		adv_kill_digi_between_rooms(false);
		_G(game).new_room = 908;
		break;

	case 55:
		_G(game).new_room = 304;
		_G(game).new_section = 3;
		break;

	case 56:
		_G(game).new_room = 494;
		_G(game).new_section = 4;
		break;

	default:
		break;
	}

	if (_G(MouseState).ButtonState)
		_buttonDown = true;
	else if (_buttonDown) {
		_buttonDown = false;
		disable_player_commands_and_fade_init(55);
	}
}

void Room907::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
