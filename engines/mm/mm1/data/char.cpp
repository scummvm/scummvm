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

namespace MM {
namespace MM1 {

void Character::synchronize(Common::Serializer &s) {
	s.syncBytes((byte *)_name, 16);
	s.syncAsByte(_sex);
	s.syncAsByte(_alignmentInitial);
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
	s.syncAsByte(_sp2);
	s.syncAsByte(_sp1);
	s.syncAsUint16LE(_gems);
	s.syncAsUint16LE(_hp);
	s.syncAsUint16LE(_hp2);
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
	_sp1 = _sp2 = 0;
	_gems = 0;
	_hp = _hp2 = _hpMax = 0;
	_gold = 0;
	_ac = 0;
	_food = 0;
	_condition = 0;
	Common::fill(_equipped, _equipped + INVENTORY_COUNT, 0);
	Common::fill(_backpack, _backpack + INVENTORY_COUNT, 0);

	_field11 = 0;
	_v58 = _v59 = _v62 = _v63 = _v64 = _v65 = 0;
	_v66 = _v67 = _v6c = _v6f = 0;
}

} // namespace MM1
} // namespace MM
