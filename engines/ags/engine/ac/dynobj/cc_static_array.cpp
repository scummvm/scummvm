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

#include "ags/engine/ac/dynobj/cc_static_array.h"
#include "ags/engine/ac/dynobj/cc_dynamic_object.h"

namespace AGS3 {

void CCStaticArray::Create(ICCDynamicObject *mgr, int elem_legacy_size, int elem_real_size, int elem_count) {
	_mgr = mgr;
	_elemLegacySize = elem_legacy_size;
	_elemRealSize = elem_real_size;
	_elemCount = elem_count;
}

const char *CCStaticArray::GetElementPtr(const char *address, intptr_t legacy_offset) {
	return address + (legacy_offset / _elemLegacySize) * _elemRealSize;
}

const char *CCStaticArray::GetFieldPtr(const char *address, intptr_t offset) {
	return GetElementPtr(address, offset);
}

void CCStaticArray::Read(const char *address, intptr_t offset, void *dest, int size) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->Read(el_ptr, offset % _elemLegacySize, dest, size);
}

uint8_t CCStaticArray::ReadInt8(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadInt8(el_ptr, offset % _elemLegacySize);
}

int16_t CCStaticArray::ReadInt16(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadInt16(el_ptr, offset % _elemLegacySize);
}

int32_t CCStaticArray::ReadInt32(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadInt32(el_ptr, offset % _elemLegacySize);
}

float CCStaticArray::ReadFloat(const char *address, intptr_t offset) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadFloat(el_ptr, offset % _elemLegacySize);
}

void CCStaticArray::Write(const char *address, intptr_t offset, void *src, int size) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->Write(el_ptr, offset % _elemLegacySize, src, size);
}

void CCStaticArray::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteInt8(el_ptr, offset % _elemLegacySize, val);
}

void CCStaticArray::WriteInt16(const char *address, intptr_t offset, int16_t val) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteInt16(el_ptr, offset % _elemLegacySize, val);
}

void CCStaticArray::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteInt32(el_ptr, offset % _elemLegacySize, val);
}

void CCStaticArray::WriteFloat(const char *address, intptr_t offset, float val) {
	const char *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteFloat(el_ptr, offset % _elemLegacySize, val);
}

} // namespace AGS3
