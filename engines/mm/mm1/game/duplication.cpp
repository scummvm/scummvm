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

#include "mm/mm1/game/duplication.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

bool Duplication::duplicate(Character &c, Inventory &inv, int itemIndex) {
	if (c._backpack.full())
		// No space to duplicate
		return false;

	if (getRandomNumber(100) == 100) {
		// OMG: The original seriously had this fringe
		// case that happens so rarely
		inv.removeAt(itemIndex);	// Break item
		return false;

	} else if (inv[itemIndex]._id >= 230) {
		// Item range that can't be duplicated
		return false;

	} else {
		// Add a copy of the item
		c._backpack.add(inv[itemIndex]._id, inv[itemIndex]._charges);
		return true;
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
