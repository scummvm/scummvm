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

#include "common/algorithm.h"
#include "mm/mm1/data/char.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

static const int CLASS_HP_PER_LEVEL[6] = {
	12, 10, 10, 8, 6, 8
};

void Character::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_name, 16);
	s.syncAsByte(_sex);
	s.syncAsByte(_alignmentInitial);
	s.syncAsByte(_alignment);
	s.syncAsByte(_race);
	s.syncAsByte(_class);

	s.syncAsByte(_intBase);
	s.syncAsByte(_int);
	s.syncAsByte(_mgtBase);
	s.syncAsByte(_mgt);
	s.syncAsByte(_perBase);
	s.syncAsByte(_per);
	s.syncAsByte(_endBase);
	s.syncAsByte(_end);
	s.syncAsByte(_spdBase);
	s.syncAsByte(_spd);
	s.syncAsByte(_acyBase);
	s.syncAsByte(_acy);
	s.syncAsByte(_lucBase);
	s.syncAsByte(_luc);

	s.syncAsByte(_levelBase);
	s.syncAsByte(_level);
	s.syncAsByte(_ageBase);
	s.syncAsByte(_age);
	s.syncAsUint32LE(_exp);
	s.syncAsUint16LE(_sp);
	s.syncAsUint16LE(_spMax);
	s.syncAsByte(_sp2);
	s.syncAsByte(_maxSpellLevel);
	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_hpBase);
	s.syncAsUint16LE(_hp);
	s.syncAsUint16LE(_hpMax);
	s.syncAsUint16LE(_gold);
	s.skip(2);
	s.syncAsByte(_ac);
	s.syncAsByte(_food);
	s.syncAsByte(_condition);

	s.syncBytes(_equipped, INVENTORY_COUNT);
	s.syncBytes(_backpack, INVENTORY_COUNT);

	// TODO: Figure purpose of remaining unknown fields
	s.skip(51);
}

void Character::clear() {
	Common::fill(_name, _name + 16, 0);
	_sex = (Sex)0;
	_alignmentInitial = (Alignment)0;
	_alignment = (Alignment)0;
	_race = (Race)0;
	_class = (CharacterClass)0;
	_int = _mgt = _per = _end = 0;
	_spd = _acy = _luc = 0;
	_level = 0;
	_age = 0;
	_exp = 0;
	_sp = _spMax = 0;
	_maxSpellLevel = _sp2 = 0;
	_gems = 0;
	_hpBase = _hp = _hpMax = 0;
	_gold = 0;
	_ac = 0;
	_food = 0;
	_condition = 0;
	Common::fill(_equipped, _equipped + INVENTORY_COUNT, 0);
	Common::fill(_backpack, _backpack + INVENTORY_COUNT, 0);

	_alignmentInitial = GOOD;
	_alignment = GOOD;
	_v58 = _v59 = _v62 = _v63 = _v64 = _v65 = 0;
	_v66 = _v67 = _v6c = _v6f = 0;
}

Character::LevelIncrease Character::increaseLevel() {
	_level = ++_levelBase;
	_age = ++_ageBase;
	if (_ageBase > 220)
		_ageBase = 220;
	_v6c += 2;

	int classNum = _class == NONE ? ROBBER : _class;
	int newHP = g_engine->getRandomNumber(CLASS_HP_PER_LEVEL[classNum - 1]);

	if (_endBase >= 40)
		newHP += 10;
	else if (_endBase >= 35)
		newHP += 9;
	else if (_endBase >= 30)
		newHP += 8;
	else if (_endBase >= 27)
		newHP += 7;
	else if (_endBase >= 24)
		newHP += 6;
	else if (_endBase >= 21)
		newHP += 5;
	else if (_endBase >= 19)
		newHP += 4;
	else if (_endBase >= 17)
		newHP += 3;
	else if (_endBase >= 15)
		newHP += 2;
	else if (_endBase >= 13)
		newHP += 1;
	else if (_endBase >= 9)
		newHP += 0;
	else if (_endBase >= 7)
		newHP = MAX(newHP - 1, 1);
	else if (_endBase >= 5)
		newHP = MAX(newHP - 2, 1);
	else
		newHP = MAX(newHP - 3, 1);

	_hpBase += newHP;
	_hp = _hpMax = _hpBase;

	int gainedSpells = 0;
	if (classNum < ARCHER) {
		if (_levelBase < 7)
			gainedSpells = 0;
		else if (_levelBase == 7)
			gainedSpells = 1;
		else if (_levelBase == 9)
			gainedSpells = 2;
		else if (_levelBase == 11)
			gainedSpells = 3;
		else if (_levelBase == 13)
			gainedSpells = 4;
	} else if (classNum < SORCERER) {
		if (_levelBase == 3)
			gainedSpells = 2;
		else if (_levelBase == 5)
			gainedSpells = 3;
		else if (_levelBase == 7)
			gainedSpells = 4;
		else if (_levelBase == 9)
			gainedSpells = 5;
		else if (_levelBase == 11)
			gainedSpells = 6;
		else if (_levelBase == 13)
			gainedSpells = 7;
	}

	LevelIncrease result;
	result._numHP = newHP;
	result._numSpells = gainedSpells;
	return result;
}

Character::BuyResult Character::buyItem(byte itemId) {
	getItem(itemId);

	// Check if backpack is full
	int slotIndex = 0;
	while (slotIndex < INVENTORY_COUNT && _backpack[slotIndex])
		++slotIndex;
	if (slotIndex == INVENTORY_COUNT)
		return BUY_BACKPACK_FULL;

	// Check character has enough gold
	Item &item = g_globals->_currItem;
	if (_gold < item._cost)
		return BUY_NOT_ENOUGH_GOLD;

	// Add the item
	_gold -= item._cost;
	_backpack[slotIndex] = itemId;
	_backpack14[slotIndex] = item._field14;

	return BUY_SUCCESS;
}

} // namespace MM1
} // namespace MM
