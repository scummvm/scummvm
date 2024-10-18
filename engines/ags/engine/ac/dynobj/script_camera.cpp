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

#include "ags/engine/ac/dynobj/script_camera.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/util/bbop.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

ScriptCamera::ScriptCamera(int id) : _id(id) {
}

const char *ScriptCamera::GetType() {
	return "Camera2";
}

int ScriptCamera::Dispose(void *address, bool force) {
	// Note that ScriptCamera is a reference to actual Camera object,
	// and this deletes the reference, while camera may remain in GameState.
	delete this;
	return 1;
}

size_t ScriptCamera::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t);
}

void ScriptCamera::Serialize(const void *address, Stream *out) {
	out->WriteInt32(_id);
}

void ScriptCamera::Unserialize(int index, Stream *in, size_t data_sz) {
	_id = in->ReadInt32();
	ccRegisterUnserializedObject(index, this, this);
}

ScriptCamera *Camera_Unserialize(int handle, Stream *in, size_t data_sz) {
	// The way it works now, we must not create a new script object,
	// but acquire one from the GameState, which keeps the first reference.
	// This is essential because GameState should be able to invalidate any
	// script references when Camera gets removed.
	const int id = in->ReadInt32();
	if (id >= 0) {
		auto scam = _GP(play).RegisterRoomCamera(id, handle);
		if (scam)
			return scam;
	}
	return new ScriptCamera(-1); // make invalid reference
}

} // namespace AGS3
