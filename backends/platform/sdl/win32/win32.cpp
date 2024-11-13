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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
// required for SHGFP_TYPE_CURRENT in shlobj.h
#define _WIN32_IE 0x500
#endif
#include <shlobj.h>
#include <tchar.h>

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/textconsole.h"

#include "backends/audiocd/win32/win32-audiocd.h"
#include "backends/platform/sdl/win32/win32.h"
#include "backends/platform/sdl/win32/win32-window.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"
#include "backends/saves/windows/windows-saves.h"
#include "backends/fs/windows/windows-fs-factory.h"
#include "backends/taskbar/win32/win32-taskbar.h"
#include "backends/updates/win32/win32-updates.h"
#include "backends/dialogs/win32/win32-dialogs.h"

#include "common/memstream.h"
#include "common/ustr.h"

#if defined(USE_TTS)
#include "backends/text-to-speech/windows/windows-text-to-speech.h"
#endif

#define DEFAULT_CONFIG_FILE "scummvm.ini"

OSystem_Win32::OSystem_Win32() :
	_isPortable(false) {
}

void OSystem_Win32::init() {
	// Initialize File System Factory
	_fsFactory = new WindowsFilesystemFactory();

	// Create Win32 specific window
	initSDL();
	_window = new SdlWindow_Win32();

#if defined(USE_TASKBAR)
	// Initialize taskbar manager
	_taskbarManager = new Win32TaskbarManager((SdlWindow_Win32*)_window);
#endif

#if defined(USE_SYSDIALOGS)
	// Initialize dialog manager
	_dialogManager = new Win32DialogManager((SdlWindow_Win32*)_window);
#endif

#if defined(USE_JPEG)
	initializeJpegLibraryForWin95();
#endif
	// Invoke parent implementation of this method
	OSystem_SDL::init();
}

WORD GetCurrentSubsystem() {
	// HMODULE is the module base address. And the PIMAGE_DOS_HEADER is located at the beginning.
	PIMAGE_DOS_HEADER EXEHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(nullptr);
	assert(EXEHeader->e_magic == IMAGE_DOS_SIGNATURE);
	// PIMAGE_NT_HEADERS is bitness dependant.
	// Conveniently, since it's for our own process, it's always the correct bitness.
	// IMAGE_NT_HEADERS has to be found using a byte offset from the EXEHeader,
	// which requires the ugly cast.
	PIMAGE_NT_HEADERS PEHeader = (PIMAGE_NT_HEADERS)(((char*)EXEHeader) + EXEHeader->e_lfanew);
	assert(PEHeader->Signature == IMAGE_NT_SIGNATURE);
	return PEHeader->OptionalHeader.Subsystem;
}

void OSystem_Win32::initBackend() {
	// The console window is enabled for the console subsystem,
	// since Windows already creates the console window for us
	ConfMan.registerDefault("console", GetCurrentSubsystem() == IMAGE_SUBSYSTEM_WINDOWS_CUI);

	// Enable or disable the window console window
	if (ConfMan.getBool("console")) {
		if (AllocConsole()) {
			freopen("CONIN$","r",stdin);
			freopen("CONOUT$","w",stdout);
			freopen("CONOUT$","w",stderr);
		}
		SetConsoleTitle(TEXT("ScummVM Status Window"));
	} else {
		FreeConsole();
	}

	// Create the savefile manager
	if (_savefileManager == nullptr)
		_savefileManager = new WindowsSaveFileManager(_isPortable);

#if defined(USE_SPARKLE)
	// Initialize updates manager
	if (!_isPortable) {
		_updateManager = new Win32UpdateManager((SdlWindow_Win32*)_window);
	}
#endif

	// Initialize text to speech
#ifdef USE_TTS
	_textToSpeechManager = new WindowsTextToSpeechManager();
#endif

	// Invoke parent implementation of this method
	OSystem_SDL::initBackend();
}

#ifdef USE_OPENGL
OSystem_SDL::GraphicsManagerType OSystem_Win32::getDefaultGraphicsManager() const {
	return GraphicsManagerOpenGL;
}
#endif

bool OSystem_Win32::hasFeature(Feature f) {
	if (f == kFeatureDisplayLogFile || f == kFeatureOpenUrl)
		return true;

#ifdef USE_SYSDIALOGS
	if (f == kFeatureSystemBrowserDialog)
		return true;
#endif

	return OSystem_SDL::hasFeature(f);
}

bool OSystem_Win32::displayLogFile() {
	if (_logFilePath.empty())
		return false;

	// Try opening the log file with the default text editor
	// log files should be registered as "txtfile" by default and thus open in the default text editor
	TCHAR *tLogFilePath = Win32::stringToTchar(_logFilePath.toString(Common::Path::kNativeSeparator));
	SHELLEXECUTEINFO sei;

	memset(&sei, 0, sizeof(sei));
	sei.fMask  = SEE_MASK_FLAG_NO_UI;
	sei.hwnd   = getHwnd();
	sei.lpFile = tLogFilePath;
	sei.nShow  = SW_SHOWNORMAL;

	if (ShellExecuteEx(&sei)) {
		free(tLogFilePath);
		return true;
	}

	// ShellExecute with the default verb failed, try the "Open with..." dialog
	PROCESS_INFORMATION processInformation;
	STARTUPINFO startupInfo;
	memset(&processInformation, 0, sizeof(processInformation));
	memset(&startupInfo, 0, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);

	TCHAR cmdLine[MAX_PATH * 2];  // CreateProcess may change the contents of cmdLine
	_stprintf(cmdLine, TEXT("rundll32 shell32.dll,OpenAs_RunDLL %s"), tLogFilePath);
	BOOL result = CreateProcess(nullptr,
	                            cmdLine,
	                            nullptr,
	                            nullptr,
	                            FALSE,
	                            NORMAL_PRIORITY_CLASS,
	                            nullptr,
	                            nullptr,
	                            &startupInfo,
	                            &processInformation);
	free(tLogFilePath);
	if (result) {
		CloseHandle(processInformation.hProcess);
		CloseHandle(processInformation.hThread);
		return true;
	}

	return false;
}

bool OSystem_Win32::openUrl(const Common::String &url) {
	TCHAR *tUrl = Win32::stringToTchar(url);
	SHELLEXECUTEINFO sei;

	memset(&sei, 0, sizeof(sei));
	sei.cbSize = sizeof(sei);
	sei.fMask  = SEE_MASK_FLAG_NO_UI;
	sei.hwnd   = getHwnd();
	sei.lpFile = tUrl;
	sei.nShow  = SW_SHOWNORMAL;

	BOOL success = ShellExecuteEx(&sei);

	free(tUrl);
	if (!success) {
		warning("ShellExecuteEx failed: error = %08lX", GetLastError());
		return false;
	}
	return true;
}

void OSystem_Win32::logMessage(LogMessageType::Type type, const char *message) {
	OSystem_SDL::logMessage(type, message);

#if defined( USE_WINDBG )
	TCHAR *tMessage = Win32::stringToTchar(message);
	OutputDebugString(tMessage);
	free(tMessage);
#endif
}

Common::String OSystem_Win32::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && defined(USE_TRANSLATION)
	// We can not use "setlocale" (at least not for MSVC builds), since it
	// will return locales like: "English_USA.1252", thus we need a special
	// way to determine the locale string for Win32.
	TCHAR langName[9];
	TCHAR ctryName[9];

	if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, langName, ARRAYSIZE(langName)) != 0 &&
		GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SISO3166CTRYNAME, ctryName, ARRAYSIZE(ctryName)) != 0) {
		Common::String localeName = Win32::tcharToString(langName);
		localeName += "_";
		localeName += Win32::tcharToString(ctryName);

		return localeName;
	}
#endif // USE_DETECTLANG
	// Falback to SDL implementation
	return OSystem_SDL::getSystemLanguage();
}

Common::Path OSystem_Win32::getDefaultIconsPath() {
	TCHAR iconsPath[MAX_PATH];

	if (_isPortable) {
		Win32::getProcessDirectory(iconsPath, MAX_PATH);
		_tcscat(iconsPath, TEXT("\\Icons\\"));
	} else {
		// Use the Application Data directory of the user profile
		if (!Win32::getApplicationDataDirectory(iconsPath)) {
			return Common::Path();
		}
		_tcscat(iconsPath, TEXT("\\Icons\\"));
		CreateDirectory(iconsPath, nullptr);
	}

	return Common::Path(Win32::tcharToString(iconsPath), Common::Path::kNativeSeparator);
}

Common::Path OSystem_Win32::getDefaultDLCsPath() {
	TCHAR dlcsPath[MAX_PATH];

	if (_isPortable) {
		Win32::getProcessDirectory(dlcsPath, MAX_PATH);
		_tcscat(dlcsPath, TEXT("\\DLCs\\"));
	} else {
		// Use the Application Data directory of the user profile
		if (!Win32::getApplicationDataDirectory(dlcsPath)) {
			return Common::Path();
		}
		_tcscat(dlcsPath, TEXT("\\DLCs\\"));
		CreateDirectory(dlcsPath, nullptr);
	}

	return Common::Path(Win32::tcharToString(dlcsPath), Common::Path::kNativeSeparator);
}

Common::Path OSystem_Win32::getScreenshotsPath() {
	// If the user has configured a screenshots path, use it
	Common::Path screenshotsPath = ConfMan.getPath("screenshotpath");
	if (!screenshotsPath.empty()) {
		return screenshotsPath;
	}

	TCHAR picturesPath[MAX_PATH];
	if (_isPortable) {
		Win32::getProcessDirectory(picturesPath, MAX_PATH);
		_tcscat(picturesPath, TEXT("\\Screenshots\\"));
	} else {
		// Use the My Pictures folder
		HRESULT hr = SHGetFolderPathFunc(nullptr, CSIDL_MYPICTURES, nullptr, SHGFP_TYPE_CURRENT, picturesPath);
		if (hr != S_OK) {
			if (hr != E_NOTIMPL) {
				warning("Unable to locate My Pictures directory");
			}
			return Common::Path();
		}
		_tcscat(picturesPath, TEXT("\\ScummVM Screenshots\\"));
	}

	// If the directory already exists (as it should in most cases),
	// we don't want to fail, but we need to stop on other errors (such as ERROR_PATH_NOT_FOUND)
	if (!CreateDirectory(picturesPath, nullptr)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			error("Cannot create ScummVM Screenshots folder");
	}

	return Common::Path(Win32::tcharToString(picturesPath), Common::Path::kNativeSeparator);
}

Common::Path OSystem_Win32::getDefaultConfigFileName() {
	TCHAR configFile[MAX_PATH];

	// if this is the first time the default config file name is requested
	// then we need detect if we should run in portable mode. (and if it's
	// never requested before the backend is initialized then a config file
	// was provided on the command line and portable mode doesn't apply.)
	if (!backendInitialized()) {
		_isPortable = detectPortableConfigFile();
	}

	if (_isPortable) {
		// Use the current process directory in portable mode
		Win32::getProcessDirectory(configFile, MAX_PATH);
		_tcscat(configFile, TEXT("\\" DEFAULT_CONFIG_FILE));
	} else {
		// Use the Application Data directory of the user profile
		if (Win32::getApplicationDataDirectory(configFile)) {
			_tcscat(configFile, TEXT("\\" DEFAULT_CONFIG_FILE));

			FILE *tmp = nullptr;
			if ((tmp = _tfopen(configFile, TEXT("r"))) == nullptr) {
				// Check windows directory
				TCHAR oldConfigFile[MAX_PATH];
				uint ret = GetWindowsDirectory(oldConfigFile, MAX_PATH);
				if (ret == 0 || ret > MAX_PATH)
					error("Cannot retrieve the path of the Windows directory");

				_tcscat(oldConfigFile, TEXT("\\" DEFAULT_CONFIG_FILE));
				if ((tmp = _tfopen(oldConfigFile, TEXT("r")))) {
					_tcscpy(configFile, oldConfigFile);

					fclose(tmp);
				}
			} else {
				fclose(tmp);
			}
		} else {
			// Check windows directory
			uint ret = GetWindowsDirectory(configFile, MAX_PATH);
			if (ret == 0 || ret > MAX_PATH)
				error("Cannot retrieve the path of the Windows directory");

			_tcscat(configFile, TEXT("\\" DEFAULT_CONFIG_FILE));
		}
	}

	return Common::Path(Win32::tcharToString(configFile), Common::Path::kNativeSeparator);
}

Common::Path OSystem_Win32::getDefaultLogFileName() {
	TCHAR logFile[MAX_PATH];

	if (_isPortable) {
		Win32::getProcessDirectory(logFile, MAX_PATH);
	} else {
		// Use the Application Data directory of the user profile
		if (!Win32::getApplicationDataDirectory(logFile)) {
			return Common::Path();
		}
		_tcscat(logFile, TEXT("\\Logs"));
		CreateDirectory(logFile, nullptr);
	}

	_tcscat(logFile, TEXT("\\scummvm.log"));

	return Common::Path(Win32::tcharToString(logFile), Common::Path::kNativeSeparator);
}

bool OSystem_Win32::detectPortableConfigFile() {
	// ScummVM operates in a "portable mode" if there is a config file in the
	// same directory as the executable. In this mode, the executable's
	// directory is used instead of the user's profile for application files.
	// This approach is modeled off of the portable mode in Notepad++.

	// Check if there is a config file in the same directory as the executable.
	TCHAR portableConfigFile[MAX_PATH];
	Win32::getProcessDirectory(portableConfigFile, MAX_PATH);
	_tcscat(portableConfigFile, TEXT("\\" DEFAULT_CONFIG_FILE));
	FILE *file = _tfopen(portableConfigFile, TEXT("r"));
	if (file == nullptr) {
		return false;
	}
	fclose(file);

	// Check if we're running from Program Files on Vista+.
	// If so then don't attempt to use local files due to UAC.
	// (Notepad++ does this too.)
	if (Win32::confirmWindowsVersion(6, 0)) {
		TCHAR programFiles[MAX_PATH];
		if (SHGetFolderPathFunc(nullptr, CSIDL_PROGRAM_FILES, nullptr, SHGFP_TYPE_CURRENT, programFiles) == S_OK) {
			_tcscat(portableConfigFile, TEXT("\\"));
			if (_tcsstr(portableConfigFile, programFiles) == portableConfigFile) {
				return false;
			}
		}
	}

	return true;
}

namespace {

class Win32ResourceArchive final : public Common::Archive {
	friend BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam);
public:
	Win32ResourceArchive();

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;
private:
	typedef Common::List<Common::Path> FilenameList;

	FilenameList _files;
};

BOOL CALLBACK EnumResNameProc(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam) {
	if (IS_INTRESOURCE(lpszName))
		return TRUE;

	Win32ResourceArchive *arch = (Win32ResourceArchive *)lParam;
	Common::String filename = Win32::tcharToString(lpszName);
	// We use / as path separator in resources
	arch->_files.push_back(Common::Path(filename, '/'));
	return TRUE;
}

Win32ResourceArchive::Win32ResourceArchive() {
	EnumResourceNames(nullptr, MAKEINTRESOURCE(256), &EnumResNameProc, (LONG_PTR)this);
}

bool Win32ResourceArchive::hasFile(const Common::Path &path) const {
	for (FilenameList::const_iterator i = _files.begin(); i != _files.end(); ++i) {
		if (i->equalsIgnoreCase(path))
			return true;
	}

	return false;
}

int Win32ResourceArchive::listMembers(Common::ArchiveMemberList &list) const {
	int count = 0;

	for (FilenameList::const_iterator i = _files.begin(); i != _files.end(); ++i, ++count)
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(*i, *this)));

	return count;
}

const Common::ArchiveMemberPtr Win32ResourceArchive::getMember(const Common::Path &path) const {
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *Win32ResourceArchive::createReadStreamForMember(const Common::Path &path) const {
	// We store paths in resources using / separator
	Common::String name = path.toString('/');
	TCHAR *tName = Win32::stringToTchar(name);
	HRSRC resource = FindResource(nullptr, tName, MAKEINTRESOURCE(256));
	free(tName);

	if (resource == nullptr)
		return nullptr;

	HGLOBAL handle = LoadResource(nullptr, resource);

	if (handle == nullptr)
		return nullptr;

	const byte *data = (const byte *)LockResource(handle);

	if (data == nullptr)
		return nullptr;

	uint32 size = SizeofResource(nullptr, resource);

	if (size == 0)
		return nullptr;

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

uint32 OSystem_Win32::getOSDoubleClickTime() const {
	return GetDoubleClickTime();
}

// libjpeg-turbo uses SSE instructions that error on at least some Win95 machines.
// These can be disabled with an environment variable. Fixes bug #13643
#if defined(USE_JPEG)
void OSystem_Win32::initializeJpegLibraryForWin95() {
	OSVERSIONINFO versionInfo;
	ZeroMemory(&versionInfo, sizeof(versionInfo));
	versionInfo.dwOSVersionInfoSize = sizeof(versionInfo);
	GetVersionEx(&versionInfo);

	// Is Win95?
	if (versionInfo.dwMajorVersion == 4 && versionInfo.dwMinorVersion == 0) {
		// Disable SSE instructions in libjpeg-turbo.
		// This limits detected extensions to 3DNOW and MMX.
		_tputenv(TEXT("JSIMD_FORCE3DNOW=1"));
	}
}
#endif

#endif
