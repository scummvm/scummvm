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

#ifndef AGS_STD_UTILITY_H
#define AGS_STD_UTILITY_H

#include "common/textconsole.h"

namespace AGS3 {
namespace std {

template<class T1, class T2>
struct pair {
	T1 first;
	T2 second;

	pair() {
	}
	pair(T1 first_, T2 second_) : first(first_), second(second_) {
	}
};

template< class T1, class T2 >
pair<T1, T2> make_pair(T1 first, T2 second) {
	return pair<T1, T2>(first, second);
}

// STRUCT TEMPLATE remove_reference
template <class _Ty>
struct remove_reference {
	typedef _Ty type;
};

template<class _Ty>
struct remove_reference<_Ty &> {
	typedef _Ty type;
};

template<class _Ty>
struct remove_reference<_Ty &&> {
	typedef _Ty type;
};

template <class _Ty>
using remove_reference_t = typename remove_reference<_Ty>::type;

// FUNCTION TEMPLATE move
// TODO: Haven't been able to get this to properly work to reset
// the source when moving the contents of std::vector arrays
template <class _Ty>
constexpr remove_reference_t<_Ty> &&move(_Ty &&_Arg) noexcept {
	return static_cast<remove_reference_t<_Ty> &&>(_Arg);
}

} // namespace std
} // namespace AGS3

#endif
