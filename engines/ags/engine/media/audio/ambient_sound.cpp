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

#include "ags/engine/media/audio/ambient_sound.h"
#include "ags/engine/media/audio/audio.h"
#include "ags/engine/media/audio/sound_clip.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

bool AmbientSound::IsPlaying() {
	if (channel <= 0)
		return false;
	return AudioChans::ChannelIsPlaying(channel);
}

void AmbientSound::ReadFromFile(Stream *in) {
	channel = in->ReadInt32();
	x = in->ReadInt32();
	y = in->ReadInt32();
	vol = in->ReadInt32();
	num = in->ReadInt32();
	maxdist = in->ReadInt32();
}

void AmbientSound::WriteToFile(Stream *out) {
	out->WriteInt32(channel);
	out->WriteInt32(x);
	out->WriteInt32(y);
	out->WriteInt32(vol);
	out->WriteInt32(num);
	out->WriteInt32(maxdist);
}

} // namespace AGS3
