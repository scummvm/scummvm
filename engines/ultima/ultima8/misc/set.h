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

#ifndef ULTIMA8_MISC_SET_H
#define ULTIMA8_MISC_SET_H

#include "common/algorithm.h"
#include "common/array.h"

namespace Ultima {
namespace Ultima8 {

template<class T>
class Set {
	struct Comparitor {
		bool operator()(const T &a, const T &b) const {
			return a == b;
		}
	};
private:
	Common::Array<T> _items;
	Comparitor _comparitor;
public:
	typedef T *iterator;
	typedef const T *const_iterator;

	iterator begin() { return _items.begin(); }
	iterator end() { return _items.end(); }
	const_iterator begin() const { return _items.begin(); }
	const_iterator end() const { return _items.end(); }

	/**
	 * Clear the set
	 */
	void clear() {
		_items.clear();
	}

	/**
	 * Inserts a new item
	 */
	void insert(T val) {
		_items.push_back(val);
		Common::sort(begin(), end(), _comparitor);
	}

	/**
	 * Inserts a range of items
	 */
	void insert(iterator first, iterator last) {
		for (; first != last; ++first)
			_items.push_back(*first);
		Common::sort(begin(), end(), _comparitor);
	}

	/**
	 * Swaps a set
	 */
	void swap(Set<T> &arr) {
		_items.swap(arr);
	}

	/**
	 * Find an item
	 */
	iterator find(const T item) {
		iterator it = begin();
		for (; it != end() && *it != item; ++it) {}
		return it;
	}
	const_iterator find(const T item) const {
		const_iterator it = begin();
		for (; it != end() && *it != item; ++it) {
		}
		return it;
	}
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
