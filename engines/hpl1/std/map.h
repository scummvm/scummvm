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

#ifndef HPL1_STD_MAP_H
#define HPL1_STD_MAP_H

#include "hpl1/std/pair.h"
#include "hpl1/std/tree.h"

namespace Hpl1 {
namespace Std {

template<class Key, class Val, class CompFunc = Common::Less<Key> >
class map {
	using TreeT = Tree<pair<Key, Val>, Key, PairKey<Key, Val>, CompFunc>;

public:
	using value_type = pair<Key, Val>;
	using iterator = typename TreeT::BasicIterator;
	using const_iterator = typename TreeT::ConstIterator;

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
	const_iterator lower_bound(const Key &key) const {
		return _items.lowerBound(key);
	}

	iterator lower_bound(const Key &key) {
		return _items.lowerBound(key);
	}

	iterator upper_bound(const Key &key) {
		return _items.upperBound(key);
	}

	/**
	 * Find the entry with the given key
	 */
	iterator find(const Key &theKey) {
		iterator it = _items.lowerBound(theKey);
		if (it != this->end() && compareEqual(it->first, theKey))
			return it;
		return this->end();
	}

	const_iterator find(const Key &theKey) const {
		const_iterator it = _items.lowerBound(theKey);
		if (it != this->end() && compareEqual(it->first, theKey))
			return it;
		return this->end();
	}

	/**
	 * Square brackets operator accesses items by key, creating if necessary
	 */
	Val &operator[](const Key &theKey) {
		iterator it = _items.lowerBound(theKey);
		if (it == this->end() || !compareEqual(it->first, theKey)) {
			return _items.insert(theKey).second;
		}
		return *it->second;
	}

	/**
	 * Erases an entry in the map
	 */
	iterator erase(iterator it) {
		return _items.erase(it);
	}

	iterator erase(iterator first, iterator last) {
		return _items.erase(first, last);
	}

	iterator erase(const Key &theKey) {
		iterator it = find(theKey);
		if (it != this->end())
			return erase(it);
		return it;
	}

	pair<iterator, bool> insert(const value_type &val) {
		iterator it = _items.lowerBound(val.first);
		if (it == this->end() || !compareEqual(it->first, val.first))
			return {_items.insert(val), true};
		return {it, false};
	}

	/**
	 * Returns the size of the map
	 */
	size_t size() const {
		return _items.size();
	}

	bool empty() const {
		return _items.isEmpty();
	}

	/**
	 * Returns the number of elements with a matching key
	 */
	size_t count(const Key &theKey) {
		int count_ = 0;
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (compareEqual(it->first, theKey))
				++count_;
		}
		return count_;
	}

private:
	bool compareEqual(const Key &a, const Key &b) {
		return !_comp(a, b) && !_comp(b, a);
	}

	TreeT _items;
	CompFunc _comp;
};

} // namespace Std
} // namespace Hpl1

#endif
