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

#ifndef ULTIMA3_DATA_ARENA_H
#define ULTIMA3_DATA_ARENA_H

#include "common/scummsys.h"

namespace Ultima {
namespace Ultima3 {
namespace Data {

constexpr int ARENA_SIZE = 11;
constexpr int ARENA_MONSTERS = 8;
constexpr int ARENA_PLAYERS = 4;

// Marks a party member who isn't in the fight
constexpr byte ARENA_ABSENT = 0xFF;

/**
 * The ground a fight takes place on: an 11x11 area of tiles, the places the
 * monsters may start from, and where each of the party starts out. As the
 * fight goes on it also records who is where, with what they stand on
 */
struct Arena {
	byte _tiles[ARENA_SIZE * ARENA_SIZE] = {};
	byte _monsterX[ARENA_MONSTERS] = {};
	byte _monsterY[ARENA_MONSTERS] = {};
	byte _monsterUnder[ARENA_MONSTERS] = {};

	// Hit points, with none meaning there's no monster
	byte _monsterHp[ARENA_MONSTERS] = {};

	byte _playerX[ARENA_PLAYERS] = {};
	byte _playerY[ARENA_PLAYERS] = {};
	byte _playerUnder[ARENA_PLAYERS] = {};
	byte _playerTile[ARENA_PLAYERS] = {};

	/**
	 * Loads an arena from a .ULT file
	 */
	void load(const char *filename);

	byte tile(int x, int y) const {
		return _tiles[y * ARENA_SIZE + x];
	}

	void setTile(int x, int y, byte value) {
		_tiles[y * ARENA_SIZE + x] = value;
	}
};

} // namespace Data
} // namespace Ultima3
} // namespace Ultima

#endif
