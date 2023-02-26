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

#include "ags/engine/ac/dynobj/cc_dynamic_array.h"
#include  "ags/globals.h"

namespace AGS3 {

// return the type name of the object
const char *CCDynamicArray::GetType() {
	return CC_DYNAMIC_ARRAY_TYPE_NAME;
}

int CCDynamicArray::Dispose(const char *address, bool force) {
	address -= 8;

	// If it's an array of managed objects, release their ref counts;
	// except if this array is forcefully removed from the managed pool,
	// in which case just ignore these.
	if (!force) {
		int *elementCount = (int *)const_cast<char *>(address);
		if (elementCount[0] & ARRAY_MANAGED_TYPE_FLAG) {
			elementCount[0] &= ~ARRAY_MANAGED_TYPE_FLAG;
			for (int i = 0; i < elementCount[0]; i++) {
				if (elementCount[2 + i] != 0) {
					ccReleaseObjectReference(elementCount[2 + i]);
				}
			}
		}
	}

	delete[] address;
	return 1;
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCDynamicArray::Serialize(const char *address, char *buffer, int bufsize) {
	const int *sizeInBytes = &((const int *)address)[-1];
	int sizeToWrite = *sizeInBytes + 8;
	if (sizeToWrite > bufsize) {
		// buffer not big enough, ask for a bigger one
		return -sizeToWrite;
	}
	memcpy(buffer, address - 8, sizeToWrite);
	return sizeToWrite;
}

void CCDynamicArray::Unserialize(int index, const char *serializedData, int dataSize) {
	char *newArray = new char[dataSize];
	memcpy(newArray, serializedData, dataSize);
	ccRegisterUnserializedObject(index, &newArray[8], this);
}

DynObjectRef CCDynamicArray::Create(int numElements, int elementSize, bool isManagedType) {
	char *newArray = new char[numElements * elementSize + 8]();
	int *sizePtr = (int *)newArray;
	sizePtr[0] = numElements;
	sizePtr[1] = numElements * elementSize;
	if (isManagedType)
		sizePtr[0] |= ARRAY_MANAGED_TYPE_FLAG;
	void *obj_ptr = &newArray[8];
	int32_t handle = ccRegisterManagedObject(obj_ptr, this);
	if (handle == 0) {
		delete[] newArray;
		return DynObjectRef(0, nullptr);
	}
	return DynObjectRef(handle, obj_ptr);
}


const char *CCDynamicArray::GetFieldPtr(const char *address, intptr_t offset) {
	return address + offset;
}

void CCDynamicArray::Read(const char *address, intptr_t offset, void *dest, int size) {
	memcpy(dest, address + offset, size);
}

uint8_t CCDynamicArray::ReadInt8(const char *address, intptr_t offset) {
	return *(const uint8_t *)(address + offset);
}

int16_t CCDynamicArray::ReadInt16(const char *address, intptr_t offset) {
	return *(const int16_t *)(address + offset);
}

int32_t CCDynamicArray::ReadInt32(const char *address, intptr_t offset) {
	return *(const int32_t *)(address + offset);
}

float CCDynamicArray::ReadFloat(const char *address, intptr_t offset) {
	return *(const float *)(address + offset);
}

void CCDynamicArray::Write(const char *address, intptr_t offset, void *src, int size) {
	memcpy((void *)(const_cast<char *>(address) + offset), src, size);
}

void CCDynamicArray::WriteInt8(const char *address, intptr_t offset, uint8_t val) {
	*(uint8_t *)(const_cast<char *>(address) + offset) = val;
}

void CCDynamicArray::WriteInt16(const char *address, intptr_t offset, int16_t val) {
	*(int16_t *)(const_cast<char *>(address) + offset) = val;
}

void CCDynamicArray::WriteInt32(const char *address, intptr_t offset, int32_t val) {
	*(int32_t *)(const_cast<char *>(address) + offset) = val;
}

void CCDynamicArray::WriteFloat(const char *address, intptr_t offset, float val) {
	*(float *)(const_cast<char *>(address) + offset) = val;
}

DynObjectRef DynamicArrayHelpers::CreateStringArray(const std::vector<const char *> items) {
	// NOTE: we need element size of "handle" for array of managed pointers
	DynObjectRef arr = _GP(globalDynamicArray).Create(items.size(), sizeof(int32_t), true);
	if (!arr.second)
		return arr;
	// Create script strings and put handles into array
	int32_t *slots = static_cast<int32_t *>(arr.second);
	for (auto s : items) {
		DynObjectRef str = _G(stringClassImpl)->CreateString(s);
		// We must add reference count, because the string is going to be saved
		// within another object (array), not returned to script directly
		ccAddObjectReference(str.first);
		*(slots++) = str.first;
	}

	return arr;
}

} // namespace AGS3
