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

#ifndef HPL1_STD_MULTIMAP_H
#define HPL1_STD_MULTIMAP_H

#include "common/array.h"
#include "hpl1/std/pair.h"

namespace Hpl1 {

namespace Std {

template<class Key, class Val, class CompFunc = Common::Less<Key> >
class multimap {
public:
	using value_type = pair<Key, Val>;
	using iterator = typename Common::Array<value_type>::iterator;
	using const_iterator = typename Common::Array<value_type>::const_iterator;

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
		return lowerBound(this->begin(), this->end(), key, [&](value_type const &p, Key const &k) { return _comp(p.first, k); });
	}

	iterator lower_bound(const Key &key) {
		return lowerBound(this->begin(), this->end(), key, [&](value_type const &p, Key const &k) { return _comp(p.first, k); });
	}

	iterator upper_bound(const Key &key) {
		return upperBound(this->begin(), this->end(), key, [&](Key const &k, value_type const &p) { return _comp(k, p.first); });
	}

	/**
	 * Find the entry with the given key
	 */
	iterator find(const Key &theKey) {
		iterator it = this->lower_bound(theKey);

		if (it != this->end() && compareEqual(it->first, theKey))
			return it;
		return this->end();
	}

	const_iterator find(const Key &theKey) const {
		const_iterator it = this->lower_bound(theKey);

		if (it != this->end() && compareEqual(it->first, theKey))
			return it;
		return this->end();
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
		const iterator first = find(theKey);
		iterator last = first;
		while (last != this->end() && compareEqual(last->first, theKey))
			++last;
		return _items.erase(first, last);
	}

	iterator insert(const value_type &val) {
		iterator it = this->lower_bound(val.first);
		size_t idx = it - this->begin();
		_items.insert_at(idx, val);
		return it;
	}

	/**
	 * Returns the size of the map
	 */
	size_t size() const {
		return _items.size();
	}

	bool empty() const {
		return _items.empty();
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

	Common::Array<value_type> _items;
	CompFunc _comp;
};

} // namespace Std
} // namespace Hpl1

#endif
