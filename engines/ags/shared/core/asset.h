//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
// AssetInfo and AssetLibInfo - classes describing generic asset library.
//
//=============================================================================

#ifndef __AGS_CN_CORE__ASSET_H
#define __AGS_CN_CORE__ASSET_H

#include <vector>
#include "util/string.h"

namespace AGS
{
namespace Common
{

// Information on single asset
struct AssetInfo
{
    // A pair of filename and libuid is assumed to be unique in game scope
    String      FileName;   // filename associated with asset
    int32_t     LibUid;     // uid of library, containing this asset
    soff_t      Offset;     // asset's position in library file (in bytes)
    soff_t      Size;       // asset's size (in bytes)

    AssetInfo();
};

typedef std::vector<AssetInfo> AssetVec;

// Information on multifile asset library
struct AssetLibInfo
{
    String BaseFileName;               // library's base (head) filename
    String BaseFilePath;               // full path to the base filename
    std::vector<String> LibFileNames;  // filename for each library part

    // Library contents
    AssetVec AssetInfos; // information on contained assets

    void Unload();
};

} // namespace Common
} // namespace AGS

#endif // __AGS_CN_CORE__ASSET_H
