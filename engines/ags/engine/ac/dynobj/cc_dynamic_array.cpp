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
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *CCDynamicArray::TypeName = "CCDynamicArray";

// return the type name of the object
const char *CCDynamicArray::GetType() {
	return TypeName;
}

int CCDynamicArray::Dispose(void *address, bool force) {
	// If it's an array of managed objects, release their ref counts;
	// except if this array is forcefully removed from the managed pool,
	// in which case just ignore these.
	if (!force) {
		const Header &hdr = GetHeader(address);
		bool is_managed = (hdr.ElemCount & ARRAY_MANAGED_TYPE_FLAG) != 0;
		const uint32_t el_count = hdr.ElemCount & (~ARRAY_MANAGED_TYPE_FLAG);

		if (is_managed) { // Dynamic array of managed pointers: subref them directly
			const uint32_t *handles = reinterpret_cast<const uint32_t *>(address);
			for (uint32_t i = 0; i < el_count; ++i) {
				if (handles[i] > 0)
					ccReleaseObjectReference(handles[i]);
			}
		}
	}

	delete[] (static_cast<uint8_t *>(address) - MemHeaderSz);
	return 1;
}

size_t CCDynamicArray::CalcSerializeSize(const void *address) {
	const Header &hdr = GetHeader(address);
	return hdr.TotalSize + FileHeaderSz;
}

void CCDynamicArray::Serialize(const void *address, Stream *out) {
	const Header &hdr = GetHeader(address);
	out->WriteInt32(hdr.ElemCount);
	out->WriteInt32(hdr.TotalSize);
	out->Write(address, hdr.TotalSize); // elements
}

void CCDynamicArray::Unserialize(int index, Stream *in, size_t data_sz) {
	uint8_t *new_arr = new uint8_t[(data_sz - FileHeaderSz) + MemHeaderSz];
	Header &hdr = reinterpret_cast<Header &>(*new_arr);
	hdr.ElemCount = in->ReadInt32();
	hdr.TotalSize = in->ReadInt32();
	in->Read(new_arr + MemHeaderSz, data_sz - FileHeaderSz);
	ccRegisterUnserializedObject(index, &new_arr[MemHeaderSz], this);
}

/* static */ DynObjectRef CCDynamicArray::Create(int numElements, int elementSize, bool isManagedType) {
	uint8_t *new_arr = new uint8_t[numElements * elementSize + MemHeaderSz];
	memset(new_arr, 0, numElements * elementSize + MemHeaderSz);
	Header &hdr = reinterpret_cast<Header &>(*new_arr);
	hdr.ElemCount = numElements | (ARRAY_MANAGED_TYPE_FLAG * isManagedType);
	hdr.TotalSize = elementSize * numElements;
	void *obj_ptr = &new_arr[MemHeaderSz];
	int32_t handle = ccRegisterManagedObject(obj_ptr, &_GP(globalDynamicArray));
	if (handle == 0) {
		delete[] new_arr;
		return DynObjectRef();
	}
	return DynObjectRef(handle, obj_ptr, &_GP(globalDynamicArray));
}

DynObjectRef DynamicArrayHelpers::CreateStringArray(const std::vector<const char *> items) {
	// NOTE: we need element size of "handle" for array of managed pointers
	DynObjectRef arr = _GP(globalDynamicArray).Create(items.size(), sizeof(int32_t), true);
	if (!arr.Obj)
		return arr;
	// Create script strings and put handles into array
	int32_t *slots = static_cast<int32_t *>(arr.Obj);
	for (auto s : items) {
		DynObjectRef str = ScriptString::Create(s);
		// We must add reference count, because the string is going to be saved
		// within another object (array), not returned to script directly
		ccAddObjectReference(str.Handle);
		*(slots++) = str.Handle;
	}

	return arr;
}

} // namespace AGS3
