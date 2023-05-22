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

#ifndef COMMON_MEMORY_H
#define COMMON_MEMORY_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_memory Memory
 * @ingroup common
 *
 * @brief Functions for managing the memory.
 * @{
 */

/**
 * Fills memory [dst, dst + count) with the value val.
 *
 * This is a replacement function for Common::fill, using an unrolled
 * loop to maximize performance on most architectures.
 *
 * This fill operation is extensively used throughout the graphics code,
 * so this counts as one of the main bottlenecks. Please replace it with
 * assembly when possible!
 *
 * Note that pointers passed to these functions must be aligned correctly.
 */
void memset2(uint16 *dst, uint16 val, size_t count);
void memset4(uint32 *dst, uint32 val, size_t count);
void memset8(uint64 *dst, uint64 val, size_t count);

/**
 * Copies data from the range [first, last) to [dst, dst + (last - first)).
 * It requires the range [dst, dst + (last - first)) to be valid and
 * uninitialized.
 */
template<class In, class Type>
Type *uninitialized_copy(In first, In last, Type *dst) {
	while (first != last)
		new ((void *)dst++) Type(*first++);
	return dst;
}

/**
 * Initializes the memory [first, first + (last - first)) with the value x.
 * It requires the range [first, first + (last - first)) to be valid and
 * uninitialized.
 */
/*template<class Type, class Value>
void uninitialized_fill(Type *first, Type *last, const Value &x) {
	while (first != last)
		new ((void *)first++) Type(x);
}*/

/**
 * Initializes the memory [dst, dst + n) with the value x.
 * It requires the range [dst, dst + n) to be valid and
 * uninitialized.
 */
template<class Type, class Value>
void uninitialized_fill_n(Type *dst, size_t n, const Value &x) {
	while (n--)
		new ((void *)dst++) Type(x);
}

/** @} */

} // End of namespace Common

#endif
