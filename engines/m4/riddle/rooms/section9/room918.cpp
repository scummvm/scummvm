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

#include "m4/riddle/rooms/section9/room918.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room918::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	_G(player).walker_in_this_scene = false;
}

void Room918::init() {
	player_set_commands_allowed(false);
	digi_preload("908m06", 908);
	digi_preload("908R04", 908);
	digi_preload("908m07", 908);
	digi_preload("908r05", 908);
	digi_preload("908r05a", 908);
	digi_preload("908m07z", 908);
	digi_preload("908m08", 908);
	digi_preload("908m08a", 908);
	digi_preload("908r06", 908);
	digi_preload("908m09", 908);
	digi_preload("908m09a", 908);
	digi_preload("908m09b", 908);
	digi_preload("908r07", 908);
	digi_preload("908r07a", 908);
	digi_preload("908r07b", 908);
	digi_preload("908r07c", 908);
	digi_preload("908m10", 908);
	digi_preload("908m10a", 908);
	digi_preload("908m10b", 908);
	digi_preload("950_s48e", 950);
	digi_preload("950_s48f", 950);
	digi_preload("950_s48a", 950);
	digi_preload("950_s50", 950);
	digi_preload("950_s47", 950);
	interface_hide();
	_G(kernel).call_daemon_every_loop = true;
	digi_play_loop("950_s47", 3, 70, -1, 950);
	_roomStates_field4 = series_stream("918all", 5, 256, 10);
	series_stream_break_on_frame(_roomStates_field4, 3, 1);
}

void Room918::daemon() {
	switch (_G(kernel).trigger) {
		
	case 1:
		series_stream_break_on_frame(_roomStates_field4, 35, 30);
		digi_play("908m06", 2, 255, -1, 908);
		break;

	case 30:
		series_stream_break_on_frame(_roomStates_field4, 64, 40);
		series_stream_check_series(_roomStates_field4, 6);
		digi_play("908R04", 1, 255, -1, 908);
		break;

	case 40:
		series_stream_break_on_frame(_roomStates_field4, 103, 50);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908m07", 1, 255, -1, 908);
		break;

	case 50:
		series_stream_break_on_frame(_roomStates_field4, 121, 60);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908r05", 1, 255, -1, 908);
		break;

	case 60:
		series_stream_break_on_frame(_roomStates_field4, 133, 70);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908r05a", 1, 255, -1, 908);
		break;

	case 70:
		series_stream_break_on_frame(_roomStates_field4, 144, 75);
		digi_play("908m07z", 2, 255, -1, 908);
		break;

	case 75:
		series_stream_break_on_frame(_roomStates_field4, 174, 80);
		digi_play("908m08", 2, 255, -1, 908);
		break;

	case 80:
		series_stream_check_series(_roomStates_field4, 5);
		series_stream_break_on_frame(_roomStates_field4, 210, 90);
		digi_play("908m08a", 2, 255, -1, 908);
		break;

	case 90:
		series_stream_break_on_frame(_roomStates_field4, 245, 100);
		series_stream_check_series(_roomStates_field4, 6);
		digi_play("908r06", 1, 255, -1, 908);
		break;

	case 100:
		series_stream_break_on_frame(_roomStates_field4, 271, 102);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908m09", 2, 255, -1, 908);
		break;

	case 102:
		series_stream_break_on_frame(_roomStates_field4, 291, 104);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908m09a", 2, 255, -1, 908);
		break;

	case 104:
		series_stream_break_on_frame(_roomStates_field4, 326, 110);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908m09b", 2, 255, -1, 908);
		break;

	case 110:
		series_stream_break_on_frame(_roomStates_field4, 333, 112);
		series_stream_check_series(_roomStates_field4, 6);
		digi_play("908r07", 1, 255, -1, 908);
		break;

	case 112:
		series_stream_break_on_frame(_roomStates_field4, 339, 114);
		series_stream_check_series(_roomStates_field4, 6);
		digi_play("908r07a", 1, 255, -1, 908);
		break;

	case 114:
		series_stream_break_on_frame(_roomStates_field4, 354, 116);
		series_stream_check_series(_roomStates_field4, 6);
		digi_play("908r07b", 1, 255, -1, 908);
		break;

	case 116:
		series_stream_break_on_frame(_roomStates_field4, 382, 120);
		series_stream_check_series(_roomStates_field4, 6);
		digi_play("908r07c", 1, 255, -1, 908);
		break;

	case 120:
		series_stream_break_on_frame(_roomStates_field4, 390, 122);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908m10", 2, 255, -1, 908);
		break;

	case 122:
		series_stream_break_on_frame(_roomStates_field4, 435, 130);
		series_stream_check_series(_roomStates_field4, 5);
		digi_play("908m10a", 2, 255, -1, 908);
		break;

	case 130:
		digi_play("908m10b", 2, 255, -1, 908);
		series_stream_break_on_frame(_roomStates_field4, 470, 290);
		break;

	case 200:
		digi_stop(2);
		break;

	case 290:
		digi_play("950_s48e", 1, 255, -1, -1);
		series_stream_break_on_frame(_roomStates_field4, 480, 300);
		break;

	case 300:
		series_stream_check_series(_roomStates_field4, 7);
		series_stream_break_on_frame(_roomStates_field4, 486, 320);
		digi_play("950_s48f", 2, 255, 310, -1);
		break;

	case 310:
		digi_play("950_s48a", 1, 255, 325, -1);
		digi_stop(3);
		digi_play("950_s50", 3, 180, -1, -1);
		break;

	case 320:
		disable_player_commands_and_fade_init(true);
		break;

	case 325:
		digi_play("908f01", 1, 255, 340, 908);
		break;

	case 340:
		adv_kill_digi_between_rooms(false);
		digi_stop(1);
		digi_stop(2);
		digi_unload("908m06");
		digi_unload("908R04");
		digi_unload("908m07");
		digi_unload("908r05");
		digi_unload("908r05a");
		digi_unload("908m07z");
		digi_unload("908m08");
		digi_unload("908m08a");
		digi_unload("908r06");
		digi_unload("908m09");
		digi_unload("908m09a");
		digi_unload("908m09b");
		digi_unload("908r07");
		digi_unload("908r07a");
		digi_unload("908r07b");
		digi_unload("908r07c");
		digi_unload("908m10");
		digi_unload("908m10a");
		digi_unload("908m10b");
		digi_unload("950_s48e");
		digi_unload("950_s48f");
		digi_unload("950_s48a");
		digi_unload("950_s47");
		_G(game).new_room = 917;
		break;
	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
