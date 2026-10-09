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

#ifndef ULTIMA3_LOGIC_LOGIC_H
#define ULTIMA3_LOGIC_LOGIC_H

#include "ultima/ultima3/data/roster.h"

namespace Ultima {
namespace Ultima3 {

// The numbering is shared with the wind direction
enum Direction {
	DIR_NONE = 0,
	DIR_NORTH = 1,
	DIR_EAST = 2,
	DIR_SOUTH = 3,
	DIR_WEST = 4
};

/**
 * Game rules common to every location: the turn counter, and the upkeep
 * of the party each turn
 */
class Logic {
private:
	// Shared by every kind of location, so the upkeep carries on unbroken
	static int _healCounter;
	static int _upkeepCounter;

	/**
	 * Gives a character a magic point
	 */
	void regenerateMagicPoint(Data::RosterEntry &e);

	/**
	 * Uses up some of a character's food, starving them once it runs out
	 */
	void applyHunger(int slot);

public:
	virtual ~Logic() {}

	/**
	 * Counts a turn taken by the party
	 * @param amount	How many moves to add, defaulting to one per party member
	 */
	void incrementMoveCounter(int amount = -1);

	/**
	 * Applies the effects of a turn passing on the party: magic point
	 * regeneration, hunger, poison and natural healing. Outside the
	 * overworld this only happens on every fourth turn
	 */
	void processPartyTurnEffects(bool everyTurn);

	/**
	 * Reduces the hit points of a party member, killing them if they run
	 * out. Returns true if they were killed
	 */
	bool damageCharacter(int slot, int amount);
};

} // namespace Ultima3
} // namespace Ultima

#endif
