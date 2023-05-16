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

#ifndef AGS_STD_FUNCTIONAL_H
#define AGS_STD_FUNCTIONAL_H

namespace AGS3 {
namespace std {

template <class _Arg, class _Result>
struct unary_function { // base class for unary functions
	using argument_type = _Arg;
	using result_type = _Result;
};

template <class _Arg1, class _Arg2, class _Result>
struct binary_function { // base class for binary functions
	using first_argument_type = _Arg1;
	using second_argument_type = _Arg2;
	using result_type = _Result;
};

template <typename _Fty>
struct function {
	_Fty *_fn;

	function() : _fn(nullptr) {}
	function(_Fty *fn) : _fn(fn) {}

	operator _Fty &() {
		return *_fn;
	}

	operator bool() {
		return _fn != nullptr;
	}
};

} // namespace std
} // namespace AGS3

#endif
