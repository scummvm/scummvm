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

#ifndef COMMON_ARRAY_H
#define COMMON_ARRAY_H

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/textconsole.h" // For error()
#include "common/memory.h"

namespace Common {

/**
 * This class implements a dynamically sized container, which
 * can be accessed similar to a regular C++ array. Accessing
 * elements is performed in constant time (like with plain arrays).
 * In addition, one can append, insert and remove entries (this
 * is the 'dynamic' part). Doing that in general takes time
 * proportional to the number of elements in the array.
 *
 * The container class closest to this in the C++ standard library is
 * std::vector. However, there are some differences.
 */
template<class T>
class Array {
public:
	typedef T *iterator;
	typedef const T *const_iterator;

	typedef T value_type;

	typedef uint size_type;

protected:
	size_type _capacity;
	size_type _size;
	T *_storage;

public:
	Array() : _capacity(0), _size(0), _storage(nullptr) {}

	/**
	 * Constructs an array with `count` default-inserted instances of T. No
	 * copies are made.
	 */
	explicit Array(size_type count) : _size(count) {
		allocCapacity(count);
		for (size_type i = 0; i < count; ++i)
			new ((void *)&_storage[i]) T();
	}

	/**
	 * Constructs an array with `count` copies of elements with value `value`.
	 */
	Array(size_type count, const T &value) : _size(count) {
		allocCapacity(count);
		uninitialized_fill_n(_storage, count, value);
	}

	Array(const Array<T> &array) : _capacity(array._size), _size(array._size), _storage(nullptr) {
		if (array._storage) {
			allocCapacity(_size);
			uninitialized_copy(array._storage, array._storage + _size, _storage);
		}
	}

	/**
	 * Construct an array by copying data from a regular array.
	 */
	template<class T2>
	Array(const T2 *array, size_type n) {
		_size = n;
		allocCapacity(n);
		uninitialized_copy(array, array + _size, _storage);
	}

	~Array() {
		freeStorage(_storage, _size);
		_storage = nullptr;
		_capacity = _size = 0;
	}

	/** Appends element to the end of the array. */
	void push_back(const T &element) {
		if (_size + 1 <= _capacity)
			new ((void *)&_storage[_size++]) T(element);
		else
			insert_aux(end(), &element, &element + 1);
	}

	void push_back(const Array<T> &array) {
		if (_size + array.size() <= _capacity) {
			uninitialized_copy(array.begin(), array.end(), end());
			_size += array.size();
		} else
			insert_aux(end(), array.begin(), array.end());
	}

	/** Removes the last element of the array. */
	void pop_back() {
		assert(_size > 0);
		_size--;
		// We also need to destroy the last object properly here.
		_storage[_size].~T();
	}

	/** Returns a pointer to the underlying memory serving as element storage. */
	const T *data() const {
		return _storage;
	}

	/** Returns a pointer to the underlying memory serving as element storage. */
	T *data() {
		return _storage;
	}

	/** Returns a reference to the first element of the array. */
	T &front() {
		assert(_size > 0);
		return _storage[0];
	}

	/** Returns a reference to the first element of the array. */
	const T &front() const {
		assert(_size > 0);
		return _storage[0];
	}

	/** Returns a reference to the last element of the array. */
	T &back() {
		assert(_size > 0);
		return _storage[_size-1];
	}

	/** Returns a reference to the last element of the array. */
	const T &back() const {
		assert(_size > 0);
		return _storage[_size-1];
	}


	void insert_at(size_type idx, const T &element) {
		assert(idx <= _size);
		insert_aux(_storage + idx, &element, &element + 1);
	}

	void insert_at(size_type idx, const Array<T> &array) {
		assert(idx <= _size);
		insert_aux(_storage + idx, array.begin(), array.end());
	}

	/**
	 * Inserts element before pos.
	 */
	void insert(iterator pos, const T &element) {
		insert_aux(pos, &element, &element + 1);
	}

	T remove_at(size_type idx) {
		assert(idx < _size);
		T tmp = _storage[idx];
		copy(_storage + idx + 1, _storage + _size, _storage + idx);
		_size--;
		// We also need to destroy the last object properly here.
		_storage[_size].~T();
		return tmp;
	}

	// TODO: insert, remove, ...

	T &operator[](size_type idx) {
		assert(idx < _size);
		return _storage[idx];
	}

	const T &operator[](size_type idx) const {
		assert(idx < _size);
		return _storage[idx];
	}

	Array<T> &operator=(const Array<T> &array) {
		if (this == &array)
			return *this;

		freeStorage(_storage, _size);
		_size = array._size;
		allocCapacity(_size);
		uninitialized_copy(array._storage, array._storage + _size, _storage);

		return *this;
	}

	size_type size() const {
		return _size;
	}

	void clear() {
		freeStorage(_storage, _size);
		_storage = nullptr;
		_size = 0;
		_capacity = 0;
	}

	iterator erase(iterator pos) {
		copy(pos + 1, _storage + _size, pos);
		_size--;
		// We also need to destroy the last object properly here.
		_storage[_size].~T();
		return pos;
	}

	bool empty() const {
		return (_size == 0);
	}

	bool operator==(const Array<T> &other) const {
		if (this == &other)
			return true;
		if (_size != other._size)
			return false;
		for (size_type i = 0; i < _size; ++i) {
			if (_storage[i] != other._storage[i])
				return false;
		}
		return true;
	}

	bool operator!=(const Array<T> &other) const {
		return !(*this == other);
	}

	iterator       begin() {
		return _storage;
	}

	iterator       end() {
		return _storage + _size;
	}

	const_iterator begin() const {
		return _storage;
	}

	const_iterator end() const {
		return _storage + _size;
	}

	void reserve(size_type newCapacity) {
		if (newCapacity <= _capacity)
			return;

		T *oldStorage = _storage;
		allocCapacity(newCapacity);

		if (oldStorage) {
			// Copy old data
			uninitialized_copy(oldStorage, oldStorage + _size, _storage);
			freeStorage(oldStorage, _size);
		}
	}

	void resize(size_type newSize) {
		reserve(newSize);
		for (size_type i = _size; i < newSize; ++i)
			new ((void *)&_storage[i]) T();
		_size = newSize;
	}

	void assign(const_iterator first, const_iterator last) {
		resize(distance(first, last)); // FIXME: ineffective?
		T *dst = _storage;
		while (first != last)
			*dst++ = *first++;
	}

protected:
	static size_type roundUpCapacity(size_type capacity) {
		// Round up capacity to the next power of 2;
		// we use a minimal capacity of 8.
		size_type capa = 8;
		while (capa < capacity)
			capa <<= 1;
		return capa;
	}

	void allocCapacity(size_type capacity) {
		_capacity = capacity;
		if (capacity) {
			_storage = (T *)malloc(sizeof(T) * capacity);
			if (!_storage)
				::error("Common::Array: failure to allocate %u bytes", capacity * (size_type)sizeof(T));
		} else {
			_storage = nullptr;
		}
	}

	void freeStorage(T *storage, const size_type elements) {
		for (size_type i = 0; i < elements; ++i)
			storage[i].~T();
		free(storage);
	}

	/**
	 * Insert a range of elements coming from this or another array.
	 * Unlike std::vector::insert, this method does not accept
	 * arbitrary iterators, mainly because our iterator system is
	 * seriously limited and does not distinguish between input iterators,
	 * output iterators, forward iterators or random access iterators.
	 *
	 * So, we simply restrict to Array iterators. Extending this to arbitrary
	 * random access iterators would be trivial.
	 *
	 * Moreover, this method does not handle all cases of inserting a subrange
	 * of an array into itself; this is why it is private for now.
	 */
	iterator insert_aux(iterator pos, const_iterator first, const_iterator last) {
		assert(_storage <= pos && pos <= _storage + _size);
		assert(first <= last);
		const size_type n = last - first;
		if (n) {
			const size_type idx = pos - _storage;
			if (_size + n > _capacity || (_storage <= first && first <= _storage + _size)) {
				T *const oldStorage = _storage;

				// If there is not enough space, allocate more.
				// Likewise, if this is a self-insert, we allocate new
				// storage to avoid conflicts.
				allocCapacity(roundUpCapacity(_size + n));

				// Copy the data from the old storage till the position where
				// we insert new data
				uninitialized_copy(oldStorage, oldStorage + idx, _storage);
				// Copy the data we insert
				uninitialized_copy(first, last, _storage + idx);
				// Afterwards copy the old data from the position where we
				// insert.
				uninitialized_copy(oldStorage + idx, oldStorage + _size, _storage + idx + n);

				freeStorage(oldStorage, _size);
			} else if (idx + n <= _size) {
				// Make room for the new elements by shifting back
				// existing ones.
				// 1. Move a part of the data to the uninitialized area
				uninitialized_copy(_storage + _size - n, _storage + _size, _storage + _size);
				// 2. Move a part of the data to the initialized area
				copy_backward(pos, _storage + _size - n, _storage + _size);

				// Insert the new elements.
				copy(first, last, pos);
			} else {
				// Copy the old data from the position till the end to the new
				// place.
				uninitialized_copy(pos, _storage + _size, _storage + idx + n);

				// Copy a part of the new data to the position inside the
				// initialized space.
				copy(first, first + (_size - idx), pos);

				// Copy a part of the new data to the position inside the
				// uninitialized space.
				uninitialized_copy(first + (_size - idx), last, _storage + _size);
			}

			// Finally, update the internal state
			_size += n;
		}
		return pos;
	}

};

/**
 * Double linked list with sorted nodes.
 */
template<class T>
class SortedArray : public Array<T> {
public:
	typedef T *iterator;
	typedef uint size_type;

	SortedArray(int (*comparator)(const void *, const void *)) {
		_comparator = comparator;
	}

	/**
	 * Inserts element at the sorted position.
	 */
	void insert(const T &element) {
		if (!this->_size) {
			this->insert_aux(this->_storage, &element, &element + 1);
			return;
		}

		T *where = bsearchMin(element);

		if (where > this->_storage + this->_size)
			Array<T>::push_back(element);
		else
			Array<T>::insert(where, element);
	}

private:
	T &operator[](size_type idx);

	void insert_at(size_type idx, const T &element);

	void insert_at(size_type idx, const Array<T> &array);

	void insert(iterator pos, const T &element);

	void push_back(const T &element);

	void push_back(const Array<T> &array);

	// Based on code Copyright (C) 2008-2009 Ksplice, Inc.
	// Author: Tim Abbott <tabbott@ksplice.com>
	// Licensed under GPLv2+
	T *bsearchMin(void *key) {
		uint start_ = 0, end_ = this->_size;
		int result;

		while (start_ < end_) {
			uint mid = start_ + (end_ - start_) / 2;

			result = this->_comparator(key, this->_storage[mid]);
			if (result < 0)
				end_ = mid;
			else if (result > 0)
				start_ = mid + 1;
			else
				return &this->_storage[mid];
		}

		return &this->_storage[start_];
	}

	int (*_comparator)(const void *, const void *);
};

} // End of namespace Common

#endif
