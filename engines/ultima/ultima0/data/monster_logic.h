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

#ifndef ULTIMA0_MONSTER_LOGIC_H
#define ULTIMA0_MONSTER_LOGIC_H

#include "ultima/ultima0/data/data.h"

namespace Ultima {
namespace Ultima0 {

class MonsterLogic {
private:
	/**
	 * Shows a message in the dungeon status area
	 */
	static void showLines(const Common::String &msg);

	/**
	 * Monster Attacks
	 */
	static int attack(MonsterEntry &m, PlayerInfo &p);

	/**
	 * Monster Moves
	 */
	static void move(MonsterEntry &m, PlayerInfo &p, DungeonMapInfo &d);

	/**
	 * Can monster move to a square
	 */
	static bool canMoveTo(DungeonMapInfo &d, int x, int y);

	/**
	 * Monster Stealing
	 */
	static int steal(MonsterEntry &m, PlayerInfo &p);

public:
	/**
	 * Check Monsters Attacking
	 */
	static void checkForAttacks(PlayerInfo &p, DungeonMapInfo &d);
};

} // namespace Ultima0
} // namespace Ultima

#endif
