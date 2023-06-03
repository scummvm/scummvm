/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_RB_TREE_H
#define COMMON_RB_TREE_H

#include "common/func.h"
#include "common/util.h"
#include "common/scummsys.h"

namespace Common {

template<typename Key, typename Val>
struct PairFirst {
	const Key &operator()(const Pair<Key, Val> &p) {
		return p.first;
	}
};

template<typename T>
struct Identity {
	const T &operator()(const T &t) {
		return t;
	}
};

/**
 * @defgroup common_rb_tree Red-black tree
 * @ingroup common
 *
 * @brief API for operating on a red black tree.
 *
 * @{
 */

/**
 * Red-black tree implementation with insertion and deletion algorithms based on the ones
 * found in Introduction to Algorithms by Cormen, Leiserson, Rivest and Stein.
 * Used in the implementation of Common::StableMap and Common::MultiMap
 */
template<class ValueType, class Key, class KeyProj, class KeyComp = Common::Less<Key> >
class RBTree {
public:
	enum class Color {
		kRed,
		kBlack,
	};

	struct Node {
		Node *parent;
		Node *left;
		Node *right;
		Color color;
		ValueType value;
	};

	template<typename Ref, typename Ptr>
	class Iterator {
	public:
		friend RBTree;

		Iterator() = default;
		Iterator(const Iterator &) = default;
		Iterator &operator=(const Iterator &) = default;

		Ref operator*() const { return _current->value; }

		Ptr operator->() const { return &_current->value; }

		Iterator &operator++() {
			if (_current->right) {
				_current = leftmost(_current->right);
			} else {
				auto p = _current->parent;
				while (p && p->right == _current) {
					_current = p;
					p = p->parent;
				}
				_current = p;
			}
			return *this;
		}

		Iterator operator++(int) {
			auto temp = *this;
			++(*this);
			return temp;
		}

		bool operator==(const Iterator &rhs) const {
			return _current == rhs._current;
		}

		bool operator!=(const Iterator &rhs) const {
			return _current != rhs._current;
		}

	private:
		explicit Iterator(Node *n) : _current(n) {}

		Node *_current = nullptr;
	};

	using BasicIterator = Iterator<ValueType &, ValueType *>;             /*!< RBTree iterator. */
	using ConstIterator = Iterator<const ValueType &, const ValueType *>; /*!< Const-qualified RBTree iterator. */

	RBTree(KeyComp comp = {}) : _comp(Common::move(comp)) {
	}

	/** Construct an RBTree as a copy of the given tree @p other . */
	RBTree(const RBTree &other) : _comp(other._comp) {
		for (const auto &val : other)
			insert(val);
	}

	/** Construct an RBTree by moving the contents of the given tree (using the C++11 move semantics). */
	RBTree(RBTree &&other) : _root(other._root), _leftmost(other._leftmost), _size(other._size), _comp(Common::move(other._comp)) {
	}

	/** Assign the given tree to this tree. */
	RBTree &operator=(const RBTree &rhs) {
		*this = RBTree(rhs);
		return *this;
	}

	/** Moves the contents of the given tree into this tree (using the C++11 move semantics). */
	RBTree &operator=(RBTree &&rhs) {
		clear();
		_root = rhs._root;
		_leftmost = rhs._leftmost;
		_size = rhs._size;
		_comp = Common::move(rhs._comp);
		rhs._root = nullptr;
		rhs._leftmost = nullptr;
		rhs._size = 0;
		return *this;
	}

	/** Clears the contents of the tree. */
	void clear() {
		erase(begin(), end());
		_size = 0;
		_root = nullptr;
		_leftmost = nullptr;
	}

	/** Return an iterator pointing to the first element in the tree. */
	BasicIterator begin() { return BasicIterator{_leftmost}; }

	/** Return a const iterator pointing to the first element of the tree. */
	ConstIterator begin() const { return ConstIterator{_leftmost}; }

	/** Return an iterator pointing to the last element in the tree. */
	BasicIterator end() { return BasicIterator{nullptr}; }

	/** Return a const iterator pointing to the last element of the tree. */
	ConstIterator end() const { return ConstIterator{nullptr}; }

	/**
	 * Returns an iterator to the first item thas is not less than @p key
	 * in the tree (or end() if this cannot be found).
	 */
	BasicIterator lowerBound(const Key &key) {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(KeyProj()(it->value), key)) {
				res = it;
				it = it->left;
			} else {
				it = it->right;
			}
		}
		return BasicIterator{res};
	}

	/**
	 * Returns a const iterator to the first item thas is not less than @p key
	 * in the tree (or end() if this cannot be found).
	 */
	ConstIterator lowerBound(const Key &key) const {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(KeyProj()(it->value), key)) {
				res = it;
				it = it->left;
			} else {
				it = it->right;
			}
		}
		return ConstIterator{res};
	}

	/**
	 * Returns an iterator to the first item bigger than @p key
	 * in the tree (or end() if this cannot be found).
	 */
	BasicIterator upperBound(const Key &key) {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(key, KeyProj()(it->value))) {
				it = it->right;
			} else {
				res = it;
				it = it->left;
			}
		}
		return BasicIterator{res};
	}

	/**
	 * Return a const iterator to the first item bigger than @p key
	 * in the tree (or end() if this cannot be found).
	 */
	ConstIterator upperBound(const Key &key) const {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(key, KeyProj()(it->value))) {
				it = it->right;
			} else {
				res = it;
				it = it->left;
			}
		}
		return ConstIterator{res};
	}

	/** Erases the item in the tree pointed by @p it .*/
	BasicIterator erase(BasicIterator it) {
		Node *const z = it._current;
		Node *y = z;
		assert(y);
		const auto ret = it++;
		Color y_prev_color = y->color;
		Node *x;
		Node *xp = nullptr;
		if (!y->left) {
			x = y->right;
			xp = y->parent; // since x is put in y's place by the next call
			transplant(y, y->right);
		} else if (!y->right) {
			x = y->left;
			xp = y->parent;
			transplant(y, y->left);
		} else {
			y = leftmost(z->right);
			y_prev_color = y->color;
			x = y->right;
			if (y != z->right) {
				xp = y->parent;
				transplant(y, y->right);
				y->right = z->right;
				y->right->parent = y;
			} else {
				xp = y;
			}
			transplant(z, y);
			y->left = z->left;
			y->left->parent = y;
			y->color = z->color;
		}
		if (y_prev_color == Color::kBlack)
			fixDelete(x, xp);
		delete z;
		--_size;
		return ret;
	}

	/**
	 * Erase the elements from @p first to @p last and return an iterator pointing to the next element in the tree.
	 * @note
	 * If [first, last) is not a valid range for the tree, the behaviour is undefined.
	 */
	BasicIterator erase(BasicIterator first, BasicIterator last) {
		while (first != last)
			erase(first++);
		return last;
	}

	/** Inserts a value @p val into the tree returning an iterator for the inserted value. */
	BasicIterator insert(const ValueType &val) {
		return internalInsert(&_root, val);
	}

	/**
	 * Inserts the element @p val starting from @p start instead of the tree's root.
	 * This operations is meant for efficient insertions after a call to lowerBound.
	 * For example:
	 * @code
	 * auto it = tree.lowerBound(value);
	 * if (it == tree.end() || *it != value)
	 *    tree.insert(it, value);
	 * @endcode
	 * inserts 'value' if its not contained in 'tree' (assumes that key and value type are the same)
	 * @note
	 * If @p start is not the lower bound, or it's not end(),
	 * the resulting tree could be unsorted.
	 */
	BasicIterator insert(BasicIterator start, const ValueType &val) {
		if (start == end())
			return insert(val);
		return internalInsert(&start._current, val);
	}

	/** Returns the number of values in the tree. */
	size_t size() const { return _size; }

	/**
	 * Returns true if the tree is empty.
	 * Shorthand for:
	 * @code
	 * tree.size() == 0
	 * @endcode
	 */
	bool isEmpty() const { return _size == 0; }

	~RBTree() { clear(); }

private:
	KeyComp _comp;
	Node *_root = nullptr;
	Node *_leftmost = nullptr;
	size_t _size = 0;

	Node *merge(Node *left, Node *right) {
		if (!right)
			return left;
		else if (!left)
			return right;
		else {
			auto lm = leftmost(right);
			lm->left = left;
			left->parent = lm;
			return right;
		}
	}

	BasicIterator internalInsert(Node **starting_point, const ValueType &val) {
		auto it = starting_point;
		Node *parent = nullptr;
		while (*it) {
			parent = *it;
			if (_comp(KeyProj()((*it)->value), KeyProj()(val))) {
				it = &(*it)->right;
			} else {
				it = &(*it)->left;
			}
		}
		*it = new Node{
			parent,
			nullptr,
			nullptr,
			Color::kRed,
			val,
		};
		if (!_leftmost || (parent == _leftmost && _leftmost->left == *it))
			_leftmost = *it;
		++_size;
		auto ret = BasicIterator{*it};
		fixInsert(*it);
		return ret;
	}

	static Node *leftmost(Node *n) {
		while (n->left)
			n = n->left;
		return n;
	}

	// neither rotate changes _leftmost
	void rotateLeft(Node *t) {
		assert(t);
		assert(t->right);
		Node *r = t->right;
		Node *p = t->parent;
		// set r->left as t->right
		t->right = r->left;
		if (r->left)
			r->left->parent = t;
		// set the parent of r
		r->parent = p;
		if (!p)
			_root = r;
		else if (p->right == t)
			p->right = r;
		else
			p->left = r;
		// set the parent of t
		t->parent = r;
		r->left = t;
	}

	void rotateRight(Node *t) {
		assert(t);
		assert(t->left);
		Node *l = t->left;
		Node *p = t->parent;
		assert(p != l);
		// set l->right as t->left
		t->left = l->right;
		if (l->right)
			l->right->parent = t;
		// set the parent of l
		l->parent = p;
		if (!p)
			_root = l;
		else if (p->right == t)
			p->right = l;
		else
			p->left = l;
		// set the parent of t
		l->right = t;
		t->parent = l;
	}

	void transplant(Node *t, Node *u) {
		if (!t->parent) {
			_root = u;
		} else if (t == t->parent->left) {
			t->parent->left = u;
		} else {
			t->parent->right = u;
		}
		if (u) {
			u->parent = t->parent;
		}
		if (t == _leftmost)
			_leftmost = u ? leftmost(u) : t->parent;
	}

	void fixInsert(Node *t) {
		while (t->parent && t->parent->color == Color::kRed) {
			Node *p = t->parent;
			Node *g = p->parent;
			// cannot be null since p is not _root
			assert(g);
			if (p == g->left) {
				Node *const u = g->right;
				if (u && u->color == Color::kRed) {
					p->color = u->color = Color::kBlack;
					g->color = Color::kRed;
					t = g;
				} else {
					if (t == p->right) {
						rotateLeft(p);
						t = p;
						p = t->parent;
					}
					p->color = Color::kBlack;
					// g is not changed by the previous rotation
					g->color = Color::kRed;
					rotateRight(g);
				}
			} else {
				Node *const u = g->left;
				if (u && u->color == Color::kRed) {
					p->color = u->color = Color::kBlack;
					g->color = Color::kRed;
					t = g;
				} else {
					if (t == p->left) {
						rotateRight(p);
						t = p;
						p = t->parent;
					}
					p->color = Color::kBlack;
					g->color = Color::kRed;
					rotateLeft(g);
				}
			}
		}
		_root->color = Color::kBlack;
	}

	void fixDelete(Node *t, Node *p) {
		while (t != _root && (!t || t->color == Color::kBlack)) {
			if (t == p->left) {
				// since the deleted node was black and t is in its
				// place, it has to have a sibling
				Node *b = p->right;
				assert(b);
				if (b->color == Color::kRed) {
					b->color = Color::kBlack;
					p->color = Color::kRed;
					rotateLeft(p);
					p = b->left;
					// since b was red, it had two black children,
					// the right one now being p->right
					b = p->right;
				}
				if ((!b->left || b->left->color == Color::kBlack) &&
					(!b->right || b->right->color == Color::kBlack)) {
					b->color = Color::kRed;
					t = p;
					p = p->parent;
				} else {
					if (!b->right || b->right->color == Color::kBlack) {
						// b->left exists, since b->right is black, and it is red
						// because of one of the above checks
						b->left->color = Color::kBlack;
						b->color = Color::kRed;
						rotateRight(b);
						b = p->right; // p is not changed by the rotation
					}
					b->color = p->color;
					p->color = Color::kBlack;
					if (b->right)
						b->right->color = Color::kBlack;
					rotateLeft(p);
					break;
				}
			} else { // same as above case but left and right swapped
				Node *b = p->left;
				assert(b);
				if (b->color == Color::kRed) {
					b->color = Color::kBlack;
					p->color = Color::kRed;
					rotateRight(p);
					p = b->right;
					b = p->left;
				}
				if ((!b->left || b->left->color == Color::kBlack) &&
					(!b->right || b->right->color == Color::kBlack)) {
					b->color = Color::kRed;
					t = p;
					p = p->parent;
				} else {
					if (!b->left || b->left->color == Color::kBlack) {
						b->right->color = Color::kBlack;
						b->color = Color::kRed;
						rotateLeft(b);
						b = p->left;
					}
					b->color = p->color;
					p->color = Color::kBlack;
					if (b->left)
						b->left->color = Color::kBlack;
					rotateRight(p);
					break;
				}
			}
		}
		if (t)
			t->color = Color::kBlack;
	}
};

/** @} */

} // End of namespace Common

#endif
