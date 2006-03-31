/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#ifndef COMMON_MAP_H
#define COMMON_MAP_H

#include "common/scummsys.h"
#include "common/func.h"

namespace Common {


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
template <class Key, class Value, class Comparator = Less<Key> >
class Map {
protected:
	struct BaseNode {
		Key _key;
		Value _value;
		BaseNode() {}
		BaseNode(const Key &key) : _key(key) {}
	};
	struct Node : BaseNode {
		Node *_parent;
		Node *_left, *_right;
		Node() : _parent(0), _left(0), _right(0) {}
		Node(const Key &key, Node *parent) : BaseNode(key), _parent(parent), _left(0), _right(0) {}
	};

	Node *_root;
	Node *_header;

	Comparator _cmp;

	uint _size;

private:
	Map<Key, Value, Comparator>(const Map<Key, Value, Comparator> &map);
	Map<Key, Value, Comparator> &operator =(const Map<Key, Value, Comparator> &map);

public:
	class const_iterator {
		friend class Map<Key, Value, Comparator>;
	protected:
		Node *_node;
		const_iterator(Node *node) : _node(node) {}

	public:
		const_iterator() : _node(0) {}

		const BaseNode &operator *() const { assert(_node != 0); return *_node; }
		const BaseNode *operator->() const { assert(_node != 0); return _node; }
		bool operator ==(const const_iterator &iter) const { return _node == iter._node; }
		bool operator !=(const const_iterator &iter) const { return !(*this == iter); }
		const_iterator operator ++() {
			if (!_node)
				return *this;
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
			
			return *this;
		}
	};

public:
	Map<Key, Value, Comparator>() : _root(0), _size(0) {
		_header = new Node();
		_header->_right = _header->_left = _header;
	}

#ifndef __SYMBIAN32__
	virtual ~Map<Key, Value, Comparator>()
#else
	~Map()
#endif
	{
		clearNodes(_root);
		delete _header;
		_root = _header = 0;
		_size = 0;
	}

	/*
	 * Return the object for the given key. If no match is found, a new entry
	 * with the given key and the default data value is inserted.
	 */
	Value &operator [](const Key &key) {
		Node *node;
		if (!_root) {
			node = _root = new Node(key, _header);
			_size++;
		} else {
			node = findOrCreateNode(_root, key);
		}
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
		_size = 0;
	}

	bool empty() const {
		return (_root == 0);
	}

	uint size() const { return _size; }

	size_t erase(const Key &key) {
		// TODO - implement efficiently. Indeed, maybe switch to using red-black trees?
		// For now, just a lame, bad remove algorithm. Rule: don't remove elements
		// from one of our maps if you need to be efficient :-)
		Node *node = findNode(_root, key);
		if (!node)
			return 0; // key wasn't present, so no work has to be done

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

		return 1;
	}
	
	void merge(const Map<Key, Value, Comparator> &map) {
		merge(map._root);
	}

	const_iterator	begin() const {
		Node *node = _root;
		if (node) {
			while (node->_left)
				node = node->_left;
		}
		return const_iterator(node);
	}

	const_iterator	end() const {
		return const_iterator();
	}

	const_iterator	find(const Key &key) const {
		Node *node = findNode(_root, key);
		if (node)
			return const_iterator(node);
		return end();
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
		while (node) {
			if (_cmp(key, node->_key))
				node = node->_left;
			else if (_cmp(node->_key, key))
				node = node->_right;
			else
				return node;
		}
		return 0;
	}

	Node *findOrCreateNode(Node *node, const Key &key) {
		Node *prevNode = 0;
		bool left = true;
		while (node) {
			prevNode = node;
			if (_cmp(key, node->_key)) {
				left = true;
				node = node->_left;
			} else if (_cmp(node->_key, key)) {
				left = false;
				node = node->_right;
			} else
				return node;
		}
		node = new Node(key, prevNode);
		_size++;
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

}	// End of namespace Common

#endif
