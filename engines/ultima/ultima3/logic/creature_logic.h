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

#include "common/array.h"
#include "ultima/ultima3/data/map.h"

namespace Ultima {
namespace Ultima3 {

/**
 * The movement of the creatures on a map: the monsters that roam the world
 * and the people of the towns and castles
 */
class CreatureLogic {
public:
	/**
	 * A blast of fire breathed or fired at the party by a dragon or a ship,
	 * given by where it starts in the 11x11 viewport and how it steps
	 */
	struct Breath {
		int _x, _y;
		int _dx, _dy;
	};

private:
	Common::Array<Breath> _breaths;

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

	/**
	 * Gives a dragon or ship a chance to fire at the party
	 */
	void breathAttack(int index);

public:
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

	/**
	 * Returns the attacks made during the last update, and forgets them
	 */
	Common::Array<Breath> takeBreaths() {
		Common::Array<Breath> result = _breaths;
		_breaths.clear();
		return result;
	}
};

} // namespace Ultima3
} // namespace Ultima

#endif
