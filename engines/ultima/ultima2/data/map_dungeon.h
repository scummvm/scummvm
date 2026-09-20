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

#ifndef ULTIMA2_DATA_MAP_DUNGEON_H
#define ULTIMA2_DATA_MAP_DUNGEON_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Ultima {
namespace Ultima2 {
namespace Data {

constexpr int DUNGEON_WIDTH = 16;
constexpr int DUNGEON_HEIGHT = 16;
constexpr int DUNGEON_LEVELS = 16;

enum DungeonTerrain {
	DUNGEON_FLOOR = 0x00,
	DUNGEON_LADDER_UP = 0x10,
	DUNGEON_LADDER_DOWN = 0x20,
	DUNGEON_LADDER_UP_DOWN = 0x30,
	DUNGEON_CHEST = 0x40,
	DUNGEON_WALL = 0x80,
	DUNGEON_DOOR = 0xC0,
	DUNGEON_SECRET_DOOR = 0xE0
};

/**
 * A dungeon or tower's 16 levels, each a 16x16 grid of cells (dungeons
 * number levels top to bottom, towers bottom to top). The high nibble of
 * each cell byte is its terrain (see DungeonTerrain); the low 3 bits flag
 * monster presence there, independent of the per-map MapMonsters slots,
 * and get cleared on a kill while preserving the terrain nibble.
 */
struct MapDungeon {
	byte _cells[DUNGEON_LEVELS][DUNGEON_HEIGHT][DUNGEON_WIDTH] = {};

	void load(int mapEra, int mapType);

	void synchronize(Common::Serializer &s) {
		s.syncBytes(&_cells[0][0][0], sizeof(_cells));
	}

	/**
	 * Returns the cell at the given position on a level. Addresses it the
	 * way the original does - a byte index within the level's 256-byte
	 * page - so positions just outside the 16x16 grid alias into
	 * neighbouring cells rather than being out of range
	 */
	byte &cell(int level, int x, int y) {
		byte row = (byte)y;
		int carry = 0;
		for (int i = 0; i < 4; ++i) {
			carry = row >> 7;
			row = (byte)(row << 1);
		}

		int index = (row + (byte)x + carry) & 0xFF;
		return _cells[level & 0xF][index / DUNGEON_WIDTH][index % DUNGEON_WIDTH];
	}

	DungeonTerrain terrainAt(int level, int x, int y) const {
		return (DungeonTerrain)(_cells[level][y][x] & 0xF0);
	}
	bool hasMonsterAt(int level, int x, int y) const {
		return (_cells[level][y][x] & 7) != 0;
	}
};

} // namespace Data
} // namespace Ultima2
} // namespace Ultima

#endif
