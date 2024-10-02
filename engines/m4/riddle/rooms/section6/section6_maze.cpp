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

#include "m4/riddle/rooms/section6/section6_maze.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Maze::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
}

void Maze::daemon() {
	if (_G(kernel).trigger == 1) {
		player_set_commands_allowed(true);
	}
}

void Maze::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");

	if (player_said("gear") && player_said("STATUE")) {
		digi_play("623R02", 1, 255, -1, 623);
	} else if (lookFlag && player_said("RED BOULDER")) {
		digi_play("623R01", 1, 255, -1, 623);
	} else if (lookFlag && player_said_any("GREY ROCK", "STONE")) {
		digi_play("623R03", 1);
	} else if (lookFlag && player_said("CLAY")) {
		digi_play("623R04", 1);
	} else if (lookFlag && player_said(" ")) {
		digi_play("623R05", 1);
	} else if (lookFlag && player_said("STARFISH")) {
		digi_play("625R01", 1);
	} else if (lookFlag && player_said("SEAHORSE")) {
		digi_play("626R01", 1);
	} else if (lookFlag && player_said("SCALLOP")) {
		digi_play("644R01", 1);
	} else if (lookFlag && player_said("SHARK'S TOOTH")) {
		digi_play("651R01", 1);
	} else if (takeFlag && player_said("clay")) {
		digi_play("607r08", 1);
	} else if (player_said("HORN/PULL CORD/WATER", "CLAY")) {
		digi_play("com136", 1, 255, -1, 997);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Maze::checkExit(const char *action, int newRoom) {
	if (player_said(action)) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			_G(player).command_ready = false;
			break;

		case 1:
			digi_stop(1);
			digi_stop(2);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s28C", 3, 90);
			_G(game).setRoom(newRoom);
			_G(player).command_ready = false;
			break;

		default:
			break;
		}
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
