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

	writeString(22, 0, STRING["dialogs.blacksmith.backpack"]);
	drawItems();
}

void BlacksmithSellItem::drawItems() {
	const Inventory &inv = g_globals->_currCharacter->_backpack;

	for (uint idx = 0; idx < INVENTORY_COUNT; ++idx) {
		writeChar(17, 1 + idx, 'A' + idx);
		writeString(") ");

		if (idx < inv.size()) {
			g_globals->_items.getItem(inv[idx]._id);
			writeString(g_globals->_currItem._name);
			_textPos.x = 35;
			writeNumber(g_globals->_currItem.getSellCost());
		}
	}
}

void BlacksmithSellItem::selectItem(uint index) {
	Character &c = *g_globals->_currCharacter;
	Inventory &inv = c._backpack;
	assert(index < inv.size());
	g_globals->_items.getItem(inv[index]._id);

	c._gold += g_globals->_currItem.getSellCost();
	inv.removeAt(index);
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
