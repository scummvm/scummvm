/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "common/scummsys.h"
#include <assert.h>

namespace Common {

/*
TODO: Add a true list class, based on a linked list data structure

template <class T>
class List {
protected:
	template <class T>
	class Node {
		Node<T> *_prev;
		Node<T> *_next;
		T _data;
	}

	template <class T>
	class Iterator {
	friend class List<T>;
	private:
		Node<T> *_node;
	
	public:
		Node<T> &operator++() {
			_node = _node->_next;
		}
		
		T& operator*() const {
			return _node->_data;
		}
		T* operator->() const {
			return &(_node->_data);
		}
	};

	Iterator<T> *_anchor;

public:
	typedef Node<T> *iterator;
	typedef const Node<T> *const_iterator;

public:
	List<T>() : _anchor(...) {}
	List<T>(const List<T>& list) : _anchor(...) {
		... copy list ...
	}

	~List<T>() {
		if (_data)
			delete [] _data;
	}

	void push_back(const T& element) {
		...
	}

	void push_back(const List<T>& list) {
		...
	}

	void insert(iterator pos, const T& element) {
		...
	}

	void insert(iterator pos, iterator beg, Iterator end) {
		...
	}

	void erase(iterator beg, Iterator end) {
		...
	}

	void remove(const T &val) {
	}


	List<T>& operator  =(const List<T>& list) {
		// Careful here: handle self-assignment properly! I.e. situations like
		//    List x;
		//    ...
		//    x = x;
		// In particular, we can't just do this:
		//    clear();
		//    insert(_first, list.begin(), list.end());
		...
	}

	uint size() const {
		int size = 0;
		for (const_iterator i = begin(); i != end(); ++i)
			size++;
		return size;
	}

	void clear() {
		erase(begin(), end());
	}
	
	bool isEmpty() const { 
		return (_anchor._node == _anchor._node._next);
	}


	iterator		begin() {
		Iterator iter = _anchor;
		return ++iter;
	}

	iterator		end() {
		return _anchor;
	}

	const_iterator	begin() const {
		Iterator iter = _anchor;
		return ++iter;
	}

	const_iterator	end() const {
		return _anchor;
	}
};
*/

} // End of namespace Common

#endif
