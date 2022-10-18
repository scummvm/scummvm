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
	typedef T reference;
	typedef const T const_reference;

	vector() : Common::Array<T>() {}
	vector(size_t newSize) : Common::Array<T>() {
		Common::Array<T>::resize(newSize);
	}
	vector(size_t newSize, const T elem) {
		resize(newSize, elem);
	}

	void swap(vector &arr) {
		SWAP(this->_capacity, arr._capacity);
		SWAP(this->_size, arr._size);
		SWAP(this->_storage, arr._storage);
	}

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

	void pop_front() {
		Common::Array<T>::remove_at(0);
	}

	void resize(size_t newSize) {
		Common::Array<T>::resize(newSize);
	}
	void resize(size_t newSize, const T elem) {
		size_t oldSize = Common::Array<T>::size();
		resize(newSize);
		for (size_t idx = oldSize; idx < newSize; ++idx)
			this->operator[](idx) = elem;
	}

	T at(size_t index) const {
		return (*this)[index];
	}
};

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
};

template<class VAL>
class deque : public Common::List<VAL> {
public:
	VAL operator[](uint index) {
		for (typename Common::List<VAL>::iterator it = this->begin();
				it != this->end(); ++it, --index) {
			if (index == 0)
				return *it;
		}

		error("Invalid index");
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
	typename Common::List<T>::iterator insert(typename Common::List<T>::iterator pos,
			const T &element) {
		Common::List<T>::insert(pos, element);
		return pos;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(Common::List<T>::reverse_begin());
	}
	reverse_iterator rend() {
		return reverse_iterator(Common::List<T>::end());
	}
};

/**
 * Queue ordered by a provided priority function
 * NOTE: Unlike in the C std library, we have to provde a comparitor that sorts
 * the array so that the smallest priority comes last
 */
template <class _Ty, class _Container, class _Pr>
class priority_queue {
public:
	priority_queue() : c(), comp() {}

	explicit priority_queue(const _Pr &_Pred) : c(), comp(_Pred) {}

	priority_queue(const _Pr &_Pred, const _Container &_Cont) : c(_Cont), comp(_Pred) {
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	priority_queue(_InIt _First, _InIt _Last, const _Pr &_Pred, const _Container &_Cont) : c(_Cont), comp(_Pred) {
		c.insert(c.end(), _First, _Last);
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	priority_queue(_InIt _First, _InIt _Last) : c(_First, _Last), comp() {
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	priority_queue(_InIt _First, _InIt _Last, const _Pr &_Pred) : c(_First, _Last), comp(_Pred) {
		make_heap(c.begin(), c.end(), comp);
	}

	bool empty() const {
		return c.empty();
	}

	size_t size() const {
		return c.size();
	}

	typename _Container::const_reference top() const {
		return c.back();
	}

	void push(const typename _Container::value_type &_Val) {
		c.push_back(_Val);
		Common::sort(c.begin(), c.end(), comp);
	}

	void pop() {
		c.pop_back();
	}

	void swap(priority_queue &_Right) {
		SWAP(c, _Right.c);
		SWAP(comp, _Right.comp);
	}

protected:
	_Container c;
	_Pr comp;
};

} // End of namespace Std
} // End of namespace Ultima

#endif
