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

#include "mm/mm1/views/view_characters.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void ViewCharacters::draw() {
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
	for (int charNum = 0; charNum < 18; ++charNum) {
		if (roster._towns[charNum]) {
			const Character &re = roster[charNum];
			Common::String charName = re._name;
			pad_string(charName, 16, '.');

			Common::String level = Common::String::format("(%d)L%d",
					roster._towns[charNum], re._level);
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
	writeString(12, 24, STRING["dialogs.view_characters.legend2"]);
}

bool ViewCharacters::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		close();
	} else if (msg.keycode >= Common::KEYCODE_a &&
		msg.keycode <= (Common::KEYCODE_a + (int)_charIndexes.size() - 1)) {
		// Character selected
		uint charIndex = _charIndexes[msg.keycode - Common::KEYCODE_a];
		g_globals->_currCharacter = &g_globals->_roster[charIndex];
		addView("ViewCharacter");
	}

	return false;
}

/*------------------------------------------------------------------------*/

void CharacterStats::printStats() {
	Character &re = *g_globals->_currCharacter;
	printSummary();

	newLine();
	writeString(STRING["stats.attributes.int"]);
	writeNumber(re._int);
	_textPos.x = 8;
	writeString(STRING["stats.attributes.level"]);
	writeNumber(re._level);
	_textPos.x = 18;
	writeString(STRING["stats.attributes.age"]);
	writeNumber(re._level);
	_textPos.x = 27;
	writeString(STRING["stats.attributes.exp"]);
	writeNumber(re._exp);

	newLine();
	writeString(STRING["stats.attributes.mgt"]);
	writeNumber(re._mgt);

	newLine();
	writeString(STRING["stats.attributes.per"]);
	writeNumber(re._per);
	_textPos.x = 8;
	writeString(STRING["stats.attributes.sp"]);
	writeNumber(re._sp);
	_textPos.x = 16;
	writeChar('/');
	writeNumber(re._spMax);
	_textPos.x = 22;
	writeChar('(');
	writeNumber(re._sp1);
	writeChar(')');
	_textPos.x = 26;
	writeString(STRING["stats.attributes.gems"]);
	writeNumber(re._gems);

	newLine();
	writeString(STRING["stats.attributes.end"]);
	writeNumber(re._end);

	newLine();
	writeString(STRING["stats.attributes.spd"]);
	writeNumber(re._spd);
	_textPos.x = 8;
	writeString(STRING["stats.attributes.hp"]);
	writeNumber(re._hp);
	_textPos.x = 16;
	writeChar('/');
	writeNumber(re._hpMax);
	_textPos.x = 26;
	writeString(STRING["stats.attributes.gold"]);
	writeNumber(re._gold);

	newLine();
	writeString(STRING["stats.attributes.acy"]);
	writeNumber(re._acy);

	newLine();
	writeString(STRING["stats.attributes.luc"]);
	writeNumber(re._luc);
	_textPos.x = 8;
	writeString(STRING["stats.attributes.ac"]);
	writeNumber(re._ac);
	_textPos.x = 26;
	writeString(STRING["stats.attributes.food"]);
	writeNumber(re._food);

	newLine();
	newLine();
	printCondition();
	printInventory();
}

void CharacterStats::printSummary() {
	Character &re = *g_globals->_currCharacter;
	writeString(1, 0, re._name);

	_textPos.x = 17;
	writeString(": ");

	writeChar((re._sex == MALE) ? 'M' : (re._sex == FEMALE ? 'F' : 'O'));

	_textPos.x++;
	writeString((re._alignment >= GOOD && re._alignment <= EVIL) ?
		STRING[Common::String::format("stats.alignments.%d", re._alignment)] :
		STRING["stats.none"]
	);

	if (re._race >= HUMAN && re._race <= HALF_ORC)
		writeString(26, 0, STRING[Common::String::format("stats.races.%d", re._race)]);
	else
		writeString(26, 0, STRING["stats.none"]);

	if (re._class >= KNIGHT && re._class <= ROBBER)
		writeString(32, 0, STRING[Common::String::format("stats.classes.%d", re._class)]);
	else
		writeString(32, 0, STRING["stats.none"]);
}

void CharacterStats::printCondition() {
	Character &re = *g_globals->_currCharacter;
	writeString(STRING["stats.attributes.cond"]);
	_textPos.x++;
	int cond = re._condition;

	if (cond == 0) {
		writeString(STRING["stats.conditions.good"]);
	} else if (cond == ERADICATED) {
		writeString(STRING["stats.conditions.eradicated"]);
	} else {
		if (cond & BAD_CONDITION) {
			// Fatal conditions
			if (cond & DEAD)
				writeString(STRING["stats.conditions.dead"]);
			if (cond & STONE)
				writeString(STRING["stats.conditions.stone"]);
		} else {
			if (cond & UNCONSCIOUS)
				writeString(STRING["stats.conditions.unconscious"]);
			if (cond & PARALYZED)
				writeString(STRING["stats.conditions.paralyzed"]);
			if (cond & POISONED)
				writeString(STRING["stats.conditions.poisoned"]);
			if (cond & DISEASED)
				writeString(STRING["stats.conditions.diseased"]);
			if (cond & SILENCED)
				writeString(STRING["stats.conditions.silenced"]);
			if (cond & BLINDED)
				writeString(STRING["stats.conditions.blinded"]);
			if (cond & ASLEEP)
				writeString(STRING["stats.conditions.asleep"]);
		}

		--_textPos.x;
		writeChar(' ');
	}
}

void CharacterStats::printInventory() {
	Character &re = *g_globals->_currCharacter;
	writeString(0, 12, STRING["stats.inventory"]);

	// Print the equipped and backpack items
	for (int i = 0; i < INVENTORY_COUNT; ++i) {
		// Equippied item
		writeChar(0, 13 + i, '1' + i);
		writeChar(')');
		_textPos.x++;
		if (re._equipped[i])
			writeString(STRING[Common::String::format("stats.items.%d",
				re._equipped[i])]);

		// Backpack item
		writeChar(20, 13 + i, 'A' + i);
		writeChar(')');
		_textPos.x++;
		if (re._backpack[i])
			writeString(STRING[Common::String::format("stats.items.%d",
				re._backpack[i])]);
	}
}

/*------------------------------------------------------------------------*/

void ViewCharacter::draw() {
	assert(g_globals->_currCharacter);
	clearSurface();
	printStats();

	switch (_state) {
	case DISPLAY:
		writeString(6, 21, STRING["dialogs.view_character.rename"]);
		writeString(6, 22, STRING["dialogs.view_character.delete"]);
		escToGoBack();
		break;

	case RENAME:
		writeString(6, 21, STRING["dialogs.view_character.name"]);
		writeString(_newName);
		writeChar('_');
		break;

	case DELETE:
		writeString(6, 21, STRING["dialogs.view_character.are_you_sure"]);
		break;
	}
}

bool ViewCharacter::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		if (_state != DISPLAY) {
			redraw();
		} else {
			close();
		}

		_state = DISPLAY;
		return true;
	}

	switch (_state) {
	case DISPLAY:
		if ((msg.flags & Common::KBD_CTRL) && msg.keycode == Common::KEYCODE_n) {
			_state = RENAME;
			_newName = "";
			redraw();
		} else if ((msg.flags & Common::KBD_CTRL) && msg.keycode == Common::KEYCODE_d) {
			_state = DELETE;
			redraw();
		}
		break;

	case RENAME:
		if (msg.ascii >= 32 && msg.ascii <= 127) {
			_newName += toupper(msg.ascii);
			redraw();
		}
		if (msg.keycode == Common::KEYCODE_RETURN || _newName.size() == 15) {
			strncpy(g_globals->_currCharacter->_name, _newName.c_str(), 16);
			_state = DISPLAY;
			redraw();
		} else if (msg.keycode == Common::KEYCODE_BACKSPACE &&
				!_newName.empty()) {
			_newName.deleteLastChar();
			redraw();
		}
		break;

	case DELETE:
		if (msg.keycode == Common::KEYCODE_y) {
			// Removes the character and returns to View All Characters
			g_globals->_roster.remove(g_globals->_currCharacter);
			close();
		} else {
			// Any other keypress returns to display mode
			redraw();
		}

		_state = DISPLAY;
		break;
	}

	return true;
}

bool ViewCharacter::msgAction(const ActionMessage &msg) {
	if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6) {
		g_globals->_currCharacter = &g_globals->_party[
			msg._action - KEYBIND_VIEW_PARTY1];
		addView();
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
