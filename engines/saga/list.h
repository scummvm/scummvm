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
	typedef typename Common::List<T>::iterator iterator;
	typedef typename Common::List<T>::const_iterator const_iterator;
	typedef int (*CompareFunction) (const T& a, const T& b);

	iterator insert(const T& element, CompareFunction compareFunction) {
		int res;

		for (typename Common::List<T>::iterator i = Common::List<T>::begin(); i != Common::List<T>::end(); ++i) {
			res = compareFunction(element, *i);
			if	(res < 0) {
				Common::List<T>::insert(i, element);
				return --i;
			}
		}
		Common::List<T>::push_back(element);
		return --Common::List<T>::end();
	}
};

} // End of namespace Saga

#endif
