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

#include "ags/engine/media/audio/queued_audio_item.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

void QueuedAudioItem::ReadFromSavegame_v321(Stream *in) {
	audioClipIndex = in->ReadInt16();
	priority = in->ReadInt16();
	repeat = in->ReadBool();
	in->Seek(3);     // alignment padding to int32
	in->ReadInt32(); // cachedClip 32-bit ptr (legacy format)
}

void QueuedAudioItem::ReadFromSavegame(Stream *in) {
	audioClipIndex = in->ReadInt16();
	priority = in->ReadInt16();
	repeat = in->ReadBool();
	in->ReadInt32(); // reserved
}

void QueuedAudioItem::WriteToSavegame(Stream *out) const {
	out->WriteInt16(audioClipIndex);
	out->WriteInt16(priority);
	out->WriteBool(repeat);
	out->WriteInt32(0); // reserved
}

} // namespace AGS3
