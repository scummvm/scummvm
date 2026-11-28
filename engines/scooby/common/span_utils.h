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

#ifndef SCOOBY_COMMON_SPAN_UTILS_H
#define SCOOBY_COMMON_SPAN_UTILS_H

#include "common/scummsys.h"

#include "common/algorithm.h"
#include "common/array.h"
#include "common/span.h"
#include "common/type_traits.h"

// The engine uses ScummVM's bounds-checked Common::Span. These helpers cover what Common::Span does not
// provide itself: making a span over a Common::Array, filling a span, and copying
// a span into a new Common::Array.

namespace Scooby {

using Common::Span;

template<typename T>
inline Span<T> makeSpan(Common::Array<T> &a) {
	return Span<T>(a.data(), a.size());
}

template<typename T>
inline Span<const T> makeSpan(const Common::Array<T> &a) {
	return Span<const T>(a.data(), a.size());
}

// Sets every element of span to value.
template<typename T, typename V>
inline void fillSpan(const Span<T> &span, const V &value) {
	Common::fill(span.data(), span.data() + span.size(), value);
}

// Copies the span's elements into a new array.
template<typename T>
inline Common::Array<typename Common::remove_const<T>::type> toArray(const Span<T> &span) {
	return Common::Array<typename Common::remove_const<T>::type>(span.data(), span.size());
}
} // namespace Scooby

#endif // SCOOBY_COMMON_SPAN_UTILS_H
