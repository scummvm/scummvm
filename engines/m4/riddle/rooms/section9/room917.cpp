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

#include "m4/riddle/rooms/section9/room917.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room917::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room917::init() {
	player_set_commands_allowed(false);
	adv_kill_digi_between_rooms(false);
	if (_G(flags)[V291]) {
		digi_preload("950_s47", 950);
		digi_preload("907r02", 950);
		_roomStates_tt = series_stream("917zep01", 12, 256, -1);
		kernel_timing_trigger(180, 1, nullptr);
		digi_play_loop("950_s47", 3, 70, -1, 950);
	} else {
		digi_preload("908f01", 908);
		digi_preload("908w01", 908);
		digi_preload("908d01", 908);
		digi_preload("riptheme", 917);
		digi_preload("950_s48", 950);
		digi_preload("950_s48a", 950);
		digi_preload("950_s48b", 950);
		digi_preload("950_s48c", 950);
		digi_preload("950_s50", 950);
		_roomStates_tt = series_stream("917zep02", 10, 256, -1);
		series_stream_break_on_frame(_roomStates_tt, 1, 31);
		digi_play("950_s50", 3, 180, 50, -1);
	}
	interface_hide();
	_G(kernel).call_daemon_every_loop = true;
}

void Room917::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_play("907r02", 2, 255, 3, 907);
		kernel_timing_trigger(180, 2, nullptr);
		break;

	case 2:
		pal_fade_init(&_G(master_palette)[0], 0, 255, 0, 90, -1);
		break;

	case 3:
		_G(flags)[V291] = true;
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s47", 3, 70, -1, 950);
		digi_stop(1);
		digi_stop(2);
		digi_unload("907R02");
		_G(game).new_room = 918;
		break;

	case 11:
		digi_play("908f01", 1, 255, 12, 908);
		break;

	case 12:
		kernel_timing_trigger(45, 13, nullptr);
		break;

	case 13:
		digi_play("908w01", 1, 255, 14, 908);
		break;

	case 14:
		kernel_timing_trigger(65, 15, nullptr);
		break;

	case 31:
		series_stream_break_on_frame(_roomStates_tt, 9, 32);
		digi_play("950_s48", 2, 180, -1, -1);
		break;

	case 32:
		series_stream_break_on_frame(_roomStates_tt, 19, 33);
		digi_play("950_s48a", 2, 220, -1, -1);
		break;

	case 33:
		series_stream_break_on_frame(_roomStates_tt, 26, 34);
		digi_play("950_s48b", 2, 240, -1, -1);
		break;

	case 34:
		series_stream_break_on_frame(_roomStates_tt, 57, 60);
		digi_play("950_s48c", 2, 255, -1, -1);
		kernel_timing_trigger(45, 35, nullptr);
		break;

	case 35:
		digi_play("908d01", 1, 255, -1, 908);
		break;

	case 50:
		digi_play("950_s50", 3, 110, -1, -1);
		break;

	case 60:
		digi_play("950_s48b", 2, 255, -1, -1);
		series_stream_break_on_frame(_roomStates_tt, 85, 70);
		break;

	case 70:
		series_stream_break_on_frame(_roomStates_tt, 118, 80);
		digi_play("riptheme", 3, 255, -1, 917);
		break;

	case 80:
		disable_player_commands_and_fade_init(90);
		digi_stop(1);
		digi_stop(2);
		break;

	case 90:
		adv_kill_digi_between_rooms(false);
		digi_unload("907F01");
		digi_unload("907W01");
		digi_unload("907D01");
		digi_unload("950_S48");
		digi_unload("950_S48a");
		digi_unload("950_S48b");
		digi_unload("950_S48c");
		digi_unload("950_S50");
		MaxMem(nullptr);
		_G(game).new_room = 493;
		_G(game).new_section = 4;
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
