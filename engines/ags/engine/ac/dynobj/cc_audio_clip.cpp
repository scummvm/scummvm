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

#include "ags/engine/ac/dynobj/cc_audio_clip.h"
#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *CCAudioClip::GetType() {
	return "AudioClip";
}

size_t CCAudioClip::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t);
}

void CCAudioClip::Serialize(const void *address, Stream *out) {
	const ScriptAudioClip *ach = static_cast<const ScriptAudioClip *>(address);
	out->WriteInt32(ach->id);
}

void CCAudioClip::Unserialize(int index, Stream *in, size_t data_sz) {
	int id = in->ReadInt32();
	ccRegisterUnserializedObject(index, &_GP(game).audioClips[id], this);
}

} // namespace AGS3
