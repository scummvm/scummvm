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
// Implementation from sprcache.cpp specific to Engine runtime
//
//=============================================================================

// Headers, as they are in sprcache.cpp
#ifdef _MANAGED
// ensure this doesn't get compiled to .NET IL
#pragma unmanaged
#pragma warning (disable: 4996 4312)  // disable deprecation warnings
#endif

#include "ac/gamestructdefines.h"
#include "ac/spritecache.h"
#include "util/compress.h"

//=============================================================================
// Engine-specific implementation split out of sprcache.cpp
//=============================================================================

void SpriteCache::InitNullSpriteParams(sprkey_t index)
{
    // make it a blue cup, to avoid crashes
    _sprInfos[index].Width = _sprInfos[0].Width;
    _sprInfos[index].Height = _sprInfos[0].Height;
    _spriteData[index].Image = nullptr;
    _spriteData[index].Offset = _spriteData[0].Offset;
    _spriteData[index].Size = _spriteData[0].Size;
    _spriteData[index].Flags = SPRCACHEFLAG_REMAPPED;
}
