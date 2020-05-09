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

#ifndef BACKEND_SDL_SYS_H
#define BACKEND_SDL_SYS_H

// The purpose of this header is to include the SDL headers in a uniform
// fashion, even on the Symbian port.
// Moreover, it contains a workaround for the fact that SDL_rwops.h uses
// a FILE pointer in one place, which conflicts with common/forbidden.h.
// The SDL 1.3 headers also include strings.h

#include "common/scummsys.h"

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_FILE)
#undef FILE
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp)
#undef strcasecmp
#define strcasecmp FAKE_strcasecmp
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp)
#undef strncasecmp
#define strncasecmp FAKE_strncasecmp
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_exit)
#undef exit
#define exit FAKE_exit
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_abort)
#undef abort
#define abort FAKE_abort
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_system)
#undef system
#define system FAKE_system
#endif

// Fix compilation with MacPorts SDL 2
// It needs various (usually forbidden) symbols from time.h
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_time_h

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_asctime)
	#undef asctime
	#define asctime FAKE_asctime
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_clock)
	#undef clock
	#define clock FAKE_clock
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_ctime)
	#undef ctime
	#define ctime FAKE_ctime
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_difftime)
	#undef difftime
	#define difftime FAKE_difftime
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_getdate)
	#undef getdate
	#define getdate FAKE_getdate
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_gmtime)
	#undef gmtime
	#define gmtime FAKE_gmtime
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_localtime)
	#undef localtime
	#define localtime FAKE_localtime
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_mktime)
	#undef mktime
	#define mktime FAKE_mktime
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_time)
	#undef time
	#define time FAKE_time
	#endif

#endif // FORBIDDEN_SYMBOL_EXCEPTION_time_h

// HACK: SDL might include windows.h which defines its own ARRAYSIZE.
// However, we want to use the version from common/util.h. Thus, we make sure
// that we actually have this definition after including the SDL headers.
#if defined(ARRAYSIZE) && defined(COMMON_UTIL_H)
#define HACK_REDEFINE_ARRAYSIZE
#undef ARRAYSIZE
#endif

// HACK to fix compilation with SDL 2.0 in MSVC.
// In SDL 2.0, intrin.h is now included in SDL_cpuinfo.h, which includes
// setjmp.h. SDL_cpuinfo.h is included from SDL.h and SDL_syswm.h.
// Thus, we remove the exceptions for setjmp and longjmp before these two
// includes. Unfortunately, we can't use SDL_VERSION_ATLEAST here, as SDL.h
// hasn't been included yet at this point.
#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && defined(_MSC_VER)
// We unset any fake definitions of setjmp/longjmp here

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#undef setjmp
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#undef longjmp
#endif

#endif

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

// Ignore warnings from system headers pulled by SDL
#pragma warning(push)
#pragma warning(disable:4121) // alignment of a member was sensitive to packing
#include <SDL_syswm.h>
#pragma warning(pop)

// Restore the forbidden exceptions from the hack above
#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && defined(_MSC_VER)

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#undef setjmp
#define setjmp(a)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#ifndef FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#undef longjmp
#define longjmp(a,b)	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#endif

// SDL_syswm.h will include windows.h on Win32. We need to undefine its
// ARRAYSIZE definition because we supply our own.
#undef ARRAYSIZE

#ifdef HACK_REDEFINE_ARRAYSIZE
#undef HACK_REDEFINE_ARRAYSIZE
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))
#endif

// In a moment of brilliance Xlib.h included by SDL_syswm.h #defines the
// following names. In a moment of mental breakdown, which occurred upon
// gazing at Xlib.h, LordHoto decided to undefine them to prevent havoc.
#ifdef Status
#undef Status
#endif

#ifdef Bool
#undef Bool
#endif

#ifdef True
#undef True
#endif

#ifdef False
#undef False
#endif

// Finally forbid FILE again (if it was forbidden to start with)
#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_FILE)
#undef FILE
#define FILE	FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp)
#undef strcasecmp
#define strcasecmp     FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp)
#undef strncasecmp
#define strncasecmp    FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_exit)
#undef exit
#define exit(a) FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_abort)
#undef abort
#define abort() FORBIDDEN_SYMBOL_REPLACEMENT
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_system)
#undef system
#define system(a) FORBIDDEN_SYMBOL_REPLACEMENT
#endif

// re-forbid all those time.h symbols again (if they were forbidden)
#ifndef FORBIDDEN_SYMBOL_EXCEPTION_time_h

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_asctime)
	#undef asctime
	#define asctime(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_clock)
	#undef clock
	#define clock() FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_ctime)
	#undef ctime
	#define ctime(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_difftime)
	#undef difftime
	#define difftime(a,b) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_getdate)
	#undef getdate
	#define getdate(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_gmtime)
	#undef gmtime
	#define gmtime(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_localtime)
	#undef localtime
	#define localtime(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_mktime)
	#undef mktime
	#define mktime(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

	#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_time)
	#undef time
	#define time(a) FORBIDDEN_SYMBOL_REPLACEMENT
	#endif

#endif // FORBIDDEN_SYMBOL_EXCEPTION_time_h

#endif
