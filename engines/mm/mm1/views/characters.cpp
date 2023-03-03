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

#include "mm/mm1/views/characters.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void Characters::draw() {
	drawTextBorder();

	Roster &roster = g_globals->_roster;
	writeString(11, 0, STRING["dialogs.view_characters.title"]);
	int lineNum = 0;
	_charIndexes.clear();

	if (g_globals->_roster.empty()) {
		writeString(8, 5, STRING["dialogs.misc.no_characters"]);
		escToGoBack();
		return;
	}

	// Loop to print characters
	for (int charNum = 0; charNum < ROSTER_COUNT; ++charNum) {
		if (roster._towns[charNum]) {
			const Character &re = roster[charNum];
			Common::String charName = re._name;
			pad_string(charName, 16, '.');

			Common::String level = Common::String::format("(%d)L%d",
					roster._towns[charNum], re._level._current);
			pad_string(level, 7);

			Common::String className = (re._class >= KNIGHT && re._class <= ROBBER) ?
				STRING[Common::String::format("stats.classes.%d", (int)re._class)] :
				STRING["stats.none"];

			// Form line like: A) charName...(1)L1  Knight
			Common::String line = Common::String::format("(%c) %s%s%s",
				'A' + lineNum, charName.c_str(), level.c_str(), className.c_str());
			writeString(3, 3 + lineNum++, line);

			_charIndexes.push_back(charNum);
		}
	}

	// Print legend at the bottom
	writeString(6, 22, Common::String::format(
		STRING["dialogs.view_characters.legend1"].c_str(),
		'A' + (int)_charIndexes.size() - 1));
	writeString(12, 24, STRING["dialogs.misc.go_back"]);
}

bool Characters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode >= Common::KEYCODE_a &&
		msg.keycode <= (Common::KEYCODE_a + (int)_charIndexes.size() - 1)) {
		// Character selected
		uint charIndex = _charIndexes[msg.keycode - Common::KEYCODE_a];
		g_globals->_currCharacter = &g_globals->_roster[charIndex];
		_characterView.addView();
	}

	return false;
}

bool Characters::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
