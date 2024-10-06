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

#include "m4/riddle/rooms/section9/room908.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room908::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room908::init() {
	player_set_commands_allowed(false);
	digi_preload("950_s12", 950);
	digi_preload("908m01", 908);
	digi_preload("908m01a", 908);
	digi_preload("908m01b", 908);
	digi_preload("908r01", 908);
	digi_preload("908r01a", 908);
	digi_preload("908r01b", 908);
	digi_preload("908m02", 908);
	digi_preload("908r02", 908);
	digi_preload("908m03", 908);
	digi_preload("908m04", 908);
	digi_preload("908r03", 908);
	digi_preload("908r03a", 908);
	digi_preload("908m05", 908);
	digi_preload("908m05a", 908);
	digi_preload("909m01", 909);
	digi_preload("910m01", 910);
	digi_preload("911s01", 911);
	digi_preload("911k01", 911);
	digi_preload("912k01", 912);
	digi_preload("912s01", 912);
	digi_preload("950_s13", 950);
	_roomStates_field8 = series_load("Hold frame 908", -1, nullptr);
	interface_hide();
	AddSystemHotkey(KEY_ESCAPE, escapePressed);
	_G(kernel).call_daemon_every_loop = true;
	_roomStates_field4 = series_stream("conversation half frames", 8, 256, 10);
	series_stream_break_on_frame(_roomStates_field4, 1, 71);
	kernel_timing_trigger(60, 200, nullptr);
}

void Room908::daemon() {
	switch (_G(kernel).trigger) {
	case 10:
		_roomStates_field4 = series_place_sprite("Hold frame 908", 0, 0, 0, 100, 0);
		break;

	case 30:
		digi_stop(1);
		digi_stop(2);
		digi_stop(3);
		_G(game).new_room = 352;
		_G(game).new_section = 3;
		break;

	case 55:
		_G(game).new_room = 304;
		_G(game).new_section = 3;
		break;

	case 56:
		_G(game).new_room = 494;
		_G(game).new_section = 4;
		break;

	case 71:
		series_stream_break_on_frame(_roomStates_field4, 15, 72);
		digi_play("908m01", 2, 255, -1, 908);
		break;

	case 72:
		series_stream_break_on_frame(_roomStates_field4, 40, 73);
		digi_play("908m01a", 2, 255, -1, 908);
		break;

	case 73:
		series_stream_break_on_frame(_roomStates_field4, 51, 74);
		digi_play("908m01b", 2, 255, -1, 908);
		break;

	case 74:
		series_stream_break_on_frame(_roomStates_field4, 59, 75);
		series_stream_check_series(_roomStates_field4, 9);
		digi_play("908r01", 2, 255, -1, 908);
		break;

	case 75:
		series_stream_break_on_frame(_roomStates_field4, 66, 76);
		series_stream_check_series(_roomStates_field4, 8);
		digi_play("908r01a", 2, 255, -1, 908);
		break;

	case 76:
		series_stream_break_on_frame(_roomStates_field4, 82, 77);
		series_stream_check_series(_roomStates_field4, 10);
		digi_play("908r01b", 2, 255, -1, 908);
		break;

	case 77:
		series_stream_break_on_frame(_roomStates_field4, 95, 78);
		series_stream_check_series(_roomStates_field4, 8);
		digi_play("908m02", 2, 255, -1, 908);
		break;

	case 78:
		series_stream_break_on_frame(_roomStates_field4, 121, 79);
		series_stream_check_series(_roomStates_field4, 9);
		digi_play("908r02", 2, 255, -1, 908);
		break;

	case 79:
		series_stream_break_on_frame(_roomStates_field4, 138, 80);
		series_stream_check_series(_roomStates_field4, 8);
		digi_play("908m03", 2, 255, -1, 908);
		break;

	case 80:
		series_stream_break_on_frame(_roomStates_field4, 158, 82);
		series_stream_check_series(_roomStates_field4, 9);
		digi_play("908m04", 2, 255, -1, 908);
		break;

	case 82:
		series_stream_break_on_frame(_roomStates_field4, 175, 83);
		series_stream_check_series(_roomStates_field4, 10);
		digi_play("908r03", 2, 255, -1, 908);
		break;

	case 83:
		series_stream_break_on_frame(_roomStates_field4, 192, 84);
		series_stream_check_series(_roomStates_field4, 9);
		digi_play("908r03a", 2, 255, -1, 908);
		break;

	case 84:
		series_stream_break_on_frame(_roomStates_field4, 200, 85);
		series_stream_check_series(_roomStates_field4, 8);
		digi_play("908m05", 2, 255, -1, 908);
		break;

	case 85:
		series_stream_break_on_frame(_roomStates_field4, 233, 86);
		series_stream_check_series(_roomStates_field4, 9);
		digi_play("908m05a", 2, 255, -1, 908);
		break;

	case 86:
		series_stream_break_on_frame(_roomStates_field4, 250, 87);
		series_stream_check_series(_roomStates_field4, 8);
		digi_play("909m01", 2, 255, -1, 909);
		break;

	case 87:
		digi_play("910m01", 2, 255, -1, 910);
		series_stream_break_on_frame(_roomStates_field4, 251, 88);
		break;

	case 88:
		series_stream_check_series(_roomStates_field4, 85);
		series_stream_break_on_frame(_roomStates_field4, 252, 89);
		break;

	case 89:
		series_stream_check_series(_roomStates_field4, 8);
		series_stream_break_on_frame(_roomStates_field4, 259, 91);
		break;

	case 91:
		series_stream_break_on_frame(_roomStates_field4, 278, 92);
		digi_play("950_s13", 1, 255, -1, 950);
		break;

	case 92:
		series_stream_break_on_frame(_roomStates_field4, 295, 93);
		digi_play("911s01", 2, 255, -1, 911);
		break;

	case 93:
		series_stream_break_on_frame(_roomStates_field4, 318, 94);
		digi_play("911k01", 2, 255, -1, 911);
		break;

	case 94:
		series_stream_break_on_frame(_roomStates_field4, 339, -1);
		series_stream_check_series(_roomStates_field4, 10);
		digi_play("912k01", 2, 255, 95, 912);
		break;

	case 95:
		digi_play("912s01", 2, 255, 96, 912);
		break;

	case 96:
		disable_player_commands_and_fade_init(30);
		break;

	case 200:
		digi_stop(2);
		digi_play_loop("950_s12", 1, 255, -1, 950);
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

void Room908::parser() {
	if (player_said("TSA", nullptr, nullptr))
		_G(player).command_ready = false;
}

void Room908::escapePressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}
} // namespace Rooms
} // namespace Riddle
} // namespace M4
