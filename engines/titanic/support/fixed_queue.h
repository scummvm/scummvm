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

#ifndef TITANIC_FIXED_QUEUE_H
#define TITANIC_FIXED_QUEUE_H

#include "common/scummsys.h"
#include "common/array.h"

namespace Titanic {

/**
 * Extremely simple fixed size queue class.
 */
template<class T, uint MAX_SIZE = 10>
class FixedQueue {
	typedef uint size_type;
protected:
	Common::Array<T> _data;
	size_type _topIndex;
public:
	FixedQueue() : _topIndex(0) {
		_data.reserve(MAX_SIZE);
	}

	/**
	 * Returns the size of the queue in use
	 */
	size_type size() const { return _data.size() - _topIndex; }

	/**
	 * Returns the amount of free remaining space in the queue
	 */
	size_type freeSize() const { return MAX_SIZE - size(); }

	/**
	 * Returns true if the queue is empty
	 */
	bool empty() const {
		return size() == 0;
	}

	/**
	 * Returns true if the queue is full
	 */
	bool full() const {
		return freeSize() == 0;
	}

	/**
	 * Clears the queue
	 */
	void clear() {
		_data.clear();
		_topIndex = 0;
	}

	/**
	 * If the tail of the queue in use has reached the end of the internal
	 * array, pushes all pending data back to the start of the array
	 */
	void compact() {
		if (_data.size() == MAX_SIZE && _topIndex > 0) {
			if (_topIndex < MAX_SIZE)
				Common::copy(&_data[_topIndex], &_data[0] + MAX_SIZE, &_data[0]);
			_data.resize(size());
			_topIndex = 0;
		}
	}

	/**
	 * Adds a value to the end of the queue
	 */
	void push(const T &v) {
		assert(size() < MAX_SIZE);
		compact();
		_data.push_back(v);
	}

	/**
	 * Returns the top value on the queue
	 */
	const T &top() const {
		assert(size() > 0);
		return _data[_topIndex];
	}

	/**
	 * Returns the top value on the queue
	 */
	T &top() {
		assert(size() > 0);
		return _data[_topIndex];
	}

	/**
	 * Pops the top value off the queue
	 */
	T pop() {
		T tmp = top();
		++_topIndex;
		return tmp;
	}

	/**
	 * Returns values from within the queue without popping them
	 */
	T &operator[](size_type i) {
		assert(i < size());
		return _data[_topIndex + i];
	}

	/**
	 * Returns values from within the queue without popping them
	 */
	const T &operator[](size_type i) const {
		assert(i < size());
		return _data[_topIndex + i];
	}
};

} // End of namespace Titanic

#endif
