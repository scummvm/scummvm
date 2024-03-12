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

#include "mm/mm1/views/spells/detect_magic.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

DetectMagic::DetectMagic() : SpellView("DetectMagic") {
	_bounds = getLineBounds(20, 24);
}

void DetectMagic::draw() {
	clearSurface();
	escToGoBack(0);

	writeString(0, 0, STRING["dialogs.spells.detect_charges"]);
	_textPos.x = 20;
	_textPos.y = 0;

	getMagicStrings();
	Inventory &inv = g_globals->_currCharacter->_backpack;
	for (uint i = 0; i < inv.size(); ++i) {
		writeChar('A' + i);
		writeChar(')');
		writeString(_strings[i]);

		// Move to write position for next item (if any)
		if (_textPos.x < 30) {
			_textPos.x = 30;
		} else {
			_textPos.x = 20;
			_textPos.y++;
		}
	}
}

bool DetectMagic::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT || msg._action == KEYBIND_ESCAPE)
		close();

	return true;
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
