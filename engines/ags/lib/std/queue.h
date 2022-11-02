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

#ifndef AGS_STD_QUEUE_H
#define AGS_STD_QUEUE_H

#include "ags/lib/std/algorithm.h"
#include "ags/lib/std/vector.h"
#include "common/queue.h"

namespace AGS3 {
namespace std {

template<class T>
using queue = Common::Queue<T>;

/**
 * FIXME: The current implementation requires the reverse
 * greater/lesser comparitor than the original does.
 * If this is fixed, also change the router finder's use
 */
template<class T, class Container = vector<T>, class Comparitor = typename Common::Less<T> >
class priority_queue {
private:
	Container _container;
	Comparitor _comparitor;
public:
	priority_queue() {}

	bool empty() const {
		return _container.empty();
	}

	const T &top() const {
		return _container.front();
	}

	void push(const T &item) {
		_container.push_back(item);
		Common::sort(_container.begin(), _container.end(), _comparitor);
	}

	void pop() {
		_container.remove_at(0);
	}
};

template<class T>
class deque {
private:
	vector<T> _intern;
public:
	deque() = default;
	typedef typename vector<T>::iterator iterator;
	typedef const typename vector<T>::const_iterator const_iterator;
	typedef typename vector<T>::reverse_iterator reverse_iterator;
	typedef const typename vector<T>::const_reverse_iterator const_reverse_iterator;

	void clear() {
		_intern.clear();
	}
	void insert(const T &item) {
		_intern.push_back(item);
	}
	void push_back(const T &item) {
		_intern.push_back(item);
	}
	void push_front(const T &item) {
		_intern.push_front(item);
	}
	void pop_back() {
		_intern.pop_back();
	}
	void pop_front() {
		_intern.remove_at(0);
	}
	const T &front() const {
		return _intern.front();
	}
	const T &back() const {
		return _intern.back();
	}

	void resize(size_t newSize) {
		_intern.resize(newSize);
	}

	size_t size() const {
		return _intern.size();
	}

	T at(size_t idx) {
		return _intern[idx];
	}

	const_iterator cbegin() {
		return _intern.cbegin();
	}
	const_iterator cend() {
		return _intern.cend();
	}
	reverse_iterator rbegin() {
		return _intern.rbegin();
	}
	reverse_iterator rend() {
		return _intern.rend();
	}
	const_reverse_iterator rbegin() const {
		return _intern.rbegin();
	}
	const_reverse_iterator rend() const {
		return _intern.rend();
	}
	const_reverse_iterator crbegin() const {
		return _intern.crbegin();
	}
	const_reverse_iterator crend() const {
		return _intern.crend();
	}
};

} // namespace std
} // namespace AGS3

#endif
