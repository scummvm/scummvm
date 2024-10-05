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
#include "m4/riddle/rooms/section9/room903.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room903::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room903::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

void Room903::init() {
	player_set_commands_allowed(false);
	digi_preload("INTMOAN", 901);
	digi_preload("ricochet", 902);
	series_load("903 hold frame");
	interface_hide();
	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
	kernel_timing_trigger(150, 100, nullptr);
}

void Room903::daemon() {
	switch (_G(kernel).trigger) {
	case 10:
		kernel_timing_trigger(300, 20, nullptr);
		break;

	case 20:
	case 201:
		disable_player_commands_and_fade_init(30);
		break;

	case 30:
		digi_stop(3);
		adv_kill_digi_between_rooms(false);
		_G(game).new_room = 905;
		break;

	case 55:
		_G(game).new_room = 304;
		_G(game).new_section = 3;
		break;

	case 56:
		_G(game).new_room = 494;
		_G(game).new_section = 4;
		break;

	case 100:
		_roomStates_field4 = series_stream("EVERYTHING IN 903", 6, 2560, 200);
		series_stream_break_on_frame(_roomStates_field4, 59, 102);
		break;

	case 102:
		digi_play("gunshot2", 3, 255, -1, 902);
		series_stream_break_on_frame(_roomStates_field4, 63, 104);
		break;

	case 104:
		digi_play("ricochet", 2, 255, -1, 902);
		series_stream_break_on_frame(_roomStates_field4, 292, 106);
		break;

	case 106:
		digi_play("INTMOAN", 2, 255, -1, 901);
		break;

	case 200:
		digi_unload("introcu-");
		digi_unload("gunshot1");
		digi_unload("gunshot2");
		digi_unload("ricochet");
		MaxMem(nullptr);
		series_plain_play("903 hold frame", -1, 0, 100, 256, 3000, -1, false);
		kernel_timing_trigger(120, 201, nullptr);
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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
