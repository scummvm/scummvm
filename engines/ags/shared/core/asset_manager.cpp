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

#include "common/memstream.h"
#include "common/std/algorithm.h"
#include "common/std/utility.h"
#include "ags/shared/core/platform.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/multi_file_lib.h"
#include "ags/shared/util/path.h"
#include "ags/shared/util/string_utils.h" // cbuf_to_string_and_free

namespace AGS3 {
namespace AGS {
namespace Shared {

inline static bool IsAssetLibDir(const AssetLibInfo *lib) {
	return lib->BaseFileName.IsEmpty();
}

bool AssetManager::AssetLibEx::TestFilter(const String &filter) const {
	return filter == "*" ||
		(std::find(Filters.begin(), Filters.end(), filter) != Filters.end());
}

// Asset library sorting function, directories have priority
bool SortLibsPriorityDir(const AssetLibInfo *lib1, const AssetLibInfo *lib2) {
	// first element is less if it's a directory while second is a lib
	return IsAssetLibDir(lib1) && !IsAssetLibDir(lib2);
}

// Asset library sorting function, packages have priority
bool SortLibsPriorityLib(const AssetLibInfo *lib1, const AssetLibInfo *lib2) {
	// first element is less if it's a lib while second is a directory
	return !IsAssetLibDir(lib1) && IsAssetLibDir(lib2);
}

/* static */ bool AssetManager::IsDataFile(const String &data_file) {
	Stream *in = File::OpenFileCI(data_file.GetCStr(), Shared::kFile_Open, Shared::kFile_Read);
	if (in) {
		MFLUtil::MFLError err = MFLUtil::TestIsMFL(in, true);
		delete in;
		return err == MFLUtil::kMFLNoError;
	}
	return false;
}

/* static */ AssetError AssetManager::ReadDataFileTOC(const String &data_file, AssetLibInfo &lib) {
	Stream *in = File::OpenFileCI(data_file.GetCStr(), Shared::kFile_Open, Shared::kFile_Read);
	if (in) {
		MFLUtil::MFLError err = MFLUtil::ReadHeader(lib, in);
		delete in;
		return (err != MFLUtil::kMFLNoError) ? kAssetErrLibParse : kAssetNoError;
	}
	return kAssetErrNoLibFile;
}

void AssetManager::SetSearchPriority(AssetSearchPriority priority) {
	_libsPriority = priority;
	_libsSorter = _libsPriority == kAssetPriorityDir ? SortLibsPriorityDir : SortLibsPriorityLib;
	std::sort(_activeLibs.begin(), _activeLibs.end(), _libsSorter);
}

AssetSearchPriority AssetManager::GetSearchPriority() const {
	return _libsPriority;
}

AssetError AssetManager::AddLibrary(const String &path, const AssetLibInfo **out_lib) {
	return AddLibrary(path, "", out_lib);
}

AssetError AssetManager::AddLibrary(const String &path, const String &filters, const AssetLibInfo **out_lib) {
	if (path.IsEmpty())
		return kAssetErrNoLibFile;

	for (const auto &lib : _libs) {
		if (Path::ComparePaths(lib->BasePath, path) == 0) {
			// already present, only assign new filters
			lib->Filters = filters.Split(',');
			if (out_lib)
				*out_lib = lib;
			return kAssetNoError;
		}
	}

	AssetLibEx *lib;
	AssetError err = RegisterAssetLib(path, lib);
	if (err != kAssetNoError)
		return err;
	lib->Filters = filters.Split(',');
	auto place = std::upper_bound(_activeLibs.begin(), _activeLibs.end(), lib, _libsSorter);
	_activeLibs.insert(place, lib);
	if (out_lib)
		*out_lib = lib;
	return kAssetNoError;
}

void AssetManager::RemoveLibrary(const String &path) {
	int idx = 0;
	for (auto it = _libs.begin(); it != _libs.end(); ++it, ++idx) {
		if (Path::ComparePaths((*it)->BasePath, path) == 0) {
			_libs.remove_at(idx);
			_activeLibs.remove(*it);
			return;
		}
	}
}

void AssetManager::RemoveAllLibraries() {
	for (uint i = 0; i < _libs.size(); ++i)
		delete _libs[i];

	_libs.clear();
	_activeLibs.clear();
}

size_t AssetManager::GetLibraryCount() const {
	return _libs.size();
}

const AssetLibInfo *AssetManager::GetLibraryInfo(size_t index) const {
	return index < _libs.size() ? _libs[index] : nullptr;
}

bool AssetManager::DoesAssetExist(const String &asset_name, const String &filter) const {
	for (const auto &lib : _activeLibs) {
		if (!lib->TestFilter(filter))
			continue; // filter does not match

		if (IsAssetLibDir(lib)) {
			String filename = File::FindFileCI(lib->BaseDir, asset_name);
			if (!filename.IsEmpty() && File::IsFile(filename)) return true;
		} else {
			for (const auto &a : lib->AssetInfos) {
				if (a.FileName.CompareNoCase(asset_name) == 0) return true;
			}
		}
	}
	return false;
}

void AssetManager::FindAssets(std::vector<String> &assets, const String &wildcard,
		const String &filter) const {
	String pattern = StrUtil::WildcardToRegex(wildcard);

	for (const auto *lib : _activeLibs) {
		auto match = std::find(lib->Filters.begin(), lib->Filters.end(), filter);
		if (match == lib->Filters.end())
			continue; // filter does not match

		if (IsAssetLibDir(lib)) {
			for (FindFile ff = FindFile::OpenFiles(lib->BaseDir, wildcard);
				!ff.AtEnd(); ff.Next())
				assets.push_back(ff.Current());
		} else {
			for (const auto &a : lib->AssetInfos) {
				if (pattern == "*" || (*pattern.GetCStr() &&
						Common::String(a.FileName.GetCStr()).hasSuffixIgnoreCase(pattern.GetCStr() + 1)))
					assets.push_back(a.FileName);
			}
		}
	}

	// Sort and remove duplicates
	std::sort(assets.begin(), assets.end());
	assets.erase(std::unique(assets.begin(), assets.end()), assets.end());
}

AssetError AssetManager::RegisterAssetLib(const String &path, AssetLibEx *&out_lib) {
	// Test for a directory
	std::unique_ptr<AssetLibEx> lib;
	if (File::IsDirectory(path)) {
		lib.reset(new AssetLibEx());
		lib->BasePath = Path::MakeAbsolutePath(path);
		lib->BaseDir = Path::GetDirectoryPath(lib->BasePath);

		// TODO: maybe parse directory for the file reference? idk if needed
	}
	// ...else try open a data library
	else {
		Stream *in = File::OpenFileCI(path.GetCStr(), Shared::kFile_Open, Shared::kFile_Read);
		if (!in)
			return kAssetErrNoLibFile; // can't be opened, return error code

		lib.reset(new AssetLibEx());
		MFLUtil::MFLError mfl_err = MFLUtil::ReadHeader(*lib, in);
		delete in;

		if (mfl_err != MFLUtil::kMFLNoError)
			return kAssetErrLibParse;

		lib->BasePath = Path::MakeAbsolutePath(path);
		lib->BaseDir = Path::GetDirectoryPath(lib->BasePath);
		lib->BaseFileName = Path::GetFilename(lib->BasePath);
		lib->LibFileNames[0] = lib->BaseFileName;

		// Find out real library files in the current filesystem and save them
		for (size_t i = 0; i < lib->LibFileNames.size(); ++i) {
			lib->RealLibFiles.push_back(File::FindFileCI(lib->BaseDir, lib->LibFileNames[i]));
		}
	}

	out_lib = lib.release();
	_libs.push_back(out_lib);

	return kAssetNoError;
}

Stream *AssetManager::OpenAsset(const String &asset_name, const String &filter) const {
	for (const auto *lib : _activeLibs) {
		if (!lib->TestFilter(filter)) continue; // filter does not match

		Stream *s = nullptr;
		if (IsAssetLibDir(lib))
			s = OpenAssetFromDir(lib, asset_name);
		else
			s = OpenAssetFromLib(lib, asset_name);
		if (s)
			return s;
	}
	return nullptr;
}

Stream *AssetManager::OpenAssetFromLib(const AssetLibEx *lib, const String &asset_name) const {
	for (const auto &a : lib->AssetInfos) {
		if (a.FileName.CompareNoCase(asset_name) == 0) {
			String libfile = lib->RealLibFiles[a.LibUid];
			if (libfile.IsEmpty())
				return nullptr;
			return File::OpenFile(libfile, a.Offset, a.Offset + a.Size);
		}
	}
	return nullptr;
}

Stream *AssetManager::OpenAssetFromDir(const AssetLibEx *lib, const String &file_name) const {
	String found_file = File::FindFileCI(lib->BaseDir, file_name);
	if (found_file.IsEmpty())
		return nullptr;
	return File::OpenFileRead(found_file);
}

Stream *AssetManager::OpenAsset(const String &asset_name) const {
	return OpenAsset(asset_name, "");
}

Common::SeekableReadStream *AssetManager::OpenAssetStream(const String &asset_name) const {
	return OpenAssetStream(asset_name, "");
}

Common::SeekableReadStream *AssetManager::OpenAssetStream(const String &asset_name, const String &filter) const {
	Stream *stream = OpenAsset(asset_name, filter);
	if (!stream)
		return nullptr;

	// Get the contents of the asset
	size_t assetSize = stream->GetLength();
	byte *data = (byte *)malloc(assetSize);
	stream->Read(data, assetSize);
	delete stream;

	return new Common::MemoryReadStream(data, assetSize, DisposeAfterUse::YES);
}

String GetAssetErrorText(AssetError err) {
	switch (err) {
	case kAssetNoError:
		return "No error.";
	case kAssetErrNoLibFile:
		return "Asset library file not found or could not be opened.";
	case kAssetErrLibParse:
		return "Not an asset library or unsupported format.";
	case kAssetErrNoManager:
		return "Asset manager is not initialized.";
	}
	return "Unknown error.";
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
