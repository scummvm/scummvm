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

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <new>
#include <limits>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

/* newlib ctype.h defines _X for hex digit flag.
   This conflicts with the use of _X as a variable name. */
#undef _X

#if defined(DINGUX) || defined(_3DS)
#define RES_W_OVERLAY 320
#define RES_H_OVERLAY 200
#else
#define RES_W_OVERLAY 640
#define RES_H_OVERLAY 400
#endif

// HACK: With MinGW, GRIM engine seems to crash when using setjmp and longjmp if not using builtin versions
#if defined __MINGW64__ || defined __MINGW32__
#include <setjmp.h>
#undef setjmp
#undef longjmp
#define setjmp(a) (__builtin_setjmp(a))
#define longjmp(a, b) (__builtin_longjmp(a, b))
#endif
