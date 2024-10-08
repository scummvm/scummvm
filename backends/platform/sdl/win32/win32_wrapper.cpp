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

// For _tcscat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat

#define WIN32_LEAN_AND_MEAN
#include <sdkddkver.h>
#if !defined(_WIN32_IE) || (_WIN32_IE < 0x500)
 // required for SHGetSpecialFolderPath and SHGFP_TYPE_CURRENT in shlobj.h
#undef _WIN32_IE
#define _WIN32_IE 0x500
#endif
#if !defined(_WIN32_WINNT) || (_WIN32_WINNT < 0x500)
 // required for VER_MAJORVERSION, VER_MINORVERSION and VER_GREATER_EQUAL in winnt.h
 // set to Windows 2000 which is the minimum needed for these constants
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x500
#endif
#include <windows.h>
#include <shellapi.h> // for CommandLineToArgvW()
#include <shlobj.h>
#include <tchar.h>

#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/textconsole.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

// VerSetConditionMask, VerifyVersionInfo and SHGetFolderPath didn't appear until Windows 2000,
// so we need to check for them at runtime
ULONGLONG VerSetConditionMaskFunc(ULONGLONG dwlConditionMask, DWORD dwTypeMask, BYTE dwConditionMask) {
	typedef ULONGLONG(WINAPI *VerSetConditionMaskFunction)(ULONGLONG conditionMask, DWORD typeMask, BYTE conditionOperator);

	VerSetConditionMaskFunction verSetConditionMask = (VerSetConditionMaskFunction)(void *)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "VerSetConditionMask");
	if (verSetConditionMask == nullptr)
		return 0;

	return verSetConditionMask(dwlConditionMask, dwTypeMask, dwConditionMask);
}

BOOL VerifyVersionInfoFunc(LPOSVERSIONINFOEXA lpVersionInformation, DWORD dwTypeMask, DWORDLONG dwlConditionMask) {
	typedef BOOL(WINAPI *VerifyVersionInfoFunction)(LPOSVERSIONINFOEXA versionInformation, DWORD typeMask, DWORDLONG conditionMask);

	VerifyVersionInfoFunction verifyVersionInfo = (VerifyVersionInfoFunction)(void *)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "VerifyVersionInfoA");
	if (verifyVersionInfo == nullptr)
		return FALSE;

	return verifyVersionInfo(lpVersionInformation, dwTypeMask, dwlConditionMask);
}

HRESULT SHGetFolderPathFunc(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath) {
	typedef HRESULT (WINAPI *SHGetFolderPathFunc)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

	SHGetFolderPathFunc pSHGetFolderPath = (SHGetFolderPathFunc)(void *)GetProcAddress(GetModuleHandle(TEXT("shell32.dll")), 
#ifndef UNICODE
		"SHGetFolderPathA"
#else
		"SHGetFolderPathW"
#endif
	);
	if (pSHGetFolderPath)
		return pSHGetFolderPath(hwnd, csidl, hToken, dwFlags, pszPath);

	typedef BOOL (WINAPI *SHGetSpecialFolderPathFunc)(HWND hwnd, LPTSTR pszPath, int csidl, BOOL fCreate);

	SHGetSpecialFolderPathFunc pSHGetSpecialFolderPath = (SHGetSpecialFolderPathFunc)(void *)GetProcAddress(GetModuleHandle(TEXT("shell32.dll")),
#ifndef UNICODE
		"SHGetSpecialFolderPathA"
#else
		"SHGetSpecialFolderPathW"
#endif
	);
	if (pSHGetSpecialFolderPath)
		return pSHGetSpecialFolderPath(hwnd, pszPath, csidl & ~CSIDL_FLAG_MASK, csidl & CSIDL_FLAG_CREATE) ? S_OK : E_FAIL;

	return E_NOTIMPL;
}

namespace Win32 {

bool getApplicationDataDirectory(TCHAR *applicationDataDirectory) {
	HRESULT hr = SHGetFolderPathFunc(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, applicationDataDirectory);
	if (hr != S_OK) {
		if (hr != E_NOTIMPL) {
			warning("Unable to locate application data directory");
		}
		return false;
	}

	_tcscat(applicationDataDirectory, TEXT("\\ScummVM"));
	if (!CreateDirectory(applicationDataDirectory, NULL)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			error("Cannot create ScummVM application data folder");
		}
	}

	return true;
}

void getProcessDirectory(TCHAR *processDirectory, DWORD size) {
	GetModuleFileName(NULL, processDirectory, size);
	processDirectory[size - 1] = '\0'; // termination not guaranteed

	// remove executable and final path separator
	TCHAR *lastSeparator = _tcsrchr(processDirectory, '\\');
	if (lastSeparator != NULL) {
		*lastSeparator = '\0';
	}
}

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

// for using ScopedPtr with malloc/free
template <typename T>
struct Freer {
	inline void operator()(T *object) {
		free(object);
	}
};

bool moveFile(const Common::String &src, const Common::String &dst) {
	Common::ScopedPtr<TCHAR, Freer<TCHAR>> tSrc(stringToTchar(src));
	Common::ScopedPtr<TCHAR, Freer<TCHAR>> tDst(stringToTchar(dst));

	if (MoveFileEx(tSrc.get(), tDst.get(), MOVEFILE_REPLACE_EXISTING)) {
		return true;
	}

	// MoveFileEx may not be supported on the platform (Win9x)
	if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) {
		// Fall back to deleting the destination before using MoveFile.
		// MoveFile requires that the destination not already exist.
		DeleteFile(tDst.get());
		return MoveFile(tSrc.get(), tDst.get());
	}

	return false;
}

bool isDriveCD(char driveLetter) {
	TCHAR drivePath[] = TEXT("x:\\");
	drivePath[0] = (TCHAR)driveLetter;

	return (GetDriveType(drivePath) == DRIVE_CDROM);
}

wchar_t *ansiToUnicode(const char *s) {
#ifndef UNICODE
	uint codePage = CP_ACP;
#else
	uint codePage = CP_UTF8;
#endif
	DWORD size = MultiByteToWideChar(codePage, 0, s, -1, nullptr, 0);

	if (size > 0) {
		LPWSTR result = (LPWSTR)calloc(size, sizeof(WCHAR));
		if (MultiByteToWideChar(codePage, 0, s, -1, result, size) != 0)
			return result;
	}

	return nullptr;
}

char *unicodeToAnsi(const wchar_t *s) {
#ifndef UNICODE
	uint codePage = CP_ACP;
#else
	uint codePage = CP_UTF8;
#endif
	DWORD size = WideCharToMultiByte(codePage, 0, s, -1, nullptr, 0, nullptr, nullptr);

	if (size > 0) {
		char *result = (char *)calloc(size, sizeof(char));
		if (WideCharToMultiByte(codePage, 0, s, -1, result, size, nullptr, nullptr) != 0)
			return result;
	}

	return nullptr;
}

TCHAR *stringToTchar(const Common::String& s) {
#ifndef UNICODE
	char *t = (char *)malloc(s.size() + 1);
	Common::strcpy_s(t, s.size() + 1, s.c_str());
	return t;
#else
	return ansiToUnicode(s.c_str());
#endif
}

Common::String tcharToString(const TCHAR *t) {
#ifndef UNICODE
	return t;
#else
	char *utf8 = unicodeToAnsi(t);
	Common::String s = utf8;
	free(utf8);
	return s;
#endif
}

#ifdef UNICODE
char **getArgvUtf8(int *argc) {
	// get command line arguments in wide-character
	LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), argc);

	// convert each argument to utf8
	char **argv = (char **)malloc((*argc + 1) * sizeof(char *));
	for (int i = 0; i < *argc; ++i) {
		argv[i] = Win32::unicodeToAnsi(wargv[i]);
	}
	argv[*argc] = nullptr; // null terminated array

	LocalFree(wargv);
	return argv;
}

void freeArgvUtf8(int argc, char **argv) {
	for (int i = 0; i < argc; ++i) {
		free(argv[i]);
	}
	free(argv);
}
#endif

} // End of namespace Win32
