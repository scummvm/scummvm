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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPT_STRING_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPT_STRING_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {

struct ScriptString final : AGSCCDynamicObject, ICCStringClass {
	int Dispose(void *address, bool force) override;
	const char *GetType() override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

	DynObjectRef CreateString(const char *fromText) override;

	ScriptString() = default;
	ScriptString(const char *text);
	ScriptString(char *text, bool take_ownership);
	char *GetTextPtr() const {
		return _text;
	}

protected:
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(void *address) override;
	// Write object data into the provided stream
	void Serialize(void *address, AGS::Shared::Stream *out) override;

private:
	// TODO: the preallocated text buffer may be assigned externally;
	// find out if it's possible to refactor while keeping same functionality
	char *_text = nullptr;
	size_t _len = 0;
};

} // namespace AGS3

#endif
