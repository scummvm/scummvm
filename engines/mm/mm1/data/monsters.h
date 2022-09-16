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

#include "common/str.h"

namespace MM {
namespace MM1 {

#define MONSTERS_COUNT 195

struct Monster {
	Common::String _name; // char _name[15];
	byte _count;
	byte _attr[16];
};

class Monsters {
private:
	Monster _monsters[MONSTERS_COUNT];

private:
	/**
	 * Get the next value from a read line
	 */
	byte getNextValue(Common::String &line);
public:
	Monsters() {}

	/**
	 * Load the monster list
	 */
	bool load();

	const Monster &operator[](uint i) {
		assert(i >= 1 && i <= MONSTERS_COUNT);
		return _monsters[i];
	}
};

} // namespace MM1
} // namespace MM

#endif
