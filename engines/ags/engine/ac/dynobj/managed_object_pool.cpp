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

#include "common/std/vector.h"
#include "ags/engine/ac/dynobj/managed_object_pool.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/util/string_utils.h"               // fputstring, etc
#include "ags/shared/script/cc_common.h"
#include "ags/shared/script/cc_internal.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

const auto OBJECT_CACHE_MAGIC_NUMBER = 0xa30b;
const auto SERIALIZE_BUFFER_SIZE = 10240;
const auto GARBAGE_COLLECTION_INTERVAL = 1024;
const auto RESERVED_SIZE = 2048;

int ManagedObjectPool::Remove(ManagedObject &o, bool force) {
	const bool can_remove = o.callback->Dispose(o.addr, force) != 0;
	if (!(can_remove || force))
		return 0;

	available_ids.push(o.handle);
	handleByAddress.erase(o.addr);
	ManagedObjectLog("Line %d Disposed managed object handle=%d", currentline, o.handle);
	o = ManagedObject();
	return 1;
}

int32_t ManagedObjectPool::AddRef(int32_t handle) {
	if (handle < 1 || (size_t)handle >= objects.size())
		return 0;

	auto &o = objects[handle];
	if (!o.isUsed())
		return 0;
	o.refCount++;
	ManagedObjectLog("Line %d AddRef: handle=%d new refcount=%d", _G(currentline), o.handle, o.refCount);
	return o.refCount;
}

int ManagedObjectPool::CheckDispose(int32_t handle) {
	if (handle < 1 || (size_t)handle >= objects.size())
		return 1;

	auto &o = objects[handle];
	if (!o.isUsed()) {
		return 1;
	}
	if (o.refCount >= 1) {
		return 0;
	}
	return Remove(o);
}

int32_t ManagedObjectPool::SubRef(int32_t handle) {
	if (handle < 1 || (size_t)handle >= objects.size()) {
		return 0;
	}
	auto &o = objects[handle];
	if (!o.isUsed()) {
		return 0;
	}

	o.refCount--;
	const auto newRefCount = o.refCount;
	const auto canBeDisposed = (o.addr != disableDisposeForObject);
	if (canBeDisposed && o.refCount <= 0) {
		Remove(o);
	}
	// object could be removed at this point, don't use any values.
	ManagedObjectLog("Line %d SubRef: handle=%d new refcount=%d canBeDisposed=%d", _G(currentline), handle, newRefCount, canBeDisposed);
	return newRefCount;
}

int32_t ManagedObjectPool::AddressToHandle(void *addr) {
	if (addr == nullptr) {
		return 0;
	}
	auto it = handleByAddress.find(addr);
	if (it == handleByAddress.end()) {
		return 0;
	}
	return it->_value;
}

// this function is called often (whenever a pointer is used)
void *ManagedObjectPool::HandleToAddress(int32_t handle) {
	if (handle < 1 || (size_t)handle >= objects.size()) {
		return nullptr;
	}
	auto &o = objects[handle];
	if (!o.isUsed()) {
		return nullptr;
	}
	return o.addr;
}

// this function is called often (whenever a pointer is used)
ScriptValueType ManagedObjectPool::HandleToAddressAndManager(int32_t handle, void *&object, IScriptObject *&manager) {
	if ((handle < 1 || (size_t)handle >= objects.size()) || !objects[handle].isUsed()) {
		object = nullptr;
		manager = nullptr;
		return kScValUndefined;
	}
	auto &o = objects[handle];
	object = (void *)(o.addr); // WARNING: This strips the const from the char* pointer.
	manager = o.callback;
	return o.obj_type;
}

int ManagedObjectPool::RemoveObject(void *address) {
	if (address == nullptr) {
		return 0;
	}
	auto it = handleByAddress.find(address);
	if (it == handleByAddress.end()) {
		return 0;
	}

	auto &o = objects[it->_value];
	return Remove(o, true);
}

void ManagedObjectPool::RunGarbageCollectionIfAppropriate() {
	if (objectCreationCounter <= GARBAGE_COLLECTION_INTERVAL) {
		return;
	}
	RunGarbageCollection();
	objectCreationCounter = 0;
}

void ManagedObjectPool::RunGarbageCollection() {
	for (int i = 1; i < nextHandle; i++) {
		auto &o = objects[i];
		if (!o.isUsed()) {
			continue;
		}
		if (o.refCount < 1) {
			Remove(o);
		}
	}
	ManagedObjectLog("Ran garbage collection");
}

int ManagedObjectPool::Add(int handle, void *address, IScriptObject *callback, ScriptValueType obj_type)
{
    auto &o = objects[handle];
    assert(!o.isUsed());

    o = ManagedObject(obj_type, handle, address, callback);

    handleByAddress.insert({address, handle});
    ManagedObjectLog("Allocated managed object type=%s, handle=%d, addr=%08X", callback->GetType(), handle, address);
    return handle;
}

int ManagedObjectPool::AddObject(void *address, IScriptObject *callback, ScriptValueType obj_type) {
	int32_t handle;

	if (!available_ids.empty()) {
		handle = available_ids.front();
		available_ids.pop();
	} else {
		handle = nextHandle++;
		if ((size_t)handle >= objects.size()) {
			objects.resize(handle + 1024, ManagedObject());
		}
	}

	objectCreationCounter++;
	return Add(handle, address, callback, obj_type);
}

int ManagedObjectPool::AddUnserializedObject(void *address, IScriptObject *callback, ScriptValueType obj_type, int handle) {
	if (handle < 1) {
		cc_error("Attempt to assign invalid handle: %d", handle);
		return 0;
	}
	if ((size_t)handle >= objects.size()) {
		objects.resize(handle + 1024, ManagedObject());
	}

	return Add(handle, address, callback, obj_type);
}

void ManagedObjectPool::WriteToDisk(Stream *out) {

	// use this opportunity to clean up any non-referenced pointers
	RunGarbageCollection();

	std::vector<uint8_t> serializeBuffer;
	serializeBuffer.resize(SERIALIZE_BUFFER_SIZE);

	out->WriteInt32(OBJECT_CACHE_MAGIC_NUMBER);
	out->WriteInt32(2);  // version

	int size = 0;
	for (int i = 1; i < nextHandle; i++) {
		auto const &o = objects[i];
		if (o.isUsed()) {
			size += 1;
		}
	}
	out->WriteInt32(size);

	for (int i = 1; i < nextHandle; i++) {
		auto const &o = objects[i];
		if (!o.isUsed()) {
			continue;
		}

		// handle
		out->WriteInt32(o.handle);
		// write the type of the object
		StrUtil::WriteCStr(o.callback->GetType(), out);
		// now write the object data
		int bytesWritten = o.callback->Serialize(o.addr, &serializeBuffer.front(), serializeBuffer.size());
		if ((bytesWritten < 0) && ((size_t)(-bytesWritten) > serializeBuffer.size())) {
			// buffer not big enough, re-allocate with requested size
			serializeBuffer.resize(-bytesWritten);
			bytesWritten = o.callback->Serialize(o.addr, &serializeBuffer.front(), serializeBuffer.size());
		}
		assert(bytesWritten >= 0);
		out->WriteInt32(bytesWritten);
		out->Write(&serializeBuffer.front(), bytesWritten);
		out->WriteInt32(o.refCount);

		ManagedObjectLog("Wrote handle = %d", o.handle);
	}
}

int ManagedObjectPool::ReadFromDisk(Stream *in, ICCObjectCollectionReader *reader) {
	if (in->ReadInt32() != OBJECT_CACHE_MAGIC_NUMBER) {
		cc_error("Data was not written by ccSeralize");
		return -1;
	}

	char typeNameBuffer[200];
	std::vector<char> serializeBuffer;
	serializeBuffer.resize(SERIALIZE_BUFFER_SIZE);

	auto version = in->ReadInt32();

	switch (version) {
	case 1: {
		// IMPORTANT: numObjs is "nextHandleId", which is why we iterate from 1 to numObjs-1
		int numObjs = in->ReadInt32();
		for (int i = 1; i < numObjs; i++) {
			StrUtil::ReadCStr(typeNameBuffer, in, sizeof(typeNameBuffer));
			if (typeNameBuffer[0] != 0) {
				size_t numBytes = in->ReadInt32();
				if (numBytes > serializeBuffer.size()) {
					serializeBuffer.resize(numBytes);
				}
				in->Read(&serializeBuffer.front(), numBytes);
				// Delegate work to ICCObjectReader
				reader->Unserialize(i, typeNameBuffer, &serializeBuffer.front(), numBytes);
				objects[i].refCount = in->ReadInt32();
				ManagedObjectLog("Read handle = %d", objects[i].handle);
			}
		}
	}
	break;
	case 2: {
		// This is actually number of objects written.
		int objectsSize = in->ReadInt32();
		for (int i = 0; i < objectsSize; i++) {
			auto handle = in->ReadInt32();
			assert(handle >= 1);
			StrUtil::ReadCStr(typeNameBuffer, in, sizeof(typeNameBuffer));
			assert(typeNameBuffer[0] != 0);
			size_t numBytes = in->ReadInt32();
			if (numBytes > serializeBuffer.size()) {
				serializeBuffer.resize(numBytes);
			}
			in->Read(&serializeBuffer.front(), numBytes);
			// Delegate work to ICCObjectReader
			reader->Unserialize(handle, typeNameBuffer, &serializeBuffer.front(), numBytes);
			objects[handle].refCount = in->ReadInt32();
			ManagedObjectLog("Read handle = %d", objects[i].handle);
		}
	}
	break;
	default:
		cc_error("Invalid data version: %d", version);
		return -1;
	}

	// re-adjust next handles. (in case saved in random order)
	available_ids = std::queue<int32_t>();
	nextHandle = 1;

	for (const auto &o : objects) {
		if (o.isUsed()) {
			nextHandle = o.handle + 1;
		}
	}
	for (int i = 1; i < nextHandle; i++) {
		if (!objects[i].isUsed()) {
			available_ids.push(i);
		}
	}

	return 0;
}

// de-allocate all objects
void ManagedObjectPool::reset() {
	for (int i = 1; i < nextHandle; i++) {
		auto &o = objects[i];
		if (!o.isUsed()) {
			continue;
		}
		Remove(o, true);
	}
	available_ids = std::queue<int32_t>();
	nextHandle = 1;
}

ManagedObjectPool::ManagedObjectPool() : objectCreationCounter(0), nextHandle(1), available_ids(), objects(RESERVED_SIZE, ManagedObject()), handleByAddress() {
	handleByAddress.reserve(RESERVED_SIZE);
}

} // namespace AGS3
