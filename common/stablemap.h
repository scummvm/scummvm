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

#ifndef COMMON_STABLEMAP_H
#define COMMON_STABLEMAP_H

#include "common/rb_tree.h"

namespace Common {

/**
 * @defgroup common_stablemap stable map
 * @ingroup common
 *
 * @brief API for operating on a key value map
 *
 * @{
 */

/**
 * Ordered associative container meant as a drop-in replacement
 * for the C++ standard library's std::map.
 */
template<class Key, class Val, class CompFunc = Common::Less<Key> >
class StableMap {
	using TreeT = RBTree<Pair<Key, Val>, Key, PairFirst<Key, Val>, CompFunc>;

public:
	using value_type = Pair<Key, Val>;                    /*!< Type of the value obtained by dereferencing a StableMap iterator. */
	using iterator = typename TreeT::BasicIterator;       /*!< StableMap iterator. */
	using const_iterator = typename TreeT::ConstIterator; /*!< Const-qualified StableMap iterator. */

	/** Clears the contents of the map */
	void clear() {
		_items.clear();
	}

	/** Return an iterator pointing to the first element in the map. */
	iterator begin() {
		return _items.begin();
	}

	/** Return an iterator pointing to the last element in the map. */
	iterator end() {
		return _items.end();
	}

	/** Return a const iterator pointing to the first element in the map. */
	const_iterator begin() const {
		return _items.begin();
	}

	/** Return a const iterator pointing to the last element in the map. */
	const_iterator end() const {
		return _items.end();
	}

	/**
	 * Returns an iterator to the first item thas is not less than @p key
	 * in the map (or end() if this cannot be found).
	 */
	const_iterator lower_bound(const Key &key) const {
		return _items.lowerBound(key);
	}

	/**
	 * Returns a const iterator to the first item thas is not less than @p key
	 * in the map (or end() if this cannot be found).
	 */
	iterator lower_bound(const Key &key) {
		return _items.lowerBound(key);
	}

	/**
	 * Returns an iterator to the first item bigger than @p key
	 * in the map (or end() if this cannot be found).
	 */
	iterator upper_bound(const Key &key) {
		return _items.upperBound(key);
	}

	/**
	 * Returns an iterator to the item with the given key,
	 * or end() if the item was not found.
	 */
	iterator find(const Key &theKey) {
		iterator it = _items.lowerBound(theKey);
		if (it != this->end() && compareEqual(it->first, theKey))
			return it;
		return this->end();
	}

	/**
	 * Returns a const iterator to the item with the given key,
	 * or end() if the item was not found.
	 */
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
			iterator i = _items.insert(value_type(theKey, Val()));
			return i->second;
		}
		return it->second;
	}

	/** Erases the item in the map pointed by @p it .*/
	iterator erase(iterator it) {
		return _items.erase(it);
	}

	/**
	 * Erase the elements from @p first to @p last and return an iterator pointing to the next element in the map.
	 * @note
	 * If [first, last) is not a valid range for the map, the behaviour is undefined.
	 */
	iterator erase(iterator first, iterator last) {
		return _items.erase(first, last);
	}

	/**
	 * Erases the item with the given key.
	 * Returns the number of elements removed (0 or 1)
	 */
	size_t erase(const Key &theKey) {
		iterator it = find(theKey);
		if (it != this->end()) {
			erase(it);
			return 1;
		}
		return 0;
	}

	/**
	 * Tries to insert the value @p val in the map. If the value is inserted,
	 * the pair (it, true), where it is the iterator to the inserted value,
	 * is returned, otherwise (lower_bound(val), false) is returned
	 */
	Pair<iterator, bool> insert(const value_type &val) {
		iterator it = _items.lowerBound(val.first);
		if (it == this->end() || !compareEqual(it->first, val.first))
			return {_items.insert(val), true};
		return {it, false};
	}

	/** Returns the size of the map. */
	size_t size() const {
		return _items.size();
	}

	/**
	 * Returns true if the map is empty.
	 * Shorthand for:
	 * @code
	 * map.size() == 0
	 * @endcode
	 */
	bool empty() const {
		return _items.isEmpty();
	}

	/**
	 * Returns the number of elements with a matching key
	 */
	size_t count(const Key &key) {
		int count_ = 0;
		for (iterator it = this->begin(); it != this->end(); ++it) {
			if (compareEqual(it->first, key))
				++count_;
		}
		return count_;
	}

private:
	bool compareEqual(const Key &a, const Key &b) const {
		return !_comp(a, b) && !_comp(b, a);
	}

	TreeT _items;
	CompFunc _comp;
};

/** @} */

} // End of namespace Common

#endif
