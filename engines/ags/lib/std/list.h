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

#ifndef AGS_STD_LIST_H
#define AGS_STD_LIST_H

#include "common/list.h"

namespace AGS3 {
namespace std {

template<class T>
class list : public Common::List<T> {
public:
struct reverse_iterator {
private:
	typename Common::List<T>::iterator _it;
public:
	reverse_iterator(typename Common::List<T>::iterator it) : _it(it) {
	}
	reverse_iterator() {
	}

	T operator*() const {
		return *_it;
	}

	reverse_iterator &operator++() {
		--_it;
		return *this;
	}

	bool operator==(const reverse_iterator &rhs) {
		return _it == rhs._it;
	}
	bool operator!=(const reverse_iterator &rhs) {
		return _it != rhs._it;
	}
};
public:
	typename Common::List<T>::iterator insert(typename Common::List<T>::iterator pos,
			const T &element) {
		Common::List<T>::insert(pos, element);
		return --pos;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(Common::List<T>::reverse_begin());
	}
	reverse_iterator rend() {
		return reverse_iterator(Common::List<T>::end());
	}

	void splice(typename Common::List<T>::iterator pos, list<T>& /*other*/, typename Common::List<T>::iterator it ) {
		// We insert it before pos in this list
		typename Common::List<T>::NodeBase *n = static_cast<typename Common::List<T>::NodeBase *>(it._node);
		typename Common::List<T>::NodeBase *nPos = static_cast<typename Common::List<T>::NodeBase*>(pos._node);
		if (n == nPos || n->_next == nPos)
			return;
		// Remove from current position
		n->_prev->_next = n->_next;
		n->_next->_prev = n->_prev;
		// Insert in new position
		n->_next = nPos;
		n->_prev = nPos->_prev;
		n->_prev->_next = n;
		n->_next->_prev = n;
	}
};

} // namespace std
} // namespace AGS3

#endif
