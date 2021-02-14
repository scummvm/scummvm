/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_AC_DYNOBJ_MANAGEDOBJECTPOOL_H
#define AGS_ENGINE_AC_DYNOBJ_MANAGEDOBJECTPOOL_H

#include "ags/lib/std/vector.h"
#include "ags/lib/std/queue.h"
#include "ags/lib/std/map.h"
#include "ags/engine/script/runtimescriptvalue.h"
#include "ags/engine/ac/dynobj/cc_dynamicobject.h"   // ICCDynamicObject
#include "ags/shared/util/string_types.h"

namespace AGS3 {
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

struct Pointer_Hash {
	uint operator()(const char *v) const {
		uint x = static_cast<uint>(reinterpret_cast<uintptr>(v));
		return x + (x >> 3);
	}
};

struct ManagedObjectPool final {
private:
	// TODO: find out if we can make handle size_t
	struct ManagedObject {
		ScriptValueType obj_type;
		int handle;
		// TODO: this makes no sense having this as "const char*",
		// void* will be proper (and in all related functions)
		const char *addr;
		ICCDynamicObject *callback;
		int refCount;

		bool isUsed() const {
			return obj_type != kScValUndefined;
		}

		ManagedObject()
			: obj_type(kScValUndefined), handle(0), addr(nullptr), callback(nullptr), refCount(0) {
		}
		ManagedObject(ScriptValueType obj_type_, int handle_, const char *addr_, ICCDynamicObject *callback_)
			: obj_type(obj_type_), handle(handle_), addr(addr_), callback(callback_), refCount(0) {
		}
	};

	int objectCreationCounter;  // used to do garbage collection every so often

	int nextHandle{}; // TODO: manage nextHandle's going over INT32_MAX !
	std::queue<int> available_ids;
	std::vector<ManagedObject> objects;
	std::unordered_map<const char *, int, Pointer_Hash> handleByAddress;

	void Init(int theHandle, const char *theAddress, ICCDynamicObject *theCallback, ScriptValueType objType);
	int Remove(ManagedObject &o, bool force = false);

	void RunGarbageCollection();

public:

	int AddRef(int handle);
	int CheckDispose(int handle);
	int SubRef(int handle);
	int AddressToHandle(const char *addr);
	const char *HandleToAddress(int handle);
	ScriptValueType HandleToAddressAndManager(int handle, void *&object, ICCDynamicObject *&manager);
	int RemoveObject(const char *address);
	void RunGarbageCollectionIfAppropriate();
	int AddObject(const char *address, ICCDynamicObject *callback, bool plugin_object);
	int AddUnserializedObject(const char *address, ICCDynamicObject *callback, bool plugin_object, int handle);
	void WriteToDisk(Shared::Stream *out);
	int ReadFromDisk(Shared::Stream *in, ICCObjectReader *reader);
	void reset();
	ManagedObjectPool();

	const char *disableDisposeForObject{ nullptr };
};

extern ManagedObjectPool pool;

#ifdef DEBUG_MANAGED_OBJECTS
#define ManagedObjectLog(...) Debug::Printf(kDbgGroup_ManObj, kDbgMsg_Debug, __VA_ARGS__)
#else
#define ManagedObjectLog(...)
#endif

} // namespace AGS3

#endif
