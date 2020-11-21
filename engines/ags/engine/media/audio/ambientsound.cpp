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

#include "media/audio/ambientsound.h"
#include "media/audio/audio.h"
#include "media/audio/soundclip.h"
#include "util/stream.h"

using AGS::Common::Stream;

bool AmbientSound::IsPlaying() {
	if (channel <= 0)
		return false;
	return channel_is_playing(channel);
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
