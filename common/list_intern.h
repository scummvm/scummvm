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

		Iterator() : _node(0) {}
		explicit Iterator(NodeBase *node) : _node(node) {}

		// Prefix inc
		Self &operator++() {
			if (_node)
				_node = _node->_next;
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
				_node = _node->_prev;
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

	template<typename T>
	struct ConstIterator {
		typedef ConstIterator<T>	Self;
		typedef const Node<T> *	NodePtr;
		typedef const T &		ValueRef;
		typedef const T *		ValuePtr;
		typedef const T			ValueType;

		const NodeBase *_node;

		ConstIterator() : _node(0) {}
		explicit ConstIterator(const NodeBase *node) : _node(node) {}
		ConstIterator(const Iterator<T> &x) : _node(x._node) {}

		// Prefix inc
		Self &operator++() {
			if (_node)
				_node = _node->_next;
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
				_node = _node->_prev;
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

	// Assumes it's a reverse of the Iterator class.
	// Further templating could be added if Iterator is ever derived from
	template<typename T>
	struct ReverseIterator {
		typedef ReverseIterator<T>	Self;
		typedef Node<T> *	NodePtr;
		typedef T &			ValueRef;
		typedef T *			ValuePtr;
		typedef T			ValueType;

		NodeBase *_node;

		// Constructor and Copy
		ReverseIterator() : _node(0) {}
		explicit ReverseIterator(NodeBase* node) : _node(node) {}
		explicit ReverseIterator(Iterator<T>& iter) : _node(iter._node) {}

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
			assert(_node);
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}

		bool operator==(const Self &x) const {
			if (_node)
				return _node == x._node;

			return false;
		}

		bool operator!=(const Self &x) const {
			if (_node)
				return _node != x._node;

			return false;
		}
	};

	// Assumes it's a reverse of the ConstIterator class.
	// Further templating could be added if ConstIterator is ever derived from
	template<typename T>
	struct ConstReverseIterator {
		typedef ConstReverseIterator<T>	Self;
		typedef const Node<T> *			NodePtr;
		typedef const T &				ValueRef;
		typedef const T *				ValuePtr;
		typedef const T					ValueType;

		const NodeBase *_node;

		// Constructor and Copy
		ConstReverseIterator() : _node(0) {}
		explicit ConstReverseIterator(NodeBase *node) : _node(node) {}
		explicit ConstReverseIterator(ConstIterator<T>& iter) : _node(iter._node) {}
		ConstReverseIterator(const ConstReverseIterator<T> &x) : _node(x._node) {}

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
			assert(_node);
			return static_cast<NodePtr>(_node)->_data;
		}
		ValuePtr operator->() const {
			return &(operator*());
		}

		bool operator==(const Self &x) const {
			if (_node)
				return _node == x._node;

			return false;
		}

		bool operator!=(const Self &x) const {
			if (_node)
				return _node != x._node;

			return false;
		}
	};

	// Iterator vs. ConstIterator
	template<typename T>
	bool operator==(const Iterator<T>& a, const ConstIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const Iterator<T>& a, const ConstIterator<T>& b) {
		return a._node != b._node;
	}

	// == and != are commutative
	template<typename T>
	bool operator==(const ConstIterator<T>& a, const Iterator<T>& b) {
		return b==a;
	}

	template<typename T>
	bool operator!=(const ConstIterator<T>& a, const Iterator<T>& b) {
		return b!=a;
	}

	// ReverseIterator vs. ConstReverseIterator
	template<typename T>
	bool operator==(const ReverseIterator<T>& a, const ConstReverseIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const ReverseIterator<T>& a, const ConstReverseIterator<T>& b) {
		return a._node != b._node;
	}

	// == and != are commutative
	template<typename T>
	bool operator==(const ConstReverseIterator<T>& a, const ReverseIterator<T>& b) {
		return b==a;
	}

	template<typename T>
	bool operator!=(const ConstReverseIterator<T>& a, const ReverseIterator<T>& b) {
		return b!=a;
	}

	// Iterator vs. ReverseIterator
	template<typename T>
	bool operator==(Iterator<T>& a, ReverseIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(Iterator<T>& a, ReverseIterator<T>& b) {
		return a._node != b._node;
	}

	// == and != are commutative
	template<typename T>
	bool operator==(ReverseIterator<T>& a, Iterator<T>& b) {
		return b==a;
	}

	template<typename T>
	bool operator!=(ReverseIterator<T>& a, Iterator<T>& b) {
		return b!=a;
	}

	// ReverseIterator vs ConstIterator
	template<typename T>
	bool operator==(const ReverseIterator<T>& a, const ConstIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const ReverseIterator<T>& a, const ConstIterator<T>& b) {
		return a._node != b._node;
	}

	// == and != are commutative
	template<typename T>
	bool operator==(const ConstIterator<T>& a, const ReverseIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const ConstIterator<T>& a, const ReverseIterator<T>& b) {
		return a._node != b._node;
	}

	// Iterator vs. ConstReverseIterator
	template<typename T>
	bool operator==(const Iterator<T>& a, const ConstReverseIterator<T>& b) {
		return a._node == b._node;
	}

	template<typename T>
	bool operator!=(const Iterator<T>& a, const ConstReverseIterator<T>& b) {
		return a._node != b._node;
	}
}


} // End of namespace Common

#endif
