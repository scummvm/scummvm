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

#include "mm/mm1/views_enh/character_base.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define COL1_X 30
#define COL3_X 220

void CharacterBase::printStats() {
	const Character &c = *g_globals->_currCharacter;
	printSummary();

	writeLine(4, STRING["stats.attributes.int"], ALIGN_RIGHT, COL1_X);
	writeLine(4, Common::String::format("%u", c._intelligence._base), ALIGN_LEFT, COL1_X);
	writeLine(5, STRING["stats.attributes.mgt"], ALIGN_RIGHT, COL1_X);
	writeLine(5, Common::String::format("%u", c._might._base), ALIGN_LEFT, COL1_X);
	writeLine(6, STRING["stats.attributes.per"], ALIGN_RIGHT, COL1_X);
	writeLine(6, Common::String::format("%u", c._personality._base), ALIGN_LEFT, COL1_X);
	writeLine(7, STRING["stats.attributes.end"], ALIGN_RIGHT, COL1_X);
	writeLine(7, Common::String::format("%u", c._endurance._base), ALIGN_LEFT, COL1_X);
	writeLine(8, STRING["stats.attributes.spd"], ALIGN_RIGHT, COL1_X);
	writeLine(8, Common::String::format("%u", c._speed._base), ALIGN_LEFT, COL1_X);
	writeLine(9, STRING["stats.attributes.acy"], ALIGN_RIGHT, COL1_X);
	writeLine(9, Common::String::format("%u", c._accuracy._base), ALIGN_LEFT, COL1_X);
	writeLine(10, STRING["stats.attributes.luc"], ALIGN_RIGHT, COL1_X);
	writeLine(10, Common::String::format("%u", c._luck._base), ALIGN_LEFT, COL1_X);

	writeLine(4, STRING["stats.attributes.level"], ALIGN_RIGHT, 90);
	writeNumber(c._level);
	writeLine(4, STRING["stats.attributes.age"], ALIGN_LEFT, 120);
	writeNumber(c._age);

	writeLine(6, STRING["stats.attributes.sp"], ALIGN_RIGHT, 90);
	writeLine(6, Common::String::format("%u", c._sp._current), ALIGN_LEFT, 90);
	writeLine(6, Common::String::format("/%u", c._sp._base), ALIGN_LEFT, 120);
	writeLine(6, Common::String::format("(%u)", c._spellLevel._current), ALIGN_LEFT, 160);

	writeLine(8, STRING["stats.attributes.hp"], ALIGN_RIGHT, 90);
	writeLine(8, Common::String::format("%u", c._hpCurrent), ALIGN_LEFT, 90);
	writeLine(8, Common::String::format("/%u", c._hpMax), ALIGN_LEFT, 120);

	writeLine(10, STRING["stats.attributes.ac"], ALIGN_RIGHT, 90);
	writeLine(10, Common::String::format("%u", c._ac._current), ALIGN_LEFT, 90);

	writeLine(4, STRING["stats.attributes.exp"], ALIGN_RIGHT, COL3_X);
	writeLine(4, Common::String::format("%u", c._exp), ALIGN_LEFT, COL3_X);
	writeLine(6, STRING["stats.attributes.gems"], ALIGN_RIGHT, COL3_X);
	writeLine(6, Common::String::format("%u", c._gems), ALIGN_LEFT, COL3_X);
	writeLine(8, STRING["stats.attributes.gold"], ALIGN_RIGHT, COL3_X);
	writeLine(8, Common::String::format("%u", c._gold), ALIGN_LEFT, COL3_X);
	writeLine(10, STRING["stats.attributes.food"], ALIGN_RIGHT, COL3_X);
	writeLine(10, Common::String::format("%u", c._food), ALIGN_LEFT, COL3_X);

/*	newLine();
	newLine();
	printCondition();
	printInventory();
	*/
}

#define LINE1_Y 5

void CharacterBase::printSummary() {
	const Character &c = *g_globals->_currCharacter;
	writeString(35, LINE1_Y, c._name);

	writeString(120, LINE1_Y, ": ");

	writeString((c._sex == MALE) ? "M " : (c._sex == FEMALE ? "F " : "O "));

	writeString((c._alignment >= GOOD && c._alignment <= EVIL) ?
		STRING[Common::String::format("stats.alignments.%d", c._alignment)] :
		STRING["stats.none"]
	);
	writeChar(' ');

	if (c._race >= HUMAN && c._race <= HALF_ORC)
		writeString(STRING[Common::String::format("stats.races.%d", c._race)]);
	else
		writeString(STRING["stats.none"]);
	writeChar(' ');

	if (c._class >= KNIGHT && c._class <= ROBBER)
		writeString(STRING[Common::String::format("stats.classes.%d", c._class)]);
	else
		writeString(STRING["stats.none"]);
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
	const Character &c = *g_globals->_currCharacter;
	ScrollView::draw();

	Graphics::ManagedSurface s = getSurface();
	c._faceSprites.draw(&s, 0, Common::Point(_innerBounds.left, _innerBounds.top));

	printStats();
}

bool CharacterBase::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
