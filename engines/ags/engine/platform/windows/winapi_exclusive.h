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

//=============================================================================
//
// An excerpt from <windows.h>, declaring most commonly used WinAPI types
// and attributes. Meant to avoid including <windows.h> itself when it may
// cause naming conflicts with its dreaded heap of macros.
//
//=============================================================================

#ifndef AGS_ENGINE_PLATFORM_WINDOWS_WINAPI_EXCLUSIVE_H
#define AGS_ENGINE_PLATFORM_WINDOWS_WINAPI_EXCLUSIVE_H

#ifndef _WINDOWS_ // do not include if windows.h was included first
#define _WINDOWS_ // there can be only one

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef char                CHAR;
#define CONST               const
typedef CONST CHAR *LPCSTR, *PCSTR;

#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINBASEAPI DECLSPEC_IMPORT
#define WINAPI __stdcall

typedef void *PVOID;
typedef PVOID HANDLE;
typedef HANDLE HINSTANCE;
typedef HANDLE HMODULE;
typedef HANDLE HWND;

#define FAR
#define NEAR

typedef int (FAR WINAPI *FARPROC)();

#endif

#endif
