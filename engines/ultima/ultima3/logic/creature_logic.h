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

#ifndef ULTIMA3_LOGIC_CREATURE_LOGIC_H
#define ULTIMA3_LOGIC_CREATURE_LOGIC_H

#include "ultima/ultima3/data/map.h"

namespace Ultima {
namespace Ultima3 {

/**
 * The movement of the creatures on a map: the monsters that roam the world
 * and the people of the towns and castles
 */
class CreatureLogic {
private:
	// Alternates when travelling by horse or ship, so that creatures only
	// get to move on every other step
	int _stepToggle = 1;

	/**
	 * Returns the sign of the distance that moving along an axis has to
	 * cover to reach a target, taking the shortest way round the world
	 */
	int stepToward(int from, int to) const;

	/**
	 * Works out where a creature would stand after a step toward the party
	 */
	void stepTowardParty(int index, int &x, int &y) const;

	/**
	 * Returns true if a creature may step to a position
	 */
	bool canMoveTo(int index, int x, int y) const;

	/**
	 * Moves a creature to a position, leaving behind what it was standing on
	 */
	void moveTo(int index, int x, int y);

	/**
	 * Every so often adds a new monster to the world
	 */
	void spawnMonster();

public:
	// How many turns remain of time being held still
	int _negateTimeTurns = 0;

	/**
	 * Returns the creature standing at a position, or -1
	 */
	int creatureAt(int x, int y) const;

	/**
	 * Lets each creature take a turn, and may add a new monster to the world
	 * @param moved		True if the party just moved, which matters
	 *		when they're riding or sailing
	 * @returns		The creature that has reached the party and so starts a
	 *		fight, or -1
	 */
	int update(bool moved);
};

} // namespace Ultima3
} // namespace Ultima

#endif
