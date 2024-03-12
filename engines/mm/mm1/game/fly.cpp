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

#include "mm/mm1/game/fly.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

static const byte FLY_MAP_ID1[20] = {
	1, 0, 4, 5, 0x12,
	2, 3, 0x11, 5, 6,
	2, 1, 4, 6, 0x1A,
	3, 3, 4, 1, 0x1B
};

static const byte FLY_MAP_ID2[20] = {
	0xF, 0xA, 3, 5, 1,
	5, 7, 0xA, 0xB, 7,
	0xB, 1, 9, 1, 0xB,
	1, 0xD, 0xF, 8, 1
};

static const byte FLY_MAP_X[20] = {
	15, 8, 11, 0, 9,
	15, 3, 10, 4, 11,
	15, 3, 3, 7, 12,
	14, 11, 5, 7, 15
};

static const byte FLY_MAP_Y[20] = {
	7, 10, 0, 8, 11,
	7, 2, 10, 0, 0,
	15, 3, 9, 0, 6,
	14, 15, 15, 7, 15
};

void Fly::fly(int mapIndex) {
	if (mapIndex != -1) {
		Maps::Maps &maps = *g_maps;
		int id = FLY_MAP_ID1[mapIndex] | ((int)FLY_MAP_ID2[mapIndex] << 8);

		maps._mapPos.x = FLY_MAP_X[mapIndex];
		maps._mapPos.y = FLY_MAP_Y[mapIndex];
		maps.changeMap(id, 2);
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
