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

#include "mm/mm1/views/spells/recharge_item.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

void RechargeItem::show() {
	UIElement *view = dynamic_cast<RechargeItem *>(g_events->findView("RechargeItem"));
	assert(view);

	view->open();
}

RechargeItem::RechargeItem() : SpellView("RechargeItem") {
	_bounds = getLineBounds(20, 24);
}

void RechargeItem::draw() {
	clearSurface();
	escToGoBack(0);

	writeString(10, 0, STRING["dialogs.charcater.which_item"]);
}

bool RechargeItem::msgKeypress(const KeypressMessage &msg) {
	Inventory &inv = g_globals->_currCharacter->_backpack;

	if (msg.keycode >= Common::KEYCODE_a &&
			msg.keycode < (Common::KEYCODE_a + (int)inv.size())) {
		int itemIndex = msg.keycode - Common::KEYCODE_a;
		Item *item = g_globals->_items.getItem(inv[itemIndex]._id);

		if (g_engine->getRandomNumber(100) == 100) {
			// OMG: The original seriously had this fringe
			// case that happens so rarely
			inv.removeAt(itemIndex);	// Break item
			spellFailed();
		} else {
			inv[itemIndex]._charges = MIN(
				inv[itemIndex]._charges +
				g_engine->getRandomNumber(4),
				(int)item->_maxCharges);
		}
	}

	return true;
}

bool RechargeItem::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		spellFailed();
		return true;
	}

	return false;
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
