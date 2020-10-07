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
 */

#ifndef COMMON_HASH_PTR_H
#define COMMON_HASH_PTR_H

#include "common/func.h"

namespace Common {

/**
 * @defgroup common_hashmap_ptr Hash table pointer
 * @ingroup common_hashmap
 *
 * @brief Template for a HashMap pointer.
 *
 * @{
 */

/**
 * Partial specialization of the Hash functor allowing to use pointers as HashMap keys.
 */
template<typename T>
struct Hash<T *> {
	uint operator()(T * const &v) const {
		uint x = static_cast<uint>(reinterpret_cast<uintptr>(v));
		return x + (x >> 3);
	}
};

/** @} */

} // End of namespace Common

#endif
