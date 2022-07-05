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
#include "mm/mm1/data/character.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

static const int CLASS_HP_PER_LEVEL[6] = {
	12, 10, 10, 8, 6, 8
};

static const int8 SPELLS_SP_GEMS[47 * 2] = {
	 0, 0, 0, 0, 0, -1, 0, 0,
	 0, 1, 0, 0, 0,  0, 0, 0,
	 1, 0, 0, 0, 0,  0, 0, 1,
	 0, 0, 0, 0, 2,  0, 2, 2,
	 0, 0, 0, 3, 3,
	 4, 4, 4, 4, 4,
	10, 5, 5, 5, 5,

	 0,   0, -1,  0, 0, 0, 0, 0,
	 0,   0,  1,  0, 0, 0, 0, 0,
	-1,   0,  1, -1, 0, 0, 1, 0,
	 0,   0,  2,  0, 0, 2, 2, 2,
	 0,   0,  3,  3, 3,
	 4,   4,  4,  4, 4,
	 5, 100,  5,  5, 5
};

static const byte SPELL_FLAGS[47 * 2] = {
	 1,  1,  9,  4,  2,  4,  0,  1,
	 4,  5,  9,  0,  0,  0,  9,  9,
	 2,  4,  4,  2,  9,  9,  2,  2,
	 6,  6,  0,  0,  6, 25,  5,  2,
	17,  0,  1,  4,  4,
	17,  4,  6,  4,  2,
	 1,  1,  0,  6, 25,

	 1,  2,  9,  9,  0,  2,  2,  9,
	 9,  9,  9,  2,  2,  5,  5,  9,
	 9, 18,  1,  9,  1,  1,  1,  9,
	 9,  9,  9,  9,  2,  0,  1,  1,
	17,  0,  9,  2,  2,
	 1,  9,  2,  0,  2,
	 2,  2, 17,  1,  1
};

enum SpellFlag {
	SF_COMBAT_ONLY = 1,
	SF_OUTDOORS_ONLY = 0x10
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

	_intelligence.synchronize(s);
	_might.synchronize(s);
	_personality.synchronize(s);
	_endurance.synchronize(s);
	_speed.synchronize(s);
	_accuracy.synchronize(s);
	_luck.synchronize(s);
	_level.synchronize(s);
	_age.synchronize(s);

	s.syncAsUint32LE(_exp);
	s.syncAsByte(_sp._current);
	s.syncAsByte(_v2c);
	s.syncAsByte(_sp._base);
	s.syncAsByte(_v2e);
	_slvl.synchronize(s);

	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_hpBase);
	s.syncAsUint16LE(_hp);
	s.syncAsUint16LE(_hpMax);

	// Gold field is annoying by being 3 bytes
	uint goldLo = _gold & 0xffff;
	uint goldHi = _gold >> 16;
	s.syncAsUint16LE(goldLo);
	s.syncAsByte(goldHi);
	if (s.isLoading())
		_gold = goldLo | (goldHi << 16);

	_ac.synchronize(s);
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
	_intelligence = _might = _personality = _endurance = 0;
	_speed = _accuracy = _luck = 0;
	_level = 0;
	_age = 0;
	_exp = 0;
	_sp = 0;
	_slvl = 0;
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

	Inventory::Entry e = _backpack[itemIndex];
	_backpack.removeAt(itemIndex);
	dest._backpack.add(e._id, e._field14);

	return TRADE_SUCCESS;
}


Character::LevelIncrease Character::increaseLevel() {
	++_level;
	++_age;
	if (_age._base > 220)
		_age._base = 220;
	_v6c += 2;

	int classNum = _class == NONE ? ROBBER : _class;
	int newHP = g_engine->getRandomNumber(CLASS_HP_PER_LEVEL[classNum - 1]);

	if (_endurance._base >= 40)
		newHP += 10;
	else if (_endurance._base >= 35)
		newHP += 9;
	else if (_endurance._base >= 30)
		newHP += 8;
	else if (_endurance._base >= 27)
		newHP += 7;
	else if (_endurance._base >= 24)
		newHP += 6;
	else if (_endurance._base >= 21)
		newHP += 5;
	else if (_endurance._base >= 19)
		newHP += 4;
	else if (_endurance._base >= 17)
		newHP += 3;
	else if (_endurance._base >= 15)
		newHP += 2;
	else if (_endurance._base >= 13)
		newHP += 1;
	else if (_endurance._base >= 9)
		newHP += 0;
	else if (_endurance._base >= 7)
		newHP = MAX(newHP - 1, 1);
	else if (_endurance._base >= 5)
		newHP = MAX(newHP - 2, 1);
	else
		newHP = MAX(newHP - 3, 1);

	_hpBase += newHP;
	_hp = _hpMax = _hpBase;

	int gainedSpells = 0;
	if (classNum < ARCHER) {
		if (_level._base < 7)
			gainedSpells = 0;
		else if (_level._base == 7)
			gainedSpells = 1;
		else if (_level._base == 9)
			gainedSpells = 2;
		else if (_level._base == 11)
			gainedSpells = 3;
		else if (_level._base == 13)
			gainedSpells = 4;
	} else if (classNum < SORCERER) {
		if (_level._base == 3)
			gainedSpells = 2;
		else if (_level._base == 5)
			gainedSpells = 3;
		else if (_level._base == 7)
			gainedSpells = 4;
		else if (_level._base == 9)
			gainedSpells = 5;
		else if (_level._base == 11)
			gainedSpells = 6;
		else if (_level._base == 13)
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
	_intelligence.reset();
	_might.reset();
	_personality.reset();
	_endurance.reset();
	_speed.reset();
	_personality.reset();
	_endurance.reset();
	_speed.reset();
	_accuracy.reset();
	_luck.reset();
	_level.reset();
	_slvl.reset();
}

void Character::updateAC() {
	int ac = _ac._base;

	if (_speed >= 40)
		ac += 9;
	else if (_speed >= 35)
		ac += 8;
	else if (_speed >= 30)
		ac += 7;
	else if (_speed >= 25)
		ac += 6;
	else if (_speed >= 21)
		ac += 5;
	else if (_speed >= 19)
		ac += 4;
	else if (_speed >= 17)
		ac += 3;
	else if (_speed >= 15)
		ac += 2;
	else if (_speed >= 13)
		ac += 1;
	else if (_speed >= 9)
		ac += 0;
	else if (_speed >= 7)
		ac = MAX(ac - 1, 0);
	else if (_speed >= 5)
		ac = MAX(ac - 2, 0);
	else
		ac = MAX(ac - 3, 0);

	_ac._current = ac;
}

Common::String Character::getConditionString() const {
	Common::String result;
	int cond = _condition;

	if (cond == 0) {
		result += STRING["stats.conditions.good"];
	} else if (cond == ERADICATED) {
		result += STRING["stats.conditions.eradicated"];
	} else {
		if (cond & BAD_CONDITION) {
			// Fatal conditions
			if (cond & DEAD)
				result += STRING["stats.conditions.dead"];
			if (cond & STONE)
				result += STRING["stats.conditions.stone"];
		} else {
			if (cond & UNCONSCIOUS)
				result += STRING["stats.conditions.unconscious"];
			if (cond & PARALYZED)
				result += STRING["stats.conditions.paralyzed"];
			if (cond & POISONED)
				result += STRING["stats.conditions.poisoned"];
			if (cond & DISEASED)
				result += STRING["stats.conditions.diseased"];
			if (cond & SILENCED)
				result += STRING["stats.conditions.silenced"];
			if (cond & BLINDED)
				result += STRING["stats.conditions.blinded"];
			if (cond & ASLEEP)
				result += STRING["stats.conditions.asleep"];
		}

		result.deleteLastChar();
	}

	return result;
}

SpellResult Character::castSpell(int lvl, int num) {
	int lvlNum;
	int setNum = _class == ARCHER || _class == SORCERER ? 1 : 0;

	// Figure the offset in the spell list
	int spellNum = 0;
	for (lvlNum = 2; lvlNum < MIN(lvl, 5); ++lvlNum)
		spellNum += 8;
	for (lvlNum = 5; lvlNum < lvl; ++lvlNum)
		spellNum += 5;

	// Get required SP
	int spellIndex = setNum * 47 + spellNum;
	int requiredSp = lvl - 1;

	if (SPELLS_SP_GEMS[spellIndex] < 0)
		// required SP increases with character's level
		requiredSp = _level;

	if (!_v2c) {
		if (SPELLS_SP_GEMS[spellIndex] < 0 && _sp._current < _level._current)
			return SR_NOT_ENOUGH_SP;
		if ((lvl - 1) > _sp._current)
			return SR_NOT_ENOUGH_SP;
	}

	int requiredGems = (uint8)SPELLS_SP_GEMS[spellIndex] & 0x7f;
	if (requiredGems && requiredGems > _gems)
		return SR_NOT_ENOUGH_GEMS;

	return performSpell(spellNum, requiredSp, requiredGems);
}

SpellResult Character::performSpell(int spellIndex, int requiredSp, int baseSp) {
	const Maps::Map &map = *g_maps->_currentMap;

	if (SPELL_FLAGS[spellIndex] & SF_COMBAT_ONLY)
		return SR_COMBAT_ONLY;
	if ((SPELL_FLAGS[spellIndex] & SF_OUTDOORS_ONLY) && !(map[0] & 0x80))
		return SR_OUTDOORS_ONLY;

	// TODO: Selection of cast on

	return SR_OK;
}

void Character::cast2() {

}

} // namespace MM1
} // namespace MM
