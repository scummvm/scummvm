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

#include "common/file.h"
#include "mm/mm1/utils/roster.h"

namespace MM {
namespace MM1 {

void RosterEntry::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_name, 16);
	s.syncAsByte(_sex);
	s.skip(1);
	s.syncAsByte(_alignment);
	s.syncAsByte(_race);
	s.syncAsByte(_class);
	s.skip(1);
	s.syncAsByte(_int);
	s.skip(1);
	s.syncAsByte(_mgt);
	s.skip(1);
	s.syncAsByte(_per);
	s.skip(1);
	s.syncAsByte(_end);
	s.skip(1);
	s.syncAsByte(_spd);
	s.skip(1);
	s.syncAsByte(_acy);
	s.skip(1);
	s.syncAsByte(_luc);
	s.skip(1);

	s.syncAsByte(_level);
	s.syncAsByte(_age);
	s.skip(1);
	s.syncAsUint32LE(_exp);
	s.syncAsUint16LE(_sp);
	s.syncAsUint16LE(_spMax);
	s.skip(1);
	s.syncAsByte(_sp1);
	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_hp);
	s.skip(2);
	s.syncAsUint16LE(_hpMax);
	s.syncAsUint16LE(_gold);
	s.skip(2);
	s.syncAsByte(_ac);
	s.syncAsByte(_food);
	s.syncAsByte(_condition);

	s.syncBytes(_equipped, INVENTORY_COUNT);
	s.syncBytes(_backpack, INVENTORY_COUNT);

	s.skip(51);
}

void RosterEntry::clear() {
	Common::fill(_name, _name + 16, 0);
	_sex = (Sex)0;
	_alignment = (Alignment)0;
	_race = (Race)0;
	_class = (CharacterClass)0;
	_int = _mgt = _per = _end = 0;
	_spd = _acy = _luc = 0;
	_level = 0;
	_age = 0;
	_exp = 0;
	_sp = _spMax = 0;
	_sp1 = 0;
	_gems = 0;
	_hp = _hpMax = 0;
	_gold = 0;
	_ac = 0;
	_food = 0;
	_condition = 0;
	Common::fill(_equipped, _equipped + INVENTORY_COUNT, 0);
	Common::fill(_backpack, _backpack + INVENTORY_COUNT, 0);
}

void Roster::synchronize(Common::Serializer &s) {
	for (int i = 0; i < CHARACTERS_COUNT; ++i)
		_items[i].synchronize(s);

	for (int i = 0; i < CHARACTERS_COUNT; ++i)
		s.syncAsByte(_nums[i]);
}

void Roster::loadDefaults() {
	Common::File f;
	if (!f.open("roster.dta"))
		error("Could not open roster.dta");

	Common::Serializer s(&f, nullptr);
	synchronize(s);
}

void Roster::remove(RosterEntry *entry) {
	entry->clear();

	size_t idx = entry - _items;
	_nums[idx] = 0;
}

bool Roster::empty() const {
	for (uint i = 0; i < CHARACTERS_COUNT; ++i) {
		if (_nums[i])
			return false;
	}

	return true;
}

bool Roster::full() const {
	for (uint i = 0; i < CHARACTERS_COUNT; ++i) {
		if (!_nums[i])
			return false;
	}

	return true;
}

} // namespace MM1
} // namespace MM
