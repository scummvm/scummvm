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

/**
 * A set of templates which removes const and/or volatile specifiers.
 * Use the remove_*_t<T> variants.
 */
template<class T> struct remove_cv {
	typedef T type;
};
template<class T> struct remove_cv<const T> {
	typedef T type;
};
template<class T> struct remove_cv<volatile T> {
	typedef T type;
};
template<class T> struct remove_cv<const volatile T> {
	typedef T type;
};

template<class T> struct remove_const {
	typedef T type;
};
template<class T> struct remove_const<const T> {
	typedef T type;
};

template<class T> struct remove_volatile {
	typedef T type;
};
template<class T> struct remove_volatile<volatile T> {
	typedef T type;
};

/**
 * A set of templates which add const or volatile qualifiers
 */
template<class T>
struct add_cv {
	typedef const volatile T type;
};
template<class T>
struct add_const {
	typedef const T type;
};
template<class T> struct add_volatile {
	typedef volatile T type;
};

template<class T>
using remove_cv_t        = typename remove_cv<T>::type;
template<class T>
using remove_const_t     = typename remove_const<T>::type;
template<class T>
using remove_volatile_t  = typename remove_volatile<T>::type;

template<class T>
using add_cv_t        = typename add_cv<T>::type;
template<class T>
using add_const_t     = typename add_const<T>::type;
template<class T>
using add_volatile_t  = typename add_volatile<T>::type;

/**
 * A set of templates which removes the reference over types.
 * Use remove_reference_t<T> for this.
 */
template<class T>
struct remove_reference {
	typedef T type;
};
template<class T>
struct remove_reference<T &> {
	typedef T type;
};
template<class T>
struct remove_reference<T &&> {
	typedef T type;
};

template<class T>
using remove_reference_t = typename remove_reference<T>::type;

template<bool b, class T, class F>
struct conditional {
	typedef T type;
};
template<class T, class F>
struct conditional<false, T, F> {
	typedef F type;
};

template<bool b, class T, class F>
using conditional_t = typename conditional<b, T, F>::type;

} // End of namespace Common

#endif
