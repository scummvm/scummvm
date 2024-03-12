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

#ifndef HPL1_ALGORITHMS
#define HPL1_ALGORITHMS

#include "common/array.h"

namespace Hpl1 {

template<typename T, typename V>
void resizeAndFill(Common::Array<T> &container, const typename Common::Array<T>::size_type newSize, const V &value) {
	const auto oldSize = container.size();
	container.resize(newSize);
	for (auto i = oldSize; i < newSize; ++i)
		container[i] = value;
}

template<typename RandomIt, typename Pred>
RandomIt removeIf(RandomIt begin, RandomIt end, Pred pred) {
	for (auto i = begin; i != end; ++i) {
		if (!pred(*i))
			*begin++ = *i;
	}
	return begin;
}

} // namespace Hpl1

#endif // HPL1_ALGORITHMS
