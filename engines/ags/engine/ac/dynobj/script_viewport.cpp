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

#include "ags/engine/ac/dynobj/script_viewport.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/util/bbop.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

ScriptViewport::ScriptViewport(int id) : _id(id) {
}

const char *ScriptViewport::GetType() {
	return "Viewport2";
}

int ScriptViewport::Dispose(const char *address, bool force) {
	// Note that ScriptViewport is a reference to actual Viewport object,
	// and this deletes the reference, while viewport may remain in GameState.
	delete this;
	return 1;
}

int ScriptViewport::Serialize(const char *address, char *buffer, int bufsize) {
	StartSerialize(buffer);
	SerializeInt(_id);
	return EndSerialize();
}

void ScriptViewport::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	_id = UnserializeInt();
	ccRegisterUnserializedObject(index, this, this);
}

ScriptViewport *Viewport_Unserialize(int handle, const char *serializedData, int dataSize) {
	// The way it works now, we must not create a new script object,
	// but acquire one from the GameState, which keeps the first reference.
	// This is essential because GameState should be able to invalidate any
	// script references when Viewport gets removed.
	const int id = BBOp::Int32FromLE(*((const int *)serializedData));
	if (id >= 0) {
		auto scview = _GP(play).RegisterRoomViewport(id, handle);
		if (scview)
			return scview;
	}
	return new ScriptViewport(-1); // make invalid reference
}

} // namespace AGS3
