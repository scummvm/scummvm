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
#include "ags/engine/ac/dynobj/cc_script_object.h"

namespace AGS3 {

void CCStaticArray::Create(IScriptObject *mgr, size_t elem_script_size, size_t elem_mem_size, size_t elem_count) {
	_mgr = mgr;
	_elemScriptSize = elem_script_size;
	_elemMemSize = elem_mem_size;
	_elemCount = elem_count;
}

void *CCStaticArray::GetFieldPtr(void *address, intptr_t offset) {
	return GetElementPtr(address, offset);
}

void CCStaticArray::Read(void *address, intptr_t offset, uint8_t *dest, size_t size) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->Read(el_ptr, offset % _elemScriptSize, dest, size);
}

uint8_t CCStaticArray::ReadInt8(void *address, intptr_t offset) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadInt8(el_ptr, offset % _elemScriptSize);
}

int16_t CCStaticArray::ReadInt16(void *address, intptr_t offset) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadInt16(el_ptr, offset % _elemScriptSize);
}

int32_t CCStaticArray::ReadInt32(void *address, intptr_t offset) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadInt32(el_ptr, offset % _elemScriptSize);
}

float CCStaticArray::ReadFloat(void *address, intptr_t offset) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->ReadFloat(el_ptr, offset % _elemScriptSize);
}

void CCStaticArray::Write(void *address, intptr_t offset, const uint8_t *src, size_t size) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->Write(el_ptr, offset % _elemScriptSize, src, size);
}

void CCStaticArray::WriteInt8(void *address, intptr_t offset, uint8_t val) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteInt8(el_ptr, offset % _elemScriptSize, val);
}

void CCStaticArray::WriteInt16(void *address, intptr_t offset, int16_t val) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteInt16(el_ptr, offset % _elemScriptSize, val);
}

void CCStaticArray::WriteInt32(void *address, intptr_t offset, int32_t val) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteInt32(el_ptr, offset % _elemScriptSize, val);
}

void CCStaticArray::WriteFloat(void *address, intptr_t offset, float val) {
	void *el_ptr = GetElementPtr(address, offset);
	return _mgr->WriteFloat(el_ptr, offset % _elemScriptSize, val);
}

} // namespace AGS3
