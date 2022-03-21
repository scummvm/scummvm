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
// Functions related to finding and opening game assets.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_ASSET_HELPER_H
#define AGS_ENGINE_AC_ASSET_HELPER_H

#include "ags/lib/std/memory.h"
#include "ags/lib/std/utility.h"
#include "ags/shared/util/string.h"
#include "ags/shared/core/asset_manager.h"

namespace AGS3 {

struct PACKFILE;

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using AGS::Shared::AssetPath;
using AGS::Shared::Stream;
using AGS::Shared::String;

// Looks for valid asset library everywhere and returns path, or empty string if failed
String  find_assetlib(const String &filename);

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
PACKFILE *PackfileFromAsset(const AssetPath &path);

} // namespace AGS3

#endif
