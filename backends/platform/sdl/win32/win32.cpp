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

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...
#include <shellapi.h>
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
// required for SHGFP_TYPE_CURRENT in shlobj.h
#define _WIN32_IE 0x500
#endif
#include <shlobj.h>

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"

#include "backends/audiocd/win32/win32-audiocd.h"
#include "backends/platform/sdl/win32/win32.h"
#include "backends/platform/sdl/win32/win32-window.h"
#include "backends/saves/windows/windows-saves.h"
#include "backends/fs/windows/windows-fs-factory.h"
#include "backends/taskbar/win32/win32-taskbar.h"
#include "backends/updates/win32/win32-updates.h"

#include "common/memstream.h"

#define DEFAULT_CONFIG_FILE "scummvm.ini"

void OSystem_Win32::init() {
	// Initialize File System Factory
	_fsFactory = new WindowsFilesystemFactory();

	// Create Win32 specific window
	_window = new SdlWindow_Win32();

#if defined(USE_TASKBAR)
	// Initialize taskbar manager
	_taskbarManager = new Win32TaskbarManager(_window);
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

void OSystem_Win32::initBackend() {
	// Console window is enabled by default on Windows
	ConfMan.registerDefault("console", true);

	// Enable or disable the window console window
	if (ConfMan.getBool("console")) {
		if (AllocConsole()) {
			freopen("CONIN$","r",stdin);
			freopen("CONOUT$","w",stdout);
			freopen("CONOUT$","w",stderr);
		}
		SetConsoleTitle("ScummVM Status Window");
	} else {
		FreeConsole();
	}

	// Create the savefile manager
	if (_savefileManager == 0)
		_savefileManager = new WindowsSaveFileManager();

#if defined(USE_SPARKLE)
	// Initialize updates manager
	_updateManager = new Win32UpdateManager();
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}


bool OSystem_Win32::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile || f == kFeatureOpenUrl)
		return true;

	return OSystem_SDL::hasFeature(f);
}

bool OSystem_Win32::displayLogFile() {
	if (_logFilePath.empty())
		return false;

	// Try opening the log file with the default text editor
	// log files should be registered as "txtfile" by default and thus open in the default text editor
	HINSTANCE shellExec = ShellExecute(NULL, NULL, _logFilePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	if ((intptr_t)shellExec > 32)
		return true;

	// ShellExecute with the default verb failed, try the "Open with..." dialog
	PROCESS_INFORMATION processInformation;
	STARTUPINFO startupInfo;
	memset(&processInformation, 0, sizeof(processInformation));
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	char cmdLine[MAX_PATH * 2];  // CreateProcess may change the contents of cmdLine
	sprintf(cmdLine, "rundll32 shell32.dll,OpenAs_RunDLL %s", _logFilePath.c_str());
	BOOL result = CreateProcess(NULL,
	                            cmdLine,
	                            NULL,
	                            NULL,
	                            FALSE,
	                            NORMAL_PRIORITY_CLASS,
	                            NULL,
	                            NULL,
	                            &startupInfo,
	                            &processInformation);
	if (result)
		return true;

	return false;
}

bool OSystem_Win32::openUrl(const Common::String &url) {
	const uint64 result = (uint64)ShellExecute(0, 0, /*(wchar_t*)nativeFilePath.utf16()*/url.c_str(), 0, 0, SW_SHOWNORMAL);
	// ShellExecute returns a value greater than 32 if successful
	if (result <= 32) {
		warning("ShellExecute failed: error = %u", result);
		return false;
	}
	return true;
}

void OSystem_Win32::logMessage(LogMessageType::Type type, const char *message) {
	OSystem_SDL::logMessage(type, message);

#if defined( USE_WINDBG )
	OutputDebugString(message);
#endif
}

Common::String OSystem_Win32::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && defined(USE_TRANSLATION)
	// We can not use "setlocale" (at least not for MSVC builds), since it
	// will return locales like: "English_USA.1252", thus we need a special
	// way to determine the locale string for Win32.
	char langName[9];
	char ctryName[9];

	if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, langName, sizeof(langName)) != 0 &&
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, ctryName, sizeof(ctryName)) != 0) {
		Common::String localeName = langName;
		localeName += "_";
		localeName += ctryName;

		return localeName;
	}
#endif // USE_DETECTLANG
	// Falback to SDL implementation
	return OSystem_SDL::getSystemLanguage();
}

Common::String OSystem_Win32::getScreenshotsPath() {
	Common::String screenshotsPath = ConfMan.get("screenshotpath");
	if (!screenshotsPath.empty()) {
		if (!screenshotsPath.hasSuffix("\\") && !screenshotsPath.hasSuffix("/"))
			screenshotsPath += "\\";
		return screenshotsPath;
	}

	// Use the My Pictures folder.
	char picturesPath[MAXPATHLEN];

	// SHGetFolderPath didn't appear until Windows 2000, so we need to check for it at runtime
	typedef HRESULT (WINAPI *SHGetFolderPathFunc)(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
	SHGetFolderPathFunc pSHGetFolderPath = (SHGetFolderPathFunc)GetProcAddress(GetModuleHandle(TEXT("shell32.dll")), "SHGetFolderPathA");

	if (pSHGetFolderPath) {
		if (pSHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, picturesPath) != S_OK) {
			warning("Unable to access My Pictures directory");
			return Common::String();
		}
	} else {
		if (!SHGetSpecialFolderPath(NULL, picturesPath, CSIDL_MYPICTURES, FALSE)) {
			warning("Unable to access My Pictures directory");
			return Common::String();
		}
	}

	screenshotsPath = Common::String(picturesPath) + "\\ScummVM Screenshots\\";

	// If the directory already exists (as it should in most cases),
	// we don't want to fail, but we need to stop on other errors (such as ERROR_PATH_NOT_FOUND)
	if (!CreateDirectory(screenshotsPath.c_str(), NULL)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			error("Cannot create ScummVM Screenshots folder");
	}

	return screenshotsPath;
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

			// If the directory already exists (as it should in most cases),
			// we don't want to fail, but we need to stop on other errors (such as ERROR_PATH_NOT_FOUND)
			if (!CreateDirectory(configFile, NULL)) {
				if (GetLastError() != ERROR_ALREADY_EXISTS)
					error("Cannot create Application data folder");
			}
		}

		strcat(configFile, "\\ScummVM");
		if (!CreateDirectory(configFile, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				error("Cannot create ScummVM application data folder");
		}

		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);

		FILE *tmp = NULL;
		if ((tmp = fopen(configFile, "r")) == NULL) {
			// Check windows directory
			char oldConfigFile[MAXPATHLEN];
			uint ret = GetWindowsDirectory(oldConfigFile, MAXPATHLEN);
			if (ret == 0 || ret > MAXPATHLEN)
				error("Cannot retrieve the path of the Windows directory");

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
		uint ret = GetWindowsDirectory(configFile, MAXPATHLEN);
		if (ret == 0 || ret > MAXPATHLEN)
			error("Cannot retrieve the path of the Windows directory");

		strcat(configFile, "\\" DEFAULT_CONFIG_FILE);
	}

	return configFile;
}

Common::WriteStream *OSystem_Win32::createLogFile() {
	// Start out by resetting _logFilePath, so that in case
	// of a failure, we know that no log file is open.
	_logFilePath.clear();

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

		strcat(logFile, "\\ScummVM");
		CreateDirectory(logFile, NULL);
		strcat(logFile, "\\Logs");
		CreateDirectory(logFile, NULL);
		strcat(logFile, "\\scummvm.log");

		Common::FSNode file(logFile);
		Common::WriteStream *stream = file.createWriteStream();
		if (stream)
			_logFilePath= logFile;

		return stream;
	} else {
		return 0;
	}
}

namespace {

class Win32ResourceArchive : public Common::Archive {
	friend BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);
public:
	Win32ResourceArchive();

	virtual bool hasFile(const Common::String &name) const;
	virtual int listMembers(Common::ArchiveMemberList &list) const;
	virtual const Common::ArchiveMemberPtr getMember(const Common::String &name) const;
	virtual Common::SeekableReadStream *createReadStreamForMember(const Common::String &name) const;
private:
	typedef Common::List<Common::String> FilenameList;

	FilenameList _files;
};

BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam) {
	if (IS_INTRESOURCE(lpszName))
		return TRUE;

	Win32ResourceArchive *arch = (Win32ResourceArchive *)lParam;
	arch->_files.push_back(lpszName);
	return TRUE;
}

Win32ResourceArchive::Win32ResourceArchive() {
	EnumResourceNames(NULL, MAKEINTRESOURCE(256), &EnumResNameProc, (LONG_PTR)this);
}

bool Win32ResourceArchive::hasFile(const Common::String &name) const {
	for (FilenameList::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		if (i->equalsIgnoreCase(name))
			return true;
	}

	return false;
}

int Win32ResourceArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FilenameList::const_iterator i = _files.begin(); i != _files.end(); ++i, ++count)
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(*i, this)));

	return count;
}

const Common::ArchiveMemberPtr Win32ResourceArchive::getMember(const Common::String &name) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *Win32ResourceArchive::createReadStreamForMember(const Common::String &name) const {
	HRSRC resource = FindResource(NULL, name.c_str(), MAKEINTRESOURCE(256));

	if (resource == NULL)
		return 0;

	HGLOBAL handle = LoadResource(NULL, resource);

	if (handle == NULL)
		return 0;

	const byte *data = (const byte *)LockResource(handle);

	if (data == NULL)
		return 0;

	uint32 size = SizeofResource(NULL, resource);

	if (size == 0)
		return 0;

	return new Common::MemoryReadStream(data, size);
}

} // End of anonymous namespace

void OSystem_Win32::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	s.add("Win32Res", new Win32ResourceArchive(), priority);

	OSystem_SDL::addSysArchivesToSearchSet(s, priority);
}

AudioCDManager *OSystem_Win32::createAudioCDManager() {
	return createWin32AudioCDManager();
}

#endif
