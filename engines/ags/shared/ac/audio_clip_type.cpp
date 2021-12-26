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

#include "ags/shared/ac/audio_clip_type.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

void AudioClipType::ReadFromFile(Stream *in) {
	id = in->ReadInt32();
	reservedChannels = in->ReadInt32();
	volume_reduction_while_speech_playing = in->ReadInt32();
	crossfadeSpeed = in->ReadInt32();
	reservedForFuture = in->ReadInt32();
}

void AudioClipType::WriteToFile(Stream *out) {
	out->WriteInt32(id);
	out->WriteInt32(reservedChannels);
	out->WriteInt32(volume_reduction_while_speech_playing);
	out->WriteInt32(crossfadeSpeed);
	out->WriteInt32(reservedForFuture);
}

void AudioClipType::ReadFromSavegame(Shared::Stream *in) {
	volume_reduction_while_speech_playing = in->ReadInt32();
	crossfadeSpeed = in->ReadInt32();
}

void AudioClipType::WriteToSavegame(Shared::Stream *out) const {
	out->WriteInt32(volume_reduction_while_speech_playing);
	out->WriteInt32(crossfadeSpeed);
}

} // namespace AGS3
