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

#ifndef PX_ASSERT_H_INCLUDED
#define PX_ASSERT_H_INCLUDED

#ifdef _DEBUG

#include <crtdbg.h>

// _ASSERT uses a do while(0) construct to prevent a problem with ending ';'s.

// For example, if _ASSERT was simply: if (!expr) ... then
//      if (obj)
//          _ASSERT(expr)
//      else
//          DoSomething()
// would expand to
//      if (obj)
//          if (!expr)
//              ...
//          else
//              DoSomething()
//
// Using {} so that _ASSERT was: { if (!expr) ... } would produce
//      if (obj)
//      {   if (!expr)
//              ....
//      }; <-------------------------- semi colon here does not go down too well
//      else
//              DoSomething()
//
// Microsoft make _ASSERT to be: do { if (!expr) ... } while(0)
// This fixes the problem but produces a "conditional expression is constant" warning.
// I have removed this by copying their ASSERT and using pxReturnFalse() instead of 0

#undef _ASSERT
#undef ASSERT
#undef VERIFY

#ifdef USE_CUSTOM_ASSERT

int CUSTOM_ASSERT(int reportType, const char *filename, int linenumber, const char *moduleName, const char *format, ...);

#define _CrtDbgReport CUSTOM_ASSERT
#endif // USE_CUSTOM_ASSERT

inline bool pxReturnFalse() { return (false); }

#define _ASSERT(expr)                                                                                                                                                              \
	do {                                                                                                                                                                       \
		if (!(expr) && (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, "%s", #expr)))                                                                           \
			_CrtDbgBreak();                                                                                                                                            \
	} while (pxReturnFalse())

// Another ASSERT with the ability to display a user message
#ifdef _ASSERT_MESSAGE
#undef _ASSERT_MESSAGE
#endif // #ifdef _ASSERT_MESSAGE

#define _ASSERT_MESSAGE(expr, message)                                                                                                                                             \
	do {                                                                                                                                                                       \
		if (!(expr) && (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, "%s\n%s", #expr, message)))                                                              \
			_CrtDbgBreak();                                                                                                                                            \
	} while (pxReturnFalse())

#ifndef ASSERT0
#define ASSERT0(x, msg) _ASSERT_MESSAGE(x, pxVString("Assertion failed: file %s, line %d", __FILE__, __LINE__));
#endif

#ifndef ASSERT1
#define ASSERT1(x, msg, value1) _ASSERT_MESSAGE(x, pxVString("Assertion failed: file %s, line %d : value=%d", __FILE__, __LINE__, value1));
#endif

#ifndef ASSERT2
#define ASSERT2(x, msg, value1, value2) _ASSERT_MESSAGE(x, pxVString("Assertion failed: file %s, line %d : values=%d, %d", __FILE__, __LINE__, value1, value2));
#endif

#ifndef ASSERT3
#define ASSERT3(x, msg, value1, value2, value3) _ASSERT_MESSAGE(x, pxVString("Assertion failed: file %s, line %d : values=%d, %d, %d", __FILE__, __LINE__, value1, value2, value3));
#endif

#ifndef ASSERT4
#define ASSERT4(x, msg, value1, value2, value3, value4)                                                                                                                            \
	_ASSERT_MESSAGE(x, pxVString("Assertion failed: file %s, line %d : values=%d, %d, %d, %d", __FILE__, __LINE__, value1, value2, value3, value4));
#endif

// Sometimes you want ASSERT(false) to allways assert, again producing the constant warning.
// Use _ASSERT_ALLWAYS instead

#ifdef _ASSERT_ALLWAYS
#undef _ASSERT_ALLWAYS
#endif // #ifdef define _ASSERT_ALLWAYS

#define _ASSERT_ALLWAYS                                                                                                                                                            \
	do {                                                                                                                                                                       \
		if (1 == _CrtDbgReport(_CRT_ASSERT, __FILE__, __LINE__, NULL, "Assert always"))                                                                                    \
			_CrtDbgBreak();                                                                                                                                            \
	} while (pxReturnFalse())

#define ASSERT(x) _ASSERT(x)

// _VERIFY is not defined in crtdbg.h
#define _VERIFY(x) _ASSERT(x) // Verify becomes Assert.
#define VERIFY(x) _ASSERT(x)  // Verify becomes Assert.

#else // _DEBUG

#define ASSERT(expr) ((void)0)
#define _ASSERT(expr) ((void)0)
#define _ASSERT_MESSAGE(expr, msg) ((void)0)
#define _ASSERT_ALLWAYS ((void)0)
#define ASSERT0(x, msg) ((void)0)
#define ASSERT1(x, msg, a) ((void)0)
#define ASSERT2(x, msg, a, b) ((void)0)
#define ASSERT3(x, msg, a, b, c) ((void)0)
#define ASSERT4(x, msg, a, b, c, d) ((void)0)

// _VERIFY is not defined in crtdbg.h
#ifndef VERIFY
#define VERIFY(x) x // Verify removed, leaving just the code.
#endif
#define _VERIFY(x) x // Verify removed, leaving just the code.

#endif // _DEBUG

#endif // #ifndef PX_ASSERT_H_INCLUDED
