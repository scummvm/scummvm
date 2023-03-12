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

#include "mm/mm1/views_enh/character_select.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

CharacterSelect::CharacterSelect() : PartyView("CharacterSelect") {
	_bounds = Common:: Rect(225, 144, 320, 200);
}

bool CharacterSelect::msgFocus(const FocusMessage &msg) {
	_currCharacter = g_globals->_currCharacter;
	g_globals->_currCharacter = nullptr;
	return PartyView::msgFocus(msg);
}

void CharacterSelect::draw() {
	ScrollView::draw();

	_fontReduced = true;
	writeString(STRING["enhdialogs.character_select.title"]);
}

bool CharacterSelect::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		g_events->send(g_events->focusedView()->getName(),
			GameMessage("CHAR_SELECTED", -1));
		return true;
	} else {
		return PartyView::msgAction(msg);
	}
}

bool CharacterSelect::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		close();

		int charNum = g_globals->_party.indexOf(g_globals->_currCharacter);
		g_globals->_currCharacter = _currCharacter;

		g_events->send(g_events->focusedView()->getName(),
			GameMessage("CHAR_SELECTED", charNum));
		return true;
	}

	return true;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
