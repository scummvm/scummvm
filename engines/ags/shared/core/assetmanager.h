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
// Asset manager class fore reading and writing game resources
// Asset manager is a singleton
//-----------------------------------------------------------------------------
//
// The code is based on CLIB32, by Chris Jones (1998-99), DJGPP implementation
// of the CLIB reader.
//
//-----------------------------------------------------------------------------
//
// TODO:
// Ideally AssetManager should take care of enumerating all existing data
// packages and all files in them, while the game itself should not know where
// it receives the data from.
// Files should be registered based on their source package and their types.
// The user must not have access to this information, but is allowed to query
// all files of certain type (perhaps, filtered and ordered by their id).
//
//=============================================================================

#ifndef AGS_SHARED_CORE_ASSETMANAGER_H
#define AGS_SHARED_CORE_ASSETMANAGER_H

#include "util/file.h" // TODO: extract filestream mode constants or introduce generic ones

namespace AGS
{
namespace Common
{

class Stream;
struct MultiFileLib;
struct AssetLibInfo;
struct AssetInfo;

enum AssetSearchPriority
{
    // TODO: rename this to something more obvious
    kAssetPriorityUndefined,
    kAssetPriorityLib,
    kAssetPriorityDir
};

enum AssetError
{
    kAssetNoError           =  0,
    kAssetErrNoLibFile      = -1, // library file not found or can't be read
    kAssetErrLibParse       = -2, // bad library file format or read error
    kAssetErrNoManager      = -6, // asset manager not initialized
};

// Explicit location of asset data
struct AssetLocation
{
    String      FileName;   // file where asset is located
    soff_t      Offset;     // asset's position in file (in bytes)
    soff_t      Size;       // asset's size (in bytes)

    AssetLocation();
};


class AssetManager
{
public:
    static bool     CreateInstance();
    static void     DestroyInstance();
    ~AssetManager();

    static bool     SetSearchPriority(AssetSearchPriority priority);
    static AssetSearchPriority GetSearchPriority();

    // Test if given file is main data file
    static bool         IsDataFile(const String &data_file);
    // Read data file table of contents into provided struct
    static AssetError   ReadDataFileTOC(const String &data_file, AssetLibInfo &lib);

    // NOTE: this group of methods are only temporarily public
    static AssetError   SetDataFile(const String &data_file);
    static String       GetLibraryBaseFile();
    static const AssetLibInfo *GetLibraryTOC();
    static int          GetAssetCount();
    static String       GetLibraryForAsset(const String &asset_name);
    static String       GetAssetFileByIndex(int index);
    static soff_t       GetAssetOffset(const String &asset_name);
    static soff_t       GetAssetSize(const String &asset_name);
    // TODO: instead of this support streams that work in a file subsection, limited by size
    static soff_t       GetLastAssetSize();
    // TODO: this is a workaround that lets us use back-end specific kind of streams
    // to read the asset data. This is not ideal, because it limits us to reading from file.
    // The better solution could be returning a standart stream object (like std::stream,
    // or even std::streambuf), which is used to initialize both AGS and back-end compatible
    // stream wrappers.
    static bool         GetAssetLocation(const String &asset_name, AssetLocation &loc);

    static bool         DoesAssetExist(const String &asset_name);
    static Stream       *OpenAsset(const String &asset_name,
                                   FileOpenMode open_mode = kFile_Open,
                                   FileWorkMode work_mode = kFile_Read);

private:
    AssetManager();

    bool        _SetSearchPriority(AssetSearchPriority priority);
    AssetSearchPriority _GetSearchPriority();
    AssetError  _SetDataFile(const String &data_file);
    String      _GetLibraryBaseFile();
    const AssetLibInfo &_GetLibraryTOC() const;
    int         _GetAssetCount();    
    String      _GetLibraryForAsset(const String &asset_name);
    String      _GetAssetFileByIndex(int index);
    soff_t      _GetAssetOffset(const String &asset_name);
    soff_t      _GetAssetSize(const String &asset_name);
    soff_t      _GetLastAssetSize();

    AssetError  RegisterAssetLib(const String &data_file, const String &password);

    bool        _DoesAssetExist(const String &asset_name);

    AssetInfo   *FindAssetByFileName(const String &asset_name);
    String      MakeLibraryFileNameForAsset(const AssetInfo *asset);

    bool        GetAssetFromLib(const String &asset_name, AssetLocation &loc, Common::FileOpenMode open_mode, Common::FileWorkMode work_mode);
    bool        GetAssetFromDir(const String &asset_name, AssetLocation &loc, Common::FileOpenMode open_mode, Common::FileWorkMode work_mode);
    bool        GetAssetByPriority(const String &asset_name, AssetLocation &loc, Common::FileOpenMode open_mode, Common::FileWorkMode work_mode);
    Stream      *OpenAssetAsStream(const String &asset_name, FileOpenMode open_mode, FileWorkMode work_mode);

    static AssetManager     *_theAssetManager;
    AssetSearchPriority     _searchPriority;

    AssetLibInfo            &_assetLib;
    String                  _basePath;          // library's parent path (directory)
    soff_t                  _lastAssetSize;     // size of asset that was opened last time
};


String GetAssetErrorText(AssetError err);

} // namespace Common
} // namespace AGS

#endif
