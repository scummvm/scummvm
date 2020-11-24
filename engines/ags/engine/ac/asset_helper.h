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
// Functions related to finding and opening game assets.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_ASSETHELPER_H
#define AGS_ENGINE_AC_ASSETHELPER_H

#include "ags/std/memory.h"
#include "ags/std/utility.h"
#include "ags/shared/util/string.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using AGS::Shared::Stream;
using AGS::Shared::String;

// Looks for valid asset library everywhere and returns path, or empty string if failed
String  find_assetlib(const String &filename);
// Looks up for known valid asset library and returns path, or empty string if failed
String  get_known_assetlib(const String &filename);
// Looks for asset everywhere and returns opened stream, or NULL if failed
Stream *find_open_asset(const String &filename);

extern "C" {
	struct PACKFILE; // Allegro 4's own stream type
	struct DUMBFILE; // DUMB stream type
}

// AssetPath combines asset library and item names
// TODO: implement support for registering multiple libraries at once for
// the AssetManager, then we could remove assetlib parameter.
typedef std::pair<String, String> AssetPath;

// Returns the path to the audio asset, considering the given bundling type
AssetPath get_audio_clip_assetpath(int bundling_type, const String &filename);
// Returns the path to the voice-over asset
AssetPath get_voice_over_assetpath(const String &filename);

// Custom AGS PACKFILE user object
// TODO: it is preferrable to let our Stream define custom readable window instead,
// keeping this as simple as possible for now (we may require a stream classes overhaul).
struct AGS_PACKFILE_OBJ {
	std::unique_ptr<Stream> stream;
	size_t asset_size = 0u;
	size_t remains = 0u;
};
// Creates PACKFILE stream from AGS asset.
// This function is supposed to be used only when you have to create Allegro
// object, passing PACKFILE stream to constructor.
PACKFILE *PackfileFromAsset(const AssetPath &path, size_t &asset_size);
// Creates DUMBFILE stream from AGS asset. Used for creating DUMB objects
DUMBFILE *DUMBfileFromAsset(const AssetPath &path, size_t &asset_size);
bool DoesAssetExistInLib(const AssetPath &assetname);

} // namespace AGS3

#endif
