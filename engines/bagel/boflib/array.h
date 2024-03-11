
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

#ifndef BAGEL_BOFLIB_ARRAY_H
#define BAGEL_BOFLIB_ARRAY_H

#include "common/array.h"

namespace Bagel {

template<class T>
class Array : public Common::Array<T> {
public:
	int indexOf(T t) {
		for (int i = 0; i < (int)this->size(); ++i) {
			if (this->operator[](i) == t)
				return i;
		}

		return -1;
	}

	bool contains(T t) const {
		return this->indexOf(t) != -1;
	}

	void remove(T t) {
		int idx = this->indexOf(t);
		if (idx != -1)
			this->remove_at(idx);
	}
};

} // namespace Bagel

#endif
