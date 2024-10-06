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

#include "m4/m4.h"
#include "m4/riddle/rooms/section9/room905.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room905::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room905::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

void Room905::init() {
	_roomStates_field4 = -1;
	warning("Room 905 - large STUB in init");
	series_load("905 hold frame", -1, nullptr);
	g_engine->adv_camera_pan_step(3);
	player_set_commands_allowed(false);
	interface_hide();
	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
	_roomStates_tt = series_stream("RIP EATS SAND WITH HIS ANUS", 5, 2560, 666);
	series_stream_break_on_frame(_roomStates_tt, 50, 3);
}

void Room905::daemon() {
	switch (_G(kernel).trigger) {
	case 3:
		g_engine->camera_shift_xy(0, 0);
		kernel_timing_trigger(120, 4, nullptr);
		digi_play("INTMOAN", 1, 255, -1, 901);
		kernel_timing_trigger(60, 100, nullptr);
		break;

	case 4:
		if (g_engine->game_camera_panning())
			kernel_timing_trigger(120, 4, nullptr);
		else
			kernel_timing_trigger(150, 20, nullptr);
		break;

	case 20:
		disable_player_commands_and_fade_init(30);
		break;

	case 30:
		_roomStates_field4 -= 3;
		if (_roomStates_field4 <= 40) {
			adv_kill_digi_between_rooms(false);
			_G(game).new_room = 906;
		} else {
			digi_change_panning(1, _roomStates_field4);
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

	case 666:
		series_plain_play("905 hold frame", -1, 0, 100, 256, 3000);
		break;

	case 1000:
		digi_stop(2);
		break;

	default:
		break;
	}
}

void Room905::shutdown() {
	warning("STUB - Room905::shutdown");
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
