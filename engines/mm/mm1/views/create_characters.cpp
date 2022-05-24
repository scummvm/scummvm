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
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace Views {

void CreateCharacters::NewCharacter::clear() {
	Common::fill(_attribs1, _attribs1 + 7, 0);
	Common::fill(_attribs2, _attribs2 + 7, 0);
	_class = KNIGHT;
	_race = HUMAN;
	_alignment = GOOD;
	_sex = MALE;
	_name = "";

	Common::fill(_classesAllowed, _classesAllowed + 7, 0);
}

void CreateCharacters::NewCharacter::reroll() {
	clear();

	// Generate attributes
	for (int attrib = INTELLECT; attrib <= LUCK; ++attrib)
		_attribs1[attrib] = g_engine->getRandomNumber(4, 17);
	Common::copy(_attribs1, _attribs1 + 7, _attribs2);

	// Select which classes are available
	_classesAllowed[KNIGHT] = _attribs1[MIGHT] >= 12;
	_classesAllowed[PALADIN] = _attribs1[MIGHT] >= 12 && _attribs1[PERSONALITY] >= 12 &&
		_attribs1[ENDURANCE] >= 12;
	_classesAllowed[ARCHER] = _attribs1[INTELLECT] >= 12 && _attribs1[ACCURACY] >= 12;
	_classesAllowed[CLERIC] = _attribs1[PERSONALITY] >= 12;
	_classesAllowed[SORCERER] = _attribs1[INTELLECT] >= 12;
	_classesAllowed[ROBBER] = true;
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
	printAttributes();

	switch (_state) {
	case SELECT_CLASS:
		printClasses();
		if (g_globals->_roster.full())
			writeString(9, 21, STRING["dialogs.create_characters.full"]);
		escToGoBack();
		break;

	case SELECT_RACE:
		printRaces();
		break;

	case SELECT_ALIGNMENT:
		printAlignments();
		break;

	case SELECT_SEX:
		printSexes();
		break;

	case SELECT_NAME:
		printSummary(false);
		break;

	case SAVE_PROMPT:
		printSummary(true);
		break;
	}
}

void CreateCharacters::printAttributes() {
	_textPos.y = 5;

	for (int i = 0; i < 7; ++i, _textPos.y += 2) {
		_textPos.x = 18;
		if (_newChar._attribs1[i] < 10)
			writeChar(' ');
		writeNumber(_newChar._attribs1[i]);
	}
}

void CreateCharacters::printClasses() {
	if (_newChar._classesAllowed[KNIGHT])
		writeString(23, 5, STRING["stats.classes.1"]);
	if (_newChar._classesAllowed[PALADIN])
		writeString(23, 6, STRING["stats.classes.2"]);
	if (_newChar._classesAllowed[ARCHER])
		writeString(23, 7, STRING["stats.classes.3"]);
	if (_newChar._classesAllowed[CLERIC])
		writeString(23, 8, STRING["stats.classes.4"]);
	if (_newChar._classesAllowed[SORCERER])
		writeString(23, 9, STRING["stats.classes.5"]);

	writeString(23, 10, STRING["stats.classes.6"]);
}

void CreateCharacters::printRaces() {
	writeString(22, 5, STRING["dialogs.create_characters.class"]);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);

	writeString(23, 7, STRING["stats.races.1"]);
	writeString(23, 8, STRING["stats.races.2"]);
	writeString(23, 9, STRING["stats.races.3"]);
	writeString(23, 10, STRING["stats.races.4"]);
	writeString(23, 11, STRING["stats.races.5"]);

	writeString(22, 13, STRING["dialogs.create_characters.select_race"]);
	writeChar(29, 14, '5');
	writeString(21, 17, STRING["dialogs.create_characters.start_over"]);
}

void CreateCharacters::printAlignments() {
	writeString(22, 5, STRING["dialogs.create_characters.class"]);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeString(22, 6, STRING["dialogs.create_characters.race"]);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);

	writeString(23, 7, STRING["stats.alignments.1"]);
	writeString(23, 8, STRING["stats.alignments.2"]);
	writeString(23, 9, STRING["stats.alignments.3"]);

	writeString(22, 13, STRING["dialogs.create_characters.select_alignment"]);
	writeChar(29, 14, '3');
	writeString(21, 17, STRING["dialogs.create_characters.start_over"]);
}

void CreateCharacters::printSexes() {
	writeString(22, 5, STRING["dialogs.create_characters.class"]);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeString(22, 6, STRING["dialogs.create_characters.race"]);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);
	writeString(22, 7, STRING["dialogs.create_characters.alignment"]);
	writeString(STRING[Common::String::format("stats.alignments.%d", _newChar._alignment)]);

	writeString(23, 9, STRING["stats.sex.1"]);
	writeString(23, 10, STRING["stats.sex.2"]);

	writeString(22, 12, STRING["dialogs.create_characters.select_sex"]);
	writeChar(29, 14, '2');
	writeString(21, 17, STRING["dialogs.create_characters.start_over"]);
}

void CreateCharacters::printSummary(bool promptToSave) {
	writeString(22, 5, STRING["dialogs.create_characters.class"]);
	writeString(STRING[Common::String::format("stats.classes.%d", _newChar._class)]);
	writeString(22, 6, STRING["dialogs.create_characters.race"]);
	writeString(STRING[Common::String::format("stats.races.%d", _newChar._race)]);
	writeString(22, 7, STRING["dialogs.create_characters.alignment"]);
	writeString(STRING[Common::String::format("stats.alignments.%d", _newChar._alignment)]);
	writeString(22, 8, STRING["dialogs.create_characters.sex"]);
	writeString(STRING[Common::String::format("stats.sex.%d", _newChar._sex)]);

	writeString(22, 11, STRING["dialogs.create_characters.name"]);
	writeString(22, 13, _newChar._name);

	if (promptToSave) {
		writeString(22, 11, STRING["dialogs.create_characters.save_character"]);
		writeString(26, 13, "(Y/N)?");
	} else {
		writeChar('_');
		writeString(21, 17, STRING["dialogs.create_characters.start_over"]);
	}
}

bool CreateCharacters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		if (_state == SELECT_CLASS) {
			close();
		} else {
			_state = SELECT_CLASS;
			redraw();
		}
		return true;
	}

	switch (_state) {
	case SELECT_CLASS:
		if (msg.keycode == Common::KEYCODE_RETURN) {
			// Re-roll attributes
			_newChar.reroll();
			redraw();
		} else if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_6) {
			if (_newChar._classesAllowed[msg.keycode - Common::KEYCODE_0] &&
					!g_globals->_roster.full()) {
				// Selected a valid class
				_newChar._class = (CharacterClass)(msg.keycode - Common::KEYCODE_0);
				_state = SELECT_RACE;
				redraw();
			}
		}
		break;

	case SELECT_RACE:
		if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_5) {
			// Selected a valid race
			_newChar._race = (Race)(msg.keycode - Common::KEYCODE_0);
			_state = SELECT_ALIGNMENT;
			redraw();
		}
		break;

	case SELECT_ALIGNMENT:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_3) {
			// Selected a valid alignment
			_newChar._alignment = (Alignment)(msg.keycode - Common::KEYCODE_0);
			_state = SELECT_SEX;
			redraw();
		}
		break;

	case SELECT_SEX:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_2) {
			// Selected a valid sex
			_newChar._sex = (Sex)(msg.keycode - Common::KEYCODE_0);
			_state = SELECT_NAME;
			redraw();
		}
		break;

	case SELECT_NAME:
		if (msg.ascii >= 32 && msg.ascii <= 127) {
			_newChar._name += toupper(msg.ascii);
			redraw();
		}
		if (msg.keycode == Common::KEYCODE_RETURN || _newChar._name.size() == 15) {
			strncpy(g_globals->_rosterEntry->_name, _newChar._name.c_str(), 16);
			_state = SAVE_PROMPT;
			redraw();
		} else if (msg.keycode == Common::KEYCODE_BACKSPACE &&
			!_newChar._name.empty()) {
			_newChar._name.deleteLastChar();
			redraw();
		}
		break;
	}

	return true;
}

} // namespace Views
} // namespace MM1
} // namespace MM
