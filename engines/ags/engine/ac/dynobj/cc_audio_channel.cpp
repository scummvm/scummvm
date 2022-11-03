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

#include "ags/shared/util/stream.h"
#include "ags/engine/ac/dynobj/cc_audio_channel.h"
#include "ags/engine/ac/dynobj/script_audio_channel.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *CCAudioChannel::GetType() {
	return "AudioChannel";
}

size_t CCAudioChannel::CalcSerializeSize() {
	return sizeof(int32_t);
}

void CCAudioChannel::Serialize(const char *address, Stream *out) {
	const ScriptAudioChannel *ach = (const ScriptAudioChannel *)address;
	out->WriteInt32(ach->id);
}

void CCAudioChannel::Unserialize(int index, Stream *in, size_t data_sz) {
	int id = in->ReadInt32();
	ccRegisterUnserializedObject(index, &_G(scrAudioChannel)[id], this);
}

} // namespace AGS3
