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

#include "m4/burger/series_player.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {

void SeriesPlayers::clear() {
	term_message("Initializing %d series_players...", MAX_SERIES_PLAYERS);

	for (int i = 0; i < MAX_SERIES_PLAYERS; ++i)
		_players[i].clear();
}

void seriesPlayer::clear() {
	break_list = nullptr;
	series_machine = nullptr;
	shadow_machine = nullptr;
	series = -1;
	shadow_series = -1;
	index = -1;
	name = nullptr;
	in_use = false;
}

} // namespace Burger
} // namespace M4
