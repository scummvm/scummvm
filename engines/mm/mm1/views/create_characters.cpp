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

#include "mm/mm1/views/create_characters.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void CreateCharacters::NewCharacter::clear() {
	Common::fill(_attribs, _attribs + 7, 0);
	Common::fill(_classesAllowed, _classesAllowed + 7, 0);
}

void CreateCharacters::draw() {
	drawTextBorder();

	writeString(10, 0, STRING["dialogs.create_characters.title"]);
	writeString(3, 5, STRING["dialogs.create_characters.intellect"]);
	writeString(3, 7, STRING["dialogs.create_characters.might"]);
	writeString(3, 9, STRING["dialogs.create_characters.personality"]);
	writeString(3, 11, STRING["dialogs.create_characters.endurance"]);
	writeString(3, 13, STRING["dialogs.create_characters.speed"]);
	writeString(3, 15, STRING["dialogs.create_characters.accuracy"]);
	writeString(3, 17, STRING["dialogs.create_characters.luck"]);

	switch (_state) {
	case SELECT_CLASS:
		if (g_globals->_roster.full())
			writeString(9, 21, STRING["dialogs.create_characters.full"]);
		escToGoBack();
		break;

	case SELECT_RACE:
		break;
	}
}

void CreateCharacters::printAttributes() {
	_textPos.y = 5;

	for (int i = 0; i < 7; ++i, _textPos.y += 2) {
		_textPos.x = 18;
		if (_newChar._attribs[i] < 10)
			writeChar(' ');
		writeNumber(_newChar._attribs[i]);
	}
}

void CreateCharacters::printClasses() {
	if (_newChar._attribs[MIGHT] >= 12) {
		writeString(23, 5, STRING["stats.classes.1"]);
		_newChar._classesAllowed[KNIGHT] = true;
	}
	if (_newChar._attribs[MIGHT] >= 12 && _newChar._attribs[PERSONALITY] >= 12 &&
			_newChar._attribs[ENDURANCE] >= 12) {
		writeString(23, 7, STRING["stats.classes.2"]);
		_newChar._classesAllowed[PALADIN] = true;
	}
	if (_newChar._attribs[INTELLECT] >= 12 && _newChar._attribs[ACCURACY] >= 12) {
		writeString(23, 9, STRING["stats.classes.3"]);
		_newChar._classesAllowed[ARCHER] = true;
	}
	if (_newChar._attribs[PERSONALITY] >= 12) {
		writeString(23, 11, STRING["stats.classes.4"]);
		_newChar._classesAllowed[CLERIC] = true;
	}
	if (_newChar._attribs[INTELLECT] >= 12) {
		writeString(23, 13, STRING["stats.classes.5"]);
		_newChar._classesAllowed[SORCERER] = true;
	}
	writeString(23, 15, STRING["stats.classes.6"]);
	_newChar._classesAllowed[ROBBER] = true;
}

bool CreateCharacters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_RETURN) {
		replaceView("MainMenu");
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
