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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef COMMON_CPP11_COMPAT_H
#define COMMON_CPP11_COMPAT_H

#if __cplusplus >= 201103L
#error "c++11-compat.h included when C++11 is available"
#endif

//
// Custom nullptr replacement. This is not type safe as the real C++11 nullptr
// though.
//
#define nullptr 0

//
// Replacement for the override keyword. This allows compilation of code
// which uses it, but does not feature any semantic.
//
#define override

#endif
