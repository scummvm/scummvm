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

#include "mm/mm1/views/inn.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

bool Inn::msgFocus(const FocusMessage &msg) {
	// Get a list of characters in the town
	_charNums.clear();
	for (uint i = 0; i < CHARACTERS_COUNT; ++i) {
		if (g_globals->_roster._towns[i] == g_globals->_startingTown)
			_charNums.push_back(i);
	}

	return true;
}

void Inn::draw() {
	drawTextBorder();

	writeString(10, 0, Common::String::format(
		STRING["dialogs.inn.title"].c_str(),
		'0' + g_globals->_startingTown));
	writeString(STRING[Common::String::format(
		"stats.towns.%d", g_globals->_startingTown)]);
	escToGoBack();

	if (_charNums.empty()) {
		writeString(8, 5, STRING["dialogs.misc.no_characters"]);
	} else {
		writeString(10, 3, STRING["dialogs.misc.some_characters"]);

		for (uint idx = 0; idx < _charNums.size(); ++idx) {
			uint charNum = _charNums[idx];
			RosterEntry &re = g_globals->_roster[charNum];

			_textPos = (idx < 9) ? Common::Point(2, 6 + idx) :
				Common::Point(20, 6 + idx - 9);

			// Write character
			writeChar(g_globals->_partyChars.contains(charNum) ? '@' : ' ');
			writeChar('A' + idx);
			writeChar(')');
			writeString(re._name);
		}

		Common::String range = Common::String::format("'A'-'%c' ",
			'A' + _charNums.size() - 1);
		writeString(13, 19, range);
		writeString(STRING["dialogs.inn.to_view"]);
		writeString(6, 20, STRING["dialogs.inn.ctrl"]);
		writeString(range);
		writeString(STRING["dialogs.inn.add_remove"]);

		if (!g_globals->_partyChars.empty())
			writeString(13, 22, STRING["dialogs.inn.exit"]);
		if (g_globals->_partyChars.size() == PARTY_COUNT)
			writeString(10, 16, STRING["dialogs.inn.full"]);
	}
}

bool Inn::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		replaceView("MainMenu");
		return true;
	} else if (msg.keycode >= Common::KEYCODE_a &&
			msg.keycode < (Common::KeyCode)(Common::KEYCODE_a + _charNums.size())) {
		int charNum = _charNums[msg.keycode - Common::KEYCODE_a];

		if (msg.flags & Common::KBD_CTRL) {
			// Toggle in party
			if (g_globals->_partyChars.contains(charNum))
				g_globals->_partyChars.remove(charNum);
			else
				g_globals->_partyChars.push_back(charNum);

			redraw();

		} else if (msg.flags == 0) {
			// View character
			g_globals->_rosterEntry = &g_globals->_roster[charNum];
			addView("ViewCharacter");
		}
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
