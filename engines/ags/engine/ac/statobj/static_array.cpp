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

#include "ags/engine/ac/statobj/static_array.h"
#include "ags/engine/ac/dynobj/cc_dynamic_object.h"

namespace AGS3 {

void StaticArray::Create(int elem_legacy_size, int elem_real_size, int elem_count) {
	_staticMgr = nullptr;
	_dynamicMgr = nullptr;
	_elemLegacySize = elem_legacy_size;
	_elemRealSize = elem_real_size;
	_elemCount = elem_count;
}

void StaticArray::Create(ICCStaticObject *stcmgr, int elem_legacy_size, int elem_real_size, int elem_count) {
	_staticMgr = stcmgr;
	_dynamicMgr = nullptr;
	_elemLegacySize = elem_legacy_size;
	_elemRealSize = elem_real_size;
	_elemCount = elem_count;
}

void StaticArray::Create(ICCDynamicObject *dynmgr, int elem_legacy_size, int elem_real_size, int elem_count) {
	_staticMgr = nullptr;
	_dynamicMgr = dynmgr;
	_elemLegacySize = elem_legacy_size;
	_elemRealSize = elem_real_size;
	_elemCount = elem_count;
}

const char *StaticArray::GetElementPtr(const char *address, intptr_t legacy_offset) {
	return address + (legacy_offset / _elemLegacySize) * _elemRealSize;
}

char *StaticArray::GetElementPtr(char *address, intptr_t legacy_offset) {
	return address + (legacy_offset / _elemLegacySize) * _elemRealSize;
}

const char *StaticArray::GetFieldPtr(const char *address, intptr_t offset) {
	return GetElementPtr(address, offset);
}

void StaticArray::Read(const char *address, intptr_t offset, void *dest, int size) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->Read(el_ptr, offset % _elemLegacySize, dest, size);
	} else if (_dynamicMgr) {
		return _dynamicMgr->Read(el_ptr, offset % _elemLegacySize, dest, size);
	}
	memcpy(dest, el_ptr + offset % _elemLegacySize, size);
}

uint8_t StaticArray::ReadInt8(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->ReadInt8(el_ptr, offset % _elemLegacySize);
	} else if (_dynamicMgr) {
		return _dynamicMgr->ReadInt8(el_ptr, offset % _elemLegacySize);
	}

	return *(const uint8_t *)(el_ptr + offset % _elemLegacySize);
}

int16_t StaticArray::ReadInt16(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->ReadInt16(el_ptr, offset % _elemLegacySize);
	} else if (_dynamicMgr) {
		return _dynamicMgr->ReadInt16(el_ptr, offset % _elemLegacySize);
	}

	return *(const uint16_t *)(el_ptr + offset % _elemLegacySize);
}

int32_t StaticArray::ReadInt32(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->ReadInt32(el_ptr, offset % _elemLegacySize);
	} else if (_dynamicMgr) {
		return _dynamicMgr->ReadInt32(el_ptr, offset % _elemLegacySize);
	}

	return *(const uint32_t *)(el_ptr + offset % _elemLegacySize);
}

float StaticArray::ReadFloat(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->ReadFloat(el_ptr, offset % _elemLegacySize);
	} else if (_dynamicMgr) {
		return _dynamicMgr->ReadFloat(el_ptr, offset % _elemLegacySize);
	}

	return *(const float *)(el_ptr + offset % _elemLegacySize);
}

void StaticArray::Write(const char *address, intptr_t offset, void *src, int size) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->Write(el_ptr, offset % _elemLegacySize, src, size);
	} else if (_dynamicMgr) {
		return _dynamicMgr->Write(el_ptr, offset % _elemLegacySize, src, size);
	} else {
		memcpy((void *)(const_cast<char *>(el_ptr) + offset % _elemLegacySize), src, size);
	}
}

void StaticArray::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->WriteInt8(el_ptr, offset % _elemLegacySize, val);
	} else if (_dynamicMgr) {
		return _dynamicMgr->WriteInt8(el_ptr, offset % _elemLegacySize, val);
	} else {
		*(uint8_t *)(const_cast<char *>(el_ptr) + offset % _elemLegacySize) = val;
	}
}

void StaticArray::WriteInt16(const char *address, intptr_t offset, int16_t val) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->WriteInt16(el_ptr, offset % _elemLegacySize, val);
	} else if (_dynamicMgr) {
		return _dynamicMgr->WriteInt16(el_ptr, offset % _elemLegacySize, val);
	} else {
		*(uint16_t *)(const_cast<char *>(el_ptr) + offset % _elemLegacySize) = val;
	}
}

void StaticArray::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->WriteInt32(el_ptr, offset % _elemLegacySize, val);
	} else if (_dynamicMgr) {
		return _dynamicMgr->WriteInt32(el_ptr, offset % _elemLegacySize, val);
	} else {
		*(uint32_t *)(const_cast<char *>(el_ptr) + offset % _elemLegacySize) = val;
	}
}

void StaticArray::WriteFloat(const char *address, intptr_t offset, float val) {
	const char *el_ptr = GetElementPtr(address, offset);
	if (_staticMgr) {
		return _staticMgr->WriteFloat(el_ptr, offset % _elemLegacySize, val);
	} else if (_dynamicMgr) {
		return _dynamicMgr->WriteFloat(el_ptr, offset % _elemLegacySize, val);
	} else {
		*(float *)(const_cast<char *>(el_ptr) + offset % _elemLegacySize) = val;
	}
}

} // namespace AGS3
