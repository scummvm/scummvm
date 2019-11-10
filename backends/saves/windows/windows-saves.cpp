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

#if defined(WIN32) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#if defined(__GNUC__) && defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
 // required for SHGFP_TYPE_CURRENT in shlobj.h
#define _WIN32_IE 0x500
#endif
#include <shlobj.h>

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "backends/saves/windows/windows-saves.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

WindowsSaveFileManager::WindowsSaveFileManager() {
	char defaultSavepath[MAXPATHLEN];


	// Use the Application Data directory of the user profile.
	if (SHGetFolderPathFunc(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, defaultSavepath) == S_OK) {
		strcat(defaultSavepath, "\\ScummVM");
		if (!CreateDirectory(defaultSavepath, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				error("Cannot create ScummVM application data folder");
		}

		strcat(defaultSavepath, "\\Saved games");
		if (!CreateDirectory(defaultSavepath, NULL)) {
			if (GetLastError() != ERROR_ALREADY_EXISTS)
				error("Cannot create ScummVM Saved games folder");
		}

		ConfMan.registerDefault("savepath", defaultSavepath);
	} else {
		warning("Unable to access application data directory");
	}
}

#endif
