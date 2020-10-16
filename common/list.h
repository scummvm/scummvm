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

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "common/list_intern.h"

namespace Common {

/**
 * @defgroup common_list Lists
 * @ingroup common
 *
 * @brief API for managing doubly linked lists.
 *
 *		
 * @{
 */

/**
 * Simple doubly linked list, modeled after the list template of the standard
 * C++ library.
 */
template<typename t_T>
class List {
protected:
	typedef ListInternal::NodeBase		NodeBase; /*!< @todo Doc required. */
	typedef ListInternal::Node<t_T>		Node;     /*!< An element of the doubly linked list. */

	NodeBase _anchor; /*!< Pointer to the position of the element in the list. */

public:
	typedef ListInternal::Iterator<t_T>		iterator; /*!< List iterator. */
	typedef ListInternal::ConstIterator<t_T>	const_iterator; /*!< Const-qualified list iterator. */

	typedef t_T value_type; /*!< Value type of the list. */
	typedef uint size_type; /*!< Size type of the list. */

public:
	/**
	 * Construct a new empty list.
	 */
	List() {
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;
	}
	List(const List<t_T> &list) {  /*!< Construct a new list as a copy of the given @p list. */
		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;

		insert(begin(), list.begin(), list.end());
	}

	~List() {
		clear();
	}

	/**
	 * Insert an @p element before @p pos.
	 */
	void insert(iterator pos, const t_T &element) {
		insert(pos._node, element);
	}

	/**
	 * Insert elements from @p first to @p last before @p pos.
	 */
	template<typename iterator2>
	void insert(iterator pos, iterator2 first, iterator2 last) {
		for (; first != last; ++first)
			insert(pos, *first);
	}

	/**
	 * Delete the element at location @p pos and return an iterator pointing
	 * to the element after the one that was deleted.
	 */
	iterator erase(iterator pos) {
		assert(pos != end());
		return iterator(erase(pos._node)._next);
	}

	/**
	 * Delete the element at location @p pos and return an iterator pointing
	 * to the element before the one that was deleted.
	 */
	iterator reverse_erase(iterator pos) {
		assert(pos != end());
		return iterator(erase(pos._node)._prev);
	}

	/**
	 * Delete the elements between @p first and @p last (including @p first but not
	 * @p last). Return an iterator pointing to the element after the one
	 * that was deleted (that is, @p last).
	 */
	iterator erase(iterator first, iterator last) {
		NodeBase *f = first._node;
		NodeBase *l = last._node;
		while (f != l)
			f = erase(f)._next;
		return last;
	}

	/**
	 * Remove all elements that are equal to @p val from the list.
	 */
	void remove(const t_T &val) {
		NodeBase *i = _anchor._next;
		while (i != &_anchor)
			if (val == static_cast<Node *>(i)->_data)
				i = erase(i)._next;
			else
				i = i->_next;
	}

	/** Insert an @p element at the start of the list. */
	void push_front(const t_T &element) {
		insert(_anchor._next, element);
	}

	/** Append an @p element to the end of the list. */
	void push_back(const t_T &element) {
		insert(&_anchor, element);
	}

	/** Remove the first element of the list. */
	void pop_front() {
		assert(!empty());
		erase(_anchor._next);
	}

	/** Remove the last element of the list. */
	void pop_back() {
		assert(!empty());
		erase(_anchor._prev);
	}

	/** Return a reference to the first element of the list. */
	t_T &front() {
		return static_cast<Node *>(_anchor._next)->_data;
	}

	/** Return a reference to the first element of the list. */
	const t_T &front() const {
		return static_cast<Node *>(_anchor._next)->_data;
	}

	/** Return a reference to the last element of the list. */
	t_T &back() {
		return static_cast<Node *>(_anchor._prev)->_data;
	}

	/** Return a reference to the last element of the list. */
	const t_T &back() const {
		return static_cast<Node *>(_anchor._prev)->_data;
	}

    /** Assign a given @p list to this list. */
	List<t_T> &operator=(const List<t_T> &list) {
		if (this != &list) {
			iterator i;
			const iterator e = end();
			const_iterator i2;
			const_iterator e2 = list.end();

			for (i = begin(), i2 = list.begin(); (i != e) && (i2 != e2); ++i, ++i2) {
				static_cast<Node *>(i._node)->_data = static_cast<const Node *>(i2._node)->_data;
			}

			if (i == e)
				insert(i, i2, e2);
			else
				erase(i, e);
		}

		return *this;
	}

    /** Return the size of the list. */
	size_type size() const {
		size_type n = 0;
		for (const NodeBase *cur = _anchor._next; cur != &_anchor; cur = cur->_next)
			++n;
		return n;
	}

    /** Remove all elements from the list. */
	void clear() {
		NodeBase *pos = _anchor._next;
		while (pos != &_anchor) {
			Node *node = static_cast<Node *>(pos);
			pos = pos->_next;
			delete node;
		}

		_anchor._prev = &_anchor;
		_anchor._next = &_anchor;
	}

    /** Check whether the list is empty. */
	bool empty() const {
		return (&_anchor == _anchor._next);
	}

    /** Return an iterator to the start of the list.	
	 *  This can be used, for example, to iterate from the first element
	 *  of the list to the last element of the list.
	 */
	iterator		begin() {
		return iterator(_anchor._next);
	}

    /** Return a reverse iterator to the start of the list.	
	 *  This can be used, for example, to iterate from the last element
	 *  of the list to the first element of the list.
	 */
	iterator		reverse_begin() {
		return iterator(_anchor._prev);
	}

    /** Return an iterator to the end of the list. */
	iterator		end() {
		return iterator(&_anchor);
	}

    /** Return a const iterator to the start of the list.	
	 *  This can be used, for example, to iterate from the first element
	 *  of the list to the last element of the list.
	 */
	const_iterator	begin() const {
		return const_iterator(_anchor._next);
	}

    /** Return a const reverse iterator to the start of the list.	
	 *  This can be used, for example, to iterate from the last element
	 *  of the list to the first element of the list.
	 */
	const_iterator	reverse_begin() const {
		return const_iterator(_anchor._prev);
	}

    /** Return a const iterator to the end of the list. */
	const_iterator	end() const {
		return const_iterator(const_cast<NodeBase *>(&_anchor));
	}

protected:
    /**
	 * Erase an element at @p pos.
	 */
	NodeBase erase(NodeBase *pos) {
		NodeBase n = *pos;
		Node *node = static_cast<Node *>(pos);
		n._prev->_next = n._next;
		n._next->_prev = n._prev;
		delete node;
		return n;
	}

	/**
	 * Insert an @p element before @p pos.
	 */
	void insert(NodeBase *pos, const t_T &element) {
		ListInternal::NodeBase *newNode = new Node(element);
		assert(newNode);

		newNode->_next = pos;
		newNode->_prev = pos->_prev;
		newNode->_prev->_next = newNode;
		newNode->_next->_prev = newNode;
	}
};

/** @} */

} // End of namespace Common

#endif
