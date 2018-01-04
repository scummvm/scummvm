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
 */

#ifndef SCI_GRAPHICS_LISTS32_H
#define SCI_GRAPHICS_LISTS32_H

#include "common/array.h"

namespace Sci {

/**
 * StablePointerArray holds pointers in a fixed-size array that maintains
 * position of erased items until `pack` is called. It is used by DrawList,
 * RectList, and ScreenItemList. StablePointerArray takes ownership of all
 * pointers that are passed to it and deletes them when calling `erase` or when
 * destroying the StablePointerArray.
 */
template<class T, uint N>
class StablePointerArray {
	uint _size;
	T *_items[N];

public:
	typedef T **iterator;
	typedef T *const *const_iterator;
	typedef T *value_type;
	typedef uint size_type;

	StablePointerArray() : _size(0), _items() {}
	StablePointerArray(const StablePointerArray &other) : _size(other._size) {
		for (size_type i = 0; i < _size; ++i) {
			if (other._items[i] == nullptr) {
				_items[i] = nullptr;
			} else {
				_items[i] = new T(*other._items[i]);
			}
		}
	}
	~StablePointerArray() {
		for (size_type i = 0; i < _size; ++i) {
			delete _items[i];
		}
	}

	void operator=(const StablePointerArray &other) {
		clear();
		_size = other._size;
		for (size_type i = 0; i < _size; ++i) {
			if (other._items[i] == nullptr) {
				_items[i] = nullptr;
			} else {
				_items[i] = new T(*other._items[i]);
			}
		}
	}

	T *const &operator[](size_type index) const {
		assert(index < _size);
		return _items[index];
	}

	T *&operator[](size_type index) {
		assert(index < _size);
		return _items[index];
	}

	/**
	 * Adds a new pointer to the array.
	 */
	void add(T *item) {
		assert(_size < N);
		_items[_size++] = item;
	}

	iterator begin() {
		return _items;
	}

	const_iterator begin() const {
		return _items;
	}

	void clear() {
		for (size_type i = 0; i < _size; ++i) {
			delete _items[i];
			_items[i] = nullptr;
		}

		_size = 0;
	}

	iterator end() {
		return _items + _size;
	}

	const_iterator end() const {
		return _items + _size;
	}

	/**
	 * Erases the object pointed to by the given iterator.
	 */
	void erase(T *item) {
		for (iterator it = begin(); it != end(); ++it) {
			if (*it == item) {
				delete *it;
				*it = nullptr;
				break;
			}
		}
	}

	/**
	 * Erases the object pointed to by the given iterator.
	 */
	void erase(iterator &it) {
		assert(it >= _items && it < _items + _size);
		delete *it;
		*it = nullptr;
	}

	/**
	 * Erases the object pointed to at the given index.
	 */
	void erase_at(size_type index) {
		assert(index < _size);

		delete _items[index];
		_items[index] = nullptr;
	}

	/**
	 * Removes freed pointers from the pointer list.
	 */
	size_type pack() {
		iterator freePtr = begin();
		size_type newSize = 0;

		for (iterator it = begin(), last = end(); it != last; ++it) {
			if (*it != nullptr) {
				*freePtr = *it;
				++freePtr;
				++newSize;
			}
		}

		_size = newSize;
		return newSize;
	}

	/**
	 * The maximum number of elements the container is able to hold.
	 */
	size_type max_size() const {
		return N;
	}

	/**
	 * The number of populated slots in the array. The size
	 * of the array will only go down once `pack` is called.
	 */
	size_type size() const {
		return _size;
	}
};

template<typename T>
class FindByObject {
	const reg_t &_object;
public:
	FindByObject(const reg_t &object) : _object(object) {}
	bool operator()(const T entry) const {
		return entry && entry->_object == _object;
	}
};

} // End of namespace Sci
#endif
