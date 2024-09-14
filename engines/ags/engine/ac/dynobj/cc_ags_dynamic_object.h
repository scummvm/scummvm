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
// This is a collection of common implementations of the IScriptObject
// interface. Intended to be used as parent classes for majority of the
// script object managers.
//
// CCBasicObject: parent for managers that treat object contents as raw
// byte buffer.
//
// AGSCCDynamicObject, extends CCBasicObject: parent for built-in dynamic
// object managers; provides simplier serialization methods working with
// streams instead of a raw memory buffer.
//
// AGSCCStaticObject, extends CCBasicObject: a formal stub, intended as
// a parent for built-in static object managers.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_CCDYNAMIC_OBJECT_H
#define AGS_ENGINE_AC_DYNOBJ_CCDYNAMIC_OBJECT_H

#include "ags/engine/ac/dynobj/cc_script_object.h"

namespace AGS3 {

namespace AGS { namespace Shared { class Stream; } }

// CCBasicObject: basic implementation of the script object interface,
// intended to be used as a parent for object/manager classes that do not
// require specific implementation.
// * Dispose ignored, never deletes any data on its own;
// * Serialization skipped, does not save or load anything;
// * Provides default implementation for reading and writing data fields,
//   treats the contents of an object as a raw byte buffer.
struct CCBasicObject : public IScriptObject {
public:
	virtual ~CCBasicObject() = default;

	// Dispose the object
	int Dispose(void * /*address*/, bool /*force*/) override;
	// Serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	int Serialize(void * /*address*/, uint8_t * /*buffer*/, int /*bufsize*/) override;

	//
	// Legacy support for reading and writing object fields by their relative offset
	//
	void *GetFieldPtr(void *address, intptr_t offset) override;
	void Read(void *address, intptr_t offset, uint8_t *dest, size_t size) override;
	uint8_t ReadInt8(void *address, intptr_t offset) override;
	int16_t ReadInt16(void *address, intptr_t offset) override;
	int32_t ReadInt32(void *address, intptr_t offset) override;
	float ReadFloat(void *address, intptr_t offset) override;
	void Write(void *address, intptr_t offset, const uint8_t *src, size_t size) override;
	void WriteInt8(void *address, intptr_t offset, uint8_t val) override;
	void WriteInt16(void *address, intptr_t offset, int16_t val) override;
	void WriteInt32(void *address, intptr_t offset, int32_t val) override;
	void WriteFloat(void *address, intptr_t offset, float val) override;
};


// AGSCCDynamicObject: standard parent implementation for the built-in
// script objects/manager.
// * Serialization from a raw buffer; provides a virtual function that
//   accepts Stream, to be implemented in children instead.
// * Provides Unserialize interface that accepts Stream.
struct AGSCCDynamicObject : public CCBasicObject {
public:
	virtual ~AGSCCDynamicObject() = default;

	// TODO: pass savegame format version
	int Serialize(void *address, uint8_t *buffer, int bufsize) override;
	// Try unserializing the object from the given input stream
	virtual void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) = 0;

protected:
	// Savegame serialization
	// Calculate and return required space for serialization, in bytes
	virtual size_t CalcSerializeSize(const void *address) = 0;
	// Write object data into the provided stream
	virtual void Serialize(const void *address, AGS::Shared::Stream *out) = 0;
};

// CCStaticObject is a base class for managing static global objects in script.
// The static objects can never be disposed, and do not support serialization
// through IScriptObject interface.
struct AGSCCStaticObject : public CCBasicObject {
public:
	virtual ~AGSCCStaticObject() = default;

	const char *GetType() override { return "StaticObject"; }
};

} // namespace AGS3

#endif
