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

#include "mm/mm1/views_enh/spells/recharge_item.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

RechargeItem::RechargeItem() : CharacterInventory("RechargeItem") {
	clearButtons();

	addButton(2, STRING["enhdialogs.items.buttons.arms"], Common::KEYCODE_a);
	addButton(6, STRING["enhdialogs.items.buttons.backpack"], Common::KEYCODE_b);
	addButton(14, STRING["enhdialogs.items.buttons.charge"], Common::KEYCODE_c);
	addButton(16, STRING["enhdialogs.misc.exit"], Common::KEYCODE_ESCAPE);
}

bool RechargeItem::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_a || msg.keycode == Common::KEYCODE_b ||
			(msg.keycode >= Common::KEYCODE_1 && msg.keycode <= Common::KEYCODE_6)) {
		// Keys we can allow the base view to handle 
		CharacterInventory::msgKeypress(msg);

	} else if (msg.keycode == Common::KEYCODE_c) {
		selectButton(BTN_CHARGE);
	}

	return true;
}

void RechargeItem::performAction() {
	assert(_selectedButton == BTN_CHARGE);
	Inventory &inv = _mode == ARMS_MODE ? g_globals->_currCharacter->_equipped :
		g_globals->_currCharacter->_backpack;

	bool result = charge(inv, _selectedItem);
	close();

	g_events->send(InfoMessage(STRING[result ? "spells.done" : "spells.failed"]));
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
