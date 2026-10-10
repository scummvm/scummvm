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

#ifndef ULTIMA3_DATA_DUNGEON_H
#define ULTIMA3_DATA_DUNGEON_H

#include "common/scummsys.h"
#include "common/serializer.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr int DUNGEON_LEVELS = 8;
constexpr int DUNGEON_SIZE = 16;

// Kinds of dungeon square, held in the low bits of a tile
constexpr byte DTILE_TIME_LORD = 1;
constexpr byte DTILE_FOUNTAIN = 2;
constexpr byte DTILE_WIND = 3;
constexpr byte DTILE_TRAP = 4;
constexpr byte DTILE_MARK = 5;
constexpr byte DTILE_GREMLINS = 6;
constexpr byte DTILE_SIGN = 8;

// Flags in the bits above those
constexpr byte DTILE_LADDER_UP = 0x10;
constexpr byte DTILE_LADDER_DOWN = 0x20;
constexpr byte DTILE_CHEST = 0x40;
constexpr byte DTILE_WALL = 0x80;
constexpr byte DTILE_DOOR = 0xC0;

/**
 * A dungeon: eight levels of 16x16 squares, and the signs that are on them
 */
class Dungeon {
private:
	static const int SIGNS_SIZE = 0x90;

	byte _tiles[DUNGEON_LEVELS * DUNGEON_SIZE * DUNGEON_SIZE] = {};
	byte _signs[SIGNS_SIZE] = {};

public:
	/**
	 * Loads a dungeon from a .ULT file
	 */
	void load(const char *filename);

	void synchronize(Common::Serializer &s);

	/**
	 * Returns a square. The coordinates wrap around the level
	 */
	byte tile(int level, int x, int y) const {
		return _tiles[(level << 8) + ((y & (DUNGEON_SIZE - 1)) << 4) + (x & (DUNGEON_SIZE - 1))];
	}

	void setTile(int level, int x, int y, byte value) {
		_tiles[(level << 8) + ((y & (DUNGEON_SIZE - 1)) << 4) + (x & (DUNGEON_SIZE - 1))] = value;
	}

	/**
	 * Returns what the sign on a level says, or null if there isn't one
	 */
	const char *sign(int level) const;
};

} // namespace Data
} // namespace Ultima3
} // namespace Ultima

#endif
