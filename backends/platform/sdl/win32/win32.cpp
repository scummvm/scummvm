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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/scummsys.h"
#include "common/error.h"
#include "common/textconsole.h"

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...

#include "backends/platform/sdl/win32/win32.h"
#include "backends/fs/windows/windows-fs-factory.h"

#define DEFAULT_CONFIG_FILE "residual.ini"

//#define	HIDE_CONSOLE

#ifdef HIDE_CONSOLE
struct SdlConsoleHidingWin32 {
	DWORD myPid;
	DWORD myTid;
	HWND consoleHandle;
};

// console hiding for win32
static BOOL CALLBACK initBackendFindConsoleWin32Proc(HWND hWnd, LPARAM lParam) {
	DWORD pid, tid;
	SdlConsoleHidingWin32 *variables = (SdlConsoleHidingWin32 *)lParam;
	tid = GetWindowThreadProcessId(hWnd, &pid);
	if ((tid == variables->myTid) && (pid == variables->myPid)) {
		variables->consoleHandle = hWnd;
		return FALSE;
	}
	return TRUE;
}

#endif

void OSystem_Win32::init() {
#ifdef HIDE_CONSOLE
	// console hiding for win32
	SdlConsoleHidingWin32 consoleHidingWin32;
	consoleHidingWin32.consoleHandle = 0;
	consoleHidingWin32.myPid = GetCurrentProcessId();
	consoleHidingWin32.myTid = GetCurrentThreadId();
	EnumWindows (initBackendFindConsoleWin32Proc, (LPARAM)&consoleHidingWin32);

	if (!ConfMan.getBool("show_console")) {
		if (consoleHidingWin32.consoleHandle) {
			// We won't find a window with our TID/PID in case we were started from command-line
			ShowWindow(consoleHidingWin32.consoleHandle, SW_HIDE);
		}
	}
#endif

	// Initialze File System Factory
	_fsFactory = new WindowsFilesystemFactory();

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

Common::String OSystem_Win32::getDefaultConfigFileName() {
	char configFile[MAXPATHLEN];

	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", configFile, sizeof(configFile)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", configFile, sizeof(configFile)))
				error("Unable to access user profile directory");

			strcat(configFile, "\\Application Data");
			CreateDirectory(configFile, NULL);
		}

		strcat(configFile, "\\Residual");
		CreateDirectory(configFile, NULL);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

		FILE *tmp = NULL;
		if ((tmp = fopen(configFile, "r")) == NULL) {
			// Check windows directory
			char oldConfigFile[MAXPATHLEN];
			GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
			strcat(oldConfigFile, "\\" DEFAULT_CONFIG_FILE);
			if ((tmp = fopen(oldConfigFile, "r"))) {
				strcpy(configFile, oldConfigFile);

				fclose(tmp);
			}
		} else {
			fclose(tmp);
		}
	} else {
		// Check windows directory
		GetWindowsDirectory(configFile, MAXPATHLEN);
		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	}

	return configFile;
}

Common::WriteStream *OSystem_Win32::createLogFile() {
	char logFile[MAXPATHLEN];

	OSVERSIONINFO win32OsVersion;
	ZeroMemory(&win32OsVersion, sizeof(OSVERSIONINFO));
	win32OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&win32OsVersion);
	// Check for non-9X version of Windows.
	if (win32OsVersion.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) {
		// Use the Application Data directory of the user profile.
		if (win32OsVersion.dwMajorVersion >= 5) {
			if (!GetEnvironmentVariable("APPDATA", logFile, sizeof(logFile)))
				error("Unable to access application data directory");
		} else {
			if (!GetEnvironmentVariable("USERPROFILE", logFile, sizeof(logFile)))
				error("Unable to access user profile directory");

			strcat(logFile, "\\Application Data");
			CreateDirectory(logFile, NULL);
		}

		strcat(logFile, "\\Residual");
		CreateDirectory(logFile, NULL);
		strcat(logFile, "\\Logs");
		CreateDirectory(logFile, NULL);
		strcat(logFile, "\\residual.log");

		Common::FSNode file(logFile);
		return file.createWriteStream();
	} else {
		return 0;
	}
}

#endif
