/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_STD_SET_H
#define AGS_STD_SET_H

namespace AGS3 {
namespace std {

template<class T>
class set {
	struct Comparitor {
		bool operator()(const T &a, const T &b) const {
			return a == b;
		}
	};

	class Items : public Common::Array<T> {
	public:
		void swap(Items &arr) {
			SWAP(this->_capacity, arr._capacity);
			SWAP(this->_size, arr._size);
			SWAP(this->_storage, arr._storage);
		}
	};
private:
	Items _items;
	Comparitor _comparitor;
public:
	typedef T *iterator;
	typedef const T *const_iterator;

	iterator begin() {
		return _items.begin();
	}
	iterator end() {
		return _items.end();
	}
	const_iterator begin() const {
		return _items.begin();
	}
	const_iterator end() const {
		return _items.end();
	}

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
	void swap(set<T> &arr) {
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
	bool empty() const {
		return _items.empty();
	}

	/**
	 * Returns the number of matching entries
	 */
	size_t count(const T item) const {
		size_t total = 0;
		for (const_iterator it = begin(); it != end(); ++it) {
			if (*it == item)
				++total;
		}

		return total;
	}
};

} // namespace std
} // namespace AGS3

#endif
