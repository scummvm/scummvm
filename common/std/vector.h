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

/********************************************
   DISCLAIMER:

   This is a wrapper code to mimic the relevant std:: class
   Please use it ONLY when porting an existing code e.g. from the original sources

   For all new development please use classes from Common::
 *********************************************/

#ifndef COMMON_STD_VECTOR_H
#define COMMON_STD_VECTOR_H

#include "common/array.h"

namespace Std {

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

		bool operator==(const const_reverse_iterator &rhs) const {
			return _owner == rhs._owner && _index == rhs._index;
		}
		bool operator!=(const const_reverse_iterator &rhs) const {
			return !operator==(rhs);
		}
		bool operator<(const const_reverse_iterator &rhs) const {
			return _index > rhs._index;
		}
	};
public:
	using iterator = typename Common::Array<T>::iterator;
	using const_iterator = typename Common::Array<T>::const_iterator;

	constexpr vector() : Common::Array<T>() {}
	explicit vector(size_t newSize) : Common::Array<T>(newSize) {}
	vector(size_t newSize, const T &elem) : Common::Array<T>(newSize, elem) {}
	vector(std::initializer_list<T> list) : Common::Array<T>(list) {}


	template<class... Args>
	void emplace_back(Args... args) {
		T tmp(args...);
		this->push_back(tmp);
	}

	using Common::Array<T>::insert;

	void insert(const T &element) {
		this->push_back(element);
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

	T &at(size_t index) {
		return (*this)[index];
	}
	const T &at(size_t index) const {
		return (*this)[index];
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

	/**
	 * Rotates the array so that the item pointed to by the iterator becomes
	 * the first item, and the predeceding item becomes the last one
	 */
	void rotate(iterator it) {
		if (it != this->end()) {
			size_t count = it - this->begin();
			for (size_t ctr = 0; ctr < count; ++ctr) {
				this->push_back(this->front());
				this->remove_at(0);
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
		return reverse_iterator(this, (int)this->size() - 1);
	}
	reverse_iterator rend() {
		return reverse_iterator(this, -1);
	}
	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(this, (int)this->size() - 1);
	}
	const_reverse_iterator rend() const {
		return const_reverse_iterator(this, -1);
	}
	const_reverse_iterator crbegin() const {
		return const_reverse_iterator(this, (int)this->size() - 1);
	}
	const_reverse_iterator crend() const {
		return const_reverse_iterator(this, -1);
	}
};

} // namespace Std

#endif
