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

#include "common/std/utility.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS


// A pair of managed handle and abstract object pointer
typedef std::pair<int32_t, void *> DynObjectRef;


struct ICCDynamicObject {
	// WARNING: The first section of this interface is also a part of the AGS plugin API!

	// when a ref count reaches 0, this is called with the address
	// of the object. Return 1 to remove the object from memory, 0 to
	// leave it
	// The "force" flag tells system to detach the object, breaking any links and references
	// to other managed objects or game resources (instead of disposing these too).
	// TODO: it might be better to rewrite the managed pool and remove this flag at all,
	// because it makes the use of this interface prone to mistakes.
	virtual int Dispose(void *address, bool force = false) = 0;
	// return the type name of the object
	virtual const char *GetType() = 0;
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	// TODO: pass savegame format version
	virtual int Serialize(void *address, uint8_t *buffer, int bufsize) = 0;

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
	virtual void	*GetFieldPtr(void *address, intptr_t offset) = 0;
	virtual void 	Read(void *address, intptr_t offset, uint8_t *dest, size_t size) = 0;
	virtual uint8_t ReadInt8(void *address, intptr_t offset) = 0;
	virtual int16_t ReadInt16(void *address, intptr_t offset) = 0;
	virtual int32_t ReadInt32(void *address, intptr_t offset) = 0;
	virtual float	ReadFloat(void *address, intptr_t offset) = 0;
	virtual void 	Write(void *address, intptr_t offset, const uint8_t *src, size_t size) = 0;
	virtual void 	WriteInt8(void *address, intptr_t offset, uint8_t val) = 0;
	virtual void 	WriteInt16(void *address, intptr_t offset, int16_t val) = 0;
	virtual void 	WriteInt32(void *address, intptr_t offset, int32_t val) = 0;
	virtual void 	WriteFloat(void *address, intptr_t offset, float val) = 0;

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

} // namespace AGS3

#endif
