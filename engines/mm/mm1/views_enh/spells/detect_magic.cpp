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

#include "mm/mm1/views_enh/spells/detect_magic.h"
#include "mm/mm1/globals.h"

#define TEXT_X1 160
#define TEXT_X2 195

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

DetectMagic::DetectMagic() : ScrollView("DetectMagic") {
	setBounds(Common::Rect(30, 30, 210, 120));
	addButton(&g_globals->_escSprites, Common::Point(0, 64), 0, KEYBIND_ESCAPE, true);
}

void DetectMagic::draw() {
	ScrollView::draw();

	setReduced(true);
	writeString(0, 0, STRING["dialogs.spells.detect_charges"], ALIGN_RIGHT);

	getMagicStrings();

	Inventory &inv = g_globals->_currCharacter->_backpack;
	for (uint i = 0; i < inv.size(); ++i) {
		// Write item name
		writeString(0, (i + 1) * 8, Common::String::format("%c) ", 'A' + i));
		g_globals->_items.getItem(inv[i]._id);
		writeString(g_globals->_currItem._name);

		// Write out the detect status
		writeString(0, (i + 1) * 8, _strings[i], ALIGN_RIGHT);
	}

	if (inv.empty())
		writeLine(1, STRING["enhdialogs.misc.no_items"]);

	writeString(15, 66, STRING["enhdialogs.misc.go_back"]);
	setReduced(false);
}

bool DetectMagic::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT || msg._action == KEYBIND_ESCAPE)
		g_events->replaceView("Game", true);

	return true;
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
