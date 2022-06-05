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

#ifndef COMMON_TYPE_TRAITS_H
#define COMMON_TYPE_TRAITS_H

namespace Common {
	template <bool b, class T, class U> struct Conditional { typedef T type; };
	template <class T, class U> struct Conditional<false, T, U> { typedef U type; };
	template <typename T> struct RemoveConst { typedef T type; };
	template <typename T> struct RemoveConst<const T> { typedef T type; };
	template <typename T> struct AddConst { typedef const T type; };
	template <typename T> struct IsUnsigned { static const bool value = ((T(0) < T(-1))); };

	template <typename T> struct IsIntegral { static const bool value = false; };
	template<> struct IsIntegral<bool> { static const bool value = true; };
	template<> struct IsIntegral<char> { static const bool value = true; };
	template<> struct IsIntegral<signed char> { static const bool value = true; };
	template<> struct IsIntegral<unsigned char> { static const bool value = true; };
	template<> struct IsIntegral<wchar_t> { static const bool value = true; };
	template<> struct IsIntegral<short> { static const bool value = true; };
	template<> struct IsIntegral<unsigned short> { static const bool value = true; };
	template<> struct IsIntegral<int> { static const bool value = true; };
	template<> struct IsIntegral<unsigned int> { static const bool value = true; };
	template<> struct IsIntegral<long> { static const bool value = true; };
	template<> struct IsIntegral<unsigned long> { static const bool value = true; };
	template<> struct IsIntegral<long long> { static const bool value = true; };
	template<> struct IsIntegral<unsigned long long> { static const bool value = true; };
	
	template <typename T> struct IsFloatingPoint { static const bool value = false; };
	template<> struct IsFloatingPoint<float> { static const bool value = true; };
	template<> struct IsFloatingPoint<double> { static const bool value = true; };
	
	template <typename T> struct IsArithmetic { static const bool value = IsIntegral<T>::value || IsFloatingPoint<T>::value; };
} // End of namespace Common

#endif
