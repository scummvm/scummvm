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

void Inventory::clear() {
	_items.clear();
	_items.resize(INVENTORY_COUNT);
}

void Inventory::synchronize(Common::Serializer &s, bool ids) {
	for (int i = 0; i < INVENTORY_COUNT; ++i)
		s.syncAsByte(ids ? _items[i]._id : _items[i]._field14);
}

bool Inventory::empty() const {
	for (uint i = 0; i < INVENTORY_COUNT; ++i) {
		if (_items[i])
			return false;
	}
	return true;
}

bool Inventory::full() const {
	for (uint i = 0; i < INVENTORY_COUNT; ++i) {
		if (!_items[i])
			return false;
	}
	return true;
}

uint Inventory::add(byte id, byte field14) {
	uint idx = getFreeSlot();
	_items[idx]._id = id;
	_items[idx]._field14 = field14;

	return idx;
}

int Inventory::getFreeSlot() const {
	for (uint i = 0; i < INVENTORY_COUNT; ++i) {
		if (!_items[i])
			return i;
	}

	error("Inventory is full");
	return -1;
}


void Inventory::removeAt(uint idx) {
	_items.remove_at(idx);
	_items.push_back(Entry());
}

bool Inventory::hasCategory(CategoryFn fn) const {
	for (uint i = 0; i < INVENTORY_COUNT; ++i) {
		if (fn(_items[i]._id))
			return true;
	}
	return false;
}

/*------------------------------------------------------------------------*/

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
	s.syncAsByte(_slvlBase);
	s.syncAsByte(_slvl);
	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_hpBase);
	s.syncAsUint16LE(_hp);
	s.syncAsUint16LE(_hpMax);
	s.syncAsUint16LE(_gold);
	s.skip(2);
	s.syncAsByte(_ac);
	s.syncAsByte(_food);
	s.syncAsByte(_condition);

	_equipped.synchronize(s, true);
	_backpack.synchronize(s, true);
	_equipped.synchronize(s, false);
	_backpack.synchronize(s, false);

	// TODO: Figure purpose of remaining unknown fields
	s.skip(39);
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
	_slvl = _slvlBase = 0;
	_gems = 0;
	_hpBase = _hp = _hpMax = 0;
	_gold = 0;
	_ac = 0;
	_food = 0;
	_condition = 0;
	_equipped.clear();
	_backpack.clear();

	_alignmentInitial = GOOD;
	_alignment = GOOD;
	_v58 = _v59 = _v62 = _v63 = _v64 = _v65 = 0;
	_v66 = _v67 = _v6c = _v6f = 0;
}


void Character::gatherGold() {
	uint total = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		total += g_globals->_party[i]._gold;
		g_globals->_party[i]._gold = 0;
	}

	_gold = total;
}

Character::TradeResult Character::trade(int whoTo, int itemIndex) {
	Character &dest = g_globals->_party[whoTo];
	if (&dest == this)
		return TRADE_SUCCESS;

	if (dest._backpack.full())
		return TRADE_FULL;
	if (!_backpack[itemIndex])
		return TRADE_NO_ITEM;

	// TODO: do the trade

	return TRADE_SUCCESS;
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
	_backpack.add(itemId, item._val13);

	return BUY_SUCCESS;
}

void Character::updateAttributes() {
	_int = _intBase;
	_mgt = _mgtBase;
	_per = _perBase;
	_end = _endBase;
	_spd = _spdBase;
	_acy = _acyBase;
	_luc = _lucBase;
	_level = _levelBase;
	_slvl = _slvlBase;
}

void Character::updateAC() {
	int ac = _acBase;

	if (_spd >= 40)
		ac += 9;
	else if (_spd >= 35)
		ac += 8;
	else if (_spd >= 30)
		ac += 7;
	else if (_spd >= 25)
		ac += 6;
	else if (_spd >= 21)
		ac += 5;
	else if (_spd >= 19)
		ac += 4;
	else if (_spd >= 17)
		ac += 3;
	else if (_spd >= 15)
		ac += 2;
	else if (_spd >= 13)
		ac += 1;
	else if (_spd >= 9)
		ac += 0;
	else if (_spd >= 7)
		ac = MAX(ac - 1, 0);
	else if (_spd >= 5)
		ac = MAX(ac - 2, 0);
	else
		ac = MAX(ac - 3, 0);

	_ac = ac;
}

} // namespace MM1
} // namespace MM
