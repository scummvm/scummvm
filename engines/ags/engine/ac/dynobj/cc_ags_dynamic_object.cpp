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

#include "ags/shared/core/types.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/util/memory_stream.h"

namespace AGS3 {

using namespace AGS::Shared;

int CCBasicObject::Dispose(void * /*address*/, bool /*force*/) {
	return 0; // cannot be removed from memory
}

int CCBasicObject::Serialize(void * /*address*/, uint8_t * /*buffer*/, int /*bufsize*/) {
	return 0; // does not save data
}

void *CCBasicObject::GetFieldPtr(void *address, intptr_t offset) {
	return static_cast<uint8_t *>(address) + offset;
}

void CCBasicObject::Read(void *address, intptr_t offset, uint8_t *dest, size_t size) {
	memcpy(dest, static_cast<uint8_t *>(address) + offset, size);
}

uint8_t CCBasicObject::ReadInt8(void *address, intptr_t offset) {
	return *(uint8_t *)(static_cast<uint8_t *>(address) + offset);
}

int16_t CCBasicObject::ReadInt16(void *address, intptr_t offset) {
	return *(int16_t *)(static_cast<uint8_t *>(address) + offset);
}

int32_t CCBasicObject::ReadInt32(void *address, intptr_t offset) {
	return *(int32_t *)(static_cast<uint8_t *>(address) + offset);
}

float CCBasicObject::ReadFloat(void *address, intptr_t offset) {
	return *(float *)(static_cast<uint8_t *>(address) + offset);
}

void CCBasicObject::Write(void *address, intptr_t offset, const uint8_t *src, size_t size) {
	memcpy(static_cast<uint8_t *>(address) + offset, src, size);
}

void CCBasicObject::WriteInt8(void *address, intptr_t offset, uint8_t val) {
	*(uint8_t *)(static_cast<uint8_t *>(address) + offset) = val;
}

void CCBasicObject::WriteInt16(void *address, intptr_t offset, int16_t val) {
	*(int16_t *)(static_cast<uint8_t *>(address) + offset) = val;
}

void CCBasicObject::WriteInt32(void *address, intptr_t offset, int32_t val) {
	*(int32_t *)(static_cast<uint8_t *>(address) + offset) = val;
}

void CCBasicObject::WriteFloat(void *address, intptr_t offset, float val) {
	*(float *)(static_cast<uint8_t *>(address) + offset) = val;
}

int AGSCCDynamicObject::Serialize(void *address, uint8_t *buffer, int bufsize) {
	// If the required space is larger than the provided buffer,
	// then return negated required space, notifying the caller that a larger buffer is necessary
	size_t req_size = CalcSerializeSize(address);
	assert(req_size <= INT32_MAX); // dynamic object API does not support size > int32
	if (bufsize < 0 || req_size > static_cast<size_t>(bufsize))
		return -(static_cast<int32_t>(req_size));

	MemoryStream mems(reinterpret_cast<uint8_t *>(buffer), bufsize, kStream_Write);
	Serialize(address, &mems);
	return static_cast<int32_t>(mems.GetPosition());
}

} // namespace AGS3
