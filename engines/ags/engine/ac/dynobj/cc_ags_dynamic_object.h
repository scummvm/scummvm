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
// The common implementation for ICCDynamicObject interface.
// Intended to be used as a parent class for majority of the
// dynamic object managers.
//
// Basic implementation of:
// * Serialization from a raw buffer; provides a virtual function that
//   accepts Stream, to be implemented in children instead.
// * Provides Unserialize interface that accepts Stream.
// * Data Read/Write methods that treat the contents of the object as
//   a raw byte buffer.
//
//=============================================================================

#ifndef AGS_ENGINE_AC_DYNOBJ_CCDYNAMIC_OBJECT_H
#define AGS_ENGINE_AC_DYNOBJ_CCDYNAMIC_OBJECT_H

#include "ags/engine/ac/dynobj/cc_dynamic_object.h"

namespace AGS3 {

namespace AGS { namespace Shared { class Stream; } }

struct AGSCCDynamicObject : ICCDynamicObject {
protected:
	virtual ~AGSCCDynamicObject() {}
public:
	// default implementation
	int Dispose(const char *address, bool force) override;

	// TODO: pass savegame format version
	int Serialize(const char *address, char *buffer, int bufsize) override;
	// Try unserializing the object from the given input stream
	virtual void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) = 0;

	// Legacy support for reading and writing object values by their relative offset
	const char *GetFieldPtr(const char *address, intptr_t offset) override;
	void    Read(const char *address, intptr_t offset, void *dest, int size) override;
	uint8_t ReadInt8(const char *address, intptr_t offset) override;
	int16_t ReadInt16(const char *address, intptr_t offset) override;
	int32_t ReadInt32(const char *address, intptr_t offset) override;
	float   ReadFloat(const char *address, intptr_t offset) override;
	void    Write(const char *address, intptr_t offset, void *src, int size) override;
	void    WriteInt8(const char *address, intptr_t offset, uint8_t val) override;
	void    WriteInt16(const char *address, intptr_t offset, int16_t val) override;
	void    WriteInt32(const char *address, intptr_t offset, int32_t val) override;
	void    WriteFloat(const char *address, intptr_t offset, float val) override;

protected:
	// Savegame serialization
	// Calculate and return required space for serialization, in bytes
	virtual size_t CalcSerializeSize(const char *address) = 0;
	// Write object data into the provided stream
	virtual void Serialize(const char *address, AGS::Shared::Stream *out) = 0;
};

} // namespace AGS3

#endif
