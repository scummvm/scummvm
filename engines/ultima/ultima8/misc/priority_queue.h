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

#ifndef ULTIMA8_MISC_PRIORITY_QUEUE_H
#define ULTIMA8_MISC_PRIORITY_QUEUE_H

#include "common/algorithm.h"
#include "common/util.h"

namespace Ultima {
namespace Ultima8 {

/**
 * Queue ordered by a provided priority function
 * NOTE: Unlike in the C std library, we have to provde a comparitor that sorts
 * the array so that the smallest priority comes last
 */
template <class _Ty, class _Container, class _Pr>
class PriorityQueue {
public:
	PriorityQueue() : c(), comp() {}

	explicit PriorityQueue(const _Pr &_Pred) : c(), comp(_Pred) {}

	PriorityQueue(const _Pr &_Pred, const _Container &_Cont) : c(_Cont), comp(_Pred) {
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	PriorityQueue(_InIt _First, _InIt _Last, const _Pr &_Pred, const _Container &_Cont) : c(_Cont), comp(_Pred) {
		c.insert(c.end(), _First, _Last);
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	PriorityQueue(_InIt _First, _InIt _Last) : c(_First, _Last), comp() {
		make_heap(c.begin(), c.end(), comp);
	}

	template <class _InIt>
	PriorityQueue(_InIt _First, _InIt _Last, const _Pr &_Pred) : c(_First, _Last), comp(_Pred) {
		make_heap(c.begin(), c.end(), comp);
	}

	bool empty() const {
		return c.empty();
	}

	size_t size() const {
		return c.size();
	}

	typename _Container::const_reference top() const {
		return c.back();
	}

	void push(const typename _Container::value_type &_Val) {
		c.push_back(_Val);
		Common::sort(c.begin(), c.end(), comp);
	}

	void pop() {
		c.pop_back();
	}

	void swap(PriorityQueue &_Right) {
		SWAP(c, _Right.c);
		SWAP(comp, _Right.comp);
	}

protected:
	_Container c;
	_Pr comp;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
