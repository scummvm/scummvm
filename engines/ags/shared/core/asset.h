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
