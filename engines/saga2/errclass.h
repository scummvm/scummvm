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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_ERRCLASS_H
#define SAGA2_ERRCLASS_H  1

namespace Saga2 {

class gError {
protected:

	static void (*dumpHandler)(char *);
	static char     *errBuf;

	static int16        errBufSize,
	       errBufOffset;
	static uint8    saveErrFlag;

	//  Special constructor for sub-classes which doesn't do
	//  anything.
	gError(void);

	//  Append to error buffer (subclasses can use)
	static void append(char *msg, va_list args);
	static void appendf(char *msg, ...);

public:
	gError(char *msg, ...);

	static void dump(void);
	static void setDumpHandler(void (*dumpHand)(char *)) {
		dumpHandler = dumpHand;
	}
	static void saveErrs(bool flag) {
		saveErrFlag = (uint8) flag;
	}

	static void warn(char *msg, ...);
	static void setErrBufSize(int32 size);
};

class MemoryError : private gError {
public:
	MemoryError(int32 allocSize);
	MemoryError(void);
};

class DosError : private gError {
public:
	DosError(char *msg, ...);
};

/* ===================================================================== *
   Breakpoint code
 * ===================================================================== */

/* ===================================================================== *
   Assertion macros (VERIFY)
 * ===================================================================== */

#undef verify

#define verify(expr) (expr)
#define assert2(__ignore,msg) ((void)0)

#define VERIFY  assert

#ifndef ASSERT
#define ASSERT(x)  assert(x)
#define ASSERTMSG   assert2
#endif

// Extensions to set a breakpoint from code

#if DEBUG && defined(__WATCOMC__)

#  ifdef __cplusplus
void debug_breakpoint(const int linenumber, const char filename[]);
void debug_dumptext(const char text[]);
#  else
void cebug_breakpoint(const int linenumber, const char filename[]);
void cebug_dumptext(const char text[]);
#  endif

#  ifdef __cplusplus
#    define DEBUG_BREAK debug_breakpoint(__LINE__,__FILE__)
#  else
#    define DEBUG_BREAK cebug_breakpoint(__LINE__,__FILE__)
#  endif

#  ifndef NO_BREAK_ON_VERIFAIL
#    ifdef VERIFY
#      undef VERIFY
#    endif
#    define VERIFY(expr) ((expr)?(void)0:(DEBUG_BREAK,__verify(#expr,__FILE__,__LINE__ )))
#  endif    // NO_BREAK_ON_VERIFAIL

#  ifndef NO_RUNTIME_MESSAGES
#    ifdef __cplusplus
#      define DEBUG_DUMP(s) debug_dumptext(s)
#    else   // __cplusplus
#      define DEBUG_DUMP(s) cebug_dumptext(s)
#    endif  // __cplusplus
#  else     // NO_RUNTIME_MESSAGES
#    define DEBUG_DUMP(s) ((void)0)
#  endif    // NO_RUNTIME_MESSAGES

#else   // DEBUG && defined(__WATCOMC__)

#  define DEBUG_BREAK ((void)0)
#  define DEBUG_DUMP(s) ((void)0)

#endif  // DEBUG && defined(__WATCOMC__)

} // end of namespace Saga2

#endif
