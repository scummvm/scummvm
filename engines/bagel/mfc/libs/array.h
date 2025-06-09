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

#ifndef BAGEL_MFC_LIBS_ARRAY_H
#define BAGEL_MFC_LIBS_ARRAY_H

#include "common/array.h"

namespace Bagel {
namespace MFC {
namespace Libs {

template<class T>
class Array : public Common::Array<T> {
public:
	constexpr Array() : Common::Array<T>() {
	}
	explicit Array(size_t count) : Common::Array<T>(count) {
	}
	Array(size_t count, const T &value) :
		Common::Array<T>(count, value) {
	}
	Array(const Common::Array<T> &array) : Common::Array<T>(array) {
	}
	Array(Common::Array<T> &&old) : Common::Array<T>(old) {
	}
	Array(std::initializer_list<T> list) : Common::Array<T>(list) {
	}
	template<class T2>
	Array(const T2 *array, size_t n) :
		Common::Array<T>(array, n) {
	}

	int indexOf(const T &value) const {
		for (int i = 0; i < (int)this->size(); ++i) {
			if (this->operator[](i) == value)
				return i;
		}
		return -1;
	}
	bool remove(const T &value) {
		int idx = this->indexOf(value);
		if (idx != -1)
			this->remove_at(idx);
		return idx != -1;
	}
	bool contains(const T &value) const {
		return this->indexOf(value) != -1;
	}
};

} // namespace Libs
} // namespace MFC
} // namespace Bagel

#endif
