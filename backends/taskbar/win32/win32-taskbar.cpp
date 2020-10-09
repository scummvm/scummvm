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

// We cannot use common/scummsys.h directly as it will include
// windows.h and we need to do it by hand to allow excluded functions
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(WIN32) && defined(USE_TASKBAR)

// HACK: To get __MINGW64_VERSION_foo defines we need to manually include
// _mingw.h in this file because we do not include any system headers at this
// point on purpose. The defines are required to detect whether this is a
// classic MinGW toolchain or a MinGW-w64 based one.
#if defined(__MINGW32__)
#include <_mingw.h>
#endif

// Needed for taskbar functions
// HACK: MinGW-w64 based toolchains include the symbols we require in their
// headers. The 32 bit incarnation only defines __MINGW32__. This leads to
// build breakage due to clashes with our compat header. Luckily MinGW-w64
// based toolchains define __MINGW64_VERSION_foo macros inside _mingw.h,
// which is included from all system headers. Thus we abuse that to detect
// them.
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
	#include "backends/taskbar/win32/mingw-compat.h"
#else
	// We use functionality introduced with Win7 in this file.
	// To assure that including the respective system headers gives us all
	// required definitions we set Win7 as minimum version we target.
	// See: https://msdn.microsoft.com/en-us/library/windows/desktop/aa383745%28v=vs.85%29.aspx#macros_for_conditional_declarations
	#include <sdkddkver.h>
	#undef _WIN32_WINNT
	#define _WIN32_WINNT _WIN32_WINNT_WIN7

	#include <windows.h>
#endif

#include <shlobj.h>

#include "common/scummsys.h"

#include "backends/taskbar/win32/win32-taskbar.h"
#include "backends/platform/sdl/win32/win32-window.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

#include "common/textconsole.h"

// System.Title property key, values taken from http://msdn.microsoft.com/en-us/library/bb787584.aspx
const PROPERTYKEY PKEY_Title = { /* fmtid = */ { 0xF29F85E0, 0x4FF9, 0x1068, { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 } }, /* propID = */ 2 };

Win32TaskbarManager::Win32TaskbarManager(SdlWindow_Win32 *window) : _window(window), _taskbar(NULL), _count(0), _icon(NULL) {
	// Do nothing if not running on Windows 7 or later
	if (!Win32::confirmWindowsVersion(6, 1))
		return;

	CoInitialize(NULL);

	// Try creating instance (on fail, _taskbar will contain NULL)
	HRESULT hr = CoCreateInstance(CLSID_TaskbarList,
	                              0,
	                              CLSCTX_INPROC_SERVER,
	                              IID_ITaskbarList3,
	                              reinterpret_cast<void **> (&(_taskbar)));

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

Win32TaskbarManager::~Win32TaskbarManager() {
	if (_taskbar)
		_taskbar->Release();
	_taskbar = NULL;

	if (_icon)
		DestroyIcon(_icon);

	CoUninitialize();
}

void Win32TaskbarManager::setOverlayIcon(const Common::String &name, const Common::String &description) {
	//warning("[Win32TaskbarManager::setOverlayIcon] Setting overlay icon to: %s (%s)", name.c_str(), description.c_str());

	if (_taskbar == NULL)
		return;

	if (name.empty()) {
		_taskbar->SetOverlayIcon(_window->getHwnd(), NULL, L"");
		return;
	}

	// Compute full icon path
	Common::String path = getIconPath(name, ".ico");
	if (path.empty())
		return;

	HICON pIcon = (HICON)::LoadImage(NULL, path.c_str(), IMAGE_ICON, 16, 16, LR_LOADFROMFILE);
	if (!pIcon) {
		warning("[Win32TaskbarManager::setOverlayIcon] Cannot load icon!");
		return;
	}

	// Sets the overlay icon
	LPWSTR desc = Win32::ansiToUnicode(description.c_str());
	_taskbar->SetOverlayIcon(_window->getHwnd(), pIcon, desc);

	DestroyIcon(pIcon);

	free(desc);
}

void Win32TaskbarManager::setProgressValue(int completed, int total) {
	if (_taskbar == NULL)
		return;

	_taskbar->SetProgressValue(_window->getHwnd(), completed, total);
}

void Win32TaskbarManager::setProgressState(TaskbarProgressState state) {
	if (_taskbar == NULL)
		return;

	_taskbar->SetProgressState(_window->getHwnd(), (TBPFLAG)state);
}

void Win32TaskbarManager::setCount(int count) {
	if (_taskbar == NULL)
		return;

	if (count == 0) {
		_taskbar->SetOverlayIcon(_window->getHwnd(), NULL, L"");
		return;
	}

	// FIXME: This isn't really nice and could use a cleanup.
	//        The only good thing is that it doesn't use GDI+
	//        and thus does not have a dependancy on it,
	//        with the downside of being a lot more ugly.
	//        Maybe replace it by a Graphic::Surface, use
	//        ScummVM font drawing and extract the contents at
	//        the end?

	if (_count != count || _icon == NULL) {
		// Cleanup previous icon
		_count = count;
		if (_icon)
			DestroyIcon(_icon);

		Common::String countString = (count < 100 ? Common::String::format("%d", count) : "9+");

		// Create transparent background
		BITMAPV5HEADER bi;
		ZeroMemory(&bi, sizeof(BITMAPV5HEADER));
		bi.bV5Size        = sizeof(BITMAPV5HEADER);
		bi.bV5Width       = 16;
		bi.bV5Height      = 16;
		bi.bV5Planes      = 1;
		bi.bV5BitCount    = 32;
		bi.bV5Compression = BI_RGB;
		// Set 32 BPP alpha format
		bi.bV5RedMask     = 0x00FF0000;
		bi.bV5GreenMask   = 0x0000FF00;
		bi.bV5BlueMask    = 0x000000FF;
		bi.bV5AlphaMask   = 0xFF000000;

		// Get DC
		HDC hdc;
		hdc = GetDC(NULL);
		HDC hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(NULL, hdc);

		// Create a bitmap mask
		HBITMAP hBitmapMask = CreateBitmap(16, 16, 1, 1, NULL);

		// Create the DIB section with an alpha channel
		void *lpBits;
		HBITMAP hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, 0);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

		// Load the icon background
		HICON hIconBackground = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1002 /* IDI_COUNT */));
		DrawIconEx(hMemDC, 0, 0, hIconBackground, 16, 16, 0, 0, DI_NORMAL);
		DeleteObject(hIconBackground);

		// Draw the count
		LOGFONT lFont;
		memset(&lFont, 0, sizeof(LOGFONT));
		lFont.lfHeight = 10;
		lFont.lfWeight = FW_BOLD;
		lFont.lfItalic = 1;
		strcpy(lFont.lfFaceName, "Arial");

		HFONT hFont = CreateFontIndirect(&lFont);
		SelectObject(hMemDC, hFont);

		RECT rect;
		SetRect(&rect, 4, 4, 12, 12);
		SetTextColor(hMemDC, RGB(48, 48, 48));
		SetBkMode(hMemDC, TRANSPARENT);
		DrawText(hMemDC, countString.c_str(), -1, &rect, DT_NOCLIP|DT_CENTER);

		// Set the text alpha to fully opaque (we consider the data inside the text rect)
		DWORD *lpdwPixel = (DWORD *)lpBits;
		for (int x = 3; x < 12; x++) {
			for(int y = 3; y < 12; y++) {
				unsigned char *p = (unsigned char *)(lpdwPixel + x * 16 + y);

				if (p[0] != 0 && p[1] != 0 && p[2] != 0)
					p[3] = 255;
			}
		}

		// Cleanup DC
		DeleteObject(hFont);
		SelectObject(hMemDC, hOldBitmap);
		DeleteDC(hMemDC);

		// Prepare our new icon
		ICONINFO ii;
		ii.fIcon    = FALSE;
		ii.xHotspot = 0;
		ii.yHotspot = 0;
		ii.hbmMask  = hBitmapMask;
		ii.hbmColor = hBitmap;

		_icon = CreateIconIndirect(&ii);

		DeleteObject(hBitmap);
		DeleteObject(hBitmapMask);

		if (!_icon) {
			warning("[Win32TaskbarManager::setCount] Cannot create icon for count");
			return;
		}
	}

	// Sets the overlay icon
	LPWSTR desc = Win32::ansiToUnicode(Common::String::format("Found games: %d", count).c_str());
	_taskbar->SetOverlayIcon(_window->getHwnd(), _icon, desc);
	free(desc);
}

void Win32TaskbarManager::addRecent(const Common::String &name, const Common::String &description) {
	//warning("[Win32TaskbarManager::addRecent] Adding recent list entry: %s (%s)", name.c_str(), description.c_str());

	if (_taskbar == NULL)
		return;

	// ANSI version doesn't seem to work correctly with Win7 jump lists, so explicitly use Unicode interface.
	IShellLinkW *link;

	// Get the ScummVM executable path.
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);

	// Create a shell link.
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC, IID_IShellLinkW, reinterpret_cast<void **> (&link)))) {
		// Convert game name and description to Unicode.
		LPWSTR game = Win32::ansiToUnicode(name.c_str());
		LPWSTR desc = Win32::ansiToUnicode(description.c_str());

		// Set link properties.
		link->SetPath(path);
		link->SetArguments(game);

		Common::String iconPath = getIconPath(name, ".ico");
		if (iconPath.empty()) {
			link->SetIconLocation(path, 0); // No game-specific icon available
		} else {
			LPWSTR icon = Win32::ansiToUnicode(iconPath.c_str());

			link->SetIconLocation(icon, 0);

			free(icon);
		}

		// The link's display name must be set via property store.
		IPropertyStore* propStore;
		HRESULT hr = link->QueryInterface(IID_IPropertyStore, reinterpret_cast<void **> (&(propStore)));
		if (SUCCEEDED(hr)) {
			PROPVARIANT pv;
			pv.vt = VT_LPWSTR;
			pv.pwszVal = desc;

			hr = propStore->SetValue(PKEY_Title, pv);

			propStore->Commit();
			propStore->Release();
		}

		// SHAddToRecentDocs will cause the games to be added to the Recent list, allowing the user to pin them.
		SHAddToRecentDocs(SHARD_LINK, link);
		link->Release();
		free(game);
		free(desc);
	}
}

void Win32TaskbarManager::notifyError() {
	setProgressState(Common::TaskbarManager::kTaskbarError);
	setProgressValue(1, 1);
}

void Win32TaskbarManager::clearError() {
	setProgressState(kTaskbarNoProgress);
}

#endif
