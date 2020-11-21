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

#include "media/audio/ambientsound.h"
#include "media/audio/audio.h"
#include "media/audio/soundclip.h"
#include "util/stream.h"

using AGS::Common::Stream;

bool AmbientSound::IsPlaying () {
    if (channel <= 0)
        return false;
    return channel_is_playing(channel);
}

void AmbientSound::ReadFromFile(Stream *in)
{
    channel = in->ReadInt32();
    x = in->ReadInt32();
    y = in->ReadInt32();
    vol = in->ReadInt32();
    num = in->ReadInt32();
    maxdist = in->ReadInt32();
}

void AmbientSound::WriteToFile(Stream *out)
{
    out->WriteInt32(channel);
    out->WriteInt32(x);
    out->WriteInt32(y);
    out->WriteInt32(vol);
    out->WriteInt32(num);
    out->WriteInt32(maxdist);
}
