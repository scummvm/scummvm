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

#ifndef __AGS_EE_GAME__SAVEGAMECOMPONENTS_H
#define __AGS_EE_GAME__SAVEGAMECOMPONENTS_H

#include "game/savegame.h"
#include "util/stream.h"

namespace AGS
{

namespace Common { struct Interaction; }

namespace Engine
{

using Common::Stream;
typedef std::shared_ptr<Stream> PStream;

struct PreservedParams;
struct RestoredData;

namespace SavegameComponents
{
    // Reads all available components from the stream
    HSaveError    ReadAll(PStream in, SavegameVersion svg_version, const PreservedParams &pp, RestoredData &r_data);
    // Writes a full list of common components to the stream
    HSaveError    WriteAllCommon(PStream out);

    // Utility functions for reading and writing legacy interactions,
    // or their "times run" counters separately.
    void ReadTimesRun272(Interaction &intr, Stream *in);
    HSaveError ReadInteraction272(Interaction &intr, Stream *in);
    void WriteTimesRun272(const Interaction &intr, Stream *out);
    void WriteInteraction272(const Interaction &intr, Stream *out);

    // Precreates primary camera and viewport and reads legacy camera data
    void ReadLegacyCameraState(Stream *in, RestoredData &r_data);
}

} // namespace Engine
} // namespace AGS

#endif // __AGS_EE_GAME__SAVEGAMECOMPONENTS_H
