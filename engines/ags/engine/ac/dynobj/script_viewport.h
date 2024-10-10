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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPT_VIEWPORT_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPT_VIEWPORT_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {

// ScriptViewport keeps a reference to actual room Viewport in script.
struct ScriptViewport final : AGSCCDynamicObject {
public:
	ScriptViewport(int id);
	// Get viewport index; negative means the viewport was deleted
	int GetID() const {
		return _id;
	}
	void SetID(int id) {
		_id = id;
	}
	// Reset viewport index to indicate that this reference is no longer valid
	void Invalidate() {
		_id = -1;
	}

	const char *GetType() override;
	int Dispose(void *address, bool force) override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

protected:
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;

private:
	int _id = -1; // index of viewport in the game state array
};

// Unserialize viewport from the memory stream
ScriptViewport *Viewport_Unserialize(int handle, AGS::Shared::Stream *in, size_t data_sz);

} // namespace AGS3

#endif
