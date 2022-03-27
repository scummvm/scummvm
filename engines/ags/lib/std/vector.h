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

#ifndef AGS_STD_VECTOR_H
#define AGS_STD_VECTOR_H

#include "common/array.h"

namespace AGS3 {
namespace std {

template<class T>
class vector : public Common::Array<T> {
public:
	struct reverse_iterator {
	private:
		vector<T> *_owner;
		int _index;
	public:
		reverse_iterator(vector<T> *owner, int index) : _owner(owner), _index(index) {
		}
		reverse_iterator() : _owner(0), _index(-1) {
		}

		T &operator*() {
			return (*_owner)[_index];
		}

		reverse_iterator &operator++() {
			--_index;
			return *this;
		}

		bool operator==(const reverse_iterator &rhs) {
			return _owner == rhs._owner && _index == rhs._index;
		}
		bool operator!=(const reverse_iterator &rhs) {
			return !operator==(rhs);
		}
	};

	struct const_reverse_iterator {
	private:
		const vector<T> *_owner;
		int _index;
	public:
		const_reverse_iterator(const vector<T> *owner, int index) : _owner(owner), _index(index) {
		}
		const_reverse_iterator() : _owner(0), _index(-1) {
		}

		const T operator*() const {
			return (*_owner)[_index];
		}

		const_reverse_iterator &operator++() {
			--_index;
			return *this;
		}

		bool operator==(const const_reverse_iterator &rhs) {
			return _owner == rhs._owner && _index == rhs._index;
		}
		bool operator!=(const const_reverse_iterator &rhs) {
			return !operator==(rhs);
		}
	};

	using iterator = typename Common::Array<T>::iterator;
	using const_iterator = typename Common::Array<T>::const_iterator;
public:
	typedef T reference;
	typedef const T const_reference;

	vector() : Common::Array<T>() {
	}
	vector(size_t newSize) : Common::Array<T>() {
		Common::Array<T>::resize(newSize);
	}
	vector(size_t newSize, const T elem) {
		resize(newSize, elem);
	}

	iterator erase(iterator pos) {
		return Common::Array<T>::erase(pos);
	}

	iterator erase(iterator first,
				   iterator last) {
		Common::copy(last, this->_storage + this->_size, first);

		int count = (last - first);
		this->_size -= count;

		// We also need to destroy the objects beyond the new size
		for (uint idx = this->_size; idx < (this->_size + count); ++idx)
			this->_storage[idx].~T();

		return first;
	}

	void swap(vector &arr) {
		SWAP(this->_capacity, arr._capacity);
		SWAP(this->_size, arr._size);
		SWAP(this->_storage, arr._storage);
	}

	/**
	 * Rotates the array so that the item pointed to by the iterator becomes
	 * the first item, and the predeceding item becomes the last one
	 */
	void rotate(iterator it) {
		if (it != Common::Array<T>::end()) {
			size_t count = it - Common::Array<T>::begin();
			for (size_t ctr = 0; ctr < count; ++ctr) {
				Common::Array<T>::push_back(Common::Array<T>::front());
				Common::Array<T>::remove_at(0);
			}
		}
	}

	const_iterator cbegin() {
		return this->begin();
	}
	const_iterator cend() {
		return this->end();
	}
	reverse_iterator rbegin() {
		return reverse_iterator(this, (int)Common::Array<T>::size() - 1);
	}
	reverse_iterator rend() {
		return reverse_iterator(this, -1);
	}
	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(this, (int)Common::Array<T>::size() - 1);
	}
	const_reverse_iterator rend() const {
		return const_reverse_iterator(this, -1);
	}

	void pop_front() {
		Common::Array<T>::remove_at(0);
	}

	void resize(size_t newSize) {
		Common::Array<T>::resize(newSize);
	}
	void resize(size_t newSize, const T elem) {
		size_t oldSize = Common::Array<T>::size();
		resize(newSize);
		for (size_t idx = oldSize; idx < newSize; ++idx)
			this->operator[](idx) = elem;
	}

	T at(size_t index) const {
		return (*this)[index];
	}

	/**
	 * Adds an item to the array
	 */
	void insert(const T &element) {
		Common::Array<T>::push_back(element);
	}

	/**
	 * Adds an item to the array at a specified index
	 */
	void insert(iterator pos, const T &element) {
		Common::Array<T>::insert(pos, element);
	}

	/**
	 * Adds a range of items at the specified position in the array
	 */
	void insert(iterator position, const_iterator first, const_iterator last) {
		int destIndex = position - this->begin();
		for (; first != last; ++first) {
			this->insert_at(destIndex++, *first);
		}
	}

	/**
	 * Remove an element
	 */
	void remove(T element) {
		for (uint i = 0; i < this->size(); ++i) {
			if (this->operator[](i) == element) {
				this->remove_at(i);
				return;
			}
		}
	}
};

} // namespace std
} // namespace AGS3

#endif
