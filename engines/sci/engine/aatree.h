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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_ENGINE_AATREE_H
#define SCI_ENGINE_AATREE_H

#include "common/func.h"

namespace Sci {

// Andersson tree implementation.

template<typename Key>
struct AATreeNode {
	AATreeNode() : _left(this), _right(this), _level(0) { }
	AATreeNode(const Key &key, AATreeNode *left, AATreeNode *right, int level) : _key(key), _left(left), _right(right), _level(level) { }

	Key _key;
	AATreeNode *_left, *_right;
	int _level;
};

template<typename Key, class LessFunc = Common::Less<Key> >
class AATree {
public:
	AATree() {
		_bottom = new AATreeNode<Key>;
		_root = _bottom;
	}

	AATree(const AATree<Key, LessFunc> &tree) {
		_bottom = new AATreeNode<Key>;
		_root = copyTree(tree._root, tree._bottom);
	}

	const AATree<Key, LessFunc> &operator=(const AATree<Key, LessFunc> &tree) {
		if (this != &tree) {
			freeNode(_root);
			_root = copyTree(tree._root, tree._bottom);
		}

		return *this;
	}

	~AATree() {
		freeNode(_root);
		delete _bottom;
	}

	void insert(const Key &key) {
		insertNode(key, _root);
	}

	bool remove(const Key &key) {
		return removeNode(key, _root);
	}

	// Returns a pointer to the smallest Key or NULL if the tree is empty.
	const Key *findSmallest() const {
		AATreeNode<Key> *node = _root;

		if (node == _bottom)
			return NULL;

		while (node->_left != _bottom)
			node = node->_left;

		return &node->_key;
	}

private:
	AATreeNode<Key> *_root;
	AATreeNode<Key> *_bottom;
	LessFunc _less;

private:
	void freeNode(AATreeNode<Key> *node) {
		if (node == _bottom)
			return;

		freeNode(node->_left);
		freeNode(node->_right);

		delete node;
	}

	void skew(AATreeNode<Key> *&node) {
		if (node->_left->_level == node->_level) {
			// Rotate right
			AATreeNode<Key> *temp = node;
			node = node->_left;
			temp->_left = node->_right;
			node->_right = temp;
		}
	}

	void split(AATreeNode<Key> *&node) {
		if (node->_right->_right->_level == node->_level) {
			// Rotate left
			AATreeNode<Key> *temp = node;
			node = node->_right;
			temp->_right = node->_left;
			node->_left = temp;
			node->_level++;
		}
	}

	bool removeNode(const Key &key, AATreeNode<Key> *&node) {
		bool ok = false;
		static AATreeNode<Key> *last, *deleted = _bottom;

		if (node != _bottom) {
			// Search down the tree and set pointers last and deleted
			last = node;

			if (_less(key, node->_key))
				ok = removeNode(key, node->_left);
			else {
				deleted = node;
				ok = removeNode(key, node->_right);
			}

			if ((node == last) && (deleted != _bottom) && !_less(key, deleted->_key) && !_less(deleted->_key, key)) {
				// At the bottom of the tree we remove the element (if it is present)
				deleted->_key = node->_key;
				deleted = _bottom;
				node = node->_right;
				delete last;
				return true;
			}

			if ((node->_left->_level < node->_level - 1) || (node->_right->_level < node->_level - 1)) {
				// On the way back, we rebalance
				node->_level--;
				if (node->_right->_level > node->_level)
					node->_right->_level = node->_level;

				skew(node);
				skew(node->_right);
				skew(node->_right->_right);
				split(node);
				split(node->_right);
			}
		}

		return ok;
	}

	void insertNode(const Key &key, AATreeNode<Key> *&node) {
		if (node == _bottom) {
			node = new AATreeNode<Key>(key, _bottom, _bottom, 1);

			assert(node);
			return;
		}

		if (_less(key, node->_key))
			insertNode(key, node->_left);
		else if (_less(node->_key, key))
			insertNode(key, node->_right);

		skew(node);
		split(node);
	}

	AATreeNode<Key> *copyTree(AATreeNode<Key> *node, const AATreeNode<Key> *bottom) {
		if (node == bottom)
			return _bottom;

		return new AATreeNode<Key>(node->_key, copyTree(node->_left, bottom), copyTree(node->_right, bottom), node->_level);
	}
};

} // End of namespace Sci

#endif // SCI_ENIGNE_AATREE_H
