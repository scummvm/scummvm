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

#include "ac/audiocliptype.h"
#include "util/stream.h"

using AGS::Common::Stream;

void AudioClipType::ReadFromFile(Stream *in)
{
    id = in->ReadInt32();
    reservedChannels = in->ReadInt32();
    volume_reduction_while_speech_playing = in->ReadInt32();
    crossfadeSpeed = in->ReadInt32();
    reservedForFuture = in->ReadInt32();
}

void AudioClipType::WriteToFile(Stream *out)
{
    out->WriteInt32(id);
    out->WriteInt32(reservedChannels);
    out->WriteInt32(volume_reduction_while_speech_playing);
    out->WriteInt32(crossfadeSpeed);
    out->WriteInt32(reservedForFuture);
}

void AudioClipType::ReadFromSavegame(Common::Stream *in)
{
    volume_reduction_while_speech_playing = in->ReadInt32();
    crossfadeSpeed = in->ReadInt32();
}

void AudioClipType::WriteToSavegame(Common::Stream *out) const
{
    out->WriteInt32(volume_reduction_while_speech_playing);
    out->WriteInt32(crossfadeSpeed);
}
