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

#ifndef ULTIMA3_LOGIC_CHEST_LOGIC_H
#define ULTIMA3_LOGIC_CHEST_LOGIC_H

#include "ultima/ultima3/logic/logic.h"

namespace Ultima {
namespace Ultima3 {

/**
 * Opening chests, which may be trapped, and what's found inside them
 */
class ChestLogic : public Logic {
private:
	/**
	 * Returns a number from 0 up to but not including a limit
	 */
	static int rollBelow(int limit);

	/**
	 * Hurts every member of the party who can still fight
	 */
	void damageParty();

public:
	/**
	 * Rolls to see whether a party member avoids a trap, which the
	 * agile classes are better at
	 * @returns		True if they got clear of it
	 */
	bool evadesTrap(int slot);

	/**
	 * Gives a party member the contents of a chest
	 */
	void loot(int slot);

	/**
	 * Opens a chest that a party member has found, possibly setting off a
	 * trap on the way
	 */
	void open(int slot);
};

} // namespace Ultima3
} // namespace Ultima

#endif
