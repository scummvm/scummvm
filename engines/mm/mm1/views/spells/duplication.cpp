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

#include "mm/mm1/views/spells/duplication.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

Duplication::Duplication() : SpellView("Duplication") {
	_bounds = getLineBounds(20, 24);
}

void Duplication::draw() {
	clearSurface();
	escToGoBack(0);

	writeString(10, 0, STRING["dialogs.character.which"]);
}

bool Duplication::msgKeypress(const KeypressMessage &msg) {
	Inventory &inv = g_globals->_currCharacter->_backpack;

	if (msg.keycode >= Common::KEYCODE_a &&
		msg.keycode < (Common::KEYCODE_a + (int)inv.size())) {
		int itemIndex = msg.keycode - Common::KEYCODE_a;

		if (duplicate(*g_globals->_currCharacter, inv, itemIndex))
			spellDone();
		else
			spellFailed();
	}

	return true;
}

bool Duplication::msgAction(const ActionMessage &msg) {
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
