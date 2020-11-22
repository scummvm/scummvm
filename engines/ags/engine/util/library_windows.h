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

#ifndef AGS_ENGINE_UTIL_LIBRARY_WINDOWS_H
#define AGS_ENGINE_UTIL_LIBRARY_WINDOWS_H

#include "debug/out.h"
#include "platform/windows/winapi_exclusive.h"
#include "util/string.h"

namespace AGS3 {

// Because this class may be exposed to generic code in sake of inlining,
// we should avoid including <windows.h> full of macros with common names.
#ifdef __cplusplus
extern "C" {
#endif

	WINBASEAPI BOOL WINAPI FreeLibrary(HMODULE hLibModule);
	WINBASEAPI FARPROC WINAPI GetProcAddress(HMODULE hModule, LPCSTR lpProcName);
	WINBASEAPI HMODULE WINAPI LoadLibraryA(LPCSTR lpLibFileName);

#ifdef __cplusplus
} // extern "C"
#endif


namespace AGS {
namespace Engine {


class WindowsLibrary : BaseLibrary {
public:
	WindowsLibrary()
		: _library(NULL) {
	};

	virtual ~WindowsLibrary() {
		Unload();
	};

	AGS::Shared::String BuildFilename(AGS::Shared::String libraryName) {
		return String::FromFormat("%s.dll", libraryName.GetCStr());
	}

	AGS::Shared::String BuildPath(AGS::Shared::String libraryName) {
		AGS::Shared::String platformLibraryName = BuildFilename(libraryName);

		AGS::Shared::Debug::Printf("Built library path: %s", platformLibraryName.GetCStr());

		return platformLibraryName;
	}

	AGS::Shared::String GetFilenameForLib(AGS::Shared::String libraryName) override {
		return BuildFilename(libraryName);
	}

	bool Load(AGS::Shared::String libraryName) {
		Unload();

		_library = LoadLibraryA(BuildPath(libraryName).GetCStr());

		return (_library != NULL);
	}

	bool Unload() {
		if (_library) {
			return (FreeLibrary(_library) != 0);
		} else {
			return true;
		}
	}

	void *GetFunctionAddress(AGS::Shared::String functionName) {
		return GetProcAddress(_library, functionName.GetCStr());
	}

private:
	HANDLE _library;
};


typedef WindowsLibrary Library;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
