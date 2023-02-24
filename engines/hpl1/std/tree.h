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

#ifndef HPL1_TREE_H
#define HPL1_TREE_H

#include "common/func.h"
#include "common/util.h"
#include "hpl1/std/pair.h"

namespace Hpl1 {

namespace Std {

template<class Key, class Val, class KeyComp = Common::Less<Key> >
class Tree {
public:
	using ValueType = pair<Key, Val>;

	struct Node {
		Node *parent;
		Node *left;
		Node *right;
		ValueType value;
	};

	template<typename Ref, typename Ptr>
	class Iterator {
	public:
		friend Tree;

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
		Iterator(Node *n) : _current(n) {}

		Node *_current = nullptr;
	};

	using BasicIterator = Iterator<ValueType &, ValueType *>;
	using ConstIterator = Iterator<const ValueType &, const ValueType *>;

	Tree(KeyComp comp = {}) : _comp(Common::move(comp)) {
	}

	Tree(const Tree &other) : _comp(other._comp) {
		for (const auto &val : other)
			insert(val);
	}

	Tree(Tree &&other) : _root(other._root), _leftmost(other._leftmost), _size(other._size), _comp(Common::move(other._comp)) {
	}

	Tree &operator=(const Tree &rhs) {
		*this = Tree(rhs);
		return *this;
	}

	Tree &operator=(Tree &&rhs) {
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

	void clear() {
		erase(begin(), end());
		_size = 0;
		_root = nullptr;
		_leftmost = nullptr;
	}

	BasicIterator begin() { return BasicIterator{_leftmost}; }

	ConstIterator begin() const { return ConstIterator{_leftmost}; }

	BasicIterator end() { return BasicIterator{nullptr}; }

	ConstIterator end() const { return ConstIterator{nullptr}; }

	BasicIterator lowerBound(const Key &key) {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(it->value.first, key)) {
				res = it;
				it = it->left;
			} else {
				it = it->right;
			}
		}
		return BasicIterator{res};
	}

	ConstIterator lowerBound(const Key &key) const {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(it->value.first, key)) {
				res = it;
				it = it->left;
			} else {
				it = it->right;
			}
		}
		return ConstIterator{res};
	}

	BasicIterator upperBound(const Key &key) {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(key, it->value.first)) {
				it = it->right;
			} else {
				res = it;
				it = it->left;
			}
		}
		return BasicIterator{res};
	}

	ConstIterator upperBound(const Key &key) const {
		Node *it = _root;
		Node *res = nullptr;
		while (it) {
			if (!_comp(key, it->value.first)) {
				it = it->right;
			} else {
				res = it;
				it = it->left;
			}
		}
		return ConstIterator{res};
	}

	BasicIterator erase(BasicIterator it) {
		auto const u = it._current;
		if (!u)
			return {nullptr};
		auto v = merge(u->left, u->right);
		if (u->parent) {
			auto const p = u->parent;
			if (p->left == u)
				p->left = v;
			else
				p->right = v;
		} else {
			_root = v;
		}
		if (v)
			v->parent = u->parent;
		if (u == _leftmost)
			_leftmost = v ? v : u->parent;
		--_size;
		auto const ret = ++it;
		delete u;
		return ret;
	}

	BasicIterator erase(BasicIterator first, BasicIterator last) {
		while (first != last)
			erase(first++);
		return last;
	}

	BasicIterator insert(const ValueType &val) {
		auto it = &_root;
		Node *parent = nullptr;
		bool wentRight = false;
		while (*it) {
			parent = *it;
			if (_comp((*it)->value.first, val.first)) {
				it = &(*it)->right;
				wentRight = true;
			} else {
				it = &(*it)->left;
			}
		}
		*it = new Node{parent, nullptr, nullptr, val};
		if (!wentRight)
			_leftmost = *it;
		++_size;
		return BasicIterator{*it};
	}

	std::size_t size() const { return _size; }

	bool isEmpty() const { return _size == 0; }

	~Tree() { clear(); }

private:
	KeyComp _comp;
	Node *_root = nullptr;
	Node *_leftmost = nullptr;
	std::size_t _size = 0;

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

	static Node *leftmost(Node *n) {
		while (n->left)
			n = n->left;
		return n;
	}
};

} // namespace Std
} // namespace Hpl1

#endif