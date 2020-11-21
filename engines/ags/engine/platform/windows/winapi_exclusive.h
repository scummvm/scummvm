//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// An excerpt from <windows.h>, declaring most commonly used WinAPI types
// and attributes. Meant to avoid including <windows.h> itself when it may
// cause naming conflicts with its dreaded heap of macros.
//
//=============================================================================
#ifndef __AGS_EE_PLATFORM__WINAPI_EXCLUSIVE_H
#define __AGS_EE_PLATFORM__WINAPI_EXCLUSIVE_H

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

#endif // _WINDOWS_

#endif // __AGS_EE_PLATFORM__WINAPI_EXCLUSIVE_H
