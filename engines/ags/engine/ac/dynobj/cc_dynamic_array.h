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

#ifndef AGS_ENGINE_AC_DYNOBJ_CC_DYNAMICARRAY_H
#define AGS_ENGINE_AC_DYNOBJ_CC_DYNAMICARRAY_H

#include "ags/lib/std/vector.h"
#include "ags/engine/ac/dynobj/cc_dynamic_object.h"   // ICCDynamicObject

namespace AGS3 {

#define CC_DYNAMIC_ARRAY_TYPE_NAME "CCDynamicArray"
#define ARRAY_MANAGED_TYPE_FLAG    0x80000000

struct CCDynamicArray final : ICCDynamicObject {
	// return the type name of the object
	const char *GetType() override;
	int Dispose(const char *address, bool force) override;
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	int Serialize(const char *address, char *buffer, int bufsize) override;
	virtual void Unserialize(int index, const char *serializedData, int dataSize);
	// Create managed array object and return a pointer to the beginning of a buffer
	DynObjectRef Create(int numElements, int elementSize, bool isManagedType);

	// Legacy support for reading and writing object values by their relative offset
	const char *GetFieldPtr(const char *address, intptr_t offset) override;
	void    Read(const char *address, intptr_t offset, void *dest, int size) override;
	uint8_t ReadInt8(const char *address, intptr_t offset) override;
	int16_t ReadInt16(const char *address, intptr_t offset) override;
	int32_t ReadInt32(const char *address, intptr_t offset) override;
	float   ReadFloat(const char *address, intptr_t offset) override;
	void    Write(const char *address, intptr_t offset, void *src, int size) override;
	void    WriteInt8(const char *address, intptr_t offset, uint8_t val) override;
	void    WriteInt16(const char *address, intptr_t offset, int16_t val) override;
	void    WriteInt32(const char *address, intptr_t offset, int32_t val) override;
	void    WriteFloat(const char *address, intptr_t offset, float val) override;
};

// Helper functions for setting up dynamic arrays.
namespace DynamicArrayHelpers {
// Create array of managed strings
DynObjectRef CreateStringArray(const std::vector<const char *>);
} // namespace DynamicArrayHelpers

} // namespace AGS3

#endif
