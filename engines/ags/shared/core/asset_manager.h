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
#include "common/std/functional.h"
#include "common/std/memory.h"
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

/**
 * AssetPath combines asset name and optional library filter, that serves to narrow down the search
 */
struct AssetPath {
	String Name;
	String Filter;

	AssetPath(const String &name = "", const String &filter = "") : Name(name), Filter(filter) {
	}
};

class AssetManager {
public:
	AssetManager() = default;
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
	// Add library location, specifying comma-separated list of filters;
	// if library was already added before, this method will overwrite the filters only
	AssetError   AddLibrary(const String &path, const String &filters, const AssetLibInfo **lib = nullptr);
	// Remove library location from the list of asset locations
	void         RemoveLibrary(const String &path);
	// Removes all libraries
	void         RemoveAllLibraries();

	size_t       GetLibraryCount() const;
	const AssetLibInfo *GetLibraryInfo(size_t index) const;
	// Tells whether asset exists in any of the registered search locations
	bool         DoesAssetExist(const String &asset_name, const String &filter = "") const;
	inline bool  DoesAssetExist(const AssetPath &apath) const {
		return DoesAssetExist(apath.Name, apath.Filter);
	}
	// Searches in all the registered locations and collects a list of
    // assets using given wildcard pattern
    void FindAssets(std::vector<String> &assets, const String &wildcard,
		const String &filter = "") const;
	// Open asset stream in the given work mode; returns null if asset is not found or cannot be opened
	// This method only searches in libraries that do not have any defined filters
	Stream *OpenAsset(const String &asset_name) const;
	// Open asset stream, providing a single filter to search in matching libraries
	Stream *OpenAsset(const String &asset_name, const String &filter) const;
	inline Stream *OpenAsset(const AssetPath &apath) const {
		return OpenAsset(apath.Name, apath.Filter);
	}
	// Open asset stream in the given work mode; returns null if asset is not found or cannot be opened
	// This method only searches in libraries that do not have any defined filters
	Common::SeekableReadStream *OpenAssetStream(const String &asset_name) const;
	// Open asset stream, providing a single filter to search in matching libraries
	Common::SeekableReadStream *OpenAssetStream(const String &asset_name, const String &filter) const;

private:
	// AssetLibEx combines library info with extended internal data required for the manager
	struct AssetLibEx : AssetLibInfo {
		std::vector<String> Filters; // asset filters this library is matching to
		std::vector<String> RealLibFiles; // fixed up library filenames

		bool TestFilter(const String &filter) const;
	};

	// Loads library and registers its contents into the cache
	AssetError  RegisterAssetLib(const String &path, AssetLibEx *&lib);

	// Tries to find asset in the given location, and then opens a stream for reading
	Stream *OpenAssetFromLib(const AssetLibEx *lib, const String &asset_name) const;
	Stream *OpenAssetFromDir(const AssetLibEx *lib, const String &asset_name) const;

	std::vector<AssetLibEx *> _libs;
	std::vector<AssetLibEx *> _activeLibs;

	AssetSearchPriority _libsPriority = kAssetPriorityDir;
	// Sorting function, depends on priority setting
	bool (*_libsSorter)(const AssetLibInfo *, const AssetLibInfo *);
};

String GetAssetErrorText(AssetError err);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
