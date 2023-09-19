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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/burger/rooms/section3/mine_room.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

#define MINE_END 39

const MineEntry MineRoom::MINE_DATA[] = {
	{ 0, 0 }
};

const char *MineRoom::SAID[][4] = {
	{ "TUNNEL",  "311w007", "311w007z", nullptr    },
	{ "DEBRIS",  nullptr,   "311w010",  "311w011"  },
	{ "GROUND",  "311w012", "311w007z", "311w007z" },
	{ "WALL",    "311w012", "311w007z", "311w007z" },
	{ "CEILING", "311w012", "311w007z", "311w007z" },
	{ nullptr, nullptr, nullptr, nullptr }
};


void MineRoom::preload() {
	Section3Room::preload();
	_mineCtr = 0;
}

void MineRoom::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT") && player_said_any("WALL", "CEILING", "GROUND")) {
		term_message("Room #: %d", _G(flags)[V149]);
		term_message("Distance from pig: %d", getPigDistance());

		_mineCtr = (_mineCtr + 1) % 5;

		if (_mineCtr == 0) {
			wilbur_speech("311w012");
			_G(player).command_ready = false;
			return;
		}
	}

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("tunnel") && player_said_any("walk through", "GEAR")) {
		pal_fade_set_start(0);

		if (_G(hotspot_y) > 300)
			changeRoom(DIR_SOUTH);
		else if (_G(hotspot_x) < 200)
			changeRoom(DIR_WEST);
		else if (_G(hotspot_x) > 400)
			changeRoom(DIR_EAST);
		else
			changeRoom(DIR_NORTH);
	} else if (player_said("LOOK AT", "DEBRIS") && _G(game).room_id != 305) {
		_G(wilbur_should) = 407;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

int MineRoom::getPigDistance() const {
	int distance = 0;

	for (int index = _G(flags)[V149]; index != MINE_END; ++distance) {
		const MineEntry &me = MINE_DATA[index];
		index = me._indexes[me._offset];
	}

	return distance;
}

void MineRoom::changeRoom(MineDirection dir) {
	error("TODO: MineRoom::changeRoom");
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
