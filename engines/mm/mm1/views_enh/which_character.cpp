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

#include "mm/mm1/views_enh/which_character.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

WhichCharacter::WhichCharacter() : PartyView("WhichCharacter") {
	_bounds = Common::Rect(50, 103, 266, 139);
	addButton(&g_globals->_escSprites, Common::Point(176, 0), 0, KEYBIND_ESCAPE);
}

void WhichCharacter::draw() {
	PartyView::draw();
	writeString(10, 5, STRING["enhdialogs.trade.dest"]);
}

bool WhichCharacter::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		send("CharacterInventory", GameMessage("TRADE_DEST", -1));
		return true;

	case KEYBIND_VIEW_PARTY1:
	case KEYBIND_VIEW_PARTY2:
	case KEYBIND_VIEW_PARTY3:
	case KEYBIND_VIEW_PARTY4:
	case KEYBIND_VIEW_PARTY5:
	case KEYBIND_VIEW_PARTY6: {
		uint charNum = msg._action - KEYBIND_VIEW_PARTY1;
		if (charNum < g_globals->_party.size()) {
			close();
			send("CharacterInventory", GameMessage("TRADE_DEST", charNum));
		}
		return true;
	}

	default:
		return PartyView::msgAction(msg);
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
