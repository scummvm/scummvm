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

#include "mm/mm1/views_enh/which_item.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

WhichItem::WhichItem() : ScrollView("WhichItem") {
	_bounds = Common::Rect(50, 103, 266, 139);
	addButton(&g_globals->_escSprites, Common::Point(176, 0), 0, KEYBIND_ESCAPE);
}

bool WhichItem::msgGame(const GameMessage &msg) {
	if (msg._name == "DISPLAY") {
		_msg = msg._stringValue;
		addView();
		return true;
	}

	return ScrollView::msgGame(msg);
}

void WhichItem::draw() {
	ScrollView::draw();
	writeString(10, 5, _msg);
}

bool WhichItem::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_6) {
		close();
		send("CharacterInventory", GameMessage("ITEM",
			msg.keycode - Common::KEYCODE_1));
		return true;
	} else {
		return false;
	}
}

bool WhichItem::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		send("CharacterInventory", GameMessage("ITEM", -1));
		return true;
	} else {
		return false;
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
