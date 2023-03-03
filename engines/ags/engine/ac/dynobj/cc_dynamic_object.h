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
// Managed script object interface.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_CC_DYNAMIC_OBJECT_H
#define AGS_ENGINE_AC_DYNOBJ_CC_DYNAMIC_OBJECT_H

#include "ags/lib/std/utility.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

// A pair of managed handle and abstract object pointer
typedef std::pair<int32_t, void *> DynObjectRef;


// OBJECT-BASED SCRIPTING RUNTIME FUNCTIONS
// interface
struct ICCDynamicObject {
	// when a ref count reaches 0, this is called with the address
	// of the object. Return 1 to remove the object from memory, 0 to
	// leave it
	// The "force" flag tells system to detach the object, breaking any links and references
	// to other managed objects or game resources (instead of disposing these too).
	// TODO: it might be better to rewrite the managed pool and remove this flag at all,
	// because it makes the use of this interface prone to mistakes.
	virtual int Dispose(const char *address, bool force = false) = 0;
	// return the type name of the object
	virtual const char *GetType() = 0;
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	// TODO: pass savegame format version
	virtual int Serialize(const char *address, char *buffer, int bufsize) = 0;

	// Legacy support for reading and writing object values by their relative offset.
	// WARNING: following were never a part of plugin API, therefore these methods
	// should **never** be called for kScValPluginObject script objects!
	//
	// RE: GetFieldPtr()
	// According to AGS script specification, when the old-string pointer or char array is passed
	// as an argument, the byte-code does not include any specific command for the member variable
	// retrieval and instructs to pass an address of the object itself with certain offset.
	// This results in functions like StrCopy writing directly over object address.
	// There may be other implementations, but the big question is: how to detect when this is
	// necessary, because byte-code does not contain any distinct operation for this case.
	// The worst thing here is that with the current byte-code structure we can never tell whether
	// offset 0 means getting pointer to whole object or a pointer to its first field.
	virtual const char *GetFieldPtr(const char *address, intptr_t offset) = 0;
	virtual void    Read(const char *address, intptr_t offset, void *dest, int size) = 0;
	virtual uint8_t ReadInt8(const char *address, intptr_t offset) = 0;
	virtual int16_t ReadInt16(const char *address, intptr_t offset) = 0;
	virtual int32_t ReadInt32(const char *address, intptr_t offset) = 0;
	virtual float   ReadFloat(const char *address, intptr_t offset) = 0;
	virtual void    Write(const char *address, intptr_t offset, void *src, int size) = 0;
	virtual void    WriteInt8(const char *address, intptr_t offset, uint8_t val) = 0;
	virtual void    WriteInt16(const char *address, intptr_t offset, int16_t val) = 0;
	virtual void    WriteInt32(const char *address, intptr_t offset, int32_t val) = 0;
	virtual void    WriteFloat(const char *address, intptr_t offset, float val) = 0;

protected:
	ICCDynamicObject() {}
	virtual ~ICCDynamicObject() {}
};

struct ICCObjectReader {
	virtual ~ICCObjectReader() {}
	// TODO: pass savegame format version
	virtual void Unserialize(int index, const char *objectType, const char *serializedData, int dataSize) = 0;
};

struct ICCStringClass {
	virtual ~ICCStringClass() {}
	virtual DynObjectRef CreateString(const char *fromText) = 0;
};

// set the class that will be used for dynamic strings
extern void  ccSetStringClassImpl(ICCStringClass *theClass);
// register a memory handle for the object and allow script
// pointers to point to it
extern int32_t ccRegisterManagedObject(const void *object, ICCDynamicObject *, bool plugin_object = false);
// register a de-serialized object
extern int32_t ccRegisterUnserializedObject(int index, const void *object, ICCDynamicObject *, bool plugin_object = false);
// unregister a particular object
extern int   ccUnRegisterManagedObject(const void *object);
// remove all registered objects
extern void  ccUnregisterAllObjects();
// serialize all objects to disk
extern void  ccSerializeAllObjects(Shared::Stream *out);
// un-serialise all objects (will remove all currently registered ones)
extern int   ccUnserializeAllObjects(Shared::Stream *in, ICCObjectReader *callback);
// dispose the object if RefCount==0
extern void  ccAttemptDisposeObject(int32_t handle);
// translate between object handles and memory addresses
extern int32_t ccGetObjectHandleFromAddress(const void *address);
// TODO: not sure if it makes any sense whatsoever to use "const char*"
// in these functions, might as well change to char* or just void*.
extern const char *ccGetObjectAddressFromHandle(int32_t handle);

extern int ccAddObjectReference(int32_t handle);
extern int ccReleaseObjectReference(int32_t handle);

} // namespace AGS3

#endif
