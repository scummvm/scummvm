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
#include "mm/mm1/data/text.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define LINE1_Y 5
#define COL1_X 30
#define COL3_X 220
#define BACKPACK_Y 103

CharacterBase::CharacterBase(const Common::String &name) : ScrollView(name) {
	_escSprite.load("esc.icn");
}

void CharacterBase::printStats() {
	const Character &c = *g_globals->_currCharacter;
	printSummary();

	writeLine(4, "Int=", ALIGN_RIGHT, COL1_X);
	writeLine(4, Common::String::format("%u", c._intelligence._base), ALIGN_LEFT, COL1_X);
	writeLine(5, "Mgt=", ALIGN_RIGHT, COL1_X);
	writeLine(5, Common::String::format("%u", c._might._base), ALIGN_LEFT, COL1_X);
	writeLine(6, "Per=", ALIGN_RIGHT, COL1_X);
	writeLine(6, Common::String::format("%u", c._personality._base), ALIGN_LEFT, COL1_X);
	writeLine(7, "End=", ALIGN_RIGHT, COL1_X);
	writeLine(7, Common::String::format("%u", c._endurance._base), ALIGN_LEFT, COL1_X);
	writeLine(8, "Spd=", ALIGN_RIGHT, COL1_X);
	writeLine(8, Common::String::format("%u", c._speed._base), ALIGN_LEFT, COL1_X);
	writeLine(9, "Acy=", ALIGN_RIGHT, COL1_X);
	writeLine(9, Common::String::format("%u", c._accuracy._base), ALIGN_LEFT, COL1_X);
	writeLine(10, "Luc=", ALIGN_RIGHT, COL1_X);
	writeLine(10, Common::String::format("%u", c._luck._base), ALIGN_LEFT, COL1_X);

	writeLine(4, "Level=", ALIGN_RIGHT, 90);
	writeNumber(c._level);
	writeLine(4, "Age=", ALIGN_LEFT, 120);
	writeNumber(c._age);

	writeLine(6, "SP=", ALIGN_RIGHT, 90);
	writeLine(6, Common::String::format("%u", c._sp._current), ALIGN_LEFT, 90);
	writeLine(6, Common::String::format("/%u", c._sp._base), ALIGN_LEFT, 120);
	writeLine(6, Common::String::format("(%u)", c._spellLevel._current), ALIGN_LEFT, 160);

	writeLine(8, "HP=", ALIGN_RIGHT, 90);
	writeLine(8, Common::String::format("%u", c._hpCurrent), ALIGN_LEFT, 90);
	writeLine(8, Common::String::format("/%u", c._hpMax), ALIGN_LEFT, 120);

	writeLine(10, "AC=", ALIGN_RIGHT, 90);
	writeLine(10, Common::String::format("%u", c._ac._current), ALIGN_LEFT, 90);

	writeLine(4, "Exp=", ALIGN_RIGHT, COL3_X);
	writeLine(4, Common::String::format("%u", c._exp), ALIGN_LEFT, COL3_X);
	writeLine(6, "Gems=", ALIGN_RIGHT, COL3_X);
	writeLine(6, Common::String::format("%u", c._gems), ALIGN_LEFT, COL3_X);
	writeLine(8, "Gold=", ALIGN_RIGHT, COL3_X);
	writeLine(8, Common::String::format("%u", c._gold), ALIGN_LEFT, COL3_X);
	writeLine(10, "Food=", ALIGN_RIGHT, COL3_X);
	writeLine(10, Common::String::format("%u", c._food), ALIGN_LEFT, COL3_X);

	printCondition();
	printInventory();
}

void CharacterBase::printSummary() {
	const Character &c = *g_globals->_currCharacter;
	Common::String suffix = Common::String::format(": %s %s ",
		SEX_NAMES[c._sex - 1],
		((c._alignment >= GOOD && c._alignment <= EVIL) ?
		ALIGNMENT_NAMES[c._alignment - 1] :
		"None")
	);

	if (c._race >= HUMAN && c._race <= HALF_ORC)
		suffix += RACE_NAMES[c._race - 1];
	else
		suffix += "None";
	suffix += ' ';

	if (c._class >= KNIGHT && c._class <= ROBBER)
		suffix += CLASS_NAMES[c._class - 1];
	else
		suffix += "None";

	writeString(35, LINE1_Y, truncateString(c._name,
		_innerBounds.width() - 35 - (int)getStringWidth(suffix)));
	writeString(suffix);
}

void CharacterBase::printCondition() {
	const Character &c = *g_globals->_currCharacter;

	writeLine(2, "Cond=", ALIGN_RIGHT, 90);
	writeLine(2, c.getConditionString(), ALIGN_LEFT, 90);
}

void CharacterBase::printInventory() {
	const Character &c = *g_globals->_currCharacter;
	writeString(0, BACKPACK_Y, "-----<Equipped>----------<Backpack>----");
	for (int i = 0; i < 5; ++i)
		writeChar('-');

	// Print the equipped and backpack items
	for (uint i = 0; i < INVENTORY_COUNT; ++i) {
		// Equippied item
		writeString(0, BACKPACK_Y + 9 * (i + 1), Common::String::format("%c) ", '1' + i));
		if (i < c._equipped.size()) {
			g_globals->_items.getItem(c._equipped[i]._id);
			const Item &item = g_globals->_currItem;
			writeString(item._name);
		}

		// Backpack item
		writeString(160 - _innerBounds.left, BACKPACK_Y + 9 * (i + 1),
			Common::String::format("%c) ", 'A' + i));
		if (i < c._backpack.size()) {
			g_globals->_items.getItem(c._backpack[i]._id);
			const Item &item = g_globals->_currItem;
			writeString(item._name);
		}
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
