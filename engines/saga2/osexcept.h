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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_OSEXCEPT_H
#define SAGA2_OSEXCEPT_H

namespace Saga2 {

// OS specific exception handling block defs

#ifndef _WIN32

void initFaultHandler(void);
void DosExceptHandler(void);
#if 1
extern int gameKiller;
#define OSExceptBlk
#define OSExcepTrap if (gameKiller)
#define OSExceptHnd DosExceptHandler();
#else
#define OSExceptBlk try
#define OSExcepTrap catch (__WATCOM_exception)
#define OSExceptHnd DosExceptHandler();
#endif

#else   // _WIN32

DWORD WinExceptFilter(DWORD ExCode, LPEXCEPTION_POINTERS ExPtr);
void WinExceptHandler(void);
#define OSExceptBlk __try
#define OSExcepTrap __except (                             \
        WinExceptFilter(                 \
                GetExceptionCode(),          \
                GetExceptionInformation()) )
#define OSExceptHnd     WinExceptHandler()

#endif  // _WIN32

} // end of namespace Saga2

#endif
