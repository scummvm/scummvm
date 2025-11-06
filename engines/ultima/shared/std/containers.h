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

#ifndef ULTIMA_STD_CONTAINERS_H
#define ULTIMA_STD_CONTAINERS_H

#include "common/algorithm.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/list.h"
#include "common/queue.h"
#include "common/stack.h"
#include "common/util.h"

namespace Ultima {
namespace Std {

template<class T>
class vector : public Common::Array<T> {
public:
	constexpr vector() : Common::Array<T>() {}
	vector(size_t newSize) : Common::Array<T>(newSize) {}
	vector(size_t newSize, const T elem) : Common::Array<T>(newSize, elem) {}
};

template<class T>
class list : public Common::List<T> {
};

} // End of namespace Std
} // End of namespace Ultima

#endif
