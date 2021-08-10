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

#ifndef AGS_STD_MATH_H
#define AGS_STD_MATH_H

#include "common/hashmap.h"
#include "ags/lib/std/utility.h"

namespace AGS3 {
namespace std {

// Not all platforms define INFINITY
#ifndef INFINITY
#define INFINITY   ((float)(1e+300 * 1e+300)) // This must overflow
#endif

#define FLOAT_UNASSIGNED (float)999999.0

template<class T>
inline bool isUndefined(T val) {
	return val == FLOAT_UNASSIGNED;
}

} // namespace std
} // namespace AGS3

#endif
