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

#include "m4/riddle/rooms/section6/room651.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room651::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28c");
		break;

	case 647:
		if (_G(flags)[V206])
			ws_demand_location(548, 372, 11);
		else
			ws_demand_location(277, 353, 11);
		break;

	case 650:
		ws_demand_location(-30, 310, 3);
		ws_walk(20, 310, nullptr, 1, 3);
		player_set_commands_allowed(false);
		break;

	default:
		digi_preload("950_s28c");
		ws_demand_location(277, 353, 11);
		break;
	}

	digi_play_loop("950_28c", 3);
}

void Room651::parser() {
	Maze::parser();

	if (player_said("journal", "SHARK'S TOOTH")) {
		if (_G(flags)[V201]) {
			digi_play("203r54", 1);
		} else {
			if (_G(kernel).trigger == 6)
				_G(flags)[V201] = 1;
			sendWSMessage_multi(0);
		}

		_G(player).command_ready = false;
	} else {
		checkExitLeft(650);
		checkExitDown(647);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
