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

#ifndef PLATFORM_SDL_WIN32_WRAPPER_H
#define PLATFORM_SDL_WIN32_WRAPPER_H

#include "common/scummsys.h"
#include "common/str.h"

HRESULT SHGetFolderPathFunc(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

// Helper functions
namespace Win32 {

/**
 * Gets the full path to the ScummVM application data directory
 * in the user's profile and creates it if it doesn't exist.
 *
 * @param profileDirectory MAX_PATH sized output array
 *
 * @return True if the user's profile directory was found, false if
 * it was not.
 *
 * @note if the user's profile directory is found but the "ScummVM"
 * subdirectory can't be created then this function calls error().
 */
bool getApplicationDataDirectory(TCHAR *profileDirectory);

/**
 * Gets the full path to the directory that the currently executing
 * process resides in.
 *
 * @param processDirectory output array
 * @param size size in characters of output array
 */
void getProcessDirectory(TCHAR *processDirectory, DWORD size);

/**
 * Checks if the current running Windows version is greater or equal to the specified version.
 * See: https://docs.microsoft.com/en-us/windows/win32/sysinfo/operating-system-version
 *
 * @param majorVersion The major version number (x.0)
 * @param minorVersion The minor version number (0.x)
 */
bool confirmWindowsVersion(int majorVersion, int minorVersion);

/**
 * Moves a file within the same volume. Replaces any existing file.
 */
bool moveFile(const Common::String &src, const Common::String &dst);

/**
 * Returns true if the drive letter is a CDROM
 *
 * @param driveLetter The drive letter to test
 */
bool isDriveCD(char driveLetter);

/**
 * Converts a C string into a Windows wide-character string.
 * Used to interact with Win32 Unicode APIs with no ANSI fallback.
 * If UNICODE is defined then the conversion will use code page CP_UTF8,
 * otherwise CP_ACP will be used.
 *
 * @param s Source string
 * @return Converted string
 *
 * @note Return value must be freed by the caller.
 */
wchar_t *ansiToUnicode(const char *s);
/**
 * Converts a Windows wide-character string into a C string.
 * Used to interact with Win32 Unicode APIs with no ANSI fallback.
 * If UNICODE is defined then the conversion will use code page CP_UTF8,
 * otherwise CP_ACP will be used.
 *
 * @param s Source string
 * @return Converted string
 *
 * @note Return value must be freed by the caller.
 */
char *unicodeToAnsi(const wchar_t *s);

/**
 * Converts a Common::String to a TCHAR array for the purpose of passing to
 * a Windows API or CRT call. If UNICODE is defined then the string will be
 * converted from UTF8 to wide characters, otherwise the character array
 * will be copied with no conversion.
 *
 * @param s Source string
 * @return Converted string
 *
 * @note Return value must be freed by the caller.
 */
TCHAR *stringToTchar(const Common::String& s);

/**
 * Converts a TCHAR array returned from a Windows API or CRT call to a Common::String.
 * If UNICODE is defined then the wide character array will be converted to UTF8,
 * otherwise the char array will be copied with no conversion.
 *
 * @param s Source string
 * @return Converted string
 */
Common::String tcharToString(const TCHAR *s);

#ifdef UNICODE
/**
 * Returns command line arguments in argc / argv format in UTF8.
 *
 * @param argc argument count
 * @return argument array
 *
 * @note Return value must be freed by the caller with freeArgvUtf8()
 */
char **getArgvUtf8(int *argc);

/**
 * Frees an argument array created by getArgvUtf8()
 *
 * @param argc argument count in argv
 * @param argv argument array created by getArgvUtf8()
 */
void freeArgvUtf8(int argc, char **argv);
#endif

} // End of namespace Win32

#endif
