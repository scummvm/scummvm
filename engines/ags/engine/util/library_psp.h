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

#ifndef AGS_ENGINE_UTIL_LIBRARY_PSP_H
#define AGS_ENGINE_UTIL_LIBRARY_PSP_H

//include <pspsdk.h>
#include "ags/shared/util/string.h"
#include "ags/shared/debugging/out.h"

namespace AGS3 {
namespace AGS {
namespace Engine {

class PSPLibrary : BaseLibrary {
public:
	PSPLibrary()
		: _library(-1) {
	};

	virtual ~PSPLibrary() {
		Unload();
	};

	AGS::Shared::String BuildPath(char *path, AGS::Shared::String libraryName) {
		AGS::Shared::String platformLibraryName = path;
		platformLibraryName.Append(libraryName);
		platformLibraryName.Append(".prx");

		AGS::Shared::Debug::Printf("Built library path: %s", platformLibraryName.GetCStr());

		return platformLibraryName;
	}

	bool Load(AGS::Shared::String libraryName) {
		Unload();

		// Try current path
		_library = pspSdkLoadStartModule(BuildPath("./", libraryName).GetCStr(), PSP_MEMORY_PARTITION_USER);

		if (_library < 0) {
			// Try one directory higher, usually the AGS base directory
			_library = pspSdkLoadStartModule(BuildPath("../", libraryName).GetCStr(), PSP_MEMORY_PARTITION_USER);
		}

		// The PSP module and PSP library name are assumed to be the same as the file name
		_moduleName = libraryName;
		_moduleName.MakeLower();

		AGS::Shared::Debug::Printf("Result is %s %d", _moduleName.GetCStr(), _library);

		return (_library > -1);
	}

	bool Unload() {
		if (_library > -1) {
			return (sceKernelUnloadModule(_library) > -1);
		} else {
			return true;
		}
	}

	void *GetFunctionAddress(AGS::Shared::String functionName) {
		if (_library > -1) {
			// On the PSP functions are identified by an ID that is the first 4 byte of the SHA1 of the name.
			int functionId;

#if 1
			// Hardcoded values for plugin loading.
			if (functionName == "AGS_PluginV2") {
				functionId = 0x960C49BD;
			} else if (functionName == "AGS_EngineStartup") {
				functionId = 0x0F13D9E8;
			} else if (functionName == "AGS_EngineShutdown") {
				functionId = 0x2F131C76;
			} else if (functionName == "AGS_EngineOnEvent") {
				functionId = 0xE3DFFC5A;
			} else if (functionName == "AGS_EngineDebugHook") {
				functionId = 0xC37D6879;
			} else if (functionName == "AGS_EngineInitGfx") {
				functionId = 0xA428D254;
			} else {
				AGS::Shared::Debug::Printf("Function ID not found: %s", functionName.GetCStr());
				functionId = -1;
			}
#else
			// This is a possible SHA1 implementation.
			SceKernelUtilsSha1Context ctx;
			uint8_t digest[20];
			sceKernelUtilsSha1BlockInit(&ctx);
			sceKernelUtilsSha1BlockUpdate(&ctx, (uint8_t *)functionName.GetCStr(), functionName.GetLength());
			sceKernelUtilsSha1BlockResult(&ctx, digest);

			functionId = strtol(digest, NULL, 8);
#endif

			return (void *)kernel_sctrlHENFindFunction((char *)_moduleName.GetCStr(), (char *)_moduleName.GetCStr(), functionId);
		} else {
			return NULL;
		}
	}

private:
	SceUID _library;
	AGS::Shared::String _moduleName;
};


typedef PSPLibrary Library;

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
