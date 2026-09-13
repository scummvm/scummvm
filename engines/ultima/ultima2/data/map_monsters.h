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

#ifndef ULTIMA2_DATA_MAP_MONSTERS_H
#define ULTIMA2_DATA_MAP_MONSTERS_H

#include "common/scummsys.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

constexpr int MAP_MONSTER_COUNT = 32;

/**
 * Up to 32 monster/NPC slots for the current map, stored as a
 * struct-of-arrays -- one array per field, with a monster's slot index
 * (0-31) shared across all of them -- rather than one struct per monster.
 */
struct MapMonsters {
	byte _mapX[MAP_MONSTER_COUNT] = {};
	byte _mapY[MAP_MONSTER_COUNT] = {};
	byte _spellHP[MAP_MONSTER_COUNT] = {};
	byte _type[MAP_MONSTER_COUNT] = {}; // TileId x 4; 0 means the slot is empty
	byte _glyphTile[MAP_MONSTER_COUNT] = {};
	byte _offerFlag[MAP_MONSTER_COUNT] = {};
	byte _tempX[MAP_MONSTER_COUNT] = {};
	byte _tempY[MAP_MONSTER_COUNT] = {};

	void load(int mapNum1, int mapNum2);

	bool isActive(int slot) const {
		return _type[slot] != 0;
	}
	TileId tileType(int slot) const {
		return (TileId)(_type[slot] / 4);
	}
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
