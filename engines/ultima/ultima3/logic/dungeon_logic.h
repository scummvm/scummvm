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

#ifndef ULTIMA3_LOGIC_DUNGEON_LOGIC_H
#define ULTIMA3_LOGIC_DUNGEON_LOGIC_H

#include "ultima/ultima3/logic/logic.h"

namespace Ultima {
namespace Ultima3 {

/**
 * Rules for moving the party around a dungeon
 */
class DungeonLogic : public Logic {
public:
	enum Ladder {
		LADDER_NONE,
		LADDER_TAKEN,
		LADDER_OUT
	};

	/**
	 * Returns the square the party is on
	 */
	byte tile() const;

	/**
	 * Moves the party a step forward or backward, if they can
	 * @returns		True if they moved
	 */
	bool moveForward();
	bool moveBackward();

	/**
	 * Turns the party a quarter turn, if they can
	 * @returns		True if they turned
	 */
	bool turn(bool right);

	/**
	 * Climbs up a ladder. If this was from the first level it takes the party
	 * out of the dungeon
	 */
	Ladder climb();

	/**
	 * Goes down a ladder, if there's one
	 * @returns		True if they did
	 */
	bool descend();

	/**
	 * Sometimes monsters are met on an empty square
	 * @returns		The tile number of the monsters, or -1
	 */
	int rollEncounter();

	/**
	 * Moves the party to a random empty square on the level
	 */
	void teleportRandomly();
};

} // namespace Ultima3
} // namespace Ultima

#endif
