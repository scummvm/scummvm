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

#ifndef AGS_STD_QUEUE_H
#define AGS_STD_QUEUE_H

#include "ags/lib/std/algorithm.h"
#include "ags/lib/std/vector.h"
#include "common/queue.h"

namespace AGS3 {
namespace std {

template<class T>
using queue = Common::Queue<T>;

/**
 * FIXME: The current implementation requires the reverse
 * greater/lesser comparitor than the original does.
 * If this is fixed, also change the router finder's use
 */
template<class T, class Container = vector<T>, class Comparitor = typename Common::Less<T> >
class priority_queue {
private:
	Container _container;
	Comparitor _comparitor;
public:
	priority_queue() {}

	bool empty() const {
		return _container.empty();
	}

	const T &top() const {
		return _container.front();
	}

	void push(const T &item) {
		_container.push_back(item);
		Common::sort(_container.begin(), _container.end(), _comparitor);
	}

	void pop() {
		_container.remove_at(0);
	}
};

} // namespace std
} // namespace AGS3

#endif
