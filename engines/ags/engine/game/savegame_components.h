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

#ifndef AGS_ENGINE_GAME_SAVEGAME_COMPONENTS_H
#define AGS_ENGINE_GAME_SAVEGAME_COMPONENTS_H

#include "ags/engine/game/savegame.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {

namespace Shared {
struct Interaction;
} // namespace Shared

namespace Engine {

using Shared::Stream;
using Shared::Interaction;

struct PreservedParams;
struct RestoredData;

namespace SavegameComponents {
// Reads all available components from the stream
HSaveError    ReadAll(Stream *in, SavegameVersion svg_version, const PreservedParams &pp, RestoredData &r_data);
// Writes a full list of common components to the stream
HSaveError    WriteAllCommon(Stream *out);

// Utility functions for reading and writing legacy interactions,
// or their "times run" counters separately.
void ReadTimesRun272(Interaction &intr, Stream *in);
HSaveError ReadInteraction272(Interaction &intr, Stream *in);
void WriteTimesRun272(const Interaction &intr, Stream *out);
void WriteInteraction272(const Interaction &intr, Stream *out);

// Precreates primary camera and viewport and reads legacy camera data
void ReadLegacyCameraState(Stream *in, RestoredData &r_data);
} // namespace SavegameComponents

} // namespace Engine
} // namespace AGS
} // namespace AGS3

#endif
