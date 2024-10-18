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

#include "common/std/vector.h"
#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

#define ARRAY_MANAGED_TYPE_FLAG    0x80000000

struct CCDynamicArray final : AGSCCDynamicObject {
public:
	static const char *TypeName;

	struct Header {
		// May contain ARRAY_MANAGED_TYPE_FLAG
		uint32_t ElemCount = 0u;
		// TODO: refactor and store "elem size" instead
		uint32_t TotalSize = 0u;
	};

	CCDynamicArray() = default;
	~CCDynamicArray() = default;

	inline static const Header &GetHeader(const void *address) {
		return reinterpret_cast<const Header &>(*(static_cast<const uint8_t *>(address) - MemHeaderSz));
	}

	// Create managed array object and return a pointer to the beginning of a buffer
	static DynObjectRef Create(int numElements, int elementSize, bool isManagedType);

	// return the type name of the object
	const char *GetType() override;
	int Dispose(void *address, bool force) override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

private:
	// The size of the array's header in memory, prepended to the element data
	static const size_t MemHeaderSz = sizeof(Header);
	// The size of the serialized header
	static const size_t FileHeaderSz = sizeof(uint32_t) * 2;

	// Savegame serialization
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;
};

// Helper functions for setting up dynamic arrays.
namespace DynamicArrayHelpers {
// Create array of managed strings
DynObjectRef CreateStringArray(const std::vector<const char *>);
} // namespace DynamicArrayHelpers

} // namespace AGS3

#endif
