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

#ifndef COMMON_TYPE_TRAITS_H
#define COMMON_TYPE_TRAITS_H

namespace Common {
	template <typename T, T v>
	struct IntegralConstant {
		typedef T value_type;
		typedef IntegralConstant<T, v> type;
		static const T value = v;
	};

	typedef IntegralConstant<bool, true> TrueType;
	typedef IntegralConstant<bool, false> FalseType;

	template <bool Condition, typename T = void> struct EnableIf {};
	template <typename T> struct EnableIf<true, T> { typedef T type; };

	template <bool Condition, typename T = void> struct DisableIf { typedef T type; };
	template <typename T> struct DisableIf<true, T> {};

	template <bool b, typename T, typename> struct Conditional { typedef T type; };
	template <typename T, typename U> struct Conditional<false, T, U> { typedef U type; };

	template <typename T> struct RemoveConst { typedef T type; };
	template <typename T> struct RemoveConst<const T> { typedef T type; };

	template <typename T> struct AddConst { typedef const T type; };

	template <typename T> struct RemovePointer { typedef T type; };
	template <typename T> struct RemovePointer<T *> { typedef T type; };

	template <typename T> struct RemoveArray { typedef T type; };
	template <typename T> struct RemoveArray<T []> { typedef T type; };

	template <class> struct IsArray : FalseType {};
	template <class T> struct IsArray<T []> : TrueType {};
	template <class T> struct IsArray<T const []> : TrueType {};
} // End of namespace Common

#endif
