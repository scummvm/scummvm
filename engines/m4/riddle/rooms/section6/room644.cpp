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

#include "m4/riddle/rooms/section6/room644.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room644::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28c");
		break;

	case 638:
		ws_demand_location(343, 351, 1);
		break;

	case 643:
		ws_demand_location(-30, 313, 3);
		ws_walk(25, 313, nullptr, 1, 3);
		player_set_commands_allowed(false);
		break;

	case 645:
		ws_demand_location(670, 346, 9);
		ws_walk(613, 346, nullptr, 1, 9);
		player_set_commands_allowed(false);
		break;

	default:
		digi_preload("950_s28c");
		ws_demand_location(343, 351, 1);
		break;
	}

	digi_play_loop("950_28c", 3);
}

void Room644::parser() {
	Maze::parser();

	if (player_said("journal", "SCALLOP")) {
		if (_G(flags)[V207]) {
			digi_play("203r54", 1);
		} else {
			if (_G(kernel).trigger == 6)
				_G(flags)[V207] = 1;
			sketchInJournal(0);
		}

		_G(player).command_ready = false;
	} else {
		checkExitLeft(643);
		checkExitRight(645);
		checkExitDown(638);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
