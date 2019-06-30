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

#include "backends/platform/sdl/win32/win32-window.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void SdlWindow_Win32::setupIcon() {
	HMODULE handle = GetModuleHandle(NULL);
	HICON   ico    = LoadIcon(handle, MAKEINTRESOURCE(1001 /* IDI_ICON */));
	if (ico) {
		HWND hwnd = getHwnd();
		if (hwnd) {
			// Replace the handle to the icon associated with the window class by our custom icon
			SetClassLongPtr(hwnd, GCLP_HICON, (ULONG_PTR)ico);

			// Since there wasn't any default icon, we can't use the return value from SetClassLong
			// to check for errors (it would be 0 in both cases: error or no previous value for the
			// icon handle). Instead we check for the last-error code value.
			if (GetLastError() == ERROR_SUCCESS)
				return;
		}
	}

	// If no icon has been set, fallback to default path
	SdlWindow::setupIcon();
}

HWND SdlWindow_Win32::getHwnd() {
	SDL_SysWMinfo wminfo;
	if (getSDLWMInformation(&wminfo)) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		return wminfo.info.win.window;
#else
		return wminfo.window;
#endif
	}
	return NULL;
}

#endif
