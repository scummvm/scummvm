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
 */

#ifndef COMMON_LIST_INTERN_H
#define COMMON_LIST_INTERN_H

#include "common/scummsys.h"

namespace Common {

template<typename T> class List;


namespace ListInternal {
	struct NodeBase {
		NodeBase *_prev;
		NodeBase *_next;
	};

	template<typename T>
	struct Node : public NodeBase {
		T _data;

		Node(const T &x) : _data(x) {}
	};

	template<typename T> struct ConstIterator;

	template<typename T>
	struct Iterator {
		typedef Iterator<T>	Self;
		typedef Node<T> *	NodePtr;
		typedef T &			ValueRef;
		typedef T *			ValuePtr;
		typedef T			ValueType;

		NodeBase *_node;
		friend class ConstIterator<T>;
		private:		
		bool _forward;		
		public:

		Iterator() : _node(0) {}
		explicit Iterator(NodeBase *node) : _node(node), _forward(true)  {}
                explicit Iterator(NodeBase *node,bool forward) : _node(node), _forward(forward) {}

		// Prefix inc
		Self &operator++() {
			if (_node) {
				if (_forward)
					_node = _node->_next;
				else
					_node = _node->_prev;
			}
			return *this;
		}
		// Postfix inc
		Self operator++(int) {
			Self tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Self &operator--() {
			if (_node) {
				if (_forward)
					_node = _node->_prev;
				else
					_node = _node->_next;
			}
			return *this;
		}
		// Postfix dec
		Self operator--(int) {
			Self tmp(_node);
			--(*this);
			return tmp;
		}
		ValueRef operator*() const {
			assert(_node);
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}

		bool operator==(const Self &x) const {
			return _node == x._node;
		}

		bool operator!=(const Self &x) const {
			return _node != x._node;
		}
	};
	/*
	template<typename T>
	struct ReverseIterator: private Iterator<T> {

		typedef ReverseIterator<T>	Self;//TODO Think about this
		typedef Node<T> *	NodePtr;
		typedef T &			ValueRef;
		typedef T *			ValuePtr;
		typedef T			ValueType;
		NodeBase *_node;

		ReverseIterator() : _node(0) {}
		explicit ReverseIterator(NodeBase *node) : _node(node) { _node = node; }//FIXME
		
		// Prefix inc
		Self &operator++() {
			if (_node)
				_node = _node->_prev;
			return *this;
		}
		// Postfix inc
		Self operator++(int) {
			Self tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Self &operator--() {
			if (_node)
				_node = _node->_next;
			return *this;
		}
		// Postfix dec
		Self operator--(int) {
			Self tmp(_node);
			--(*this);
			return tmp;
		}
		ValueRef operator*() const {
			assert(_node);//TODO Why is that here?
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}
		
		bool operator==(const Self &x) const {
			return _node == x._node;
		}

		bool operator!=(const Self &x) const {
			return _node != x._node;
		}
		bool operator==(const Iterator<T> &x) const {
			return _node == x._node;
		}

		bool operator!=(const Iterator<T> &x) const {
			return _node != x._node;
		}

	};*/

	template<typename T>
	struct ConstIterator {
		typedef ConstIterator<T>	Self;
		typedef const Node<T> *	NodePtr;
		typedef const T &		ValueRef;
		typedef const T *		ValuePtr;

		const NodeBase *_node;
		private:
		bool _forward;
		public:
		ConstIterator() : _node(0), _forward(true) {}
		explicit ConstIterator(const NodeBase *node) : _node(node), _forward(true) {}
		explicit ConstIterator(const NodeBase *node, bool forward) : _node(node), _forward(true) {}
		ConstIterator(const Iterator<T> &x) : _node(x._node), _forward(x._forward) {}

		// Prefix inc
		Self &operator++() {
			if (_node) {
				if (_forward)
					_node = _node->_next;
				else
					_node = _node->_prev;
			}
			return *this;
		}
		// Postfix inc
		Self operator++(int) {
			Self tmp(_node);
			++(*this);
			return tmp;
		}
		// Prefix dec
		Self &operator--() {
			if (_node) {
				if (_forward)
					_node = _node->_prev;
				else
					_node = _node->_next;
			}
			return *this;
		}
		// Postfix dec
		Self operator--(int) {
			Self tmp(_node);
			--(*this);
			return tmp;
		}
		ValueRef operator*() const {
			assert(_node);
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}

		bool operator==(const Self &x) const {
			return _node == x._node;
		}

		bool operator!=(const Self &x) const {
			return _node != x._node;
		}
	};
/*
	template<typename T>
	struct ConstReverseIterator: public ConstIterator<T> {

		typedef ConstReverseIterator<T>	Self;//TODO Think about this
		typedef Node<T> *	NodePtr;
		typedef T &			ValueRef;
		typedef T *			ValuePtr;
		typedef T			ValueType;
		NodeBase *_node;

		ConstReverseIterator() : _node(0) {}
		explicit ConstReverseIterator(const NodeBase *node) : _node(node) {}
		ConstReverseIterator(const Iterator<T> &x) : _node(x._node) {}//TODO
		
		// Prefix inc
		Self &operator++() {
			if (_node)
				_node = _node->_prev;
			return *this;
		}
		// Prefix dec
		Self &operator--() {
			if (_node)
				_node = _node->_next;
			return *this;
		}
	};*/


	template<typename T>
	bool operator==(const Iterator<T>& a, const ConstIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const Iterator<T>& a, const ConstIterator<T>& b) {
		return a._node != b._node;
	}
}


} // End of namespace Common

#endif
