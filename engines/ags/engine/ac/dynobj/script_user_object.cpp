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

#include "common/std/memory.h"
#include "ags/shared/util/stream.h"
#include "ags/engine/ac/dynobj/script_user_object.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *ScriptUserObject::TypeName = "UserObject";

// return the type name of the object
const char *ScriptUserObject::GetType() {
	return TypeName;
}

/* static */ DynObjectRef ScriptUserObject::Create(size_t size) {
	uint8_t *new_data = new uint8_t[size + MemHeaderSz];
	memset(new_data, 0, size + MemHeaderSz);
	Header &hdr = reinterpret_cast<Header &>(*new_data);
	hdr.Size = size;
	void *obj_ptr = &new_data[MemHeaderSz];
	int32_t handle = ccRegisterManagedObject(obj_ptr, &_G(globalDynamicStruct));
	if (handle == 0) {
		delete[] new_data;
		return DynObjectRef();
	}
	return DynObjectRef(handle, obj_ptr, &_G(globalDynamicStruct));
}

int ScriptUserObject::Dispose(void *address, bool /*force*/) {
	delete[] (static_cast<uint8_t *>(address) - MemHeaderSz);
	return 1;
}

size_t ScriptUserObject::CalcSerializeSize(const void *address) {
	const Header &hdr = GetHeader(address);
	return hdr.Size + FileHeaderSz;
}

void ScriptUserObject::Serialize(const void *address, AGS::Shared::Stream *out) {
	const Header &hdr = GetHeader(address);
	// NOTE: we only write the data, no header at the moment
	out->Write(address, hdr.Size);
}

void ScriptUserObject::Unserialize(int index, Stream *in, size_t data_sz) {
	uint8_t *new_data = new uint8_t[(data_sz - FileHeaderSz) + MemHeaderSz];
	Header &hdr = reinterpret_cast<Header &>(*new_data);
	hdr.Size = data_sz - FileHeaderSz;
	in->Read(new_data + MemHeaderSz, data_sz - FileHeaderSz);
	ccRegisterUnserializedObject(index, &new_data[MemHeaderSz], this);
}

// Allocates managed struct containing two ints: X and Y
ScriptUserObject *ScriptStructHelpers::CreatePoint(int x, int y) {
	DynObjectRef ref = ScriptUserObject::Create(sizeof(int32_t) * 2);
	ref.Mgr->WriteInt32(ref.Obj, 0, x);
	ref.Mgr->WriteInt32(ref.Obj, sizeof(int32_t), y);
	return static_cast<ScriptUserObject *>(ref.Obj);
}

} // namespace AGS3
