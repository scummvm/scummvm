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

#ifndef HPL1_STD_SET_H
#define HPL1_STD_SET_H

#include "common/rb_tree.h"

namespace Hpl1 {
namespace Std {

template<class T, class CompFn = Common::Less<T> >
class set {
	using TreeT = Common::RBTree<T, T, Common::Identity<T>, CompFn>;

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

	/**
	 * Insert an element at the sorted position.
	 */
	Common::Pair<iterator, bool> insert(const T &item) {
		const auto it = _items.lowerBound(item);
		if (it == _items.end() || !CompareEq(*it, item)) {
			const auto position = _items.insert(it, item);
			return {position, true};
		}
		return {it, false};
	}

	void erase(iterator item) {
		_items.erase(item);
	}

	void erase(iterator first, iterator last) {
		_items.erase(first, last);
	}

	size_t erase(const T &item) {
		iterator it = find(item);
		if (it != end()) {
			_items.erase(it);
			return 1;
		}
		return 0;
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
