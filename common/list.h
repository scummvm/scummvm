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

/**
 * Simple double linked list, modelled after the list template of the standard
 * C++ library. 
 * Warning: as of now, this code is 100% untested.
 */
template <class T>
class List {
protected:
	struct NodeBase {
		NodeBase *_prev;
		NodeBase *_next;
	};
	
	template <class T2>
	struct Node : public NodeBase {
		T2 _data;
		
		Node<T2>(const T2 &x) : _data(x) {}
	};

	template <class T2>
	struct Iterator {
		NodeBase *_node;
		
		Iterator<T2>(NodeBase *node) : _node(node) {}

		// Prefix inc
		Iterator<T2> &operator++() {
			_node = _node->_next;
			return *this;
		}
		// Postfix inc
		Iterator<T2> operator++(int) {
			Iterator<T2> tmp = *this;
			_node = _node->_next;
			return tmp;
		}
		// Prefix dec
		Iterator<T2> &operator--() {
			_node = _node->_prev;
			return *this;
		}
		// Postfix dec
		Iterator<T2> operator--(int) {
			Iterator<T2> tmp = *this;
			_node = _node->_prev;
			return tmp;
		}
		T& operator*() const {
			return static_cast<Node<T2>*>(_node)->_data;
		}
		T* operator->() const {
			return &(operator*());
		}
		
		bool operator==(const Iterator<T2>& x) const {
			return _node == x._node;
		}
		
		bool operator!=(const Iterator<T2>& x) const {
			return _node != x._node;
		}
	};

	NodeBase *_anchor;

public:
	typedef Iterator<T>        iterator;
	typedef const Iterator<T>  const_iterator;

public:
	List<T>() {
		_anchor = new NodeBase;
		_anchor->_prev = _anchor;
		_anchor->_next = _anchor;
	}
	List<T>(const List<T>& list) {
		_anchor = new NodeBase;
		_anchor->_prev = _anchor;
		_anchor->_next = _anchor;

		insert(begin(), list.begin(), list.end());
	}

	~List<T>() {
		clear();
		delete _anchor;
	}

	void push_front(const T& element) {
		insert(begin(), element);
	}

	void push_back(const T& element) {
		insert(end(), element);
	}

	void insert(iterator pos, const T& element) {
		NodeBase *newNode = new Node<T>(element);
		
		newNode->_next = pos._node;
		newNode->_prev = pos._node->_prev;
		newNode->_prev->_next = newNode;
		newNode->_next->_prev = newNode;
	}

    template <typename iterator2>
	void insert(iterator pos, iterator2 first, iterator2 last) {
		for (; first != last; ++first)
			insert(pos, *first);
	}

	iterator erase(iterator pos) {
		assert(pos != end());

		NodeBase *next = pos._node->_next;
		NodeBase *prev = pos._node->_prev;
		Node<T> *node = static_cast<Node<T> *>(pos._node);
		prev->_next = next;
		next->_prev = prev;
		delete node;
		return next;
	}

	iterator erase(iterator first, iterator last) {
		while (first != last)
			erase(first++);
		return last;
	}

//	void remove(const T &val) {
//		...
//	}


	List<T>& operator  =(const List<T>& list) {
		if (this != &list) {
			// TODO: This could (and should) be optimized, by reusing
			// the existing nodes...
			clear();
			insert(begin(), list.begin(), list.end());
		}
		
		return *this;
	}

	uint size() const {
		int n = 0;
		for (const_iterator i = begin(); i != end(); ++i)
			++n;
		return n;
	}

	void clear() {
		erase(begin(), end());
	}
	
	bool isEmpty() const { 
		return (_anchor == _anchor->_next);
	}


	iterator		begin() {
		return _anchor->_next;
	}

	iterator		end() {
		return _anchor;
	}

	const_iterator	begin() const {
		return _anchor->_next;
	}

	const_iterator	end() const {
		return _anchor;
	}
};

} // End of namespace Common

#endif
