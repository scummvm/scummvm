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

	// TODO: Inventory
	s.skip(0x3f);
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

} // namespace MM1
} // namespace MM
