/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifdef _MSC_VER
// The bool warning may need to be disabled again by including this file more than once
#pragma warning(disable : 4237) // C4237: nonstandard extension used : 'bool' keyword is reserved for future use
#endif                          // _MSC_VER

// All the rest of the definitions want defining just the once

#ifndef PXCOMMON_H_INCLUDED
#define PXCOMMON_H_INCLUDED
namespace ICB {
// Disable redundant warnings resulting from warning level 4

// This warning results from the use of (while(true)) and also from the microsoft version of
// _ASSERT which uses a do{}while(false)
#ifdef _MSC_VER
#pragma warning(disable : 4127) // Disable warning: conditional expression is constant

// These warnings appear for no apparent reason

#pragma warning(disable : 4201) // C4201: nonstandard extension used : nameless struct/union
#pragma warning(disable : 4505) // C4505: '%s' : unreferenced local function has been removed
#pragma warning(disable : 4514) // Disable warning: unreferenced inline function has been removed
#pragma warning(disable : 4710) // C4710: function '' not expanded

// Although this would be damm useful, MSVC implementation seems to give no help in tracking down
// the problem when it occurs, which is frequently in nested inlines of codes
#pragma warning(disable : 4702) // C4702: unreachable code

// These functions seemed superfluous at some point, but no int32er appear.

//#pragma warning (disable : 4200 ) // warning C4200: nonstandard extension used  : zero-sized array in struct/union
#endif // _MSC_VER

// For VC4 define a bool type that is compatible with the bool defined in VC5
// Also define compatible types for true and false

#ifndef PXCOMMON_H_BOOL
#define PXCOMMON_H_BOOL

// bool, true, false - Boolean type and literals
#if defined(_MSC_VER) && (_MSC_VER < 1100) // Version 5
// Define equivalents to VC5's keywords
// The definitions are as near as VC4 can do: sizeof(bool) and values of true and false are
// identical, but the range of values allowed in bool is not (VC5 confines to true and false).

// bool changed from uint8 to uint32 to be correctly dword aligned for the psx
//  typedef unsigned char bool;
typedef unsigned int bool;
#define true((bool)(0 == 0))
#define false((bool)(0 != 0))

// VC4 does not like mutable, which causes some problems with making functions
// const. Where this occurs use RC_CONST instead, which is only const under VC5
#define RC_CONST

#else // _MSC_VER < 1100
#define RC_CONST const

#endif // _MSC_VER < 1100

#endif // PXCOMMON_H_BOOL

// For certain classes the copy constructor and assignment is not a legal operation
// Make disabling of these features easier
#define PreventClassCopy(classname)                                                                                                                                                \
	classname(const classname &) { ThrowMemoryException; }                                                                                                                     \
	void operator=(const classname &) { ThrowMemoryException; }
#define ThrowMemoryException *((volatile char *)NULL) = 0;

} // End of namespace ICB

#endif // #ifndef PXCOMMON_H_INCLUDED
