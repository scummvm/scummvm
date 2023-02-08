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

#include "mm/mm1/views/order.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

Order::Order() : TextView("Order") {
	_bounds = getLineBounds(21, 24);
}

bool Order::msgGame(const GameMessage &msg) {
	if (msg._name == "ORDER" && g_globals->_party.size() > 1) {
		// Show the dialog
		addView(this);
		return true;
	}

	return false;
}

bool Order::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_BACKSPACE &&
			!_indexes.empty()) {
		_indexes.remove_at(_indexes.size() - 1);
		redraw();

	} else if (msg.ascii >= '1' && msg.ascii <= '6') {
		// Check for the index already existing
		for (uint i = 0; i < _indexes.size(); ++i) {
			if (_indexes[i] == (uint)(msg.ascii - '0'))
				return true;
		}

		_indexes.push_back(msg.ascii - '0');

		if (_indexes.size() < g_globals->_party.size()) {
			// Redraw with the new entry added
			redraw();
		} else {
			// Reached the party size
			Common::Array<Character> oldParty = g_globals->_party;
			g_globals->_party.clear();

			for (uint i = 0; i < _indexes.size(); ++i)
				g_globals->_party.push_back(oldParty[_indexes[i] - 1]);

			close();
		}
	}

	return true;
}

bool Order::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

void Order::draw() {
	clearSurface();
	writeString(0, 0, STRING["dialogs.order.title"]);
	writeString(0, 2, STRING["dialogs.misc.go_back"]);
	writeString(17, 1, STRING["dialogs.order.old"]);

	_textPos.x = 21;
	for (uint i = 0; i < _indexes.size(); ++i)
		writeString(Common::String::format(
			" %d ", _indexes[i]));
	writeChar('^');
}

} // namespace Views
} // namespace MM1
} // namespace MM
