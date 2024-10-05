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

#include "m4/riddle/rooms/section9/room906.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room906::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room906::init() {
	_roomStates_pu = 40;
	player_set_commands_allowed(false);
	series_load("hold frame 906", -1, nullptr);
	interface_hide();
	digi_preload("906r01", -1);
	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
	_roomStates_field4 = series_stream("EVERYTHING IN 906", 6, 2560, 200);
	series_stream_break_on_frame(_roomStates_field4, 0, 2);
}

void Room906::daemon() {
	switch (_G(kernel).trigger) {
	case 2:
		series_stream_break_on_frame(_roomStates_field4, 10, 15);
		break;

	case 15:
		digi_play("906r01", 2, 225, -1, -1);
		break;

	case 30:
		_roomStates_pu -= 3;
		if (_roomStates_pu <= 0) {
			adv_kill_digi_between_rooms(false);
			_G(game).new_room = 907;
		} else {
			digi_change_panning(1, _roomStates_pu);
			kernel_timing_trigger(2, 30, nullptr);
		}

		break;

	case 55:
		_G(game).new_room = 304;
		_G(game).new_section = 3;
		break;

	case 56:
		_G(game).new_room = 494;
		_G(game).new_section = 4;
		break;

	case 200:
		series_plain_play("hold frame 906", -1, 0, 100, 256, 3000, -1, false);
		kernel_timing_trigger(120, 201, nullptr);
		break;

	case 201:
		disable_player_commands_and_fade_init(30);
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

void Room906::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
