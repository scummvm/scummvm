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

#include "core/assetmanager.h"
#include "util/misc.h" // ci_fopen
#include "util/multifilelib.h"
#include "util/path.h"
#include "util/string_utils.h"


namespace AGS
{
namespace Common
{

AssetLocation::AssetLocation()
    : Offset(0)
    , Size(0)
{
}


AssetManager *AssetManager::_theAssetManager = nullptr;

/* static */ bool AssetManager::CreateInstance()
{
    // Issue a warning - recreating asset manager is not a normal behavior
    assert(_theAssetManager == NULL);
    delete _theAssetManager;
    _theAssetManager = new AssetManager();
    _theAssetManager->SetSearchPriority(kAssetPriorityDir);
    return _theAssetManager != nullptr; // well, we should return _something_
}

/* static */ void AssetManager::DestroyInstance()
{
    delete _theAssetManager;
    _theAssetManager = nullptr;
}

AssetManager::~AssetManager()
{
    delete &_assetLib;
}

/* static */ bool AssetManager::SetSearchPriority(AssetSearchPriority priority)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_SetSearchPriority(priority) : false;
}

/* static */ AssetSearchPriority AssetManager::GetSearchPriority()
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetSearchPriority() : kAssetPriorityUndefined;
}

/* static */ bool AssetManager::IsDataFile(const String &data_file)
{
    Stream *in = ci_fopen(data_file, Common::kFile_Open, Common::kFile_Read);
    if (in)
    {
        MFLUtil::MFLError err = MFLUtil::TestIsMFL(in, true);
        delete in;
        return err == MFLUtil::kMFLNoError;
    }
    return false;
}

AssetError AssetManager::ReadDataFileTOC(const String &data_file, AssetLibInfo &lib)
{
    Stream *in = ci_fopen(data_file, Common::kFile_Open, Common::kFile_Read);
    if (in)
    {
        MFLUtil::MFLError err = MFLUtil::ReadHeader(lib, in);
        delete in;
        return (err != MFLUtil::kMFLNoError) ? kAssetErrLibParse : kAssetNoError;
    }
    return kAssetErrNoLibFile;
}

/* static */ AssetError AssetManager::SetDataFile(const String &data_file)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_SetDataFile(data_file) : kAssetErrNoManager;
}

/* static */ String AssetManager::GetLibraryForAsset(const String &asset_name)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetLibraryForAsset(asset_name) : "";
}

/* static */ soff_t AssetManager::GetAssetOffset(const String &asset_name)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetAssetOffset(asset_name) : 0;
}

/* static */ soff_t AssetManager::GetAssetSize(const String &asset_name)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetAssetSize(asset_name) : 0;
}

/* static */ soff_t AssetManager::GetLastAssetSize()
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetLastAssetSize() : 0;
}

/* static */ int AssetManager::GetAssetCount()
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetAssetCount() : 0;
}

/* static */ String AssetManager::GetAssetFileByIndex(int index)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetAssetFileByIndex(index) : "";
}

/* static */ String AssetManager::GetLibraryBaseFile()
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->_GetLibraryBaseFile() : "";
}

/* static */ const AssetLibInfo *AssetManager::GetLibraryTOC()
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? &_theAssetManager->_GetLibraryTOC() : nullptr;
}

/* static */ bool AssetManager::GetAssetLocation(const String &asset_name, AssetLocation &loc)
{
    assert(_theAssetManager != NULL);
    return _theAssetManager ? _theAssetManager->GetAssetByPriority(asset_name, loc, kFile_Open, kFile_Read) : false;
}

/* static */ bool AssetManager::DoesAssetExist(const String &asset_name)
{
    assert(_theAssetManager != NULL);
    if (!_theAssetManager)
    {
        return false;
    }
    return _theAssetManager->_DoesAssetExist(asset_name);
}

/* static */ Stream *AssetManager::OpenAsset(const String &asset_name,
                                                  FileOpenMode open_mode,
                                                  FileWorkMode work_mode)
{
    assert(_theAssetManager != NULL);
    if (!_theAssetManager)
    {
        return nullptr;
    }
    return _theAssetManager->OpenAssetAsStream(asset_name, open_mode, work_mode);
}

AssetManager::AssetManager()
    : _assetLib(*new AssetLibInfo())
    , _searchPriority(kAssetPriorityDir)
    , _lastAssetSize(0)
{
}

bool AssetManager::_SetSearchPriority(AssetSearchPriority priority)
{
    _searchPriority = priority;
    return true;
}

AssetSearchPriority AssetManager::_GetSearchPriority()
{
    return _searchPriority;
}

AssetError AssetManager::_SetDataFile(const String &data_file)
{
    if (data_file.IsEmpty())
    {
        return kAssetErrNoLibFile;
    }
    if (Path::ComparePaths(_assetLib.BaseFilePath, data_file) == 0)
    {
        return kAssetNoError;
    }
    AssetError err = RegisterAssetLib(data_file, "");
    return err;
}

String AssetManager::_GetLibraryForAsset(const String &asset_name)
{
    if (asset_name.IsEmpty())
    {
        return "";
    }
    AssetInfo *asset = FindAssetByFileName(asset_name);
    if (!asset)
    {
        // asset not found
        return "";
    }

    return MakeLibraryFileNameForAsset(asset);
}

soff_t AssetManager::_GetAssetOffset(const String &asset_name)
{
    if (asset_name.IsEmpty())
    {
        return -1;
    }
    AssetInfo *asset = FindAssetByFileName(asset_name);
    if (asset)
    {
        return asset->Offset;
    }
    return -1;
}

soff_t AssetManager::_GetAssetSize(const String &asset_name)
{
    if (asset_name.IsEmpty())
    {
        return -1;
    }
    AssetInfo *asset = FindAssetByFileName(asset_name);
    if (asset)
    {
        return asset->Size;
    }
    return -1;
}

soff_t AssetManager::_GetLastAssetSize()
{
    return _lastAssetSize;
}

int AssetManager::_GetAssetCount()
{
    return _assetLib.AssetInfos.size();
}

String AssetManager::_GetAssetFileByIndex(int index)
{
    if ((index < 0) || ((size_t)index >= _assetLib.AssetInfos.size()))
        return nullptr;

    return _assetLib.AssetInfos[index].FileName;
}

String AssetManager::_GetLibraryBaseFile()
{
    return _assetLib.BaseFileName;
}

const AssetLibInfo &AssetManager::_GetLibraryTOC() const
{
    return _assetLib;
}

bool AssetManager::_DoesAssetExist(const String &asset_name)
{
    return FindAssetByFileName(asset_name) != nullptr ||
        File::TestReadFile(asset_name);
}

AssetError AssetManager::RegisterAssetLib(const String &data_file, const String &password)
{
    // base path is current directory
    _basePath = ".";

    // open data library
    Stream *in = ci_fopen(data_file, Common::kFile_Open, Common::kFile_Read);
    if (!in)
        return kAssetErrNoLibFile; // can't be opened, return error code

    // read MultiFileLibrary header (CLIB)
    // PSP: allocate struct on the heap to avoid overflowing the stack.
    MFLUtil::MFLError mfl_err = MFLUtil::ReadHeader(_assetLib, in);
    delete in;

    if (mfl_err != MFLUtil::kMFLNoError)
    {
        _assetLib.Unload();
        return kAssetErrLibParse;
    }

    // fixup base library filename
    String nammwas = data_file;
    String data_file_fixed = data_file;
    // TODO: this algorythm should be in path/string utils
    data_file_fixed.TruncateToRightSection('\\');
    data_file_fixed.TruncateToRightSection('/');
    if (data_file_fixed.Compare(nammwas) != 0)
    {
        // store complete path
        _basePath = nammwas;
        _basePath.TruncateToLeft(nammwas.GetLength() - data_file_fixed.GetLength());
        _basePath.TrimRight('\\');
        _basePath.TrimRight('/');
    }

    // set library filename
    _assetLib.LibFileNames[0] = data_file_fixed;
    // make a lowercase backup of the original file name
    _assetLib.BaseFileName = data_file_fixed;
    _assetLib.BaseFileName.MakeLower();
    _assetLib.BaseFilePath = Path::MakeAbsolutePath(data_file);
    return kAssetNoError;
}

AssetInfo *AssetManager::FindAssetByFileName(const String &asset_name)
{
    for (size_t i = 0; i < _assetLib.AssetInfos.size(); ++i)
    {
        if (_assetLib.AssetInfos[i].FileName.CompareNoCase(asset_name) == 0)
        {
            return &_assetLib.AssetInfos[i];
        }
    }
    return nullptr;
}

String AssetManager::MakeLibraryFileNameForAsset(const AssetInfo *asset)
{
    // deduce asset library file containing this asset
    return String::FromFormat("%s/%s",_basePath.GetCStr(), _assetLib.LibFileNames[asset->LibUid].GetCStr());
}

bool AssetManager::GetAssetFromLib(const String &asset_name, AssetLocation &loc, FileOpenMode open_mode, FileWorkMode work_mode)
{
    if (open_mode != Common::kFile_Open || work_mode != Common::kFile_Read)
        return false; // creating/writing is allowed only for common files on disk

    AssetInfo *asset = FindAssetByFileName(asset_name);
    if (!asset)
        return false; // asset not found

    String libfile = cbuf_to_string_and_free( ci_find_file(nullptr, MakeLibraryFileNameForAsset(asset)) );
    if (libfile.IsEmpty())
        return false;
    loc.FileName = libfile;
    loc.Offset = asset->Offset;
    loc.Size = asset->Size;
    return true;
}

bool AssetManager::GetAssetFromDir(const String &file_name, AssetLocation &loc, FileOpenMode open_mode, FileWorkMode work_mode)
{
    String exfile = cbuf_to_string_and_free( ci_find_file(nullptr, file_name) );
    if (exfile.IsEmpty() || !Path::IsFile(exfile))
        return false;
    loc.FileName = exfile;
    loc.Offset = 0;
    loc.Size = File::GetFileSize(exfile);
    return true;
}

bool AssetManager::GetAssetByPriority(const String &asset_name, AssetLocation &loc, FileOpenMode open_mode, FileWorkMode work_mode)
{
    if (_searchPriority == kAssetPriorityDir)
    {
        // check for disk, otherwise use datafile
        return GetAssetFromDir(asset_name, loc, open_mode, work_mode) ||
            GetAssetFromLib(asset_name, loc, open_mode, work_mode);
    }
    else if (_searchPriority == kAssetPriorityLib)
    {
        // check datafile first, then scan directory
        return GetAssetFromLib(asset_name, loc, open_mode, work_mode) ||
            GetAssetFromDir(asset_name, loc, open_mode, work_mode);
    }
    return false;
}

Stream *AssetManager::OpenAssetAsStream(const String &asset_name, FileOpenMode open_mode, FileWorkMode work_mode)
{
    AssetLocation loc;
    if (GetAssetByPriority(asset_name, loc, open_mode, work_mode))
    {
        Stream *s = File::OpenFile(loc.FileName, open_mode, work_mode);
        if (s)
        {
            s->Seek(loc.Offset, kSeekBegin);
            _lastAssetSize = loc.Size;
        }
        return s;
    }
    return nullptr;
}


String GetAssetErrorText(AssetError err)
{
    switch (err)
    {
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

} // namespace Common
} // namespace AGS
