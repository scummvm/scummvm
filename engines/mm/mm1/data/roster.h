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

#ifndef MM1_UTILS_ROSTER_H
#define MM1_UTILS_ROSTER_H

#include "common/serializer.h"
#include "mm/mm1/data/char.h"

namespace MM {
namespace MM1 {

#define ROSTER_COUNT 18
#define PARTY_COUNT 6

struct Roster {
private:
	Common::String rosterSaveName() const;
public:
	Character _items[ROSTER_COUNT];
	Town _towns[ROSTER_COUNT] = { NO_TOWN };

	Character &operator[](uint idx) {
		assert(idx < ROSTER_COUNT);
		return _items[idx];
	}

	/**
	 * Synchronizes the contents of the roster
	 */
	void synchronize(Common::Serializer &s);

	/**
	 * Load the roster
	 */
	void load();

	/**
	 * Save the roster
	 */
	void save();

	/**
	 * Deletes a character
	 */
	void remove(Character *entry);

	/**
	 * Returns true if the roster is empty
	 */
	bool empty() const;

	/**
	 * Returns true if the roster is full
	 */
	bool full() const;
};

} // namespace MM1
} // namespace MM

#endif
