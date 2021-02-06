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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTCAMERA_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTCAMERA_H

#include "ags/engine/ac/dynobj/cc_agsdynamicobject.h"

namespace AGS3 {

// ScriptCamera keeps a reference to actual room Camera in script.
struct ScriptCamera final : AGSCCDynamicObject {
public:
	ScriptCamera(int id);

	// Get camera index; negative means the camera was deleted
	int GetID() const {
		return _id;
	}
	void SetID(int id) {
		_id = id;
	}
	// Reset camera index to indicate that this reference is no longer valid
	void Invalidate() {
		_id = -1;
	}

	const char *GetType() override;
	int Dispose(const char *address, bool force) override;
	int Serialize(const char *address, char *buffer, int bufsize) override;
	void Unserialize(int index, const char *serializedData, int dataSize) override;

private:
	int _id = -1; // index of camera in the game state array
};

// Unserialize camera from the memory stream
ScriptCamera *Camera_Unserialize(int handle, const char *serializedData, int dataSize);

} // namespace AGS3

#endif
