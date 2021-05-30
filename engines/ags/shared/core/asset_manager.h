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

//=============================================================================
//
// Asset manager class for reading and writing game resources.
//-----------------------------------------------------------------------------
//
// The code is based on CLIB32, by Chris Jones (1998-99), DJGPP implementation
// of the CLIB reader.
//
//-----------------------------------------------------------------------------
// TODO: consider replace/merge with PhysFS library in the future.
//
// TODO: support streams that work on a file subsection, limited by size,
// to avoid having to return an asset size separately from a stream.
// TODO: return stream as smart pointer.
//
//=============================================================================

#ifndef AGS_SHARED_CORE_ASSET_MANAGER_H
#define AGS_SHARED_CORE_ASSET_MANAGER_H

#include "common/stream.h"
#include "ags/lib/std/functional.h"
#include "ags/lib/std/memory.h"
#include "ags/shared/core/asset.h"
#include "ags/shared/util/file.h" // TODO: extract filestream mode constants or introduce generic ones

namespace AGS3 {
namespace AGS {
namespace Shared {

class Stream;
struct MultiFileLib;

enum AssetSearchPriority {
	kAssetPriorityDir,
	kAssetPriorityLib
};

enum AssetError {
	kAssetNoError = 0,
	kAssetErrNoLibFile = -1, // library file not found or can't be read
	kAssetErrLibParse = -2, // bad library file format or read error
	kAssetErrNoManager = -6, // asset manager not initialized
};

// Explicit location of asset data
struct AssetLocation {
	String      FileName;   // file where asset is located
	soff_t      Offset;     // asset's position in file (in bytes)
	soff_t      Size;       // asset's size (in bytes)

	AssetLocation();
};


class AssetManager {
public:
	AssetManager();
	~AssetManager() {
		RemoveAllLibraries();
	}

	// Test if given file is main data file
	static bool         IsDataFile(const String &data_file);
	// Read data file table of contents into provided struct
	static AssetError   ReadDataFileTOC(const String &data_file, AssetLibInfo &lib);

	// Sets asset search priority (in which order manager will search available locations)
	void         SetSearchPriority(AssetSearchPriority priority);
	// Gets current asset search priority
	AssetSearchPriority GetSearchPriority() const;

	// Add library location to the list of asset locations
	AssetError   AddLibrary(const String &path, const AssetLibInfo **lib = nullptr);
	// Add library location, specifying comma-separated list of filters
	AssetError   AddLibrary(const String &path, const String &filters, const AssetLibInfo **lib = nullptr);
	// Remove library location from the list of asset locations
	void         RemoveLibrary(const String &path);
	// Removes all libraries
	void         RemoveAllLibraries();

	size_t       GetLibraryCount() const;
	const AssetLibInfo *GetLibraryInfo(size_t index) const;
	// Tells whether asset exists in any of the registered search locations
	bool         DoesAssetExist(const String &asset_name, const String &filter = "") const;
	// Finds asset only looking for bare files in directories; returns full path or empty string if failed
	String       FindAssetFileOnly(const String &asset_name, const String &filter = "") const;
	// Open asset stream in the given work mode; returns null if asset is not found or cannot be opened
	// This method only searches in libraries that do not have any defined filters
	Stream *OpenAsset(const String &asset_name, soff_t *asset_size = nullptr,
					  FileOpenMode open_mode = kFile_Open,
					  FileWorkMode work_mode = kFile_Read) const;
	// Open asset stream, providing a single filter to search in matching libraries
	Stream *OpenAsset(const String &asset_name, const String &filter, soff_t *asset_size = nullptr,
					  FileOpenMode open_mode = kFile_Open,
					  FileWorkMode work_mode = kFile_Read) const;
	// Open asset stream in the given work mode; returns null if asset is not found or cannot be opened
	// This method only searches in libraries that do not have any defined filters
	Common::SeekableReadStream *OpenAssetStream(const String &asset_name) const;
	// Open asset stream, providing a single filter to search in matching libraries
	Common::SeekableReadStream *OpenAssetStream(const String &asset_name, const String &filter) const;

private:
	struct AssetLibEx : AssetLibInfo {
		std::vector<String> Filters; // asset filters this library is matching to
	};

	// Loads library and registers its contents into the cache
	AssetError  RegisterAssetLib(const String &path, AssetLibEx *&lib);

	// Tries to find asset in known locations, tests if it's possible to open, and fills in AssetLocation
	bool        GetAsset(const String &asset_name, const String &filter, bool dir_only, AssetLocation *loc, Shared::FileOpenMode open_mode, Shared::FileWorkMode work_mode) const;
	bool        GetAssetFromLib(const AssetLibInfo *lib, const String &asset_name, AssetLocation *loc, Shared::FileOpenMode open_mode, Shared::FileWorkMode work_mode) const;
	bool        GetAssetFromDir(const AssetLibInfo *lib, const String &asset_name, AssetLocation *loc, Shared::FileOpenMode open_mode, Shared::FileWorkMode work_mode) const;

	std::vector<AssetLibEx *> _libs;
	std::vector<AssetLibEx *> _activeLibs;

	struct LibsByPriority {
		AssetSearchPriority Priority = kAssetPriorityDir;

		bool operator()(const AssetLibInfo *x, const AssetLibInfo *y) const;
	} _libsByPriority;
};


String GetAssetErrorText(AssetError err);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
