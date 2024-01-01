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

#ifndef COMMON_INITIALIZER_LIST_H
#define COMMON_INITIALIZER_LIST_H

#include "common/scummsys.h"

//
// std::initializer_list
// Provide replacement when not available
//
#if defined(NO_CXX11_INITIALIZER_LIST)
namespace std {
template<class T>
class initializer_list {
public:
	typedef T value_type;
	typedef const T &reference;
	typedef const T &const_reference;
	typedef size_t size_type;
	typedef const T *iterator;
	typedef const T *const_iterator;

	constexpr initializer_list() noexcept = default;
	constexpr size_t size() const noexcept { return _size; };
	constexpr const T *begin() const noexcept { return _begin; };
	constexpr const T *end() const noexcept { return _begin + _size; }

private:
	// Note: begin has to be first or the compiler may get very upset
	const T *_begin = {nullptr};
	size_t _size = {0};

	// The compiler is allowed to call this constructor
	constexpr initializer_list(const T *t, size_t s) noexcept : _begin(t), _size(s) {}
};

template<class T>
constexpr const T *begin(initializer_list<T> il) noexcept {
	return il.begin();
}

template<class T>
constexpr const T *end(initializer_list<T> il) noexcept {
	return il.end();
}
} // namespace std

#else

#include <initializer_list>

#endif // NO_CXX11_INITIALIZER_LIST

#endif
