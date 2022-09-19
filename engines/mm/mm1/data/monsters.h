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

#ifndef MM1_DATA_MONSTERS_H
#define MM1_DATA_MONSTERS_H

#include "mm/mm1/gfx/dta.h"
#include "common/str.h"
#include "graphics/managed_surface.h"

namespace MM {
namespace MM1 {

#define MONSTERS_COUNT 195

struct Monster {
	Common::String _name; // char _name[15];
	byte _count;
	byte _field10;
	byte _field11;
	byte _field12;
	byte _unk[12];
	byte _field1f;
};

class Monsters {
private:
	Monster _monsters[MONSTERS_COUNT];
	Gfx::DTA _monPix;
private:
	/**
	 * Get the next value from a read line
	 */
	byte getNextValue(Common::String &line);
public:
	Monsters() : _monPix(MONPIX_DTA) {}

	/**
	 * Load the monster list
	 */
	bool load();

	/**
	 * Square brackets operator
	 */
	const Monster &operator[](uint i) {
		assert(i >= 1 && i <= MONSTERS_COUNT);
		return _monsters[i];
	}

	/**
	 * Get a monster image
	 */
	Graphics::ManagedSurface getMonsterImage(int monsterNum);
};

} // namespace MM1
} // namespace MM

#endif
