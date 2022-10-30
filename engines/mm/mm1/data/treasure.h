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

#ifndef MM1_DATA_TREASURE_H
#define MM1_DATA_TREASURE_H

#include "common/scummsys.h"

namespace MM {
namespace MM1 {

enum ContainerType {
	CLOTH_SACK = 0, LEATHER_SACK = 1, WOODEN_BOX = 2,
	WOODEN_CHEST = 3, IRON_BOX = 4, IRON_CHEST = 5,
	SILVER_BOX = 6, SILVER_CHEST = 7, GOLD_BOX = 8,
	GOLD_CHEST = 9, BLACK_BOX = 10
};

class Treasure {
private:
	byte _data[9];
public:
	byte &_container = _data[2];

	Treasure() {
		clear();
	}

	byte &operator[](uint i);

	/**
	 * Clears the treasure list
	 */
	void clear();

	/**
	 * Returns true if any treasure has been assigned
	 */
	bool present() const;
};

} // namespace MM1
} // namespace MM

#endif
