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

#include "mm/mm1/data/items.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {

uint Item::getSellCost() const {
	uint cost = _cost;
	if (_maxCharges)
		cost /= 2;
	cost /= 2;

	return cost;
}

bool ItemsArray::load() {
	Common::File f;
	if (!f.open("items.txt"))
		return false;

	resize(255);
	for (int lineNum = 0; lineNum < 255; ++lineNum) {
		Item &item = (*this)[lineNum];
		Common::String line = f.readLine();
		assert(line.size() > 20 && line[0] == '"' && line[15] == '"');

		item._name = Common::String(line.c_str() + 1, line.c_str() + 15);
		line = Common::String(line.c_str() + 16);
		while (item._name.lastChar() == ' ')
			item._name.deleteLastChar();

		item._disablements = getNextValue(line);
		item._equipMode = (EquipMode)getNextValue(line);
		item._val10 = getNextValue(line);
		item._effectId = getNextValue(line);
		item._spellId = getNextValue(line);
		item._maxCharges = getNextValue(line);
		item._cost = getNextValue(line);
		item._val16 = getNextValue(line);
		item._val17 = getNextValue(line);
	}

	return true;
}

Item *ItemsArray::getItem(byte index) const {
	assert(index > 0);
	g_globals->_currItem = (*this)[index - 1];

	return &g_globals->_currItem;
}

ItemCategory getItemCategory(byte itemId) {
	if (isWeapon(itemId))
		return ITEMCAT_WEAPON;
	if (isMissile(itemId))
		return ITEMCAT_MISSILE;
	if (isTwoHanded(itemId))
		return ITEMCAT_TWO_HANDED;
	if (isArmor(itemId))
		return ITEMCAT_ARMOR;
	if (isShield(itemId))
		return ITEMCAT_SHIELD;

	return ITEMCAT_NONE;
}

} // namespace MM1
} // namespace MM
