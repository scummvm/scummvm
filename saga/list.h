/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SAGA_LIST_H__
#define SAGA_LIST_H__

#include "common/list.h"

namespace Saga {

template <class T>
class SortedList : public Common::List<T> {
public:
	typedef int ( CompareFunction ) (const T& , const T&);

	typedef typename Common::List<T>::iterator        iterator;
	typedef typename Common::List<T>::const_iterator  const_iterator;
	
public:

	iterator pushFront(const T& element) {
		return insert(Common::List<T>::begin(), element);
	}

	iterator pushBack(const T& element) {
		return insert(Common::List<T>::end(), element);
	}

	iterator insert(iterator pos, const T& element) {
		Common::List<T>::insert(pos, element);
		return --pos;
	}

	iterator pushFront() {
		return insert(Common::List<T>::begin());
	}

	iterator pushBack() {
		return insert(Common::List<T>::end());
	}

	iterator insert(iterator pos) {
		T init;
		return insert(pos, init);
	}

	iterator pushFront(const T& element, CompareFunction* compareFunction) {
		return insert(Common::List<T>::begin(), element, compareFunction);
	}

	iterator pushBack(const T& element, CompareFunction* compareFunction) {
		return insert(Common::List<T>::end(), element, compareFunction);
	}

	iterator insert(iterator pos, const T& element, CompareFunction* compareFunction) {
		int res;

		for (iterator i = Common::List<T>::begin(); i != Common::List<T>::end(); ++i) {
			res = compareFunction(element, i.operator*());
			if	(res < 0) {
				return insert(i, element);
			}
		}
		return pushBack(element);
	}

	iterator reorderUp(iterator pos, CompareFunction* compareFunction) {
		iterator i(pos);		
		int res;

		--i;		
		while (i != Common::List<T>::end()) {
			res = compareFunction(i.operator*(), pos.operator*());
			if (res <= 0) {

				T temp(*pos);
				erase(pos);
				++i;				
				return insert(i, temp);
			}
			--i;
		}
		return pos;
	}

	iterator reorderDown(iterator pos, CompareFunction* compareFunction) {
		iterator i(pos);		
		int res;
		
		++i;
		while (i != Common::List<T>::end()) {
			res = compareFunction(i.operator*(), pos.operator*());
			if (res >= 0) {

				T temp(*pos);
				erase(pos);
				return insert(i, temp);
			}
			++i;
		}
		return pos;
	}

	iterator eraseAndPrev(iterator pos) {
		assert(pos != Common::List<T>::end());
		iterator res(pos);

		--res;
		erase(pos);
		return res;
	}

	void remove(const T* val) {
		for (iterator i = Common::List<T>::begin(); i != Common::List<T>::end(); ++i)
			if (val == i.operator->()) {
				erase(i);
				return;
			}
	}

	bool locate(const T* val, iterator& foundedIterator) {

		for (iterator i = Common::List<T>::begin(); i != Common::List<T>::end(); ++i)
			if (val == i.operator->())
			{
				foundedIterator = i;
				return true;
			}

		return false;
	}
};

} // End of namespace Saga

#endif
