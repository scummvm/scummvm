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

namespace ScummVM {

template <class Key, class Value>
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

	Map<Key, Value>(const Map<Key, Value> &map) {
		error("No copy constructor!");
		return 0;
	}
	Map<Key, Value> &operator =(const Map<Key, Value> &map) {
		error("No copy operator!");
		return 0;
	}

public:
	class Iterator {
	protected:
		Node *_node;
		
	public:
		Iterator(Node *node = 0) : _node(node) {}
		Node &operator *() { assert(_node != 0); return *_node; }
		const Node &operator *() const { assert(_node != 0); return *_node; }
        const Node *operator->() const { assert(_node != 0); return _node; }
        bool operator !=(const Iterator &iter) const { return _node != iter._node; }
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
	Map<Key, Value>() : _root(0) {
		_header = new Node();
		_header->_right = _header->_left = _header;
	}
	
	~Map<Key, Value>() {
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
			node = createNode(_root, key);
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

	void merge(const Map<Key, Value> &map) {
		// FIXME - this is a very bad algorithm.
		// Right now we insert the items from 'map' using the default iterator,
		// which gives us the objects ordered, leading to an unbalanced tree.
		// This could be fixed by switching to a red black tree, or by using a 
		// different walk order (infix comes to mind).
		if (map.isEmpty())
			return;
		Iterator x(map.begin()), e(map.end());
		for (; x != e; ++x) {
			(*this)[x->_key] = x->_value;
		}
	}

	Iterator	begin() const {
		Node *node = _root;
		if (node) {
			while (node->_left)
				node = node->_left;
		}
		return Iterator(node);
	}

	Iterator	end() const {
		return Iterator();
	}

protected:
	// Find the node matching the given key, if any
	Node *findNode(Node *node, const Key &key) const {
		while (node && (key != node->_key)) {
			if (key < node->_key) {
				node = node->_left;
			} else {
				node = node->_right;
			}
		}
		return node;
	}

	Node *createNode(Node *node, const Key &key) {
		Node *prevNode = 0;
		bool left = true;
		while (node) {
			prevNode = node;
			if (key == node->_key) {
				return node;
			} else if (key < node->_key) {
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

}	// End of namespace ScummVM

#endif
