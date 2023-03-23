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

bool WhichCharacter::msgFocus(const FocusMessage &msg) {
	_initialChar = g_globals->_currCharacter;
	return PartyView::msgFocus(msg);
}

void WhichCharacter::draw() {
	PartyView::draw();
	writeString(10, 5, STRING["enhdialogs.trade.dest"]);
}

bool WhichCharacter::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		int charNum = g_globals->_party.indexOf(g_globals->_currCharacter);
		g_globals->_currCharacter = _initialChar;

		close();
		send("CharacterInventory", GameMessage("TRADE_DEST", charNum));
		return true;
	}

	return false;
}

bool WhichCharacter::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		send("CharacterInventory", GameMessage("TRADE_DEST", -1));
		return true;
	} else if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6) {
		uint charNum = msg._action - KEYBIND_VIEW_PARTY1;
		if (charNum < g_globals->_party.size())
			send("CharacterInventory", GameMessage("TRADE_DEST", charNum));
		return true;
	} else {
		return false;
	}
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
