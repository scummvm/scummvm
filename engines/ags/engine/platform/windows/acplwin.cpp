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

#include "core/platform.h"

#if AGS_PLATFORM_OS_WINDOWS

// ********* WINDOWS *********

#include <string.h>
#include <allegro.h>
#include <allegro/platform/aintwin.h>
#include "ac/common.h"
#include "ac/draw.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "ac/global_display.h"
#include "ac/runtime_defines.h"
#include "ac/string.h"
#include "debug/out.h"
#include "gfx/graphicsdriver.h"
#include "gfx/bitmap.h"
#include "main/engine.h"
#include "platform/base/agsplatformdriver.h"
#include "platform/windows/setup/winsetup.h"
#include "plugin/agsplugin.h"
#include "util/file.h"
#include "util/stream.h"
#include "util/string_compat.h"
#include "media/audio/audio_system.h"

namespace AGS3 {

#ifndef AGS_NO_VIDEO_PLAYER
extern void dxmedia_abort_video();
extern void dxmedia_pause_video();
extern void dxmedia_resume_video();
extern char lastError[200];
#endif

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameSetup usetup;
extern int our_eip;
extern IGraphicsDriver *gfxDriver;
extern color palette[256];

#include <shlobj.h>
#include <time.h>
#include <shlwapi.h>
#include <windows.h>
#include <rpcsal.h>
#include <gameux.h>

#include <libcda.h>


#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA        0x001C
#define CSIDL_COMMON_APPDATA       0x0023
#endif

typedef struct BMP_EXTRA_INFO {
	LPDIRECTDRAWSURFACE2 surf;
	struct BMP_EXTRA_INFO *next;
	struct BMP_EXTRA_INFO *prev;
	int flags;
	int lock_nesting;
} BMP_EXTRA_INFO;

// from Allegro DDraw driver
extern "C" extern LPDIRECTDRAW2 directdraw;
extern "C" extern LPDIRECTSOUND directsound;
extern "C" extern LPDIRECTINPUTDEVICE mouse_dinput_device;
extern "C" extern LPDIRECTINPUTDEVICE key_dinput_device;

char win32SavedGamesDirectory[MAX_PATH] = "\0";
char win32AppDataDirectory[MAX_PATH] = "\0";
String win32OutputDirectory;

const unsigned int win32TimerPeriod = 1;

extern SetupReturnValue acwsetup(const ConfigTree &cfg_in, ConfigTree &cfg_out, const String &game_data_dir, const char *, const char *);

struct AGSWin32 : AGSPlatformDriver {
	AGSWin32();
	~AGSWin32();

	virtual void AboutToQuitGame();
	virtual int  CDPlayerCommand(int cmdd, int datt);
	virtual void AttachToParentConsole();
	virtual void DisplayAlert(const char *, ...);
	virtual int  GetLastSystemError();
	virtual const char *GetAllUsersDataDirectory();
	virtual const char *GetUserSavedgamesDirectory();
	virtual const char *GetUserConfigDirectory();
	virtual const char *GetUserGlobalConfigDirectory();
	virtual const char *GetAppOutputDirectory();
	virtual const char *GetIllegalFileChars();
	virtual const char *GetGraphicsTroubleshootingText();
	virtual unsigned long GetDiskFreeSpaceMB();
	virtual const char *GetNoMouseErrorString();
	virtual bool IsMouseControlSupported(bool windowed);
	virtual const char *GetAllegroFailUserHint();
	virtual eScriptSystemOSID GetSystemOSID();
	virtual int  InitializeCDPlayer();
	virtual void PostAllegroInit(bool windowed);
	virtual void PostAllegroExit();
	virtual SetupReturnValue RunSetup(const ConfigTree &cfg_in, ConfigTree &cfg_out);
	virtual void SetGameWindowIcon();
	virtual void ShutdownCDPlayer();
	virtual void WriteStdOut(const char *fmt, ...);
	virtual void WriteStdErr(const char *fmt, ...);
	virtual void DisplaySwitchOut();
	virtual void DisplaySwitchIn();
	virtual void PauseApplication();
	virtual void ResumeApplication();
	virtual void GetSystemDisplayModes(std::vector<Engine::DisplayMode> &dms);
	virtual bool EnterFullscreenMode(const Engine::DisplayMode &dm);
	virtual bool ExitFullscreenMode();
	virtual void AdjustWindowStyleForFullscreen();
	virtual void AdjustWindowStyleForWindowed();
	virtual void RegisterGameWithGameExplorer();
	virtual void UnRegisterGameWithGameExplorer();
	virtual int  ConvertKeycodeToScanCode(int keyCode);
	virtual void ValidateWindowSize(int &x, int &y, bool borderless) const;
	virtual bool LockMouseToWindow();
	virtual void UnlockMouse();

#ifndef AGS_NO_VIDEO_PLAYER
	virtual void PlayVideo(const char *name, int skip, int flags);
#endif


private:
	void add_game_to_game_explorer(IGameExplorer *pFwGameExplorer, GUID *guid, const char *guidAsText, bool allUsers);
	void remove_game_from_game_explorer(IGameExplorer *pFwGameExplorer, GUID *guid, const char *guidAsText, bool allUsers);
	void add_tasks_for_game(const char *guidAsText, const char *gameEXE, const char *workingFolder, bool allUsers);
	void get_tasks_directory(char *directoryNameBuffer, const char *guidAsText, bool allUsers);
	void update_game_explorer(bool add);
	void create_shortcut(const char *pathToEXE, const char *workingFolder, const char *arguments, const char *shortcutPath);
	void register_file_extension(const char *exePath);
	void unregister_file_extension();

	bool SetSystemDisplayMode(const DisplayMode &dm, bool fullscreen);

	bool _isDebuggerPresent; // indicates if the win app is running in the context of a debugger
	DisplayMode _preFullscreenMode; // saved display mode before switching system to fullscreen
	bool _isAttachedToParentConsole; // indicates if the win app is attached to the parent console
};

AGSWin32::AGSWin32() :
	_isDebuggerPresent(::IsDebuggerPresent() != FALSE),
	_isAttachedToParentConsole(false) {
	// Do nothing.
}

AGSWin32::~AGSWin32() {
	if (_isAttachedToParentConsole) {
		::FreeConsole();
	}
}

void check_parental_controls() {
	/* this doesn't work, it always just returns access depedning
	   on whether unrated games are allowed because of digital signature
	BOOL bHasAccess = FALSE;
	IGameExplorer* pFwGameExplorer = NULL;

	CoInitialize(NULL);
	// Create an instance of the Game Explorer Interface
	HRESULT hr = CoCreateInstance( __uuidof(GameExplorer), NULL, CLSCTX_INPROC_SERVER, __uuidof(IGameExplorer), (void**)&pFwGameExplorer);
	if( FAILED(hr) || pFwGameExplorer == NULL ) {
	  // not Vista, do nothing
	}
	else {
	  char theexename[MAX_PATH] = "e:\\code\\ags\\acwin\\release\\acwin.exe";
	  WCHAR wstrBinPath[MAX_PATH] = {0};
	  MultiByteToWideChar(CP_ACP, 0, theexename, MAX_PATH, wstrBinPath, MAX_PATH);
	  BSTR bstrGDFBinPath = SysAllocString(wstrBinPath);

	  hr = pFwGameExplorer->VerifyAccess( bstrGDFBinPath, &bHasAccess );
	  SysFreeString(bstrGDFBinPath);

	  if( FAILED(hr) || !bHasAccess ) {
	    char buff[300];
	    sprintf(buff, "Parental controls block: %X  b: %d", hr, bHasAccess);
	    quit(buff);
	  }
	  else {
	    platform->DisplayAlert("Parental controls: Access granted.");
	  }

	}

	if( pFwGameExplorer ) pFwGameExplorer->Release();
	CoUninitialize();
	*/
}

void AGSWin32::create_shortcut(const char *pathToEXE, const char *workingFolder, const char *arguments, const char *shortcutPath) {
	IShellLink *pShellLink = NULL;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&pShellLink);

	if ((SUCCEEDED(hr)) && (pShellLink != NULL)) {
		IPersistFile *pPersistFile = NULL;
		if (FAILED(pShellLink->QueryInterface(IID_IPersistFile, (void **)&pPersistFile))) {
			this->DisplayAlert("Unable to add game tasks: QueryInterface for IPersistFile failed");
			pShellLink->Release();
			return;
		}

		// Set the path to the shortcut target and add the description
		if (FAILED(pShellLink->SetPath(pathToEXE))) {
			this->DisplayAlert("Unable to add game tasks: SetPath failed");
		} else if (FAILED(pShellLink->SetWorkingDirectory(workingFolder))) {
			this->DisplayAlert("Unable to add game tasks: SetWorkingDirectory failed");
		} else if ((arguments != NULL) && (FAILED(pShellLink->SetArguments(arguments)))) {
			this->DisplayAlert("Unable to add game tasks: SetArguments failed");
		} else {
			WCHAR wstrTemp[MAX_PATH] = {0};
			MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, wstrTemp, MAX_PATH);

			if (FAILED(pPersistFile->Save(wstrTemp, TRUE))) {
				this->DisplayAlert("Unable to add game tasks: IPersistFile::Save failed");
			}
		}

		pPersistFile->Release();
	}

	if (pShellLink) pShellLink->Release();
}

void CopyStringAndRemoveInvalidFilenameChars(const char *source, char *destinationBuffer) {
	int destIdx = 0;
	for (int i = 0; i < (int)strlen(source); i++) {
		if ((source[i] != '/') &&
		        (source[i] != '\\') &&
		        (source[i] != ':') &&
		        (source[i] != '*') &&
		        (source[i] != '?') &&
		        (source[i] != '"') &&
		        (source[i] != '<') &&
		        (source[i] != '>') &&
		        (source[i] != '|') &&
		        (source[i] >= 32)) {
			destinationBuffer[destIdx] = source[i];
			destIdx++;
		}
	}
	destinationBuffer[destIdx] = 0;
}

void AGSWin32::get_tasks_directory(char *pathBuffer, const char *guidAsText, bool allUsers) {
	if (SHGetSpecialFolderPath(NULL, pathBuffer, allUsers ? CSIDL_COMMON_APPDATA : CSIDL_LOCAL_APPDATA, FALSE) == FALSE) {
		this->DisplayAlert("Unable to register game: SHGetSpecialFolderPath failed");
		return;
	}

	if (pathBuffer[strlen(pathBuffer) - 1] != '\\') {
		strcat(pathBuffer, "\\");
	}

	strcat(pathBuffer, "Microsoft\\Windows\\GameExplorer\\");
	strcat(pathBuffer, guidAsText);
	mkdir(pathBuffer);
	strcat(pathBuffer, "\\");
	strcat(pathBuffer, "PlayTasks");
	mkdir(pathBuffer);
}

void AGSWin32::add_tasks_for_game(const char *guidAsText, const char *gameEXE, const char *workingFolder, bool allUsers) {
	char pathBuffer[MAX_PATH];
	get_tasks_directory(pathBuffer, guidAsText, allUsers);
	strcat(pathBuffer, "\\");
	strcat(pathBuffer, "0");
	mkdir(pathBuffer);

	// Remove any existing "Play.lnk" from a previous version
	char shortcutLocation[MAX_PATH];
	sprintf(shortcutLocation, "%s\\Play.lnk", pathBuffer);
	::remove(shortcutLocation);

	// Generate the shortcut file name (because it can appear on
	// the start menu's Recent area)
	char sanitisedGameName[MAX_PATH];
	CopyStringAndRemoveInvalidFilenameChars(game.gamename, sanitisedGameName);
	if (sanitisedGameName[0] == 0)
		strcpy(sanitisedGameName, "Play");
	sprintf(shortcutLocation, "%s\\%s.lnk", pathBuffer, sanitisedGameName);

	create_shortcut(gameEXE, workingFolder, NULL, shortcutLocation);

	pathBuffer[strlen(pathBuffer) - 1] = '1';
	mkdir(pathBuffer);

	sprintf(shortcutLocation, "%s\\Setup game.lnk", pathBuffer);
	create_shortcut(gameEXE, workingFolder, "--setup", shortcutLocation);
}

void AGSWin32::add_game_to_game_explorer(IGameExplorer *pFwGameExplorer, GUID *guid, const char *guidAsText, bool allUsers) {
	WCHAR wstrTemp[MAX_PATH] = {0};
	bool hadError = false;

	char theexename[MAX_PATH];
	GetModuleFileName(NULL, theexename, MAX_PATH);

	MultiByteToWideChar(CP_ACP, 0, theexename, MAX_PATH, wstrTemp, MAX_PATH);
	BSTR bstrGDFBinPath = SysAllocString(wstrTemp);

	char gameDirectory[MAX_PATH];
	strcpy(gameDirectory, theexename);
	strrchr(gameDirectory, '\\')[0] = 0;

	MultiByteToWideChar(CP_ACP, 0, gameDirectory, MAX_PATH, wstrTemp, MAX_PATH);
	BSTR bstrGameDirectory = SysAllocString(wstrTemp);

	HRESULT hr = pFwGameExplorer->AddGame(bstrGDFBinPath, bstrGameDirectory, allUsers ? GIS_ALL_USERS : GIS_CURRENT_USER, guid);
	if ((FAILED(hr)) || (hr == S_FALSE)) {
		if (hr == 0x80070715) {
			// No GDF XML -- do nothing. This means the game was compiled
			// without Game Explorer support.
			hadError = true;
		} else {
			// Game already exists or error
			HRESULT updateHr = pFwGameExplorer->UpdateGame(*guid);
			if (FAILED(updateHr)) {
				this->DisplayAlert("Failed to add the game to the game explorer: %08X, %08X", hr, updateHr);
				hadError = true;
			}
		}
	} else {
		add_tasks_for_game(guidAsText, theexename, gameDirectory, allUsers);
	}

	BOOL bHasAccess = FALSE;
	hr = pFwGameExplorer->VerifyAccess(bstrGDFBinPath, &bHasAccess);

	if ((FAILED(hr) || !bHasAccess) && (!hadError)) {
		this->DisplayAlert("Windows Parental Controls will not allow you to run this game.");
	}

	SysFreeString(bstrGDFBinPath);
	SysFreeString(bstrGameDirectory);
}

#define FA_SEARCH -1
void delete_files_in_directory(const char *directoryName, const char *fileMask) {
	char srchBuffer[MAX_PATH];
	sprintf(srchBuffer, "%s\\%s", directoryName, fileMask);
	al_ffblk dfb;
	int   dun = al_findfirst(srchBuffer, &dfb, FA_SEARCH);
	while (!dun) {
		::remove(dfb.name);
		dun = al_findnext(&dfb);
	}
	al_findclose(&dfb);
}

void AGSWin32::remove_game_from_game_explorer(IGameExplorer *pFwGameExplorer, GUID *guid, const char *guidAsText, bool allUsers) {
	HRESULT hr = pFwGameExplorer->RemoveGame(*guid);
	if (FAILED(hr)) {
		this->DisplayAlert("Failed to un-register game: 0x%08X", hr);
	}
}

void AGSWin32::update_game_explorer(bool add) {
	IGameExplorer *pFwGameExplorer = NULL;

	CoInitialize(NULL);
	// Create an instance of the Game Explorer Interface
	HRESULT hr = CoCreateInstance(__uuidof(GameExplorer), NULL, CLSCTX_INPROC_SERVER, __uuidof(IGameExplorer), (void **)&pFwGameExplorer);
	if (FAILED(hr) || pFwGameExplorer == NULL) {
		Debug::Printf(kDbgMsg_Warn, "Game Explorer not found to register game, Windows Vista required");
	} else {
		ags_strupr(game.guid);
		WCHAR wstrTemp[MAX_PATH] = {0};
		GUID guid = GUID_NULL;
		MultiByteToWideChar(CP_ACP, 0, game.guid, MAX_GUID_LENGTH, wstrTemp, MAX_GUID_LENGTH);
		if (IIDFromString(wstrTemp, &guid) != S_OK) {
			this->DisplayAlert("Failed to register game: IIDFromString failed");
		} else if (add) {
			add_game_to_game_explorer(pFwGameExplorer, &guid, game.guid, true);
		} else {
			remove_game_from_game_explorer(pFwGameExplorer, &guid, game.guid, true);
		}
	}

	if (pFwGameExplorer) pFwGameExplorer->Release();
	CoUninitialize();
}

void AGSWin32::unregister_file_extension() {
	char keyname[MAX_PATH];
	sprintf(keyname, ".%s", game.saveGameFileExtension);
	if (SHDeleteKey(HKEY_CLASSES_ROOT, keyname) != ERROR_SUCCESS) {
		this->DisplayAlert("Unable to un-register the file extension. Make sure you are running this with admin rights.");
		return;
	}

	sprintf(keyname, "AGS.SaveGames.%s", game.saveGameFileExtension);
	SHDeleteKey(HKEY_CLASSES_ROOT, keyname);

	sprintf(keyname, "Software\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers\\.%s", game.saveGameFileExtension);
	SHDeleteKey(HKEY_LOCAL_MACHINE, keyname);

	// Tell Explorer to refresh its file association data
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void AGSWin32::register_file_extension(const char *exePath) {
	DWORD valType, valBufLen = MAX_PATH;
	valType = REG_SZ;
	char valBuf[MAX_PATH], keyname[MAX_PATH];
	char saveGameRegistryType[MAX_PATH];
	sprintf(saveGameRegistryType, "AGS.SaveGames.%s", game.saveGameFileExtension);

	// write HKEY_CLASSES_ROOT\.Extension = AGS.SaveGames.Extension
	strcpy(valBuf, saveGameRegistryType);
	sprintf(keyname, ".%s", game.saveGameFileExtension);
	if (RegSetValue(HKEY_CLASSES_ROOT, keyname, valType, valBuf, valBufLen)) {
		this->DisplayAlert("Unable to register file type. Make sure you are running this with Administrator rights.");
		return;
	}

	// create HKEY_CLASSES_ROOT\AGS.SaveGames.Extension
	strcpy(keyname, saveGameRegistryType);
	sprintf(valBuf, "%s Saved Game", game.gamename);
	RegSetValue(HKEY_CLASSES_ROOT, keyname, REG_SZ, valBuf, strlen(valBuf));

	// write HKEY_CLASSES_ROOT\AGS.SaveGames.Extension\DefaultIcon
	sprintf(keyname, "%s\\DefaultIcon", saveGameRegistryType);
	sprintf(valBuf, "\"%s\", 0", exePath);
	RegSetValue(HKEY_CLASSES_ROOT, keyname, REG_SZ, valBuf, strlen(valBuf));

	// write HKEY_CLASSES_ROOT\AGS.SaveGames.Extension\Shell\Open\Command
	sprintf(keyname, "%s\\Shell\\Open\\Command", saveGameRegistryType);
	sprintf(valBuf, "\"%s\" -loadSavedGame \"%%1\"", exePath);
	RegSetValue(HKEY_CLASSES_ROOT, keyname, REG_SZ, valBuf, strlen(valBuf));

	// ** BELOW IS VISTA-ONLY

	// write HKEY_CLASSES_ROOT\AGS.SaveGames.Extension, PreviewTitle
	strcpy(keyname, saveGameRegistryType);
	strcpy(valBuf, "prop:System.Game.RichSaveName;System.Game.RichApplicationName");
	SHSetValue(HKEY_CLASSES_ROOT, keyname, "PreviewTitle", REG_SZ, valBuf, strlen(valBuf));

	// write HKEY_CLASSES_ROOT\AGS.SaveGames.Extension, PreviewDetails
	strcpy(keyname, saveGameRegistryType);
	strcpy(valBuf, "prop:System.Game.RichLevel;System.DateChanged;System.Game.RichComment;System.DisplayName;System.DisplayType");
	SHSetValue(HKEY_CLASSES_ROOT, keyname, "PreviewDetails", REG_SZ, valBuf, strlen(valBuf));

	// write HKEY_CLASSES_ROOT\.Extension\ShellEx\{BB2E617C-0920-11D1-9A0B-00C04FC2D6C1}
	sprintf(keyname, ".%s\\ShellEx\\{BB2E617C-0920-11D1-9A0B-00C04FC2D6C1}", game.saveGameFileExtension);
	strcpy(valBuf, "{4E5BFBF8-F59A-4E87-9805-1F9B42CC254A}");
	RegSetValue(HKEY_CLASSES_ROOT, keyname, REG_SZ, valBuf, strlen(valBuf));

	// write HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.Extension
	sprintf(keyname, "Software\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers\\.%s", game.saveGameFileExtension);
	strcpy(valBuf, "{ECDD6472-2B9B-4B4B-AE36-F316DF3C8D60}");
	RegSetValue(HKEY_LOCAL_MACHINE, keyname, REG_SZ, valBuf, strlen(valBuf));

	// Tell Explorer to refresh its file association data
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
}

void AGSWin32::RegisterGameWithGameExplorer() {
	update_game_explorer(true);

	if (game.saveGameFileExtension[0] != 0) {
		char theexename[MAX_PATH];
		GetModuleFileName(NULL, theexename, MAX_PATH);

		register_file_extension(theexename);
	}
}

void AGSWin32::UnRegisterGameWithGameExplorer() {
	update_game_explorer(false);

	if (game.saveGameFileExtension[0] != 0) {
		unregister_file_extension();
	}
}

void AGSWin32::PostAllegroInit(bool windowed) {
	check_parental_controls();

	// Set the Windows timer resolution to 1 ms so that calls to
	// Sleep() don't take more time than specified
	MMRESULT result = timeBeginPeriod(win32TimerPeriod);
	if (result != TIMERR_NOERROR)
		Debug::Printf(kDbgMsg_Error, "Failed to set the timer resolution to %d ms", win32TimerPeriod);
}

typedef UINT(CALLBACK *Dynamic_SHGetKnownFolderPathType)(GUID &rfid, DWORD dwFlags, HANDLE hToken, PWSTR *ppszPath);
GUID FOLDERID_SAVEDGAMES = {0x4C5C32FF, 0xBB9D, 0x43b0, {0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4}};
#define _WIN32_WINNT_VISTA              0x0600
#define VER_MINORVERSION                0x0000001
#define VER_MAJORVERSION                0x0000002
#define VER_SERVICEPACKMAJOR            0x0000020
#define VER_GREATER_EQUAL               3

// These helpers copied from VersionHelpers.h in the Windows 8.1 SDK
bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor) {
	OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0 };
	DWORDLONG        const dwlConditionMask = VerSetConditionMask(
	            VerSetConditionMask(
	                VerSetConditionMask(
	                    0, VER_MAJORVERSION, VER_GREATER_EQUAL),
	                VER_MINORVERSION, VER_GREATER_EQUAL),
	            VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);

	osvi.dwMajorVersion = wMajorVersion;
	osvi.dwMinorVersion = wMinorVersion;
	osvi.wServicePackMajor = wServicePackMajor;

	return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;
}

bool IsWindowsVistaOrGreater() {
	return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
}

void determine_app_data_folder() {
	if (win32AppDataDirectory[0] != 0) {
		// already discovered
		return;
	}

	WCHAR unicodePath[MAX_PATH];
	WCHAR unicodeShortPath[MAX_PATH];
	SHGetSpecialFolderPathW(NULL, unicodePath, CSIDL_COMMON_APPDATA, FALSE);
	if (GetShortPathNameW(unicodePath, unicodeShortPath, MAX_PATH) == 0) {
		platform->DisplayAlert("Unable to get App Data dir: GetShortPathNameW failed");
		return;
	}
	WideCharToMultiByte(CP_ACP, 0, unicodeShortPath, -1, win32AppDataDirectory, MAX_PATH, NULL, NULL);

	strcat(win32AppDataDirectory, "\\Adventure Game Studio");
	mkdir(win32AppDataDirectory);
}

void determine_saved_games_folder() {
	if (win32SavedGamesDirectory[0] != 0) {
		// already discovered
		return;
	}

	WCHAR unicodeSaveGameDir[MAX_PATH] = L"";
	WCHAR unicodeShortSaveGameDir[MAX_PATH] = L"";

	if (IsWindowsVistaOrGreater()) {
		HINSTANCE hShellDLL = LoadLibrary("shell32.dll");
		Dynamic_SHGetKnownFolderPathType Dynamic_SHGetKnownFolderPath = (Dynamic_SHGetKnownFolderPathType)GetProcAddress(hShellDLL, "SHGetKnownFolderPath");

		if (Dynamic_SHGetKnownFolderPath != NULL) {
			PWSTR path = NULL;
			if (SUCCEEDED(Dynamic_SHGetKnownFolderPath(FOLDERID_SAVEDGAMES, 0, NULL, &path))) {
				if (GetShortPathNameW(path, unicodeShortSaveGameDir, MAX_PATH) > 0) {
					WideCharToMultiByte(CP_ACP, 0, unicodeShortSaveGameDir, -1, win32SavedGamesDirectory, MAX_PATH, NULL, NULL);
				}
				CoTaskMemFree(path);
			}
		}

		FreeLibrary(hShellDLL);
	} else {
		// Windows XP didn't have a "My Saved Games" folder, so create one under "My Documents"
		SHGetSpecialFolderPathW(NULL, unicodeSaveGameDir, CSIDL_PERSONAL, FALSE);
		// workaround for case where My Documents path has unicode chars (eg.
		// with Russian Windows) -- so use Short File Name instead
		if (GetShortPathNameW(unicodeSaveGameDir, unicodeShortSaveGameDir, MAX_PATH) > 0) {
			WideCharToMultiByte(CP_ACP, 0, unicodeShortSaveGameDir, -1, win32SavedGamesDirectory, MAX_PATH, NULL, NULL);
			strcat(win32SavedGamesDirectory, "\\My Saved Games");
			mkdir(win32SavedGamesDirectory);
		}
	}

	// Fallback to a subdirectory of the app data directory
	if (win32SavedGamesDirectory[0] == '\0') {
		determine_app_data_folder();
		strcpy(win32SavedGamesDirectory, win32AppDataDirectory);
		strcat(win32SavedGamesDirectory, "\\Saved Games");
		mkdir(win32SavedGamesDirectory);
	}
}

void DetermineAppOutputDirectory() {
	if (!win32OutputDirectory.IsEmpty()) {
		return;
	}

	determine_saved_games_folder();
	bool log_to_saves_dir = false;
	if (win32SavedGamesDirectory[0]) {
		win32OutputDirectory = win32SavedGamesDirectory;
		win32OutputDirectory.Append("\\.ags");
		log_to_saves_dir = mkdir(win32OutputDirectory) == 0 || errno == EEXIST;
	}

	if (!log_to_saves_dir) {
		char theexename[MAX_PATH + 1] = {0};
		GetModuleFileName(NULL, theexename, MAX_PATH);
		PathRemoveFileSpec(theexename);
		win32OutputDirectory = theexename;
	}
}

const char *AGSWin32::GetAllUsersDataDirectory() {
	determine_app_data_folder();
	return &win32AppDataDirectory[0];
}

const char *AGSWin32::GetUserSavedgamesDirectory() {
	determine_saved_games_folder();
	return win32SavedGamesDirectory;
}

const char *AGSWin32::GetUserConfigDirectory() {
	determine_saved_games_folder();
	return win32SavedGamesDirectory;
}

const char *AGSWin32::GetUserGlobalConfigDirectory() {
	DetermineAppOutputDirectory();
	return win32OutputDirectory;
}

const char *AGSWin32::GetAppOutputDirectory() {
	DetermineAppOutputDirectory();
	return win32OutputDirectory;
}

const char *AGSWin32::GetIllegalFileChars() {
	return "\\/:?\"<>|*";
}

const char *AGSWin32::GetGraphicsTroubleshootingText() {
	return "\n\nPossible causes:\n"
	       "* your graphics card drivers do not support requested resolution. "
	       "Run the game setup program and try another resolution.\n"
	       "* the graphics driver you have selected does not work. Try switching to another graphics driver.\n"
	       "* the graphics filter you have selected does not work. Try another filter.\n"
	       "* your graphics card drivers are out of date. "
	       "Try downloading updated graphics card drivers from your manufacturer's website.\n"
	       "* there is a problem with your graphics card driver configuration. "
	       "Run DXDiag using the Run command (Start->Run, type \"dxdiag.exe\") and correct any problems reported there.";
}

void AGSWin32::DisplaySwitchOut() {
	// If we have explicitly set up fullscreen mode then minimize the window
	if (_preFullscreenMode.IsValid())
		ShowWindow(win_get_window(), SW_MINIMIZE);
}

void AGSWin32::DisplaySwitchIn() {
	// If we have explicitly set up fullscreen mode then restore the window
	if (_preFullscreenMode.IsValid())
		ShowWindow(win_get_window(), SW_RESTORE);
}

void AGSWin32::PauseApplication() {
#ifndef AGS_NO_VIDEO_PLAYER
	dxmedia_pause_video();
#endif
}

void AGSWin32::ResumeApplication() {
#ifndef AGS_NO_VIDEO_PLAYER
	dxmedia_resume_video();
#endif
}

void AGSWin32::GetSystemDisplayModes(std::vector<DisplayMode> &dms) {
	dms.clear();
	GFX_MODE_LIST *gmlist = get_gfx_mode_list(GFX_DIRECTX);
	for (int i = 0; i < gmlist->num_modes; ++i) {
		const GFX_MODE &m = gmlist->mode[i];
		dms.push_back(DisplayMode(GraphicResolution(m.width, m.height, m.bpp)));
	}
	destroy_gfx_mode_list(gmlist);
}

bool AGSWin32::SetSystemDisplayMode(const DisplayMode &dm, bool fullscreen) {
	DEVMODE devmode;
	memset(&devmode, 0, sizeof(devmode));
	devmode.dmSize = sizeof(devmode);
	devmode.dmPelsWidth = dm.Width;
	devmode.dmPelsHeight = dm.Height;
	devmode.dmBitsPerPel = dm.ColorDepth;
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	return ChangeDisplaySettings(&devmode, fullscreen ? CDS_FULLSCREEN : 0) == DISP_CHANGE_SUCCESSFUL;
}

bool AGSWin32::EnterFullscreenMode(const DisplayMode &dm) {
	// Remember current mode
	get_desktop_resolution(&_preFullscreenMode.Width, &_preFullscreenMode.Height);
	_preFullscreenMode.ColorDepth = desktop_color_depth();

	// Set requested desktop mode
	return SetSystemDisplayMode(dm, true);
}

bool AGSWin32::ExitFullscreenMode() {
	if (!_preFullscreenMode.IsValid())
		return false;

	DisplayMode dm = _preFullscreenMode;
	_preFullscreenMode = DisplayMode();
	return SetSystemDisplayMode(dm, false);
}

void AGSWin32::AdjustWindowStyleForFullscreen() {
	// Remove the border in full-screen mode
	Size sz;
	get_desktop_resolution(&sz.Width, &sz.Height);
	HWND allegro_wnd = win_get_window();
	LONG winstyle = GetWindowLong(allegro_wnd, GWL_STYLE);
	SetWindowLong(allegro_wnd, GWL_STYLE, (winstyle & ~WS_OVERLAPPEDWINDOW) | WS_POPUP);
	SetWindowPos(allegro_wnd, HWND_TOP, 0, 0, sz.Width, sz.Height, 0);
}

void AGSWin32::AdjustWindowStyleForWindowed() {
	// Make a regular window with a border
	HWND allegro_wnd = win_get_window();
	LONG winstyle = GetWindowLong(allegro_wnd, GWL_STYLE);
	SetWindowLong(allegro_wnd, GWL_STYLE, (winstyle & ~WS_POPUP) | (WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX));
	// Make window go on top, but at the same time remove WS_EX_TOPMOST style (applied by Direct3D fullscreen mode)
	SetWindowPos(allegro_wnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

int AGSWin32::CDPlayerCommand(int cmdd, int datt) {
#if defined (AGS_HAS_CD_AUDIO)
	return cd_player_control(cmdd, datt);
#else
	return -1;
#endif
}

void AGSWin32::AttachToParentConsole() {
	if (_isAttachedToParentConsole)
		return;

	_isAttachedToParentConsole = ::AttachConsole(ATTACH_PARENT_PROCESS) != FALSE;
	if (_isAttachedToParentConsole) {
		// Require that both STDOUT and STDERR are valid handles from the parent process.
		if (::GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE &&
		        ::GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE) {
			// Re-open STDOUT and STDERR to the parent's.
			FILE *fp = NULL;
			freopen_s(&fp, "CONOUT$", "w", stdout);
			setvbuf(stdout, NULL, _IONBF, 0);

			freopen_s(&fp, "CONOUT$", "w", stderr);
			setvbuf(stderr, NULL, _IONBF, 0);
		} else {
			::FreeConsole();
			_isAttachedToParentConsole = false;
		}
	}
}

void AGSWin32::DisplayAlert(const char *text, ...) {
	char displbuf[2500];
	va_list ap;
	va_start(ap, text);
	vsprintf(displbuf, text, ap);
	va_end(ap);
	if (_guiMode)
		MessageBox(win_get_window(), displbuf, "Adventure Game Studio", MB_OK | MB_ICONEXCLAMATION);

	// Always write to either stderr or stdout, even if message boxes are enabled.
	if (_logToStdErr)
		AGSWin32::WriteStdErr("%s", displbuf);
	else
		AGSWin32::WriteStdOut("%s", displbuf);
}

int AGSWin32::GetLastSystemError() {
	return ::GetLastError();
}

unsigned long AGSWin32::GetDiskFreeSpaceMB() {
	DWORD returnMb = 0;
	BOOL fResult;
	our_eip = -1891;

	// On Win9x, the last 3 params cannot be null, so need to supply values for all
	__int64 i64FreeBytesToCaller, i64Unused1, i64Unused2;

	// Win95 OSR2 or higher - use GetDiskFreeSpaceEx, since the
	// normal GetDiskFreeSpace returns erroneous values if the
	// free space is > 2 GB
	fResult = GetDiskFreeSpaceEx(NULL,
	                             (PULARGE_INTEGER)&i64FreeBytesToCaller,
	                             (PULARGE_INTEGER)&i64Unused1,
	                             (PULARGE_INTEGER)&i64Unused2);

	our_eip = -1893;

	// convert down to MB so we can fit it in a 32-bit long
	i64FreeBytesToCaller /= 1000000;
	returnMb = i64FreeBytesToCaller;

	return returnMb;
}

const char *AGSWin32::GetNoMouseErrorString() {
	return "No mouse was detected on your system, or your mouse is not configured to work with DirectInput. You must have a mouse to play this game.";
}

bool AGSWin32::IsMouseControlSupported(bool windowed) {
	return true; // supported for both fullscreen and windowed modes
}

const char *AGSWin32::GetAllegroFailUserHint() {
	return "Make sure you have DirectX 5 or above installed.";
}

eScriptSystemOSID AGSWin32::GetSystemOSID() {
	return eOS_Win;
}

int AGSWin32::InitializeCDPlayer() {
#if defined (AGS_HAS_CD_AUDIO)
	return cd_player_init();
#else
	return -1;
#endif
}

#ifndef AGS_NO_VIDEO_PLAYER

void AGSWin32::PlayVideo(const char *name, int skip, int flags) {

	char useloc[250];
	sprintf(useloc, "%s\\%s", ResPaths.DataDir.GetCStr(), name);

	bool useSound = true;
	if (flags >= 10) {
		flags -= 10;
		useSound = false;
	} else {
		// for some reason DirectSound can't be shared, so uninstall
		// allegro sound before playing the video
		shutdown_sound();
	}

	bool isError = false;
	if (Common::File::TestReadFile(useloc)) {
		isError = (gfxDriver->PlayVideo(useloc, useSound, (VideoSkipType)skip, (flags > 0)) == 0);
	} else {
		isError = true;
		sprintf(lastError, "File not found: %s", useloc);
	}

	if (isError) {
		// turn "Always display as speech" off, to make sure error
		// gets displayed correctly
		int oldalways = game.options[OPT_ALWAYSSPCH];
		game.options[OPT_ALWAYSSPCH] = 0;
		Display("Video playing error: %s", lastError);
		game.options[OPT_ALWAYSSPCH] = oldalways;
	}

	if (useSound) {
		// Restore sound system
		install_sound(usetup.digicard, usetup.midicard, NULL);
		if (usetup.mod_player)
			init_mod_player(NUM_MOD_DIGI_VOICES);
	}

	set_palette_range(palette, 0, 255, 0);
}

#endif

void AGSWin32::AboutToQuitGame() {
#ifndef AGS_NO_VIDEO_PLAYER
	dxmedia_abort_video();
#endif
}

void AGSWin32::PostAllegroExit() {
	// Release the timer setting
	timeEndPeriod(win32TimerPeriod);
}

SetupReturnValue AGSWin32::RunSetup(const ConfigTree &cfg_in, ConfigTree &cfg_out) {
	String version_str = String::FromFormat("Adventure Game Studio v%s setup", get_engine_version());
	return AGS::Engine::WinSetup(cfg_in, cfg_out, usetup.data_files_dir, version_str);
}

void AGSWin32::SetGameWindowIcon() {
	SetWinIcon();
}

void AGSWin32::WriteStdOut(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	if (_isDebuggerPresent) {
		// Add "AGS:" prefix when outputting to debugger, to make it clear that this
		// is a text from the program log
		char buf[STD_BUFFER_SIZE] = "AGS: ";
		vsnprintf(buf + 5, STD_BUFFER_SIZE - 5, fmt, ap);
		OutputDebugString(buf);
		OutputDebugString("\n");
	} else {
		vprintf(fmt, ap);
		printf("\n");
	}
	va_end(ap);
}

void AGSWin32::WriteStdErr(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	if (_isDebuggerPresent) {
		// Add "AGS:" prefix when outputting to debugger, to make it clear that this
		// is a text from the program log
		char buf[STD_BUFFER_SIZE] = "AGS ERR: ";
		vsnprintf(buf + 9, STD_BUFFER_SIZE - 9, fmt, ap);
		OutputDebugString(buf);
		OutputDebugString("\n");
	} else {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
	}
	va_end(ap);
}

void AGSWin32::ShutdownCDPlayer() {
	cd_exit();
}

extern "C" const unsigned char hw_to_mycode[256];

int AGSWin32::ConvertKeycodeToScanCode(int keycode) {
	// ** HIDEOUS HACK TO WORK AROUND ALLEGRO BUG
	// the key[] array is hardcoded to qwerty keyboards, so we
	// have to re-map it to make it work on other keyboard layouts
	keycode += ('a' - 'A');
	int vkey = VkKeyScan(keycode);
	int scancode = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
	if ((scancode >= 0) && (scancode < 256))
		keycode = hw_to_mycode[scancode];
	return keycode;
}

void AGSWin32::ValidateWindowSize(int &x, int &y, bool borderless) const {
	RECT wa_rc, nc_rc;
	// This is the size of the available workspace on user's desktop
	SystemParametersInfo(SPI_GETWORKAREA, 0, &wa_rc, 0);
	// This is the maximal size that OS can reliably resize the window to (including any frame)
	const Size max_win(GetSystemMetrics(SM_CXMAXTRACK), GetSystemMetrics(SM_CYMAXTRACK));
	// This is the size of window's non-client area (frame, caption, etc)
	HWND allegro_wnd = win_get_window();
	LONG winstyle = borderless ? WS_POPUP : WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
	LONG winstyle_al = GetWindowLong(allegro_wnd, GWL_STYLE);
	SetRectEmpty(&nc_rc);
	AdjustWindowRect(&nc_rc, winstyle, FALSE);
	// Limit the window's full size to the system's window size limit,
	// and limit window's client size to the work space (visible area)
	x = Math::Min(x, (int)(max_win.Width - (nc_rc.right - nc_rc.left)));
	y = Math::Min(y, (int)(max_win.Height - (nc_rc.bottom - nc_rc.top)));
	x = Math::Clamp(x, 1, (int)(wa_rc.right - wa_rc.left));
	y = Math::Clamp(y, 1, (int)(wa_rc.bottom - wa_rc.top));
}

bool AGSWin32::LockMouseToWindow() {
	RECT rc;
	HWND allegro_wnd = win_get_window();
	GetClientRect(allegro_wnd, &rc);
	ClientToScreen(allegro_wnd, (POINT *)&rc);
	ClientToScreen(allegro_wnd, (POINT *)&rc.right);
	--rc.right;
	--rc.bottom;
	return ::ClipCursor(&rc) != 0;
}

void AGSWin32::UnlockMouse() {
	::ClipCursor(NULL);
}

AGSPlatformDriver *AGSPlatformDriver::GetDriver() {
	if (instance == NULL)
		instance = new AGSWin32();
	return instance;
}


// *********** WINDOWS-SPECIFIC PLUGIN API FUNCTIONS *************

HWND IAGSEngine::GetWindowHandle() {
	return win_get_window();
}
LPDIRECTDRAW2 IAGSEngine::GetDirectDraw2() {
	if (directdraw == NULL)
		quit("!This plugin requires DirectDraw based graphics driver (software driver).");

	return directdraw;
}
LPDIRECTDRAWSURFACE2 IAGSEngine::GetBitmapSurface(BITMAP *bmp) {
	if (directdraw == NULL)
		quit("!This plugin requires DirectDraw based graphics driver (software driver).");

	BMP_EXTRA_INFO *bei = (BMP_EXTRA_INFO *)bmp->extra;

	if (bmp == gfxDriver->GetMemoryBackBuffer()->GetAllegroBitmap())
		invalidate_screen();

	return bei->surf;
}

LPDIRECTSOUND IAGSEngine::GetDirectSound() {
	return directsound;
}

LPDIRECTINPUTDEVICE IAGSEngine::GetDirectInputKeyboard() {
	return key_dinput_device;
}

LPDIRECTINPUTDEVICE IAGSEngine::GetDirectInputMouse() {
	return mouse_dinput_device;
}

} // namespace AGS3

#endif
