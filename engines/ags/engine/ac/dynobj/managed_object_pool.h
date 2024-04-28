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

#ifndef AGS_ENGINE_AC_DYNOBJ_CC_MANAGED_OBJECT_POOL_H
#define AGS_ENGINE_AC_DYNOBJ_CC_MANAGED_OBJECT_POOL_H

#include "common/std/vector.h"
#include "common/std/queue.h"
#include "common/std/map.h"

#include "ags/shared/core/platform.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_dynamic_object.h"   // ICCDynamicObject

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

struct Pointer_Hash {
	uint operator()(const char *v) const {
		return static_cast<uint>(reinterpret_cast<uintptr>(v));
	}
};


struct ManagedObjectPool final {
private:
	// TODO: find out if we can make handle size_t
	struct ManagedObject {
		ScriptValueType obj_type;
		int32_t handle;
		// TODO: this makes no sense having this as "const char*",
		// void* will be proper (and in all related functions)
		const char *addr;
		ICCDynamicObject *callback;
		int refCount;

		bool isUsed() const {
			return obj_type != kScValUndefined;
		}

		ManagedObject() : obj_type(kScValUndefined), handle(0), addr(nullptr),
			callback(nullptr), refCount(0) {
		}
		ManagedObject(ScriptValueType theType, int32_t theHandle,
		              const char *theAddr, ICCDynamicObject *theCallback)
			: obj_type(theType), handle(theHandle), addr(theAddr),
			  callback(theCallback), refCount(0) {
		}
	};

	int objectCreationCounter;  // used to do garbage collection every so often

	int32_t nextHandle{}; // TODO: manage nextHandle's going over INT32_MAX !
	std::queue<int32_t> available_ids;
	std::vector<ManagedObject> objects;
	std::unordered_map<const char *, int32_t, Pointer_Hash> handleByAddress;

	int Add(int handle, const char *address, ICCDynamicObject *callback, ScriptValueType obj_type);
	int Remove(ManagedObject &o, bool force = false);
	void RunGarbageCollection();

public:

	int32_t AddRef(int32_t handle);
	int CheckDispose(int32_t handle);
	int32_t SubRef(int32_t handle);
	int32_t AddressToHandle(const char *addr);
	const char *HandleToAddress(int32_t handle);
	ScriptValueType HandleToAddressAndManager(int32_t handle, void *&object, ICCDynamicObject *&manager);
	int RemoveObject(const char *address);
	void RunGarbageCollectionIfAppropriate();
	int AddObject(const char *address, ICCDynamicObject *callback, ScriptValueType obj_type);
	int AddUnserializedObject(const char *address, ICCDynamicObject *callback, ScriptValueType obj_type, int handle);
	void WriteToDisk(Shared::Stream *out);
	int ReadFromDisk(Shared::Stream *in, ICCObjectReader *reader);
	void reset();
	ManagedObjectPool();

	const char *disableDisposeForObject{ nullptr };
};

// Extreme(!!) verbosity managed memory pool log
#if DEBUG_MANAGED_OBJECTS
#define ManagedObjectLog(...) Debug::Printf(kDbgGroup_ManObj, kDbgMsg_Debug, __VA_ARGS__)
#else
#define ManagedObjectLog(...)
#endif

} // namespace AGS3

#endif
