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
