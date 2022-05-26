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

#ifndef MM1_DATA_INT_ARRAY_H
#define MM1_DATA_INT_ARRAY_H

#include "common/array.h"

namespace MM {
namespace MM1 {

class IntArray : public Common::Array<uint> {
public:
	IntArray() : Common::Array<uint>() {}

	int indexOf(uint val) const {
		for (uint i = 0; i < size(); ++i) {
			if ((*this)[i] == val)
				return i;
		}

		return -1;
	}

	/**
	 * Returns true if the array contains the value
	 */
	bool contains(uint val) {
		return indexOf(val) != -1;
	}

	/**
	 * Removes a given item from the array
	 */
	void remove(uint val) {
		int idx = indexOf(val);
		if (idx != -1)
			remove_at(idx);
	}

	/**
	 * Adds an item to the array
	 */
	void push_back(uint val) {
		assert(!contains(val));
		Common::Array<uint>::push_back(val);
	}
};

} // namespace MM1
} // namespace MM

#endif
