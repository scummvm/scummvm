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

#include "mm/mm1/maps/map55.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map55::load() {
	// Data used for map graphics. Simply copied from Map 00 Sorpigal
	_data.resize(47);
	_data[0] = 1;
	_data[1] = 1;
	_data[2] = 13;
	_data[3] = 1;
	_data[4] = 11;
	_data[5] = 11;
	_data[6] = 10;
	_data[MAP_FLAGS] = 0;

	// Map structure
	_walls[7 + MAP_W * 3] = 71;
	_states[7 + MAP_W * 3] = 69;
	_walls[8 + MAP_W * 3] = 4;
	_states[8 + MAP_W * 3] = 4;
	_walls[9 + MAP_W * 3] = 116;
	_states[9 + MAP_W * 3] = 84;

	_walls[8 + MAP_W * 4] = 17;
	_states[8 + MAP_W * 4] = 16;
	_walls[8 + MAP_W * 5] = 81;
	_states[8 + MAP_W * 5] = 81 | CELL_SPECIAL;
}

void Map55::special() {
	g_events->addView("ScummVM");
}

} // namespace Maps
} // namespace MM1
} // namespace MM
