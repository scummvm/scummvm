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

#ifndef PX_WINDOWS_H_INCLUDED
#define PX_WINDOWS_H_INCLUDED

#ifndef STRICT
#define STRICT // Force strict useage of handle definitions and other things
#endif         // STRICT

#ifdef _MSC_VER
#pragma warning(disable : 4201) // C4201: nonstandard extension used : nameless struct/union
#endif

#ifdef _WIN32
#include <windows.h>
#else

namespace ICB {

// define some windows.h compatibility types

#define MAX_PATH (260)
#define _MAX_PATH MAX_PATH

typedef signed int LONG;
typedef int BOOL;
typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE HWND;

typedef struct _RECT {
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} RECT, *PRECT;

} // End of namespace ICB

#endif

#include "px_rccommon.h" // To restore our level for warning 4237 because windows.h

#endif // PX_WINDOWS_H_INCLUDED
