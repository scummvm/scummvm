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

#include "mm/mm1/views_enh/exchange.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Exchange::Exchange() : PartyView("Exchange") {
	_bounds = Common::Rect(50, 112, 266, 148);
	addButton(&g_globals->_escSprites, Common::Point(165, 0), 0, KEYBIND_ESCAPE);
}

bool Exchange::msgFocus(const FocusMessage &msg) {
	PartyView::msgFocus(msg);

	_srcCharacter = g_globals->_party.indexOf(g_globals->_currCharacter);
	assert(_srcCharacter != -1);
	return true;
}

void Exchange::draw() {
	PartyView::draw();

	writeString(10, 5, STRING["enhdialogs.exchange"]);
}

bool Exchange::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return PartyView::msgAction(msg);
}

bool Exchange::msgGame(const GameMessage &msg) {
	if (msg._name == "UPDATE") {
		int charNum = g_globals->_party.indexOf(g_globals->_currCharacter);

		if (charNum != _srcCharacter) {
			// Swap the two characters
			SWAP(g_globals->_party[charNum], g_globals->_party[_srcCharacter]);
		}

		close();
		return true;
	}

	return PartyView::msgGame(msg);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
