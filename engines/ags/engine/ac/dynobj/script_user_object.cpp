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

// return the type name of the object
const char *ScriptUserObject::GetType() {
	return "UserObject";
}

ScriptUserObject::ScriptUserObject()
	: _size(0)
	, _data(nullptr) {
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

void ScriptUserObject::Create(const char *data, Stream *in, size_t size) {
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

int ScriptUserObject::Dispose(const char *address, bool force) {
	delete this;
	return 1;
}

int ScriptUserObject::Serialize(const char *address, char *buffer, int bufsize) {
	if (_size > bufsize)
		// buffer not big enough, ask for a bigger one
		return -_size;

	memcpy(buffer, _data, _size);
	return _size;
}

void ScriptUserObject::Unserialize(int index, Stream *in, size_t data_sz) {
	Create(nullptr, in, data_sz);
	ccRegisterUnserializedObject(index, this, this);
}

const char *ScriptUserObject::GetFieldPtr(const char *address, intptr_t offset) {
	return _data + offset;
}

void ScriptUserObject::Read(const char *address, intptr_t offset, void *dest, int size) {
	memcpy(dest, _data + offset, size);
}

uint8_t ScriptUserObject::ReadInt8(const char *address, intptr_t offset) {
	return *(uint8_t *)(_data + offset);
}

int16_t ScriptUserObject::ReadInt16(const char *address, intptr_t offset) {
	return *(int16_t *)(_data + offset);
}

int32_t ScriptUserObject::ReadInt32(const char *address, intptr_t offset) {
	return *(int32_t *)(_data + offset);
}

float ScriptUserObject::ReadFloat(const char *address, intptr_t offset) {
	return *(float *)(_data + offset);
}

void ScriptUserObject::Write(const char *address, intptr_t offset, void *src, int size) {
	memcpy((void *)(_data + offset), src, size);
}

void ScriptUserObject::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
	*(uint8_t *)(_data + offset) = val;
}

void ScriptUserObject::WriteInt16(const char *address, intptr_t offset, int16_t val) {
	*(int16_t *)(_data + offset) = val;
}

void ScriptUserObject::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	*(int32_t *)(_data + offset) = val;
}

void ScriptUserObject::WriteFloat(const char *address, intptr_t offset, float val) {
	*(float *)(_data + offset) = val;
}


// Allocates managed struct containing two ints: X and Y
ScriptUserObject *ScriptStructHelpers::CreatePoint(int x, int y) {
	ScriptUserObject *suo = ScriptUserObject::CreateManaged(sizeof(int32_t) * 2);
	suo->WriteInt32((const char *)suo, 0, x);
	suo->WriteInt32((const char *)suo, sizeof(int32_t), y);
	return suo;
}

} // namespace AGS3
