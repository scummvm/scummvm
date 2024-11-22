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

#include "m4/riddle/rooms/section6/room630.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room630::init() {
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s28c");
		break;

	case 624:
		ws_demand_location(382, 326, 1);
		break;

	case 629:
		ws_demand_location(-30, 333, 3);
		ws_walk(30, 333, nullptr, 1, 3);
		player_set_commands_allowed(false);
		break;

	case 635:
		ws_demand_location(410, 276, 5);
		break;

	default:
		digi_preload("950_s28c");
		ws_demand_location(382, 326, 1);
		break;
	}

	digi_play_loop("950_s28c", 3);
}

void Room630::parser() {
	Maze::parser();

	checkExitLeft(629);
	checkExitUp(635);
	checkExitDown(624);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
