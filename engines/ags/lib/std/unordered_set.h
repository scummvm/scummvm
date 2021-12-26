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

#ifndef AGS_STD_UNORDERED_SET_H
#define AGS_STD_UNORDERED_SET_H

#include "common/array.h"
//#include "ags/lib/std/unordered_set.h"

namespace AGS3 {
namespace std {

/**
 * Unordered set
 * TODO: If needed, implement containers of items by unique hash of key
 */
template <class T, class Hash = Common::Hash<T>, class Pred = Common::EqualTo<T> >
class unordered_set : public Common::Array<T> {
private:
	Hash _hash;
	Pred _comparitor;
	public:
	struct Entry {
		const T &_value;
		Entry(const T &item) : _value(item) {}
	};
public:
	using iterator = typename Common::Array<T>::iterator;
	using const_iterator = typename Common::Array<T>::const_iterator;

	unordered_set() {}

	/**
	 * Locate an item in the set
	 */
	iterator find(const T &item) {
		iterator it;
		for (it = this->begin(); it != this->end() && *it != item; ++it) {
		}

		return it;
	}

	/**
	 * Adds an item
	 */
	Entry insert(const T &item) {
		this->push_back(item);
		return Entry(item);
	}

	/**
	 * Returns the number of keys that match the specified key
	 */
	size_t count(const T item) const {
		size_t total = 0;
		for (const_iterator it = this->begin(); it != this->end(); ++it) {
			if (*it == item)
				++total;
			else if (!_comparitor(item, *it))
				// Passed beyond possibility of matches
				break;
		}

		return total;
	}
};

} // namespace std
} // namespace AGS3

#endif
