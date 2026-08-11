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

#ifndef SCI_GRAPHICS_LISTS32_H
#define SCI_GRAPHICS_LISTS32_H

#include "common/array.h"

namespace Sci {

/**
 * StablePointerArray holds pointers in a fixed-size array that maintains
 * position of erased items until `pack` is called. It is used by RectList
 * and ScreenItemList. StablePointerArray takes ownership of all pointers
 * that are passed to it and deletes them when calling `erase` or when
 * destroying the StablePointerArray.
 *
 * StablePointerArray used to be used for DrawList, until it was discovered
 * that an LSL7 room with many screen items can overflow the fixed array.
 * StablePointerDynamicArray was created below to handle DrawList, while
 * StablePointerArray keeps the performance advantages of fixed arrays on
 * the stack when rendering frames.
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
	StablePointerArray(StablePointerArray &&other) : _size(other._size) {
		other._size = 0;
		for (size_type i = 0; i < _size; ++i) {
			_items[i] = other._items[i];
			other._items[i] = nullptr;
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

	void operator=(StablePointerArray &&other) {
		clear();
		_size = other._size;
		other._size = 0;
		for (size_type i = 0; i < _size; ++i) {
			_items[i] = other._items[i];
			other._items[i] = nullptr;
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

/**
 * StablePointerDynamicArray is like StablePointerArray above, except that
 * it uses a Common::Array for storage instead of a fixed array.
 * It is only used by DrawList, and was created upon discovering that LSL7
 * room 301 can overflow DrawList when displaying a large menu. Bug #14632
 */
template<class T, uint N>
class StablePointerDynamicArray {
	Common::Array<T *> _items;

public:
	typedef T **iterator;
	typedef T *const *const_iterator;
	typedef T *value_type;
	typedef uint size_type;

	StablePointerDynamicArray() {
		_items.reserve(N);
	}
	StablePointerDynamicArray(const StablePointerDynamicArray &other) {
		_items.reserve(MAX(N, other.size()));
		for (size_type i = 0; i < other.size(); ++i) {
			if (other._items[i] == nullptr) {
				_items.push_back(nullptr);
			} else {
				_items.push_back(new T(*other._items[i]));
			}
		}
	}
	StablePointerDynamicArray(StablePointerDynamicArray &&other) {
		_items = Common::move(other._items);
	}
	~StablePointerDynamicArray() {
		for (size_type i = 0; i < _items.size(); ++i) {
			delete _items[i];
		}
	}

	void operator=(StablePointerDynamicArray &other) {
		clear();
		for (size_type i = 0; i < other.size(); ++i) {
			if (other._items[i] == nullptr) {
				_items.push_back(nullptr);
			} else {
				_items.push_back(new T(*other._items[i]));
			}
		}
	}
	void operator=(StablePointerDynamicArray &&other) {
		clear();
		_items = Common::move(other._items);
	}

	T *const &operator[](size_type index) const {
		return _items[index];
	}

	T *&operator[](size_type index) {
		return _items[index];
	}

	/**
	 * Adds a new pointer to the array.
	 */
	void add(T *item) {
		_items.push_back(item);
	}

	iterator begin() {
		return _items.begin();
	}

	const_iterator begin() const {
		return _items.begin();
	}

	void clear() {
		for (size_type i = 0; i < _items.size(); ++i) {
			delete _items[i];
		}
		_items.resize(0);
	}

	iterator end() {
		return _items.end();
	}

	const_iterator end() const {
		return _items.end();
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
		assert(it >= begin() && it < end());
		delete *it;
		*it = nullptr;
	}

	/**
	 * Erases the object pointed to at the given index.
	 */
	void erase_at(size_type index) {
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
		_items.resize(newSize);
		return newSize;
	}

	/**
	 * The number of populated slots in the array. The size
	 * of the array will only go down once `pack` is called.
	 */
	size_type size() const {
		return _items.size();
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

#endif // SCI_GRAPHICS_LISTS32_H
