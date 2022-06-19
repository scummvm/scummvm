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

#ifndef AGS_STD_ALGORITHM_H
#define AGS_STD_ALGORITHM_H

#include "common/algorithm.h"
#include "common/util.h"

namespace AGS3 {
namespace std {

template<typename T> inline T abs(T x) {
	return ABS(x);
}
template<typename T> inline T min(T a, T b) {
	return MIN(a, b);
}
template<typename T> inline T max(T a, T b) {
	return MAX(a, b);
}
template<typename T> inline T clip(T v, T amin, T amax) {
	return CLIP(v, amin, amax);
}
template<typename T> inline T sqrt(T x) {
	return ::sqrt(x);
}
template<typename T> inline void swap(T &a, T &b) {
	SWAP(a, b);
}

template<class In, class Value>
In fill(In first, In last, const Value &val) {
	return Common::fill(first, last, val);
}

template<typename T, class StrictWeakOrdering>
void sort(T first, T last, StrictWeakOrdering comp) {
	Common::sort(first, last, comp);
}

template<typename T>
void sort(T *first, T *last) {
	Common::sort(first, last, Common::Less<T>());
}

template<class T>
void sort(T first, T last) {
	Common::sort(first, last);
}

template<class In, class T>
In find(In first, In last, const T &v) {
	return Common::find(first, last, v);
}

template<class T>
T unique(T first, T last) {
	T pos;
	for (pos = first + 1; pos < last; ++pos) {
		// Check for duplicate
		for (T existingPos = first; existingPos < last; ++existingPos) {
			if (*pos == *existingPos) {
				// Found a match, so shift values over the duplicate
				while (pos < (last - 1)) {
					T &prior = pos;
					++pos;
					prior = pos;
				}

				--last;
				break;
			}
		}
	}

	return pos;
}

template<class ForwardIt, class T>
ForwardIt lower_bound(ForwardIt first, ForwardIt last, const T &value) {
	for (ForwardIt it = first; it < last; ++it) {
		if (*it >= value)
			return it;
	}

	return last;
}

template<class ForwardIt, class T>
ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T &value) {
	for (ForwardIt it = first; it < last; ++it) {
		if (*it > value)
			return it;
	}

	return last;
}

template<class ForwardIt, class T, class Compare>
ForwardIt upper_bound(ForwardIt first, ForwardIt last, const T &value, Compare comp) {
	for (ForwardIt it = first; it < last; ++it) {
		if (comp(value, *it))
			return it;
	}

	return last;
}

template<class ForwardIt>
ForwardIt next(ForwardIt it, int n = 1) {
	ForwardIt it2 = it;
	while (n > 0) { ++it2; --n; }
	while (n < 0) { --it2; ++n; }
	return it2;
}

template<class BidirIt>
BidirIt prev(BidirIt it, int n = 1) {
	BidirIt it2 = it;
	while (n > 0) { --it2; --n; }
	while (n < 0) { ++it2; ++n; }
	return it2;
}

} // namespace std
} // namespace AGS3

#endif
