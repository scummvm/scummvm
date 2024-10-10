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
// Dynamic object management utilities.
// TODO: frankly, many of these functions could be factored out by a direct
// use of ManagedPool class.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_MANAGER_H
#define AGS_ENGINE_AC_DYNOBJ_MANAGER_H

#include "ags/shared/core/types.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/ac/dynobj/cc_script_object.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

// register a memory handle for the object and allow script
// pointers to point to it
extern int32_t ccRegisterManagedObject(void *object, IScriptObject *, ScriptValueType obj_type = kScValScriptObject);
// register a de-serialized object
extern int32_t ccRegisterUnserializedObject(int index, void *object, IScriptObject *, ScriptValueType obj_type = kScValScriptObject);
// unregister a particular object
extern int   ccUnRegisterManagedObject(void *object);
// remove all registered objects
extern void  ccUnregisterAllObjects();
// serialize all objects to disk
extern void  ccSerializeAllObjects(Shared::Stream *out);
// un-serialise all objects (will remove all currently registered ones)
extern int   ccUnserializeAllObjects(Shared::Stream *in, ICCObjectCollectionReader *callback);
// dispose the object if RefCount==0
extern void  ccAttemptDisposeObject(int32_t handle);
// translate between object handles and memory addresses
extern int32_t ccGetObjectHandleFromAddress(void *address);
extern void *ccGetObjectAddressFromHandle(int32_t handle);
extern ScriptValueType ccGetObjectAddressAndManagerFromHandle(int32_t handle, void *&object, IScriptObject *&manager);

extern int ccAddObjectReference(int32_t handle);
extern int ccReleaseObjectReference(int32_t handle);

} // namespace AGS3

#endif
