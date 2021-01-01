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

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
 // required for SHGetSpecialFolderPath in shlobj.h
#define _WIN32_IE 0x400
#endif
#include <shlobj.h>

#include "common/scummsys.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

// VerSetConditionMask, VerifyVersionInfo and SHGetFolderPath didn't appear until Windows 2000,
// so we need to check for them at runtime
ULONGLONG VerSetConditionMaskFunc(ULONGLONG dwlConditionMask, DWORD dwTypeMask, BYTE dwConditionMask) {
	typedef ULONGLONG(WINAPI *VerSetConditionMaskFunction)(ULONGLONG conditionMask, DWORD typeMask, BYTE conditionOperator);

	VerSetConditionMaskFunction verSetConditionMask = (VerSetConditionMaskFunction)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "VerSetConditionMask");
	if (verSetConditionMask == NULL)
		return 0;

	return verSetConditionMask(dwlConditionMask, dwTypeMask, dwConditionMask);
}

BOOL VerifyVersionInfoFunc(LPOSVERSIONINFOEXA lpVersionInformation, DWORD dwTypeMask, DWORDLONG dwlConditionMask) {
	typedef BOOL(WINAPI *VerifyVersionInfoFunction)(LPOSVERSIONINFOEXA versionInformation, DWORD typeMask, DWORDLONG conditionMask);

	VerifyVersionInfoFunction verifyVersionInfo = (VerifyVersionInfoFunction)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "VerifyVersionInfoA");
	if (verifyVersionInfo == NULL)
		return FALSE;

	return verifyVersionInfo(lpVersionInformation, dwTypeMask, dwlConditionMask);
}

HRESULT SHGetFolderPathFunc(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath) {
	typedef HRESULT (WINAPI *SHGetFolderPathFunc)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);

	SHGetFolderPathFunc pSHGetFolderPath = (SHGetFolderPathFunc)GetProcAddress(GetModuleHandle(TEXT("shell32.dll")), "SHGetFolderPathA");
	if (pSHGetFolderPath)
		return pSHGetFolderPath(hwnd, csidl, hToken, dwFlags, pszPath);

	return SHGetSpecialFolderPath(hwnd, pszPath, csidl & !CSIDL_FLAG_MASK, csidl & CSIDL_FLAG_CREATE) ? S_OK : E_NOTIMPL;
}

namespace Win32 {

bool confirmWindowsVersion(int majorVersion, int minorVersion) {
	OSVERSIONINFOEXA versionInfo;
	DWORDLONG conditionMask = 0;

	ZeroMemory(&versionInfo, sizeof(OSVERSIONINFOEXA));
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
	versionInfo.dwMajorVersion = majorVersion;
	versionInfo.dwMinorVersion = minorVersion;

	conditionMask = VerSetConditionMaskFunc(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	conditionMask = VerSetConditionMaskFunc(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfoFunc(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask);
}

wchar_t *ansiToUnicode(const char *s, uint codePage) {
	DWORD size = MultiByteToWideChar(codePage, 0, s, -1, NULL, 0);

	if (size > 0) {
		LPWSTR result = (LPWSTR)calloc(size, sizeof(WCHAR));
		if (MultiByteToWideChar(codePage, 0, s, -1, result, size) != 0)
			return result;
	}

	return NULL;
}

char *unicodeToAnsi(const wchar_t *s, uint codePage) {
	DWORD size = WideCharToMultiByte(codePage, 0, s, -1, NULL, 0, 0, 0);

	if (size > 0) {
		char *result = (char *)calloc(size, sizeof(char));
		if (WideCharToMultiByte(codePage, 0, s, -1, result, size, 0, 0) != 0)
			return result;
	}

	return NULL;
}

}
