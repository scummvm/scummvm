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

#include "mm/mm1/views/locations/blacksmith_subview.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

void BlacksmithSubview::drawItems() {
	for (int idx = 0; idx < INVENTORY_COUNT; ++idx) {
		writeChar(17, 1 + idx, 'A' + idx);
		writeString(") ");

		g_globals->_items.getItem(_items[idx]);
		drawIsAllowed();
		writeString(g_globals->_currItem._name);
		_textPos.x = 35;
		writeNumber(g_globals->_currItem._cost);
	}
}

void BlacksmithSubview::drawIsAllowed() {
	Character &c = *g_globals->_currCharacter;

	if (c._class != NONE && c._class <= ROBBER) {
		if (!(BLACKSMITH_CLASS_USAGE[c._class - 1] &
			g_globals->_currItem._disablements))
			return;
	}

	_textPos.x--;
	writeChar('-');
}

bool BlacksmithSubview::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_a:
	case Common::KEYCODE_b:
	case Common::KEYCODE_c:
	case Common::KEYCODE_d:
	case Common::KEYCODE_e:
	case Common::KEYCODE_f:
		selectItem(msg.keycode - Common::KEYCODE_a);
		draw();
		break;
	default:
		break;
	}

	return true;
}

bool BlacksmithSubview::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

void BlacksmithSubview::selectItem(uint index) {
	switch (g_globals->_currCharacter->buyItem(
		_items[index])) {
	case Character::BUY_BACKPACK_FULL:
		backpackFull();
		break;
	case Character::BUY_NOT_ENOUGH_GOLD:
		notEnoughGold();
		break;
	default:
		// Purchased successfully
		displayMessage(15, STRING["dialogs.blacksmith.thankyou"]);
		break;
	}
}

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM
