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

#include "mm/mm1/views/locations/blacksmith_sell_item.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

BlacksmithSellItem::BlacksmithSellItem() :
	BlacksmithSubview("BlacksmithSellItem") {
	_modeString = STRING["dialogs.location.sell"];
}

void BlacksmithSellItem::draw() {
	Location::draw();

	writeString(22, 1, STRING["dialogs.blacksmith.backpack"]);
	drawItems();
}

void BlacksmithSellItem::drawItems() {
	// TODO: List items to sell
	for (int idx = 0; idx < INVENTORY_COUNT; ++idx) {

	}
}

void BlacksmithSellItem::selectItem(uint index) {
	Character &c = *g_globals->_currCharacter;
	int itemIndex = index;

	// Some slots may be empty, so we need to iterate
	// until we found the index'ed filled in slot
	for (index = 0; index < INVENTORY_COUNT ; ++index) {
		if (c._backpack[index]) {
			if (--itemIndex == 0)
				break;
		}
	}
	if (index == INVENTORY_COUNT)
		return;

	// Remove the item
	int itemId = c._backpack[index]._id;
	int v14 = c._backpack[index]._field14;
	c._backpack.removeAt(index);

	getItem(itemId);
	if (!v14 && g_globals->_currItem._val13)
		g_globals->_currItem._cost /= 2;

	c._gold += g_globals->_currItem._cost / 2;
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
