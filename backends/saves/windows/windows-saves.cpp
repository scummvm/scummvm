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

#if defined(WIN32) && !defined(DISABLE_DEFAULT_SAVEFILEMANAGER)

// For _tcscat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <errno.h>

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "backends/saves/windows/windows-saves.h"
#include "backends/platform/sdl/win32/win32_wrapper.h"

WindowsSaveFileManager::WindowsSaveFileManager(bool isPortable) {
	TCHAR defaultSavepath[MAX_PATH];

	if (isPortable) {
		Win32::getProcessDirectory(defaultSavepath, MAX_PATH);
	} else {
		// Use the Application Data directory of the user profile
		if (!Win32::getApplicationDataDirectory(defaultSavepath)) {
			return;
		}
	}

	_tcscat(defaultSavepath, TEXT("\\Saved games"));
	if (!CreateDirectory(defaultSavepath, nullptr)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS)
			error("Cannot create ScummVM Saved games folder");
	}

	ConfMan.registerDefault("savepath", Common::Path(Win32::tcharToString(defaultSavepath), Common::Path::kNativeSeparator));
}

Common::ErrorCode WindowsSaveFileManager::removeFile(const Common::FSNode &fileNode) {
	TCHAR *tFile = Win32::stringToTchar(fileNode.getPath().toString(Common::Path::kNativeSeparator));
	int result = _tremove(tFile);
	free(tFile);
	if (result == 0)
		return Common::kNoError;
	if (errno == EACCES)
		return Common::kWritePermissionDenied;
	if (errno == ENOENT)
		return Common::kPathDoesNotExist;
	return Common::kUnknownError;
}

#endif
