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

#ifndef ULTIMA8_STD_CONTAINERS_H
#define ULTIMA8_STD_CONTAINERS_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/stack.h"

namespace Ultima8 {
namespace std {

template<class T>
class vector : public Common::Array<T> {
public:
	typedef T reference;
	typedef const T const_reference;

	Common::Array<T>::iterator erase(Common::Array<T>::iterator pos) {
		return Common::Array<T>::erase(pos);
	}

	Common::Array<T>::iterator erase(Common::Array<T>::iterator first, Common::Array<T>::iterator last) {
		bool flag = false;
		for (Common::Array<T>::iterator i = first; i != Common::Array<T>::end() && !flag; ) {
			flag = i == last;
			i = erase(i);
		}
	}
};

template<class Key, class Val, class HashFunc = Common::Hash<Key>,
         class EqualFunc = Common::EqualTo<Key> >
class map : public Common::HashMap<Key, Val, HashFunc, EqualFunc> {
};

template<class VAL>
class deque : public Common::List<VAL> {
};

template<class VAL>
class list : public Common::List<VAL> {
public:
	Common::List<VAL>::iterator insert(Common::List<VAL>::iterator pos, const VAL &element) {
		Common::List<VAL>::insert(pos, element);
		return pos;
	}
};

template<class VAL>
class stack : public Common::Stack<VAL> {
};

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
		return c.front();
	}

	void push(const typename _Container::value_type &_Val) {
		c.push_back(_Val);
		//push_heap(c.begin(), c.end(), comp);
	}

	void pop() {
		//pop_heap(c.begin(), c.end(), comp);
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

} // End of namespace std
} // End of namespace Ultima8

#endif