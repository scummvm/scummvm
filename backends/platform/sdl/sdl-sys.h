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

// Remove FILE override from common/forbidden.h, and replace
// it with an alternate slightly less unfriendly override.
#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_FILE)
#undef FILE
// Solaris has typedef __FILE FILE in several places already
#if !defined(__sun)
typedef struct { int FAKE; } FAKE_FILE;
#define FILE FAKE_FILE
#endif   // (__sun)
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strcasecmp)
#undef strcasecmp
#define strcasecmp FAKE_strcasecmp
#endif

#if !defined(FORBIDDEN_SYMBOL_ALLOW_ALL) && !defined(FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp)
#undef strncasecmp
#define strncasecmp FAKE_strncasecmp
#endif

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

#include <SDL_syswm.h>

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
// following names. In a moment of mental breakdown, which occured upon
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

// SDL 2 has major API changes. We redefine constants which got renamed to
// ease the transition. This is sometimes dangerous because the values changed
// too!
#if SDL_VERSION_ATLEAST(2, 0, 0)

// Type names which changed between SDL 1.2 and SDL 2.
#define SDLKey     SDL_Keycode
#define SDLMod     SDL_Keymod
#define SDL_keysym SDL_Keysym

// Key code constants which got renamed.
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#define SDLK_NUMLOCK   SDLK_NUMLOCKCLEAR
#define SDLK_LSUPER    SDLK_LGUI
#define SDLK_RSUPER    SDLK_RGUI
#define SDLK_PRINT     SDLK_PRINTSCREEN
#define SDLK_COMPOSE   SDLK_APPLICATION
#define SDLK_KP0       SDLK_KP_0
#define SDLK_KP1       SDLK_KP_1
#define SDLK_KP2       SDLK_KP_2
#define SDLK_KP3       SDLK_KP_3
#define SDLK_KP4       SDLK_KP_4
#define SDLK_KP5       SDLK_KP_5
#define SDLK_KP6       SDLK_KP_6
#define SDLK_KP7       SDLK_KP_7
#define SDLK_KP8       SDLK_KP_8
#define SDLK_KP9       SDLK_KP_9

// Meta key constants which got renamed.
#define KMOD_META KMOD_GUI

// SDL surface flags which got removed.
#define SDL_SRCCOLORKEY 0
#define SDL_SRCALPHA    0
#define SDL_FULLSCREEN  0x40000000

// Compatibility implementations for removed functionality.
int SDL_SetColors(SDL_Surface *surface, SDL_Color *colors, int firstcolor, int ncolors);
int SDL_SetAlpha(SDL_Surface *surface, Uint32 flag, Uint8 alpha);

#define SDL_SetColorKey SDL_SetColorKey_replacement
int SDL_SetColorKey_replacement(SDL_Surface *surface, Uint32 flag, Uint32 key);

#endif


#endif
