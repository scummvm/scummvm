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

#ifndef COMMON_LIST_H
#define COMMON_LIST_H

#include "common/scummsys.h"
#include <assert.h>

namespace Common {

template <class T>
class List {
protected:
	int _capacity;
	int _size;
	T *_data;

public:
	typedef T *Iterator;
	typedef const T *ConstIterator;

public:
	List<T>() : _capacity(0), _size(0), _data(0) {}
	List<T>(const List<T>& list) : _capacity(0), _size(0), _data(0) {
		_size = list._size;
		_capacity = _size + 32;
		_data = new T[_capacity];
		for (int i = 0; i < _size; i++)
			_data[i] = list._data[i];
	}

	~List<T>() {
		if (_data)
			delete [] _data;
	}

	void push_back(const T& element) {
		ensureCapacity(_size + 1);
		_data[_size++] = element;
	}

	void push_back(const List<T>& list) {
		ensureCapacity(_size + list._size);
		for (int i = 0; i < list._size; i++)
			_data[_size++] = list._data[i];
	}

	void insert_at(int idx, const T& element) {
		assert(idx >= 0 && idx <= _size);
		ensureCapacity(_size + 1);
		// The following loop is not efficient if you can just memcpy things around.
		// e.g. if you have a list of ints. But for real objects (String...), memcpy
		// usually isn't correct (specifically, for any class which has a non-default
		// copy behaviour. E.g. the String class uses a refCounter which has to be
		// updated whenever a String is copied.
		for (int i = _size; i > idx; i--) {
			_data[i] = _data[i-1];
		}
		_data[idx] = element;
		_size++;
	}


	// TODO: insert, remove, ...

	T& operator [](int idx) {
		assert(idx >= 0 && idx < _size);
		return _data[idx];
	}

	const T& operator [](int idx) const {
		assert(idx >= 0 && idx < _size);
		return _data[idx];
	}

	List<T>& operator  =(const List<T>& list) {
		if (_data)
			delete [] _data;
		_size = list._size;
		_capacity = _size + 32;
		_data = new T[_capacity];
		for (int i = 0; i < _size; i++)
			_data[i] = list._data[i];

		return *this;
	}

	int size() const {
		return _size;
	}

	void clear() {
		if (_data) {
			delete [] _data;
			_data = 0;
		}
		_size = 0;
		_capacity = 0;
	}
	
	bool isEmpty() const { 
		return (_size == 0);
	}


	Iterator		begin() {
		return _data;
	}

	Iterator		end() {
		return _data + _size;
	}

	ConstIterator	begin() const {
		return _data;
	}

	ConstIterator	end() const {
		return _data + _size;
	}

protected:
	void ensureCapacity(int new_len) {
		if (new_len <= _capacity)
			return;

		T *old_data = _data;
		_capacity = new_len + 32;
		_data = new T[_capacity];

		if (old_data) {
			// Copy old data
			for (int i = 0; i < _size; i++)
				_data[i] = old_data[i];
			delete [] old_data;
		}
	}
};

} // End of namespace Common

#endif
