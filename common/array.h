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
 * $URL$
 * $Id$
 */

#ifndef COMMON_ARRAY_H
#define COMMON_ARRAY_H

#include "common/scummsys.h"
#include "common/algorithm.h"

namespace Common {

template<class T>
class Array {
protected:
	uint _capacity;
	uint _size;
	T *_data;

public:
	typedef T *iterator;
	typedef const T *const_iterator;

	typedef T value_type;

public:
	Array() : _capacity(0), _size(0), _data(0) {}
	Array(const Array<T> &array) : _capacity(0), _size(0), _data(0) {
		_size = array._size;
		_capacity = _size + 32;
		_data = new T[_capacity];
		copy(array._data, array._data + _size, _data);
	}

	~Array() {
		delete[] _data;
	}

	void push_back(const T &element) {
		ensureCapacity(_size + 1);
		_data[_size++] = element;
	}

	void push_back(const Array<T> &array) {
		ensureCapacity(_size + array._size);
		copy(array._data, array._data + array._size, _data + _size);
		_size += array._size;
	}

	void insert_at(int idx, const T &element) {
		assert(idx >= 0 && (uint)idx <= _size);
		ensureCapacity(_size + 1);
		copy_backward(_data + idx, _data + _size, _data + _size + 1);
		_data[idx] = element;
		_size++;
	}

	T remove_at(int idx) {
		assert(idx >= 0 && (uint)idx < _size);
		T tmp = _data[idx];
		copy(_data + idx + 1, _data + _size, _data + idx);
		_size--;
		return tmp;
	}

	// TODO: insert, remove, ...

	T& operator[](int idx) {
		assert(idx >= 0 && (uint)idx < _size);
		return _data[idx];
	}

	const T& operator[](int idx) const {
		assert(idx >= 0 && (uint)idx < _size);
		return _data[idx];
	}

	Array<T>& operator=(const Array<T> &array) {
		if (this == &array)
			return *this;

		delete[] _data;
		_size = array._size;
		_capacity = _size + 32;
		_data = new T[_capacity];
		copy(array._data, array._data + _size, _data);

		return *this;
	}

	uint size() const {
		return _size;
	}

	void clear() {
		delete[] _data;
		_data = 0;
		_size = 0;
		_capacity = 0;
	}

	bool empty() const {
		return (_size == 0);
	}


	iterator		begin() {
		return _data;
	}

	iterator		end() {
		return _data + _size;
	}

	const_iterator	begin() const {
		return _data;
	}

	const_iterator	end() const {
		return _data + _size;
	}

	void reserve(uint newCapacity) {
		if (newCapacity <= _capacity)
			return;

		T *old_data = _data;
		_capacity = newCapacity;
		_data = new T[newCapacity];

		if (old_data) {
			// Copy old data
			copy(old_data, old_data + _size, _data);
			delete[] old_data;
		}
	}

	void resize(uint newSize) {
		if (newSize == _size)
			return;

		T *old_data = _data;
		_capacity = newSize;
		_data = new T[newSize];
		if (old_data) {
			// Copy old data
			int cnt = (_size < newSize ? _size : newSize);
			copy(old_data, old_data + cnt, _data);
			delete[] old_data;
		}
		_size = newSize;
	}

protected:
	void ensureCapacity(uint len) {
		if (len >= _capacity)
			reserve(len + 32);
	}
};

} // End of namespace Common

#endif
