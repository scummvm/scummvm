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

#ifndef HPL1_STD_MULTISET_H
#define HPL1_STD_MULTISET_H

#include "hpl1/std/tree.h"

namespace Hpl1 {
namespace Std {

template<class T, class CompFn = Common::Less<T> >
class multiset {
	using TreeT = Tree<T, T, Identity<T>, CompFn>;

public:
	using iterator = typename TreeT::BasicIterator;
	using const_iterator = typename TreeT::ConstIterator;

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

	void clear() {
		_items.clear();
	}

	bool empty() {
		return _items.size() == 0;
	}

	size_t size() {
		return _items.size();
	}

	/**
	 * Locate an item in the set
	 */
	iterator find(const T &item) {
		const auto it = _items.lowerBound(item);
		if (it != _items.end() && CompareEq(*it, item)) {
			return it;
		}
		return _items.end();
	}

	iterator insert(const T &item) {
		return _items.insert(item);
	}

	void erase(iterator item) {
		_items.erase(item);
	}

	void erase(iterator first, iterator last) {
		_items.erase(first, last);
	}

	size_t erase(const T &item) {
		size_t total = 0;
		for (auto it = _items.lowerBound(item); it != end() && CompareEq(*it, item);) {
			_items.erase(it++);
			++total;
		}
		return total;
	}

	/**
	 * Returns the number of keys that match the specified key
	 */
	size_t count(const T &item) const {
		size_t total = 0;
		for (auto it = _items.lowerBound(item); it != end() && CompareEq(*it, item); ++it)
			++total;
		return total;
	}

private:
	static bool CompareEq(const T &a, const T &b) {
		return !CompFn()(a, b) && !CompFn()(b, a);
	}

	TreeT _items;
};

} // namespace Std

} // namespace Hpl1

#endif
