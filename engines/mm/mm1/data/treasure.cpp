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
#include "mm/mm1/data/treasure.h"

namespace MM {
namespace MM1 {

#define TREASURE_COUNT 6
#define ARRAY_COUNT (TREASURE_COUNT + 3)

void Treasure::clear() {
	Common::fill(&_data[0], &_data[ARRAY_COUNT], 0);
}

void Treasure::clear0() {
	Common::fill(&_data[1], &_data[ARRAY_COUNT], 0);
}

byte &Treasure::operator[](uint i) {
	assert(i < ARRAY_COUNT);
	return _data[i];
}

bool Treasure::present() const {
	// Checks for items, treasure, and/or gems
	for (int i = 0; i < TREASURE_COUNT; ++i) {
		if (_data[i + 3])
			return true;
	}

	return false;
}

bool Treasure::hasItems() const {
	for (int i = 0; i < 3; ++i) {
		if (_items[i])
			return true;
	}

	return false;
}

byte Treasure::removeItem() {
	for (int i = 0; i < 3; ++i) {
		if (_items[i]) {
			byte result = _items[i];
			_items[i] = 0;
			return result;
		}
	}

	return 0;
}

void Treasure::synchronize(Common::Serializer &s) {
	s.syncBytes(_data, 9);
}

} // namespace MM1
} // namespace MM
