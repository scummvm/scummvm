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

#include "ags/engine/ac/dynobj/cc_audio_channel.h"
#include "ags/engine/ac/dynobj/script_audio_channel.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/globals.h"

namespace AGS3 {

const char *CCAudioChannel::GetType() {
	return "AudioChannel";
}

int CCAudioChannel::Serialize(const char *address, char *buffer, int bufsize) {
	const ScriptAudioChannel *ach = (const ScriptAudioChannel *)address;
	StartSerialize(buffer);
	SerializeInt(ach->id);
	return EndSerialize();
}

void CCAudioChannel::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	int id = UnserializeInt();
	ccRegisterUnserializedObject(index, &_G(scrAudioChannel)[id], this);
}

} // namespace AGS3
