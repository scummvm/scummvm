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

#ifndef MM1_DATA_ACTIVE_SPELLS_H
#define MM1_DATA_ACTIVE_SPELLS_H

#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {

#define ACTIVE_SPELLS_COUNT 18

struct ActiveSpellsStruct {
	byte fear;
	byte cold;
	byte fire;
	byte poison;
	byte acid;
	byte electricity;
	byte magic;
	byte light;
	byte leather_skin;
	byte levitate;
	byte walk_on_water;
	byte guard_dog;
	byte psychic_protection;
	byte bless;
	byte invisbility;
	byte shield;
	byte power_shield;
	byte cursed;
};

union ActiveSpells {
	ActiveSpellsStruct _s;
	byte _arr[ACTIVE_SPELLS_COUNT];

	ActiveSpells() { clear(); }

	/**
	 * Clear the spells
	 */
	void clear();

	/**
	 * Synchronize spell data to/from savegames
	 */
	void synchronize(Common::Serializer &s);
};

} // namespace MM1
} // namespace MM

#endif
