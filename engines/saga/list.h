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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// List functions

#ifndef SAGA_LIST_H
#define SAGA_LIST_H

#include "common/list.h"

namespace Saga {

template <class T>
class SortedList : public Common::List<T> {
public:
	typedef int (*CompareFunction) (const T& a, const T& b);

	typedef typename Common::List<T>::iterator        iterator;
	typedef typename Common::List<T>::const_iterator  const_iterator;
	typedef typename Common::List<T> Common_List;

public:

	iterator pushBack(const T& element) {
		return insert(Common_List::end(), element);
	}

	iterator insert(iterator pos, const T& element) {
		Common_List::insert(pos, element);
		return --pos;
	}

	iterator pushFront() {
		T tmp;
		return insert(Common_List::begin(), tmp);
	}

	iterator insert(iterator pos) {
		T init;
		return insert(pos, init);
	}

	iterator pushBack(const T& element, CompareFunction compareFunction) {
		return insert(Common_List::end(), element, compareFunction);
	}

	iterator insert(iterator pos, const T& element, CompareFunction compareFunction) {
		int res;

		for (iterator i = Common_List::begin(); i != Common_List::end(); ++i) {
			res = compareFunction(element, *i);
			if	(res < 0) {
				return insert(i, element);
			}
		}
		return pushBack(element);
	}
};

} // End of namespace Saga

#endif
