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

#ifndef AGS_SHARED_CORE_PLATFORM_H
#define AGS_SHARED_CORE_PLATFORM_H

#include "common/scummsys.h"

namespace AGS3 {

// platform definitions. Not intended for replacing types or checking for libraries.

// ScummVM implementation is identifying as Linux for now
#if 1
#define AGS_PLATFORM_SCUMMVM    (1)
#define AGS_PLATFORM_OS_WINDOWS (0)
#define AGS_PLATFORM_OS_LINUX   (1)
#define AGS_PLATFORM_OS_MACOS   (0)
#define AGS_PLATFORM_OS_ANDROID (0)
#define AGS_PLATFORM_OS_IOS     (0)
#define AGS_PLATFORM_OS_PSP     (0)
#define AGS_PLATFORM_OS_EMSCRIPTEN (0)
// check Android first because sometimes it can get confused with host OS
#elif defined(__ANDROID__) || defined(ANDROID)
#define AGS_PLATFORM_SCUMMVM    (0)
#define AGS_PLATFORM_OS_WINDOWS (0)
#define AGS_PLATFORM_OS_LINUX   (0)
#define AGS_PLATFORM_OS_MACOS   (0)
#define AGS_PLATFORM_OS_ANDROID (1)
#define AGS_PLATFORM_OS_IOS     (0)
#define AGS_PLATFORM_OS_PSP     (0)
#elif defined(_WIN32)
//define something for Windows (32-bit and 64-bit)
#define AGS_PLATFORM_SCUMMVM    (0)
#define AGS_PLATFORM_OS_WINDOWS (1)
#define AGS_PLATFORM_OS_LINUX   (0)
#define AGS_PLATFORM_OS_MACOS   (0)
#define AGS_PLATFORM_OS_ANDROID (0)
#define AGS_PLATFORM_OS_IOS     (0)
#define AGS_PLATFORM_OS_PSP     (0)
#elif defined(__APPLE__)
#define AGS_PLATFORM_SCUMMVM    (0)
#include "ags/shared/ags/shared/TargetConditionals.h"
#ifndef TARGET_OS_SIMULATOR
#define TARGET_OS_SIMULATOR (0)
#endif
#ifndef TARGET_OS_IOS
#define TARGET_OS_IOS (0)
#endif
#ifndef TARGET_OS_OSX
#define TARGET_OS_OSX (0)
#endif

#if TARGET_OS_SIMULATOR || TARGET_IPHONE_SIMULATOR
#define AGS_PLATFORM_OS_WINDOWS (0)
#define AGS_PLATFORM_OS_LINUX   (0)
#define AGS_PLATFORM_OS_MACOS   (0)
#define AGS_PLATFORM_OS_ANDROID (0)
#define AGS_PLATFORM_OS_IOS     (1)
#define AGS_PLATFORM_OS_PSP     (0)
#elif TARGET_OS_IOS || TARGET_OS_IPHONE
#define AGS_PLATFORM_OS_WINDOWS (0)
#define AGS_PLATFORM_OS_LINUX   (0)
#define AGS_PLATFORM_OS_MACOS   (0)
#define AGS_PLATFORM_OS_ANDROID (0)
#define AGS_PLATFORM_OS_IOS     (1)
#define AGS_PLATFORM_OS_PSP     (0)
#elif TARGET_OS_OSX || TARGET_OS_MAC
#define AGS_PLATFORM_OS_WINDOWS (0)
#define AGS_PLATFORM_OS_LINUX   (0)
#define AGS_PLATFORM_OS_MACOS   (1)
#define AGS_PLATFORM_OS_ANDROID (0)
#define AGS_PLATFORM_OS_IOS     (0)
#define AGS_PLATFORM_OS_PSP     (0)
#else
#error "Unknown Apple platform"
#endif
#elif defined(__linux__)
#define AGS_PLATFORM_OS_WINDOWS (0)
#define AGS_PLATFORM_OS_LINUX   (1)
#define AGS_PLATFORM_OS_MACOS   (0)
#define AGS_PLATFORM_OS_ANDROID (0)
#define AGS_PLATFORM_OS_IOS     (0)
#define AGS_PLATFORM_OS_PSP     (0)
#else
#error "Unknown platform"
#endif

#if 0
#define AGS_PLATFORM_WINDOWS_MINGW (1)
#else
#define AGS_PLATFORM_WINDOWS_MINGW (0)
#endif

#if defined(__LP64__)
// LP64 machine, macOS or Linux
// int 32bit | long 64bit | long long 64bit | void* 64bit
#define AGS_PLATFORM_64BIT (1)
#elif defined(_WIN64)
// LLP64 machine, Windows
// int 32bit | long 32bit | long long 64bit | void* 64bit
#define AGS_PLATFORM_64BIT (1)
#else
// 32-bit machine, Windows or Linux or macOS
// int 32bit | long 32bit | long long 64bit | void* 32bit
#define AGS_PLATFORM_64BIT (0)
#endif

#if defined(SCUMM_LITTLE_ENDIAN)
#define AGS_PLATFORM_ENDIAN_LITTLE  (1)
#define AGS_PLATFORM_ENDIAN_BIG     (0)
#elif defined(SCUMM_BIG_ENDIAN)
#define AGS_PLATFORM_ENDIAN_LITTLE  (0)
#define AGS_PLATFORM_ENDIAN_BIG     (1)
#else
#error "No endianness defined"
#endif

#if defined(SCUMM_NEED_ALIGNMENT)
#define AGS_STRICT_ALIGNMENT
#endif

#define AGS_PLATFORM_DESKTOP ((AGS_PLATFORM_OS_WINDOWS) || (AGS_PLATFORM_OS_LINUX) || (AGS_PLATFORM_OS_MACOS))

#define AGS_PLATFORM_MOBILE ((AGS_PLATFORM_OS_ANDROID) || (AGS_PLATFORM_OS_IOS))

#define AGS_HAS_DIRECT3D (AGS_PLATFORM_OS_WINDOWS)
#define AGS_HAS_OPENGL ((AGS_PLATFORM_OS_WINDOWS) || (AGS_PLATFORM_OS_LINUX) || (AGS_PLATFORM_MOBILE))
#define AGS_OPENGL_ES2 (AGS_PLATFORM_OS_ANDROID)

// Only allow searching around for game data on desktop systems;
// otherwise use explicit argument either from program wrapper, command-line
// or read from default config.
#define AGS_SEARCH_FOR_GAME_ON_LAUNCH ((AGS_PLATFORM_OS_WINDOWS) || (AGS_PLATFORM_OS_LINUX) || (AGS_PLATFORM_OS_MACOS))

#if !defined(DEBUG_MANAGED_OBJECTS)
	#define DEBUG_MANAGED_OBJECTS (0)
#endif

#if !defined(DEBUG_SPRITECACHE)
	#define DEBUG_SPRITECACHE (0)
#endif

} // namespace AGS3

#endif
