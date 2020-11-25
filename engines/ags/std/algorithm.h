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

#ifndef AGS_STD_ALGORITHM_H
#define AGS_STD_ALGORITHM_H

#include "common/algorithm.h"
#include "common/util.h"

namespace AGS3 {
namespace std {

template<typename T> inline T abs(T x) { return ABS(x); }
template<typename T> inline T min(T a, T b) { return MIN(a, b); }
template<typename T> inline T max(T a, T b) { return MAX(a, b); }
template<typename T> inline T clip(T v, T amin, T amax) { return CLIP(v, amin, amax); }
template<typename T> inline T sqrt(T x) { return ::sqrt(x); }
template<typename T> inline void swap(T a, T b) { SWAP(a, b); }

template<class In, class Value>
In fill(In first, In last, const Value &val) {
	return Common::fill(first, last, val);
}

template<typename T, class StrictWeakOrdering>
void sort(T first, T last, StrictWeakOrdering comp) {
	Common::sort<T, StrictWeakOrdering>(first, last, comp);
}

} // namespace std
} // namespace AGS3

#endif
