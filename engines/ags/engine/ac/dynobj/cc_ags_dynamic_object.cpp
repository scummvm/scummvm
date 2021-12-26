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

//include <string.h>
#include "ags/shared/core/types.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/ac/common.h"               // quit()
#include "ags/shared/util/bbop.h"

namespace AGS3 {

using namespace AGS::Shared;

// *** The script serialization routines for built-in types

int AGSCCDynamicObject::Dispose(const char *address, bool force) {
	// cannot be removed from memory
	return 0;
}

void AGSCCDynamicObject::StartSerialize(char *sbuffer) {
	bytesSoFar = 0;
	serbuffer = sbuffer;
}

void AGSCCDynamicObject::SerializeInt(int val) {
	char *chptr = &serbuffer[bytesSoFar];
	int *iptr = (int *)chptr;
	*iptr = BBOp::Int32FromLE(val);
	bytesSoFar += 4;
}

void AGSCCDynamicObject::SerializeFloat(float val) {
	char *chptr = &serbuffer[bytesSoFar];
	float *fptr = (float *)chptr;
	*fptr = BBOp::FloatFromLE(val);
	bytesSoFar += 4;
}

int AGSCCDynamicObject::EndSerialize() {
	return bytesSoFar;
}

void AGSCCDynamicObject::StartUnserialize(const char *sbuffer, int pTotalBytes) {
	bytesSoFar = 0;
	totalBytes = pTotalBytes;
	serbuffer = const_cast<char *>(sbuffer);
}

int AGSCCDynamicObject::UnserializeInt() {
	if (bytesSoFar >= totalBytes)
		quit("Unserialise: internal error: read past EOF");

	char *chptr = &serbuffer[bytesSoFar];
	bytesSoFar += 4;
	return BBOp::Int32FromLE(*((const int *)chptr));
}

float AGSCCDynamicObject::UnserializeFloat() {
	if (bytesSoFar >= totalBytes)
		quit("Unserialise: internal error: read past EOF");

	char *chptr = &serbuffer[bytesSoFar];
	bytesSoFar += 4;
	return BBOp::FloatFromLE(*((const float *)chptr));
}

const char *AGSCCDynamicObject::GetFieldPtr(const char *address, intptr_t offset) {
	return address + offset;
}

void AGSCCDynamicObject::Read(const char *address, intptr_t offset, void *dest, int size) {
	memcpy(dest, address + offset, size);
}

uint8_t AGSCCDynamicObject::ReadInt8(const char *address, intptr_t offset) {
	return *(const uint8_t *)(address + offset);
}

int16_t AGSCCDynamicObject::ReadInt16(const char *address, intptr_t offset) {
	return *(const int16_t *)(address + offset);
}

int32_t AGSCCDynamicObject::ReadInt32(const char *address, intptr_t offset) {
	return *(const int32_t *)(address + offset);
}

float AGSCCDynamicObject::ReadFloat(const char *address, intptr_t offset) {
	return *(const float *)(address + offset);
}

void AGSCCDynamicObject::Write(const char *address, intptr_t offset, void *src, int size) {
	memcpy((void *)(const_cast<char *>(address) + offset), src, size);
}

void AGSCCDynamicObject::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
	*(uint8_t *)(const_cast<char *>(address) + offset) = val;
}

void AGSCCDynamicObject::WriteInt16(const char *address, intptr_t offset, int16_t val) {
	*(int16_t *)(const_cast<char *>(address) + offset) = val;
}

void AGSCCDynamicObject::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	*(int32_t *)(const_cast<char *>(address) + offset) = val;
}

void AGSCCDynamicObject::WriteFloat(const char *address, intptr_t offset, float val) {
	*(float *)(const_cast<char *>(address) + offset) = val;
}

} // namespace AGS3
