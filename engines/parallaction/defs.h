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

#ifndef PARALLACTION_DEFS_H
#define PARALLACTION_DEFS_H

#include "common/list.h"
#include "common/rect.h"

namespace Parallaction {

#define PATH_LEN	200

template <class T>
class ManagedList : public Common::List<T> {

public:

	typedef typename Common::List<T> 				Common_List;
	typedef typename Common::List<T>::iterator 		iterator;

	typedef int (*CompareFunction) (const T& a, const T& b);

	~ManagedList() {
		clear();
	}

	void clear() {
		erase(Common_List::begin(), Common_List::end());
	}

	iterator erase(iterator pos) {
		if (pos != Common_List::end())
			delete *pos;
		return Common_List::erase(pos);
	}

	iterator erase(iterator first, iterator last) {
		for (iterator it = first; it != last; it++)
			delete *it;
		return Common_List::erase(first, last);
	}

	// keeps list ordered in *ascending* order, as expressed by the compare function
	void insertSorted(const T& element, CompareFunction compare) {
		iterator it = Common_List::begin();
		for ( ; it != Common_List::end(); it++)
			if (compare(element, *it) < 0) break;

		if (it == Common_List::end())
			Common_List::push_back(element);
		else
			Common_List::insert(it, element);
	}

	// FIXME: this routine is a copy of the sort routine that can be found in common/func.cpp
	// That wasn't usable because the 'less than' operator was hardcoded. Any comments or
	// suggestions are welcome.
	void sort(CompareFunction compare) {
		iterator first = Common_List::begin();
		iterator last = Common_List::end();

		if (first == last)
			return;

		// Simple selection sort
		iterator i(first);
		for (; i != last; ++i) {
			iterator minElem(i);
			iterator j(i);
			++j;
			for (; j != last; ++j)
				if (compare(*j, *minElem) < 0)
					minElem = j;
			if (minElem != i)
				SWAP(*minElem, *i);
		}
	}

};

} // namespace Parallaction


#endif





