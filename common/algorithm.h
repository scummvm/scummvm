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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef COMMON_ALGORITHM_H
#define COMMON_ALGORITHM_H

#include "common/scummsys.h"

namespace Common {

template<class In, class Out>
Out copy(In first, In last, Out dst) {
	while (first != last)
		*dst++ = *first++;
	return dst;
}

template<class In, class Out>
Out copy_backward(In first, In last, Out dst) {
	while (first != last)
		*--dst = *--last;
	return dst;
}

template<class In, class Out, class Op>
Out copy_if(In first, In last, Out dst, Op op) {
	while (first != last) {
		if (op(*first))
			*dst++ = *first;
		++first;
	}
	return dst;
}

template<class In, class T>
In find(In first, In last, const T &v) {
	while (first != last) {
		if (*first == v)
			return first;
		++first;
	}
	return last;
}

template<class In, class Pred>
In find_if(In first, In last, Pred p) {
	while (first != last) {
		if (p(*first))
			return first;
		++first;
	}
	return last;
}

template<class In, class Op>
Op for_each(In first, In last, Op f) {
	while (first != last) f(*first++);
	return f;
}

// Simple sort function, modelled after std::sort.
// Use it like this:  sort(container.begin(), container.end()).
// Also work on plain old int arrays etc.
template<class T>
void sort(T first, T last) {
	if (first == last)
		return;

	// Simple selection sort
	T i(first);
	for (; i != last; ++i) {
		T minElem(i);
		T j(i);
		++j;
		for (; j != last; ++j)
			if (*j < *minElem)
				minElem = j;
		if (minElem != i)
			SWAP(*minElem, *i);
	}
}

} // end of namespace Common
#endif

