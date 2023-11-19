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

#include "mm/mm1/game/recharge_item.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {

bool RechargeItem::charge(Inventory &inv, int itemIndex) {
	const Item &item = *g_globals->_items.getItem(inv[itemIndex]._id);

	if (getRandomNumber(100) == 100) {
		// OMG: The original seriously had this fringe
		// case that happens so rarely
		inv.removeAt(itemIndex);	// Break item
		return false;

	} else {
		inv[itemIndex]._charges = MIN(inv[itemIndex]._charges + getRandomNumber(4),
			(int)item._maxCharges);
		return true;
	}
}

} // namespace Game
} // namespace MM1
} // namespace MM
