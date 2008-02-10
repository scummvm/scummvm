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

	typedef typename Common::List<T>				Common_List;
	typedef typename Common::List<T>::iterator		iterator;

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

};

} // namespace Parallaction


#endif






