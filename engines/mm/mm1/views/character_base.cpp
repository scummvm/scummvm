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

#include "mm/mm1/views/character_base.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

void CharacterBase::printStats() {
	Character &re = *g_globals->_currCharacter;
	printSummary();

	writeString(0, 2, STRING["stats.attributes.int"]);
	writeNumber(re._intelligence);
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
	writeNumber(re._might);

	newLine();
	writeString(STRING["stats.attributes.per"]);
	writeNumber(re._personality);
	_textPos.x = 8;
	writeString(STRING["stats.attributes.sp"]);
	writeNumber(re._sp._current);
	_textPos.x = 16;
	writeChar('/');
	writeNumber(re._sp._base);
	_textPos.x = 22;
	writeChar('(');
	writeNumber(re._spellLevel._current);
	writeChar(')');
	_textPos.x = 26;
	writeString(STRING["stats.attributes.gems"]);
	writeNumber(re._gems);

	newLine();
	writeString(STRING["stats.attributes.end"]);
	writeNumber(re._endurance);

	newLine();
	writeString(STRING["stats.attributes.spd"]);
	writeNumber(re._speed);
	_textPos.x = 8;
	writeString(STRING["stats.attributes.hp"]);
	writeNumber(re._hpCurrent);
	_textPos.x = 16;
	writeChar('/');
	writeNumber(re._hpMax);
	_textPos.x = 26;
	writeString(STRING["stats.attributes.gold"]);
	writeNumber(re._gold);

	newLine();
	writeString(STRING["stats.attributes.acy"]);
	writeNumber(re._accuracy);

	newLine();
	writeString(STRING["stats.attributes.luc"]);
	writeNumber(re._luck);
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

void CharacterBase::printSummary() {
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

void CharacterBase::printCondition() {
	Character &c = *g_globals->_currCharacter;
	writeString(STRING["stats.attributes.cond"]);
	_textPos.x++;

	writeString(c.getConditionString());
}

void CharacterBase::printInventory() {
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
				(int)re._equipped[i]._id)]);

		// Backpack item
		writeChar(20, 13 + i, 'A' + i);
		writeChar(')');
		_textPos.x++;
		if (re._backpack[i])
			writeString(STRING[Common::String::format("stats.items.%d",
			(int)re._backpack[i]._id)]);
	}
}

void CharacterBase::draw() {
	assert(g_globals->_currCharacter);
	clearSurface();
	printStats();
}

bool CharacterBase::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
