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

#include "m4/riddle/rooms/section3/room309.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room309::preload() {
	_G(player).walker_type = 0;
	_G(player).shadow_type = 0;
	_G(player).walker_in_this_scene = false;
}

void Room309::init() {
	digi_preload("303_s02");
	player_set_commands_allowed(false);
	series_stream("first pass creature", 5, 0x100, 101);
	digi_play("303_s02", 3);
	kernel_timing_trigger(300, 100);
}

void Room309::daemon() {
	switch (_G(kernel).trigger) {
	case 100:
		digi_play("303_s02", 3);
		kernel_timing_trigger(300, 100);
		break;

	case 101:
		disable_player_commands_and_fade_init(123);
		break;

	case 123:
		_G(game).setRoom(303);
		break;

	default:
		break;
	}
}

void Room309::parser() {
	if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case 0:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			break;

		case 1:
			midi_stop();
			digi_stop(3);
			_G(game).setRoom(303);
			break;

		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
