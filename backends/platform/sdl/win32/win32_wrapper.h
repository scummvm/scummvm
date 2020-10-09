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

#ifndef PLATFORM_SDL_WIN32_WRAPPER_H
#define PLATFORM_SDL_WIN32_WRAPPER_H

#include "common/scummsys.h"

HRESULT SHGetFolderPathFunc(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);

// Helper functions
namespace Win32 {

/**
 * Checks if the current running Windows version is greater or equal to the specified version.
 * See: https://docs.microsoft.com/en-us/windows/desktop/sysinfo/operating-system-version
 *
 * @param majorVersion The major version number (x.0)
 * @param minorVersion The minor version number (0.x)
 */
bool confirmWindowsVersion(int majorVersion, int minorVersion);
/**
 * Converts a C string into a Windows wide-character string.
 * Used to interact with Win32 Unicode APIs with no ANSI fallback.
 *
 * @param s Source string
 * @param c Code Page, by default is CP_ACP (default Windows ANSI code page)
 * @return Converted string
 *
 * @note Return value must be freed by the caller.
 */
wchar_t *ansiToUnicode(const char *s, uint codePage = CP_ACP);
/**
 * Converts a Windows wide-character string into a C string.
 * Used to interact with Win32 Unicode APIs with no ANSI fallback.
 *
 * @param s Source string
 * @param c Code Page, by default is CP_ACP (default Windows ANSI code page)
 * @return Converted string
 *
 * @note Return value must be freed by the caller.
 */
char *unicodeToAnsi(const wchar_t *s, uint codePage = CP_ACP);

}

#endif
