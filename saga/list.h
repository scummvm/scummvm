/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
		return insert(begin(), element);
	}

	iterator pushBack(const T& element) {
		return insert(end(), element);
	}

	iterator insert(iterator pos, const T& element) {
		Common::List<T>::insert(pos, element);
		return --pos;
	}

	iterator pushFront() {
		return insert(begin());
	}

	iterator pushBack() {
		return insert(end());
	}

	iterator insert(iterator pos) {
		T init;
		return insert(pos, init);
	}

	iterator pushFront(const T& element, CompareFunction* compareFunction) {
		return insert(begin(), element, compareFunction);
	}

	iterator pushBack(const T& element, CompareFunction* compareFunction) {
		return insert(end(), element, compareFunction);
	}

	iterator insert(iterator pos, const T& element, CompareFunction* compareFunction) {
		int res;

		for (iterator i = begin(); i != end(); ++i) {
			res = compareFunction(element, i.operator*());
			if	(res < 0) {
				return insert(i, element);
			}
		}
		return pushBack(element);
	}

	void reorderUp(iterator pos, CompareFunction* compareFunction) {
		iterator i(pos);		
		int res;

		--i;		
		while (i != end()) {
			res = compareFunction(i.operator*(), pos.operator*());
			if (res <= 0) {

				T temp(*pos);
				erase(pos);
				++i;
				Common::List<T>::insert(i, temp);
				return;
			}
			--i;
		}
	}

	void reorderDown(iterator pos, CompareFunction* compareFunction) {
		iterator i(pos);		
		int res;
		
		++i;
		while (i != end()) {
			res = compareFunction(i.operator*(), pos.operator*());
			if (res >= 0) {

				T temp(*pos);
				erase(pos);
				Common::List<T>::insert(i, temp);
				return;
			}
			++i;
		}
	}

	iterator eraseAndPrev(iterator pos) {
		assert(pos != end());
		iterator res(pos);

		--res;
		erase(pos);
		return res;
	}

	void remove(const T* val) {
		for (iterator i = begin(); i != end(); ++i)
			if(val == i.operator->()) {
				erase(i);
				return;
			}
	}

	bool locate(const T* val, iterator& foundedIterator) {

		for (iterator i = begin(); i != end(); ++i)
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
