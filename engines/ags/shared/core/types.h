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

//=============================================================================
//
// Basic types definition
//
//=============================================================================

#ifndef AGS_SHARED_CORE_TYPES_H
#define AGS_SHARED_CORE_TYPES_H

#include "common/scummsys.h"
//include <stddef.h>
//include <stdint.h>
//include <stdlib.h> // for size_t
//include <limits.h> // for _WORDSIZE

namespace AGS3 {

#ifndef NULL
#define NULL nullptr
#endif

// Not all compilers have this. Added in clang and gcc followed
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef FORCEINLINE
#ifdef _MSC_VER
#define FORCEINLINE __forceinline

#elif defined (__GNUC__) || __has_attribute(__always_inline__)
#define FORCEINLINE inline __attribute__((__always_inline__))

#else
#define FORCEINLINE inline

#endif
#endif

typedef int64 soff_t;		// Stream offset type
typedef int64 intptr_t;

// fixed point type
#define fixed_t int
#define color_t int

#undef INT16_MIN
#undef INT16_MAX
#undef INT32_MIN
#undef INT32_MAX
#undef INT_MIN
#undef INT_MAX
#undef UINT_MAX
#undef SIZE_MAX
#define INT16_MIN     -32768
#define INT16_MAX     0x7fff
#define INT32_MIN     (-2147483647 - 1)
#define INT32_MAX       2147483647
#define INT_MIN     (-2147483647 - 1)
#define INT_MAX       2147483647
#define UINT_MAX      0xffffffff
#define SIZE_MAX      0xffffffff

#undef TRUE
#undef FALSE
#define TRUE true
#define FALSE false

// TODO: use distinct fixed point class
enum {
	kShift = 16,
	kUnit = 1 << kShift
};

/**
 * Basic union that can be either a number or a pointer. Helps avoid some
 * of the more nasty casts the codebase does, which was causing issues
 * on 64-bit systems
 */
union NumberPtr {
	int _value;
	void *_ptr;

	NumberPtr(int value) { _ptr = nullptr; _value = value; }
	NumberPtr(void *ptr) : _ptr(ptr) {}
	operator int() const { return _value; }
};

} // namespace AGS3

#endif
