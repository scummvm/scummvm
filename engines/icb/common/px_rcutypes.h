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

#ifndef _px_rcUTYPES_H
#define _px_rcUTYPES_H

#include "px_rccommon.h" // This should be included in every file before anything else

#include <limits.h>

namespace ICB {

typedef const char *cstr;

typedef unsigned int uint; // Short hand for unsigned int

#if CHAR_MAX == 255
typedef signed char int8;
typedef unsigned char uint8;
#else
typedef char int8;
typedef unsigned char uint8;
#endif

typedef unsigned int word;

#if USHRT_MAX == 0xffff
typedef short int int16;
typedef unsigned short int uint16;
#elif UINT_MAX == 0xffff
typedef short int int16;
typedef unsigned short int uint16;
#else
#error Cannot define types: int16, uint16
#endif

#if UINT_MAX == 0xffffffff
typedef int int32;
typedef unsigned int uint32;
#elif ULONG_MAX == 0xffffffff
typedef int int32;
typedef unsigned int uint32;
#else
#error Cannot define types: int32, uint32
#endif

// Definition of a boolean value that can be used across the PC and PSX.  I stopped true being 0xff because
// C++ weak typing allows you to assign a bool8 to an int8 without warning, whereupon '==' fails for TRUE8 because
// one is signed and one isn't.
typedef unsigned char bool8;
#define TRUE8 ((unsigned char)0x01)
#define FALSE8 ((unsigned char)0x00)

// end of file

} // End of namespace ICB

#endif //_px_rcUTYPES_H
