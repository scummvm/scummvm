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

#ifndef MM1_DATA_SPELLS_H
#define MM1_DATA_SPELLS_H

#include "common/serializer.h"
#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {

struct SpellsState {
	byte _mmVal1 = 0, _mmVal2 = 0;
	byte _mmVal5 = 0;
	byte _mmVal7 = 0;
	byte _resistanceType = RESISTANCE_MAGIC;

	// TODO: Is this variable different in different contexts?
	// In some places it's used to hold a new condition,
	// but others, like moonRay, uses it to hold Hp
	byte _newCondition = 0;

	/**
	 * Synchronize data to/from savegames
	 */
	void synchronize(Common::Serializer &s) {
		s.syncAsByte(_mmVal1);
		s.syncAsByte(_mmVal2);
		s.syncAsByte(_mmVal5);
		s.syncAsByte(_mmVal7);
		s.syncAsByte(_resistanceType);
		s.syncAsByte(_newCondition);
	}
};

} // namespace MM1
} // namespace MM

#endif
