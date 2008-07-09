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

#include "common/scummsys.h"

namespace Common {

/**
 * Simple double linked list, modeled after the list template of the standard
 * C++ library.
 */
template<class t_T>
class List {
protected:
#if defined (_WIN32_WCE) || defined (_MSC_VER)
//FIXME evc4 and msvc7 doesn't like it as protected member
public:
#endif
	struct NodeBase {
		NodeBase *_prev;
		NodeBase *_next;
	};

	template <class t_T2>
	struct Node : public NodeBase {
		t_T2 _data;

		Node(const t_T2 &x) : _data(x) {}
	};

	template<class t_T2>
	class Iterator {
		template<class T> friend class Iterator;
		friend class List<t_T>;
		NodeBase *_node;

#if !defined (__WINSCW__)
		explicit Iterator(NodeBase *node) : _node(node) {}
#else
		Iterator(NodeBase *node) : _node(node) {}
#endif

	public:
		Iterator() : _node(0) {}
		template<class T>
		Iterator(const Iterator<T> &c) : _node(c._node) {}

		template<class T>
		Iterator<t_T2> &operator=(const Iterator<T> &c) {
			_node = c._node;
			return *this;
		}

		// Prefix inc
		Iterator<t_T2> &operator++() {
			if (_node)
				_node = _node->_next;
			return *this;
		}
		// Postfix inc
		Iterator<t_T2> operator++(int) {
			Iterator tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Iterator<t_T2> &operator--() {
			if (_node)
				_node = _node->_prev;
			return *this;
		}
		// Postfix dec
		Iterator<t_T2> operator--(int) {
			Iterator tmp(_node);
			--(*this);
			return tmp;
		}
		t_T2 &operator*() const {
			assert(_node);
#if (__GNUC__ == 2) && (__GNUC_MINOR__ >= 95)
			return static_cast<List<t_T>::Node<t_T2> *>(_node)->_data;
#else
			return static_cast<Node<t_T2> *>(_node)->_data;
#endif
		}
		t_T2 *operator->() const {
			return &(operator*());
		}

		template<class T>
		bool operator==(const Iterator<T> &x) const {
			return _node == x._node;
		}

		template<class T>
		bool operator!=(const Iterator<T> &x) const {
			return _node != x._node;
		}
	};

	NodeBase _anchor;

public:
	typedef Iterator<t_T>			iterator;
	typedef Iterator<const t_T>	const_iterator;

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

	void push_front(const t_T &element) {
		insert(begin(), element);
	}

	void push_back(const t_T &element) {
		insert(end(), element);
	}

	void insert(iterator pos, const t_T &element) {
		NodeBase *newNode = new Node<t_T>(element);

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
		Node<t_T> *node = static_cast<Node<t_T> *>(pos._node);
		prev->_next = next;
		next->_prev = prev;
		delete node;
		return iterator(next);
	}

	iterator reverse_erase(iterator pos) {
		assert(pos != end());

		NodeBase *next = pos._node->_next;
		NodeBase *prev = pos._node->_prev;
		Node<t_T> *node = static_cast<Node<t_T> *>(pos._node);
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
		while (i != end())
			if (val == i.operator*())
				i = erase(i);
			else
				++i;
	}

	void pop_front() {
		iterator i = begin();
		i = erase(i);
	}


	List<t_T> &operator=(const List<t_T> &list) {
		if (this != &list) {
			iterator i;
			const_iterator j;

			for (i = begin(), j = list.begin();  (i != end()) && (j != list.end()) ; ++i, ++j) {
				static_cast<Node<t_T> *>(i._node)->_data = static_cast<Node<t_T> *>(j._node)->_data;
			}

			if (i == end())
				insert(i, j, list.end());
			else
				erase(i, end());
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
