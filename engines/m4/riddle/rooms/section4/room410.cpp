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

#include "m4/riddle/rooms/section4/room410.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room410::preload() {
	_G(player).walker_type = 0;
	_G(player).shadow_type = 0;
	_G(player).walker_in_this_scene = false;
}

void Room410::init() {
	digi_preload("410r04");
	digi_preload("410r05");
	digi_preload("410r01");
	digi_preload("410t01");
	digi_preload("410r02");
	digi_preload("410t02");
	digi_preload("410r03");
	digi_preload("410_s02");
	_val1 = 140;

	midi_play("danzig1", 0, 1, -1, 949);
	interface_hide();
	_pu = series_stream("410PU01", 8, 0x100, -1);
	series_stream_break_on_frame(_pu, 2, 100);
	digi_play("410_s02", 3, 140);
}

void Room410::daemon() {
	switch (_G(kernel).trigger) {
	case 20:
		disable_player_commands_and_fade_init(25);
		break;

	case 25:
	case 229:
		interface_show();
		_G(game).setRoom(402);
		break;

	case 100:
		if (_G(player).been_here_before) {
			series_stream_break_on_frame(_pu, 56, 300);
			digi_play("410r05", 1, 255, 103);
		} else {
			series_stream_break_on_frame(_pu, 71, 104);
			digi_play("410r04", 1, 255, 103);
		}
		break;

	case 103:
		midi_play("danzig1", 255, 1, -1, 949);
		break;

	case 104:
		series_stream_break_on_frame(_pu, 102, 105);
		series_stream_check_series(_pu, 5);
		digi_play("410r01", 1);
		break;

	case 105:
		series_stream_break_on_frame(_pu, 116, 110);
		series_stream_check_series(_pu, 3000);
		digi_play("410t01", 1, 255, 106);
		break;

	case 106:
		series_stream_check_series(_pu, 7);
		ws_OverrideCrunchTime(_pu);
		break;

	case 110:
		series_stream_break_on_frame(_pu, 126, 112);
		digi_play("410r02", 1);
		break;

	case 112:
		series_stream_break_on_frame(_pu, 136, 120);
		digi_play("410t02", 1);
		break;

	case 120:
		terminateMachineAndNull(_pu);
		_pu = series_stream("410PU02", 7, 0x100, -1);
		series_stream_break_on_frame(_pu, 8, 125);
		break;

	case 125:
		series_stream_break_on_frame(_pu, 19, 127);
		digi_play("410r03", 1, 255, 225);
		break;

	case 127:
		series_stream_check_series(_pu, 3000);
		break;

	case 225:
		kernel_timing_trigger(40, 227);
		break;

	case 227:
		disable_player_commands_and_fade_init(229);
		break;

	case 300:
		series_stream_check_series(_pu, 3000);
		kernel_timing_trigger(1, 305);
		disable_player_commands_and_fade_init(-1);
		break;

	case 305:
		_val1 -= 4;
		if (_val1 >= 0) {
			digi_change_panning(3, _val1);
			kernel_timing_trigger(2, 305);
		} else {
			interface_show();
			_G(game).setRoom(402);
		}
		break;

	default:
		break;
	}
}

void Room410::parser() {
	if (player_said("exit")) {
		if (_G(kernel).trigger <= 0) {
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
		} else if (_G(kernel).trigger == 1) {
			digi_stop(3);
			_G(game).setRoom(303);
		}
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
