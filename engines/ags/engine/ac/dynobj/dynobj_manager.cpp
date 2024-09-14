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

//=============================================================================
//
// C-Script run-time interpreter (c) 2001 Chris Jones
//
// You must DISABLE OPTIMIZATIONS AND REGISTER VARIABLES in your compiler
// when compiling this, or strange results can happen.
//
// There is a problem with importing functions on 16-bit compilers: the
// script system assumes that all parameters are passed as 4 bytes, which
// ints are not on 16-bit systems. Be sure to define all parameters as longs,
// or join the 21st century and switch to DJGPP or Visual C++.
//
//=============================================================================

#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/core/platform.h"
#include "ags/engine/ac/dynobj/managed_object_pool.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/script/cc_common.h"
#include "ags/shared/script/cc_internal.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// register a memory handle for the object and allow script
// pointers to point to it
int32_t ccRegisterManagedObject(void *object, IScriptObject *callback, ScriptValueType obj_type) {
	int32_t handl = _GP(pool).AddObject(object, callback, obj_type);

	ManagedObjectLog("Register managed object type '%s' handle=%d addr=%08X",
	                 ((callback == NULL) ? "(unknown)" : callback->GetType()), handl, object);

	return handl;
}

// register a de-serialized object
int32_t ccRegisterUnserializedObject(int index, void *object, IScriptObject *callback, ScriptValueType obj_type) {
	return _GP(pool).AddUnserializedObject(object, callback, obj_type, index);
}

// unregister a particular object
int ccUnRegisterManagedObject(void *object) {
	return _GP(pool).RemoveObject(object);
}

// remove all registered objects
void ccUnregisterAllObjects() {
	_GP(pool).reset();
}

// serialize all objects to disk
void ccSerializeAllObjects(Stream *out) {
	_GP(pool).WriteToDisk(out);
}

// un-serialise all objects (will remove all currently registered ones)
int ccUnserializeAllObjects(Stream *in, ICCObjectCollectionReader *callback) {
	return _GP(pool).ReadFromDisk(in, callback);
}

// dispose the object if RefCount==0
void ccAttemptDisposeObject(int32_t handle) {
	_GP(pool).CheckDispose(handle);
}

// translate between object handles and memory addresses
int32_t ccGetObjectHandleFromAddress(void *address) {
	// set to null
	if (address == nullptr)
		return 0;

	int32_t handl = _GP(pool).AddressToHandle(address);

	ManagedObjectLog("Line %d WritePtr: %08X to %d", _G(currentline), address, handl);

	if (handl == 0) {
		cc_error("Pointer cast failure: the object being pointed to is not in the managed object pool");
		return -1;
	}
	return handl;
}

void *ccGetObjectAddressFromHandle(int32_t handle) {
	if (handle == 0) {
		return nullptr;
	}
	void *addr = _GP(pool).HandleToAddress(handle);

	ManagedObjectLog("Line %d ReadPtr: %d to %08X", _G(currentline), handle, addr);

	if (addr == nullptr) {
		cc_error("Error retrieving pointer: invalid handle %d", handle);
		return nullptr;
	}
	return addr;
}

ScriptValueType ccGetObjectAddressAndManagerFromHandle(int32_t handle, void *&object, IScriptObject *&manager) {
	if (handle == 0) {
		object = nullptr;
		manager = nullptr;
		return kScValUndefined;
	}
	ScriptValueType obj_type = _GP(pool).HandleToAddressAndManager(handle, object, manager);
	if (obj_type == kScValUndefined) {
		cc_error("Error retrieving pointer: invalid handle %d", handle);
	}
	return obj_type;
}

int ccAddObjectReference(int32_t handle) {
	if (handle == 0)
		return 0;

	return _GP(pool).AddRef(handle);
}

int ccReleaseObjectReference(int32_t handle) {
	if (handle == 0)
		return 0;

	if (_GP(pool).HandleToAddress(handle) == nullptr) {
		cc_error("Error releasing pointer: invalid handle %d", handle);
		return -1;
	}

	return _GP(pool).SubRef(handle);
}

} // namespace AGS3
