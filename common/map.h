/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef COMMON_MAP_H
#define COMMON_MAP_H

#include "common/scummsys.h"

namespace Common {

/**
 * Default comparison functor: compares to objects using their </==/> operators.
 * Comparison functors ('comparators') are used by the Map template to
 * compare keys. A non-standard comparator might e.g. be implemented to
 * compare strings, ignoring the case.
 */
template <class T>
struct DefaultComparator {
  int operator()(const T& x, const T& y) const { return (x < y) ? -1 : (y < x) ? +1 : 0; }
};


/**
 * Template based map (aka dictionary) class which uniquely maps elements of
 * class Key to elements of class Value.
 *
 * @todo This implementation is fairly limited. In particular, the tree is not
 *       balanced. Ultimately this template should be reimplemented, e.g. using
 *       a red-black tree. Or if one day using Std C++ lib becomes acceptable,
 *       we can use that.
 * @todo Having unit tests for this class would be very desirable. There are a 
 *       big number of things which can go wrong in this code.
 */
template <class Key, class Value, class Comparator = DefaultComparator<Key> >
class Map {
protected:
	struct Node {
		Node *_parent;
		Node *_left, *_right;
		Key _key;
		Value _value;
		Node() : _parent(0), _left(0), _right(0) {}
		Node(const Key &key, Node *parent) : _parent(parent), _left(0), _right(0), _key(key) {}
	};
	
	Node *_root;
	Node *_header;

private:
	Map<Key, Value, Comparator>(const Map<Key, Value, Comparator> &map);
	Map<Key, Value, Comparator> &operator =(const Map<Key, Value, Comparator> &map);

public:
	class ConstIterator {
		friend class Map<Key, Value, Comparator>;
	protected:
		Node *_node;
		ConstIterator(Node *node) : _node(node) {}
		
	public:
		ConstIterator() : _node(0) {}

		Node &operator *() { assert(_node != 0); return *_node; }
		const Node &operator *() const { assert(_node != 0); return *_node; }
        const Node *operator->() const { assert(_node != 0); return _node; }
        bool operator !=(const ConstIterator &iter) const { return _node != iter._node; }
		void operator ++() {
			if (!_node)
				return;
			if (_node->_right) {
				_node = _node->_right;
				while (_node->_left)
					_node = _node->_left;
			} else {
				Node *parent = _node->_parent;
				while (_node == parent->_right) {
					_node = parent;
					parent = _node->_parent;
				}

				if (_node->_right != parent)
					_node = parent;
			}

			if (_node->_parent == 0)
				_node = 0;
		}
	};

public:
	Map<Key, Value, Comparator>() : _root(0) {
		_header = new Node();
		_header->_right = _header->_left = _header;
	}
	
	virtual ~Map<Key, Value, Comparator>() {
		clearNodes(_root);
		delete _header;
		_root = _header = 0;
	}

	/*
	 * Return the object for the given key. If no match is found, a new entry
	 * with the given key and the default data value is inserted.
	 */
	Value &operator [](const Key &key) {
		Node *node;
		if (!_root)
			node = _root = new Node(key, _header);
		else
			node = findOrCreateNode(_root, key);
		return node->_value;
	}

	const Value &operator [](const Key &key) const {
		Node *node = findNode(_root, key);
		assert(node != 0);
		return node->_value;
	}

	bool contains(const Key &key) const {
		return (findNode(_root, key) != 0);
	}

	void clear() {
		clearNodes(_root);
		_root = 0;
	}

	bool isEmpty() const {
		return (_root == 0);
	}

	Value &addKey(const Key &key) {
		Node *node;
		if (!_root)
			node = _root = new Node(key, _header);
		else
			node = findOrCreateNode(_root, key);
		return node->_value;
	}

	void remove(const Key &key) {
		// TODO - implement efficiently. Indeed, maybe switch to using red-black trees?
		// For now, just a lame, bad remove algorithm. Rule: don't remove elements
		// from one of our maps if you need to be efficient :-)
		Node *node = findNode(_root, key);
		if (!node)
			return;

		// Now we have to remove 'node'. There are two simple cases and one hard.
		Node *parent = node->_parent;
		Node *rep;
		if (!node->_left) {
			rep = node->_right;
		} else if (!node->_right) {
			rep = node->_left;
		} else {
			// We have to do it the hard way since both children are present.
			Node *n2;

			n2 = rep = node->_right;

			// Now insert the left child leftmost into our right child
			while (n2->_left)
				n2 = n2->_left;
			n2->_left = node->_left;
			n2->_left->_parent = n2;
		}

		// Replace this node with node 'rep'
		if (rep)
			rep->_parent = parent;
		if (parent == _header) // Root node?
			_root = rep;
		else if (parent->_left == node)
			parent->_left = rep;
		else
			parent->_right = rep;
		
		// Finally free the allocated memory
		delete node;
	}

	void merge(const Map<Key, Value, Comparator> &map) {
		merge(map._root);
	}

	ConstIterator	begin() const {
		Node *node = _root;
		if (node) {
			while (node->_left)
				node = node->_left;
		}
		return ConstIterator(node);
	}

	ConstIterator	end() const {
		return ConstIterator();
	}

protected:
	/** Merge the content of the given tree into this tree. */
	void merge(const Node *node) {
		if (!node)
			return;
		(*this)[node->_key] = node->_value;
		merge(node->_left);
		merge(node->_right);
	}

	/** Find and return the node matching the given key, if any. */
	Node *findNode(Node *node, const Key &key) const {
		Comparator cmp;
		while (node) {
			int val = cmp(key,node->_key);
			if (val == 0)
				return node;
			else if (val < 0)
				node = node->_left;
			else
				node = node->_right;
		}
		return 0;
	}

	Node *findOrCreateNode(Node *node, const Key &key) {
		Comparator cmp;
		Node *prevNode = 0;
		bool left = true;
		while (node) {
			int val = cmp(key, node->_key);
			prevNode = node;
			if (val == 0) {
				return node;
			} else if (val < 0) {
				left = true;
				node = node->_left;
			} else {
				left = false;
				node = node->_right;
			}
		}
		node = new Node(key, prevNode);
		if (left) {
			prevNode->_left = node;
		} else {
			prevNode->_right = node;
		}
		return node;
	}

	void clearNodes(Node *node) {
		if (!node)
			return;

		clearNodes(node->_left);
		clearNodes(node->_right);
		delete node;
	}
};

class String;

typedef Map<String, String> StringMap;

}	// End of namespace Common

#endif
