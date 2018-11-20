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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
// We need certain functions that are excluded by default
#undef NONLS
#include <windows.h>
#if defined(ARRAYSIZE)
#undef ARRAYSIZE
#endif

#include "backends/platform/sdl/win32/win32_wrapper.h"

// VerSetConditionMask and VerifyVersionInfo didn't appear until Windows 2000,
// so we need to check for them at runtime
LONGLONG VerSetConditionMaskFunc(ULONGLONG dwlConditionMask, DWORD dwTypeMask, BYTE dwConditionMask) {
	typedef BOOL(WINAPI *VerSetConditionMaskFunction)(ULONGLONG conditionMask, DWORD typeMask, BYTE conditionOperator);

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

namespace Win32 {

bool confirmWindowsVersion(int majorVersion, int minorVersion) {
	OSVERSIONINFOEX versionInfo;
	DWORDLONG conditionMask = 0;

	ZeroMemory(&versionInfo, sizeof(OSVERSIONINFOEX));
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	versionInfo.dwMajorVersion = majorVersion;
	versionInfo.dwMinorVersion = minorVersion;

	conditionMask = VerSetConditionMaskFunc(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	conditionMask = VerSetConditionMaskFunc(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

	return VerifyVersionInfoFunc(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask);
}

LPWSTR ansiToUnicode(const char *s) {
	DWORD size = MultiByteToWideChar(0, 0, s, -1, NULL, 0);

	if (size > 0) {
		LPWSTR result = new WCHAR[size];
		if (MultiByteToWideChar(0, 0, s, -1, result, size) != 0)
			return result;
	}

	return NULL;
}

}
