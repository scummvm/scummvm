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

#ifndef ULTIMA_STD_CONTAINERS_H
#define ULTIMA_STD_CONTAINERS_H

#include "common/algorithm.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/queue.h"
#include "common/stack.h"
#include "common/util.h"

namespace Ultima {
namespace Std {

template<class T>
class vector : public Common::Array<T> {
public:
	struct reverse_iterator {
	private:
		vector<T> *_owner;
		int _index;
	public:
		reverse_iterator(vector<T> *owner, int index) : _owner(owner), _index(index) {}
		reverse_iterator() : _owner(0), _index(-1) {}

		T &operator*() { return (*_owner)[_index]; }

		reverse_iterator &operator++() {
			--_index;
			return *this;
		}

		bool operator==(const reverse_iterator &rhs) {
			return _owner == rhs._owner && _index == rhs._index;
		}
		bool operator!=(const reverse_iterator &rhs) {
			return !operator==(rhs);
		}
	};

	struct const_reverse_iterator {
	private:
		const vector<T> *_owner;
		int _index;
	public:
		const_reverse_iterator(const vector<T> *owner, int index) : _owner(owner), _index(index) {
		}
		const_reverse_iterator() : _owner(0), _index(-1) {
		}

		const T operator*() const {
			return (*_owner)[_index];
		}

		const_reverse_iterator &operator++() {
			--_index;
			return *this;
		}

		bool operator==(const const_reverse_iterator &rhs) {
			return _owner == rhs._owner && _index == rhs._index;
		}
		bool operator!=(const const_reverse_iterator &rhs) {
			return !operator==(rhs);
		}
	};
public:
	constexpr vector() : Common::Array<T>() {}
	vector(size_t newSize) : Common::Array<T>(newSize) {}
	vector(size_t newSize, const T elem) : Common::Array<T>(newSize, elem) {}

	reverse_iterator rbegin() {
		return reverse_iterator(this, (int)Common::Array<T>::size() - 1);
	}
	reverse_iterator rend() {
		return reverse_iterator(this, -1);
	}
	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(this, (int)Common::Array<T>::size() - 1);
	}
	const_reverse_iterator rend() const {
		return const_reverse_iterator(this, -1);
	}
};

template<class T>
class list : public Common::List<T> {
public:
	struct reverse_iterator {
	private:
		typename Common::List<T>::iterator _it;
	public:
		reverse_iterator(typename Common::List<T>::iterator it) : _it(it) {}
		reverse_iterator() {}

		T operator*() const { return *_it; }

		reverse_iterator &operator++() {
			--_it;
			return *this;
		}

		bool operator==(const reverse_iterator &rhs) { return _it == rhs._it; }
		bool operator!=(const reverse_iterator &rhs) { return _it != rhs._it; }
	};
public:
	reverse_iterator rbegin() {
		return reverse_iterator(Common::List<T>::reverse_begin());
	}
	reverse_iterator rend() {
		return reverse_iterator(Common::List<T>::end());
	}
};

} // End of namespace Std
} // End of namespace Ultima

#endif
