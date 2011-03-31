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
 * Original code from EcWin7 - Copyright (C) 2010 Emanuele Colombo
 * https://code.google.com/p/dukto/
 */

#ifdef WIN32

// Needed for taskbar functions
#include <SDKDDKVer.h>
#include <shlobj.h>

// For Bitmap and overlay icons
#include <gdiplus.h>
using namespace Gdiplus;

// For HWND
#include <SDL_syswm.h>

#include "backends/taskbar/win32/win32-taskbar.h"

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "common/file.h"

// System.Title property key, values taken from http://msdn.microsoft.com/en-us/library/bb787584.aspx
const PROPERTYKEY PKEY_Title = { /* fmtid = */ { 0xF29F85E0, 0x4FF9, 0x1068, { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 } }, /* propID = */ 2 };

Win32TaskbarManager::Win32TaskbarManager() {
	_taskbar = NULL;
}

Win32TaskbarManager::~Win32TaskbarManager() {
	if (_taskbar)
		_taskbar->Release();
	_taskbar = NULL;

	CoUninitialize();
}

void Win32TaskbarManager::init() {
	// Do nothing if not running on Windows 7 of later
	if (!isWin7OrLater())
		return;

	CoInitialize(NULL);

	// Try creating instance (on fail, _taskbar will contain NULL)
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList,
	                              0,
	                              CLSCTX_INPROC_SERVER,
	                              IID_ITaskbarList3,
	                              reinterpret_cast<void**> (&(_taskbar)));

	if (SUCCEEDED(hr)) {
		// Initialize taskbar object
		if (FAILED(_taskbar->HrInit())) {
			_taskbar->Release();
			_taskbar = NULL;
		}
	} else {
		warning("[Win32TaskbarManager::init] Cannot create taskbar instance");
	}
}

void Win32TaskbarManager::setOverlayIcon(const Common::String &name, const Common::String &description) {
	if (_taskbar == NULL)
		return;

	if (name.empty()) {
		_taskbar->SetOverlayIcon(getHwnd(), NULL, L"");
		return;
	}

	// Compute full icon path
	Common::String path = getIconPath(name);
	if (path.empty())
		return;

	HICON pIcon = (HICON)::LoadImage(NULL, path.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);

	// Sets the overlay icon
	LPWSTR desc = ansiToUnicode(description.c_str());
	_taskbar->SetOverlayIcon(getHwnd(), pIcon, desc);

	DestroyIcon(pIcon);

	delete[] desc;
}

void Win32TaskbarManager::setProgressValue(int completed, int total) {
	if (_taskbar == NULL)
		return;

	_taskbar->SetProgressValue(getHwnd(), completed, total);
}

void Win32TaskbarManager::setProgressState(TaskbarProgressState state) {
	if (_taskbar == NULL)
		return;

	_taskbar->SetProgressState(getHwnd(), (TBPFLAG)state);
}

void Win32TaskbarManager::addRecent(const Common::String &name, const Common::String &description) {
	if (_taskbar == NULL)
		return;

	// ANSI version doesn't seem to work correctly with Win7 jump lists, so explicitly use Unicode interface.
	IShellLinkW *link;

	// Get the ScummVM executable path.
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);

	// Create a shell link.
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&link)))) {
		// Convert game name and description to Unicode.
		LPWSTR game = ansiToUnicode(name.c_str());
		LPWSTR desc = ansiToUnicode(description.c_str());

		// Set link properties.
		link->SetPath(path);
		link->SetArguments(game);

		Common::String iconPath = getIconPath(name);
		if (iconPath.empty()) {
			link->SetIconLocation(path, 0); // No game-specific icon available
		} else {
			LPWSTR icon = ansiToUnicode(iconPath.c_str());

			link->SetIconLocation(icon, 0);

			delete[] icon;
		}

		// The link's display name must be set via property store.
		IPropertyStore* propStore;
		HRESULT hr = link->QueryInterface(&propStore);
		if (SUCCEEDED(hr)) {
			PROPVARIANT pv;
			pv.vt = VT_LPWSTR;
			pv.pwszVal = desc;

			hr = propStore->SetValue(PKEY_Title, pv);

			propStore->Commit();
			propStore->Release();
		}

		// SHAddToRecentDocs will cause the games to be added to the Recent list, allowing the
		// user to pin them.
		SHAddToRecentDocs(SHARD_LINK, link);
		link->Release();
		delete[] game;
		delete[] desc;
	}
}

Common::String Win32TaskbarManager::getIconPath(Common::String target) {
	// Get extra path
	Common::String extra = ConfMan.get("extrapath");

	Common::String filename = target + ".ico";
	Common::String path = extra + filename;

	if (!Common::File::exists(filename)) {
		// Try with the game id instead of the domain name
		filename = ConfMan.get("gameid") + ".ico";

		if (!Common::File::exists(filename))
			return "";
	}

	return extra + filename;
}

bool Win32TaskbarManager::isWin7OrLater() {
   OSVERSIONINFOEX versionInfo;
   DWORDLONG conditionMask = 0;

   ZeroMemory(&versionInfo, sizeof(OSVERSIONINFOEX));
   versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
   versionInfo.dwMajorVersion = 6;
   versionInfo.dwMinorVersion = 1;

   VER_SET_CONDITION(conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
   VER_SET_CONDITION(conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);

   return VerifyVersionInfo(&versionInfo, VER_MAJORVERSION | VER_MINORVERSION, conditionMask);
}

LPWSTR Win32TaskbarManager::ansiToUnicode(const char *s) {
	DWORD size = MultiByteToWideChar(0, 0, s, -1, NULL, 0);

	if (size > 0) {
		LPWSTR result = new WCHAR[size];
		if (MultiByteToWideChar(0, 0, s, -1, result, size) != 0)
			return result;
	}

	return NULL;
}

HWND Win32TaskbarManager::getHwnd() {
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);

	if(!SDL_GetWMInfo(&wmi))
		return NULL;

	return wmi.window;
}

#endif
