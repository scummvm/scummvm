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
	Roster &roster = g_globals->_roster;
	writeString(0, 11, STRING["dialogs.view_characters.title"]);
	int lineNum = 0;
	_charIndexes.clear();

	// Loop to print characters
	for (int charNum = 0; charNum < 18; ++charNum) {
		if (roster._nums[charNum]) {
			const RosterEntry &re = roster[charNum];
			Common::String charName = re._name;
			pad_string(charName, 16, '.');

			Common::String level = Common::String::format("(%d)L%d",
					roster._nums[charNum], re._level);
			pad_string(level, 7);

			Common::String className = (re._class >= KNIGHT && re._class <= ROBBER) ?
				STRING[Common::String::format("classes.%d", (int)re._class)] :
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
		g_globals->_rosterEntry = &g_globals->_roster[
			msg.keycode - Common::KEYCODE_a];
		addView("ViewCharacter");
	}

	return false;
}

/*------------------------------------------------------------------------*/

void CharacterStats::printStats() {
	RosterEntry &re = *g_globals->_rosterEntry;
	printSummary();

	newLine();
	writeString(STRING["stats.conditions.int"]);
	writeNumber(re._int);
	_textPos.x = 8;
	writeString(STRING["stats.conditions.level"]);
	writeNumber(re._level);
	_textPos.x = 18;
	writeString(STRING["stats.conditions.age"]);
	writeNumber(re._level);
	_textPos.x = 27;
	writeString(STRING["stats.conditions.age"]);
	writeNumber(re._exp);

	newLine();
	writeString(STRING["stats.conditions.mgt"]);
	writeNumber(re._mgt);

	newLine();
	writeString(STRING["stats.conditions.per"]);
	writeNumber(re._per);
	_textPos.x = 8;
	writeString(STRING["stats.conditions.sp"]);
	writeNumber(re._sp);
	_textPos.x = 16;
	writeChar('/');
	writeNumber(re._spMax);
	_textPos.x = 22;
	writeChar('(');
	writeNumber(re._sp1);
	writeChar(')');
	_textPos.x = 26;
	writeString(STRING["stats.conditions.gems"]);
	writeNumber(re._gems);

	newLine();
	writeString(STRING["stats.conditions.end"]);
	writeNumber(re._end);

	newLine();
	writeString(STRING["stats.conditions.spd"]);
	writeNumber(re._spd);
	_textPos.x = 8;
	writeString(STRING["stats.conditions.hp"]);
	writeNumber(re._hp);
	_textPos.x = 16;
	writeChar('/');
	writeNumber(re._hpMax);
	_textPos.x = 26;
	writeString(STRING["stats.conditions.gold"]);
	writeNumber(re._gold);

	newLine();
	writeString(STRING["stats.conditions.acy"]);
	writeNumber(re._acy);

	newLine();
	writeString(STRING["stats.conditions.luc"]);
	writeNumber(re._luc);
	_textPos.x = 8;
	writeString(STRING["stats.conditions.ac"]);
	writeNumber(re._ac);
	_textPos.x = 26;
	writeString(STRING["stats.conditions.food"]);
	writeNumber(re._food);

	newLine();
	newLine();
	printCondition();
}

void CharacterStats::printSummary() {
	RosterEntry &re = *g_globals->_rosterEntry;
	writeString(1, 0, re._name);
	writeString(17, 0, Common::String::format(": %c %s",
		(re._sex == MALE) ? 'M' : (re._sex == FEMALE ? 'F' : 'O'),
		(re._alignment >= GOOD && re._alignment <= EVIL) ?
		STRING[Common::String::format("alignments.%d", re._alignment)].c_str() + 3 :
		STRING["stats.none"].c_str()
	));

	if (re._race >= HUMAN && re._race <= HALF_ORC)
		writeString(26, 0, STRING[Common::String::format("races.%d", re._race)].c_str() + 3);
	else
		writeString(26, 0, STRING["stats.none"]);

	if (re._class >= KNIGHT && re._class <= ROBBER)
		writeString(32, 0, STRING[Common::String::format("classes.%d", re._class)].c_str() + 3);
	else
		writeString(32, 0, STRING["stats.none"]);
}

void CharacterStats::printCondition() {
	RosterEntry &re = *g_globals->_rosterEntry;
	writeString(STRING["stats.conditions.cond"]);
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

/*------------------------------------------------------------------------*/

void ViewCharacter::draw() {
	assert(g_globals->_rosterEntry);
//	RosterEntry &re = *g_globals->_rosterEntry;

	Graphics::ManagedSurface *scr = getScreen();
	scr->clear();

}

bool ViewCharacter::msgKeypress(const KeypressMessage &msg) {
	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
