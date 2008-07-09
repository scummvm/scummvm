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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/branches/gsoc2008-rtl/common/stack.h $
 * $Id$
 */

#ifndef COMMON_QUEUE_H
#define COMMON_QUEUE_H

#include "common/scummsys.h"
#include "common/list.h"

namespace Common {

/**
 * Variable size Queue class, implemented using our Array class.
 */
template<class T>
class Queue {
protected:
	List<T>		_queue;
public:
	Queue<T>() {}
	Queue<T>(const List<T> &queueContent) : _queue(queueContent) {}

	bool empty() const {
		return _queue.empty();
	}
	void clear() {
		_queue.clear();
	}
	void push(const T &x) {
		_queue.push_back(x);
	}
	T back() const {
		return _queue.reverse_begin().operator*();
	}
	T front() const {
		return _queue.begin().operator*();
	}
	T pop() {
		T tmp = front();
		_queue.pop_front();
		return tmp;
	}
	int size() const {
		return _queue.size();
	}
};

} // End of namespace Common

#endif
