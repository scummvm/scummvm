
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

#ifndef BAGEL_BOFLIB_BOFFO_H
#define BAGEL_BOFLIB_BOFFO_H

namespace Bagel {

#define BOF_WINNT 1

/*
* One and only one of these BOF_platform macros must be defined to 1
* in the compiler pre-processor
*/

#ifndef BOF_DOS16
#define BOF_DOS16   0       /* 1 for 16bit DOS application */
#endif

#ifndef BOF_DOS32
#define BOF_DOS32   0       /* 1 for 32bit DOS application */
#endif

#ifndef BOF_WIN16
#define BOF_WIN16   0       /* 1 for 16bit Windows application */
#endif

#ifndef BOF_WIN32S
#define BOF_WIN32S  0       /* 1 for Win32s App */
#endif

#ifndef BOF_WINNT
#define BOF_WINNT   0       /* 1 for Win32 (Win95/NT) App */
#endif

#ifndef BOF_CONSOLE
#define BOF_CONSOLE 0       /* 1 for Win32 Console App (Win95/NT)*/
#endif

#ifndef BOF_MAC
#define BOF_MAC     0       /* 1 for (non MFC) Macintosh application */
#endif

#define BOF_WINDOWS (BOF_WIN16 || BOF_WIN32S || BOF_WINNT)
#define BOF_DOS     (BOF_DOS16 || BOF_DOS32 || BOF_CONSOLE)

/*
* BOF_WINMAC is defined when using the MAC Cross compiler
*/
#ifdef _MAC
#define BOF_WINMAC  BOF_WINNT
#else
#define BOF_WINMAC  0
#endif

/*
* If this is a Mac product (no matter what compiler we use)
*/
#define BOF_ANYMAC (BOF_WINMAC || BOF_MAC)

#if (!BOF_WINDOWS && !BOF_DOS && !BOF_MAC)
#error One of the BOF_platform macros must be defined in the pre-processor
#endif

#if (BOF_DOS && BOF_WINDOWS)
#error Only one of the BOF_platform Macros can be defined
#endif

#if (BOF_MAC && BOF_WINDOWS)
#error Only one of the BOF_platform Macros can be defined
#endif

#if (BOF_MAC && BOF_DOS)
#error Only one of the BOF_platform Macros can be defined
#endif

#if (BOF_DOS32 || BOF_MAC || BOF_WIN32S || BOF_WINNT || BOF_CONSOLE)
#define INTSIZE 4
#else
#define INTSIZE 2
#endif


/*
* Inline assembly on?
*/
#if BOF_MAC || BOF_WINMAC
#define BOF_USE_ASM 0
#else
#ifdef NO_ASM
#define BOF_USE_ASM 0
#else
#define BOF_USE_ASM 1
#endif
#endif

/*
* Accept Microsoft's new debug macro
*/
#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG 1
#endif
#endif

#ifdef DEBUG
#ifndef _DEBUG
#define _DEBUG 1
#endif
#endif

} // namespace Bagel

#endif
