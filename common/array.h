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
 */

#ifndef COMMON_ARRAY_H
#define COMMON_ARRAY_H

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/textconsole.h"	// For error()

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
 * std::vector. However, there are some differences. The most important one is
 * that std::vector has a far more sophisticated (and complicated) memory
 * management scheme. There, only elements that 'live' are actually constructed
 * (i.e., have their constructor called), and objects that are removed are
 * immediately destructed (have their destructor called).
 * With Array, this is not the case; instead, it simply uses new[] and
 * delete[] to allocate whole blocks of objects, possibly more than are
 * currently 'alive'. This simplifies memory management, but may have
 * undesirable side effects when one wants to use an Array of complex
 * data types.
 *
 * @todo Improve the storage management of this class.
 * In particular, don't use new[] and delete[], but rather
 * construct/destruct objects manually. This way, we can
 * ensure that storage which is not currently used does not
 * correspond to a live active object.
 * (This is only of interest for array of non-POD objects).
 */
template<class T>
class Array {
protected:
	uint _capacity;
	uint _size;
	T *_storage;

public:
	typedef T *iterator;
	typedef const T *const_iterator;

	typedef T value_type;

public:
	Array() : _capacity(0), _size(0), _storage(0) {}

	Array(const Array<T> &array) : _capacity(array._size), _size(array._size), _storage(0) {
		if (array._storage) {
			allocCapacity(_size);
			copy(array._storage, array._storage + _size, _storage);
		}
	}

	/**
	 * Construct an array by copying data from a regular array.
	 */
	template<class T2>
	Array(const T2 *data, int n) {
		_size = n;
		allocCapacity(n);
		copy(data, data + _size, _storage);
	}

	~Array() {
		delete[] _storage;
		_storage = 0;
		_capacity = _size = 0;
	}

	/** Appends element to the end of the array. */
	void push_back(const T &element) {
		if (_size + 1 <= _capacity)
			_storage[_size++] = element;
		else
			insert_aux(end(), &element, &element + 1);
	}

	void push_back(const Array<T> &array) {
		if (_size + array.size() <= _capacity) {
			copy(array.begin(), array.end(), end());
			_size += array.size();
		} else
			insert_aux(end(), array.begin(), array.end());
	}

	/** Removes the last element of the array. */
	void pop_back() {
		assert(_size > 0);
		_size--;
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


	void insert_at(int idx, const T &element) {
		assert(idx >= 0 && (uint)idx <= _size);
		insert_aux(_storage + idx, &element, &element + 1);
	}

	void insert_at(int idx, const Array<T> &array) {
		assert(idx >= 0 && (uint)idx <= _size);
		insert_aux(_storage + idx, array.begin(), array.end());
	}


	T remove_at(int idx) {
		assert(idx >= 0 && (uint)idx < _size);
		T tmp = _storage[idx];
		copy(_storage + idx + 1, _storage + _size, _storage + idx);
		_size--;
		return tmp;
	}

	// TODO: insert, remove, ...

	T& operator[](int idx) {
		assert(idx >= 0 && (uint)idx < _size);
		return _storage[idx];
	}

	const T& operator[](int idx) const {
		assert(idx >= 0 && (uint)idx < _size);
		return _storage[idx];
	}

	Array<T>& operator=(const Array<T> &array) {
		if (this == &array)
			return *this;

		delete[] _storage;
		_size = array._size;
		allocCapacity(_size);
		copy(array._storage, array._storage + _size, _storage);

		return *this;
	}

	uint size() const {
		return _size;
	}

	void clear() {
		delete[] _storage;
		_storage = 0;
		_size = 0;
		_capacity = 0;
	}

	bool empty() const {
		return (_size == 0);
	}

	bool operator==(const Array<T> &other) const {
		if (this == &other)
			return true;
		if (_size != other._size)
			return false;
		for (uint i = 0; i < _size; ++i) {
			if (_storage[i] != other._storage[i])
				return false;
		}
		return true;
	}
	bool operator!=(const Array<T> &other) const {
		return !(*this == other);
	}


	iterator		begin() {
		return _storage;
	}

	iterator		end() {
		return _storage + _size;
	}

	const_iterator	begin() const {
		return _storage;
	}

	const_iterator	end() const {
		return _storage + _size;
	}

	void reserve(uint newCapacity) {
		if (newCapacity <= _capacity)
			return;

		T *oldStorage = _storage;
		allocCapacity(newCapacity);

		if (oldStorage) {
			// Copy old data
			copy(oldStorage, oldStorage + _size, _storage);
			delete[] oldStorage;
		}
	}

	void resize(uint newSize) {
		reserve(newSize);
		for (uint i = _size; i < newSize; ++i)
			_storage[i] = T();
		_size = newSize;
	}

	void assign(const_iterator first, const_iterator last) {
		resize(distance(first, last)); // FIXME: ineffective?
		T *dst = _storage;
		while (first != last)
			*dst++ = *first++;
	}

protected:
	static uint roundUpCapacity(uint capacity) {
		// Round up capacity to the next power of 2;
		// we use a minimal capacity of 8.
		uint capa = 8;
		while (capa < capacity)
			capa <<= 1;
		return capa;
	}

	void allocCapacity(uint capacity) {
		_capacity = capacity;
		if (capacity) {
			_storage = new T[capacity];
			if (!_storage)
				::error("Common::Array: failure to allocate %u bytes", capacity * (uint)sizeof(T));
		} else {
			_storage = 0;
		}
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
		const uint n = last - first;
		if (n) {
			const uint idx = pos - _storage;
			T *oldStorage = _storage;
			if (_size + n > _capacity || (_storage <= first && first <= _storage + _size) ) {
				// If there is not enough space, allocate more and
				// copy old elements over.
				// Likewise, if this is a self-insert, we allocate new
				// storage to avoid conflicts. This is not the most efficient
				// way to ensure that, but probably the simplest on.
				allocCapacity(roundUpCapacity(_size + n));
				copy(oldStorage, oldStorage + idx, _storage);
				pos = _storage + idx;
			}

			// Make room for the new elements by shifting back
			// existing ones.
			copy_backward(oldStorage + idx, oldStorage + _size, _storage + _size + n);

			// Insert the new elements.
			copy(first, last, pos);

			// Finally, update the internal state
			if (_storage != oldStorage) {
				delete[] oldStorage;
			}
			_size += n;
		}
		return pos;
	}

};

} // End of namespace Common

#endif
