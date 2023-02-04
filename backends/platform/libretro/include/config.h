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

#ifndef CONFIG_H
#define CONFIG_H

#if defined(WIIU) || defined(__CELLOS_LV2__) || defined(GEKKO)
#undef  SCUMM_LITTLE_ENDIAN
#define SCUMM_BIG_ENDIAN
#else
#define SCUMM_LITTLE_ENDIAN
#endif

#define SCUMM_NEED_ALIGNMENT

/* Data types */
#ifndef SCUMMVM_DONT_DEFINE_TYPES
typedef unsigned char byte;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
typedef signed short int16;
typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned int uint;
typedef signed long long int64;
typedef unsigned long long uint64;
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__ppc64__) || defined(__powerpc64__) || defined(__LP64__) || defined(_M_ARM64)

typedef uint64 uintptr;
typedef int64 intptr;

#else

typedef uint32 uintptr;
typedef int32 intptr;

#endif

#endif /* CONFIG_H */
