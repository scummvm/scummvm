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

#include "m4/burger/rooms/room.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room::init() {
	int roomId = _G(game).room_id;

	if (roomId <= 800)
		_GINT().show();
	else
		_GINT().hide();

	// Disable commands for certain rooms
	if (roomId == 201 || roomId == 301 || roomId == 306 || roomId == 307 ||
			roomId == 401 || roomId == 501 || roomId == 511 || roomId == 512 ||
			roomId == 513 || roomId == 601 || roomId == 605 || roomId == 606 ||
			roomId == 608 || roomId == 609 || roomId == 610 || roomId == 801 ||
			roomId == 902) {
		player_set_commands_allowed(false);
	}

	if (roomId >= 950 || roomId == 902)
		mouse_hide();
	else
		mouse_show();

	// Do stuff that needs to be done each time a scene is started
	init_series_players();

	// TODO: Further room init
}

void Room::shutdown() {
	_GV()[298] = 0;
}

void Room::init_series_players() {
#ifdef TODO
	term_message("Initializing %d series_players...", MAX_SERIES_PLAYERS);

	for (int i = 0; i < MAX_SERIES_PLAYERS; ++i) {
		seriesPlayer_clear(&series_player[i]);
	}
#endif
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
