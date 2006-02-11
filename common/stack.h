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

#ifndef COMMON_STACK_H
#define COMMON_STACK_H

#include "common/scummsys.h"
#include <assert.h>
#include "common/array.h"

namespace Common {

/**
 * Extremly simple fixed size stack class.
 */
template <class T, int MAX_SIZE = 10>
class FixedStack {
protected:
	T	_stack[MAX_SIZE];
	int	_size;
public:
	FixedStack<T, MAX_SIZE>() : _size(0) {}

	bool empty() const {
		return _size <= 0;
	}
	void clear() {
		_size = 0;
	}
	void push(const T& x) {
		assert(_size < MAX_SIZE);
		_stack[_size++] = x;
	}
	T top() const {
		if (_size > 0)
			return _stack[_size - 1];
		else
			return 0;
	}
	T pop() {
		T tmp;
		assert(_size > 0);
		tmp = _stack[_size - 1];
		_stack[--_size] = 0;
		return tmp;
	}
	int size() const {
		return _size;
	}
	T operator [](int i) {
		assert(0 <= i && i < MAX_SIZE);
		return _stack[i];
	}
};


/**
 * Variable size stack class, implemented using our Array class.
 */
template <class T>
class Stack {
protected:
	Array<T>	_stack;
public:
	Stack<T>() {}

	bool empty() const {
		return _stack.isEmpty();
	}
	void clear() {
		_stack.clear();
	}
	void push(const T& x) {
		_stack.push_back(x);
	}
	T top() const {
		const int s = size();
		if (s > 0)
			return _stack[s - 1];
		else
			return 0;
	}
	T pop() {
		T tmp = top();
		_stack.remove_at(size() - 1);
		return tmp;
	}
	int size() const {
		return _stack.size();
	}
	T operator [](int i) {
		return _stack[i];
	}
};

} // End of namespace Common

#endif
