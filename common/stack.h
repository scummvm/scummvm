/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2004 The ScummVM project
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

#ifndef COMMON_STACK_H
#define COMMON_STACK_H

#include "common/scummsys.h"
#include <assert.h>

namespace Common {

/**
 * Extremly simple fixed size stack class.
 */
template <class T, int MAX_SIZE = 10>
class Stack {
protected:
	T	_stack[MAX_SIZE];
	int	_size;
public:
	Stack<T, MAX_SIZE>() : _size(0) {}
	
	bool empty() const {
		return _size <= 0;
	}
	void push(T x) {
		assert(_size < MAX_SIZE);
		_stack[_size++] = x;
	}
	T top() const {
		if (_size > 0)
			return _stack[_size - 1];
		else
			return 0;
	}
	void pop() {
		assert(_size > 0);
		_stack[--_size] = 0;
	}
	int size() const {
		return _size;
	}
	T operator [](int i) {
		assert(0 <= i && i < MAX_SIZE);
		return _stack[i];
	}
};

} // End of namespace Common

#endif
