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

#include "m4/riddle/rooms/section6/room625.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room625::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28c");
		break;

	case 624:
		ws_demand_location(-30, 310, 3);
		ws_walk(30, 310, nullptr, 1, 3);
		player_set_commands_allowed(false);
		break;

	default:
		digi_preload("950_28c");
		ws_demand_location(-30, 310, 3);
		ws_walk(30, 310, nullptr, 1, 3);
		player_set_commands_allowed(false);
		break;
	}

	digi_play_loop("950_s28c", 3);
}

void Room625::parser() {
	Maze::parser();

	if (player_said("journal", "STARFISH")) {
		if (_G(flags)[V208]) {
			digi_play("203r54", 1);
		} else {
			if (_G(kernel).trigger == 6)
				_G(flags)[V208] = 1;
			sketchInJournal(0);
		}

		_G(player).command_ready = false;
	} else {
		checkExitLeft(624);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
