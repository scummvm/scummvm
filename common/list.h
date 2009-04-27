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
 * $URL$
 * $Id$
 */

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "common/list_intern.h"

namespace Common {

/**
 * Simple double linked list, modeled after the list template of the standard
 * C++ library.
 */
template<typename t_T>
class List {
protected:
	typedef ListInternal::NodeBase		NodeBase;
	typedef ListInternal::Node<t_T>		Node;

	NodeBase _anchor;

public:
	typedef ListInternal::Iterator<t_T>		iterator;
	typedef ListInternal::ConstIterator<t_T>	const_iterator;

	typedef t_T value_type;

public:
	List() {
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;
	}
	List(const List<t_T> &list) {
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;

		insert(begin(), list.begin(), list.end());
	}

	~List() {
		clear();
	}

	void insert(iterator pos, const t_T &element) {
		ListInternal::NodeBase *newNode = new Node(element);

		newNode->_next = pos._node;
		newNode->_prev = pos._node->_prev;
		newNode->_prev->_next = newNode;
		newNode->_next->_prev = newNode;
	}

	template<typename iterator2>
	void insert(iterator pos, iterator2 first, iterator2 last) {
		for (; first != last; ++first)
			insert(pos, *first);
	}

	iterator erase(iterator pos) {
		assert(pos != end());

		NodeBase *next = pos._node->_next;
		NodeBase *prev = pos._node->_prev;
		Node *node = static_cast<Node *>(pos._node);
		prev->_next = next;
		next->_prev = prev;
		delete node;
		return iterator(next);
	}

	iterator reverse_erase(iterator pos) {
		assert(pos != end());

		NodeBase *next = pos._node->_next;
		NodeBase *prev = pos._node->_prev;
		Node *node = static_cast<Node *>(pos._node);
		prev->_next = next;
		next->_prev = prev;
		delete node;
		return iterator(prev);
	}

	iterator erase(iterator first, iterator last) {
		while (first != last)
			erase(first++);

		return last;
	}

	void remove(const t_T &val) {
		iterator i = begin();
		const iterator e = end();
		while (i != e)
			if (val == *i)
				i = erase(i);
			else
				++i;
	}

	void push_front(const t_T &element) {
		// TODO: Bypass iterators here for improved efficency
		insert(begin(), element);
	}

	void push_back(const t_T &element) {
		// TODO: Bypass iterators here for improved efficency
		insert(end(), element);
	}

	void pop_front() {
		// TODO: Bypass iterators here for improved efficency
		erase(begin());
	}

	void pop_back() {
		// TODO: Bypass iterators here for improved efficency
		erase(reverse_begin());
	}

	t_T &front() {
		return static_cast<Node *>(_anchor._next)->_data;
	}

	const t_T &front() const {
		return static_cast<Node *>(_anchor._next)->_data;
	}

	t_T &back() {
		return static_cast<Node *>(_anchor._prev)->_data;
	}

	const t_T &back() const {
		return static_cast<Node *>(_anchor._prev)->_data;
	}

	List<t_T> &operator=(const List<t_T> &list) {
		if (this != &list) {
			iterator i;
			const iterator e = end();
			const_iterator i2;
			const_iterator e2 = list.end();

			for (i = begin(), i2 = list.begin();  (i != e) && (i2 != e2) ; ++i, ++i2) {
				static_cast<Node *>(i._node)->_data = static_cast<const Node *>(i2._node)->_data;
			}

			if (i == e)
				insert(i, i2, e2);
			else
				erase(i, e);
		}

		return *this;
	}

	uint size() const {
		int n = 0;
		for (const NodeBase *cur = _anchor._next; cur != &_anchor; cur = cur->_next)
			++n;
		return n;
	}

	void clear() {
		erase(begin(), end());
	}

	bool empty() const {
		return (&_anchor == _anchor._next);
	}


	iterator		begin() {
		return iterator(_anchor._next);
	}

	iterator		reverse_begin() {
		return iterator(_anchor._prev);
	}

	iterator		end() {
		return iterator(&_anchor);
	}

	const_iterator	begin() const {
		return const_iterator(_anchor._next);
	}

	const_iterator	reverse_begin() const {
		return const_iterator(_anchor._prev);
	}

	const_iterator	end() const {
		return const_iterator(const_cast<NodeBase*>(&_anchor));
	}
};

} // End of namespace Common

#endif
