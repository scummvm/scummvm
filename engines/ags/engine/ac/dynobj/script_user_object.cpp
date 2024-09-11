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

namespace AGS3 {

using namespace AGS::Shared;

const char *ScriptUserObject::TypeName = "UserObject";

// return the type name of the object
const char *ScriptUserObject::GetType() {
	return TypeName;
}

ScriptUserObject::~ScriptUserObject() {
	delete[] _data;
}

/* static */ ScriptUserObject *ScriptUserObject::CreateManaged(size_t size) {
	ScriptUserObject *suo = new ScriptUserObject();
	suo->Create(nullptr, nullptr, size);
	ccRegisterManagedObject(suo, suo);
	return suo;
}

void ScriptUserObject::Create(const uint8_t *data, Stream *in, size_t size) {
	delete[] _data;
	_data = nullptr;

	_size = size;
	if (_size > 0) {
		_data = new char[size];
		if (data)
			memcpy(_data, data, _size);
		else if (in)
			in->Read(_data, _size);
		else
			memset(_data, 0, _size);
	}
}

int ScriptUserObject::Dispose(void * /*address*/, bool force) {
	delete this;
	return 1;
}

size_t ScriptUserObject::CalcSerializeSize(void * /*address*/) {
	return _size;
}

void ScriptUserObject::Serialize(void * /*address*/, AGS::Shared::Stream *out) {
	out->Write(_data, _size);
}

void ScriptUserObject::Unserialize(int index, Stream *in, size_t data_sz) {
	Create(nullptr, in, data_sz);
	ccRegisterUnserializedObject(index, this, this);
}

void *ScriptUserObject::GetFieldPtr(void * /*address*/, intptr_t offset) {
	return _data + offset;
}

void ScriptUserObject::Read(void * /*address*/, intptr_t offset, uint8_t *dest, size_t size) {
	memcpy(dest, _data + offset, size);
}

uint8_t ScriptUserObject::ReadInt8(void * /*address*/, intptr_t offset) {
	return *(uint8_t *)(_data + offset);
}

int16_t ScriptUserObject::ReadInt16(void * /*address*/, intptr_t offset) {
	return *(int16_t *)(_data + offset);
}

int32_t ScriptUserObject::ReadInt32(void * /*address*/, intptr_t offset) {
	return *(int32_t *)(_data + offset);
}

float ScriptUserObject::ReadFloat(void * /*address*/, intptr_t offset) {
	return *(float *)(_data + offset);
}

void ScriptUserObject::Write(void * /*address*/, intptr_t offset, const uint8_t *src, size_t size) {
	memcpy((void *)(_data + offset), src, size);
}

void ScriptUserObject::WriteInt8(void * /*address*/, intptr_t offset, uint8_t val) {
	*(uint8_t *)(_data + offset) = val;
}

void ScriptUserObject::WriteInt16(void * /*address*/, intptr_t offset, int16_t val) {
	*(int16_t *)(_data + offset) = val;
}

void ScriptUserObject::WriteInt32(void * /*address*/, intptr_t offset, int32_t val) {
	*(int32_t *)(_data + offset) = val;
}

void ScriptUserObject::WriteFloat(void * /*address*/, intptr_t offset, float val) {
	*(float *)(_data + offset) = val;
}


// Allocates managed struct containing two ints: X and Y
ScriptUserObject *ScriptStructHelpers::CreatePoint(int x, int y) {
	ScriptUserObject *suo = ScriptUserObject::CreateManaged(sizeof(int32_t) * 2);
	suo->WriteInt32(suo, 0, x);
	suo->WriteInt32(suo, sizeof(int32_t), y);
	return suo;
}

} // namespace AGS3
