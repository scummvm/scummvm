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

#ifndef AGS_STD_MAP_H
#define AGS_STD_MAP_H

#include "common/hashmap.h"
#include "ags/lib/std/utility.h"

namespace AGS3 {
namespace std {

template<class Key, class Val, class CompFunc = Common::Less<Key> >
class map {
	struct KeyValue {
		Key _key;
		Val _value;
	};
private:
	Common::Array<KeyValue> _items;
	CompFunc _comp;
	public:
	using iterator = typename Common::Array<KeyValue>::iterator;
	using const_iterator = typename Common::Array<KeyValue>::const_iterator;

	/**
	 * Clears the map
	 */
	void clear() {
		_items.clear();
	}

	/**
	 * Gets the iterator start
	 */
	iterator begin() {
		return _items.begin();
	}

	/**
	 * Get the iterator end
	 */
	iterator end() {
		return _items.end();
	}

	/**
	 * Get the const iterator start
	 */
	const_iterator begin() const {
		return _items.begin();
	}

	/**
	 * Get the const iterator end
	 */
	const_iterator end() const {
		return _items.end();
	}

	/**
	 * Returns an iterator for the first element of the map that is
	 * not less than the given key
	 */
	const_iterator lower_bound(const Key &theKey) const {
		const_iterator first = this->begin();
		const_iterator it;
		int count_ = _items.size(), step;

		while (count_ > 0) {
			it = first;
			step = count_ / 2;
			it += step;

			if (_comp(it->_key, theKey)) {
				first = ++it;
				count_ -= step + 1;
			} else {
				count_ = step;
			}
		}

		return first;
	}

	iterator lower_bound(const Key &theKey) {
		iterator first = this->begin();
		iterator it;
		int count_ = _items.size(), step;

		while (count_ > 0) {
			it = first;
			step = count_ / 2;
			it += step;

			if (_comp(it->_key, theKey)) {
				first = ++it;
				count_ -= step + 1;
			} else {
				count_ = step;
			}
		}

		return first;
	}

	/**
	 * Find the entry with the given key
	 */
	iterator find(const Key &theKey) {
		iterator it = this->lower_bound(theKey);

		if (it != this->end() && it->_key == theKey)
			return it;
		return this->end();
	}

	const_iterator find(const Key &theKey) const {
		const_iterator it = this->lower_bound(theKey);

		if (it != this->end() && it->_key == theKey)
			return it;
		return this->end();
	}

	/**
	 * Square brackets operator accesses items by key, creating if necessary
	 */
	Val &operator[](const Key &theKey) {
		iterator it = this->lower_bound(theKey);
		if (it == this->end() || it->_key != theKey) {
			size_t idx = it - this->begin();
			_items.insert_at(idx, KeyValue());
			_items[idx]._key = theKey;
			return _items[idx]._value;
		} else {
			return _items[it - this->begin()]._value;
		}
	}

	/**
	 * Erases an entry in the map
	 */
	iterator erase(iterator it) {
		iterator next = it;
		++next;
		_items.remove_at(it - begin());
		return next;
	}

	iterator erase(const Key &theKey) {
		return erase(find(theKey));
	}

	/**
	 * Returns the size of the map
	 */
	size_t size() const {
		return _items.size();
	}

	/**
	 * Returns the number of elements with a matching key
	 */
	size_t count(const Key &theKey) {
		int count_ = 0;
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (it->_key == theKey)
				++count_;
		}

		return count_;
	}
};

template<class Key, class Val, class HashFunc = Common::Hash<Key>,
         class EqualFunc = Common::EqualTo<Key> >
class unordered_map : public Common::HashMap<Key, Val, HashFunc, EqualFunc> {
public:
	pair<Key, Val> insert(pair<Key, Val> elem) {
		// unordered_map doesn't replace already existing keys
		if (this->contains(elem.first))
			return pair<Key, Val>(elem.first, this->operator[](elem.first));

		// Add item to map
		this->operator[](elem.first) = elem.second;
		return elem;
	}

	void reserve(size_t size) {
		// No implementation
	}
};

} // namespace std
} // namespace AGS3

#endif
