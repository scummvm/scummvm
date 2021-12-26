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
// Managed object, which size and contents are defined by user script
//
//=============================================================================

#ifndef AGS_ENGINE_DYNOBJ__SCRIPTUSERSTRUCT_H
#define AGS_ENGINE_DYNOBJ__SCRIPTUSERSTRUCT_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {

struct ScriptUserObject final : ICCDynamicObject {
public:
	ScriptUserObject();

protected:
	virtual ~ScriptUserObject();

public:
	static ScriptUserObject *CreateManaged(size_t size);
	void            Create(const char *data, size_t size);

	// return the type name of the object
	const char *GetType() override;
	int Dispose(const char *address, bool force) override;
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	int Serialize(const char *address, char *buffer, int bufsize) override;
	virtual void Unserialize(int index, const char *serializedData, int dataSize);

	// Support for reading and writing object values by their relative offset
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

private:
	// NOTE: we use signed int for Size at the moment, because the managed
	// object interface's Serialize() function requires the object to return
	// negative value of size in case the provided buffer was not large
	// enough. Since this interface is also a part of Plugin API, we would
	// need more significant change to program before we could use different
	// approach.
	int32_t  _size;
	char *_data;
};


// Helper functions for setting up custom managed structs based on ScriptUserObject.
namespace ScriptStructHelpers {
// Creates a managed Point object, represented as a pair of X and Y coordinates.
ScriptUserObject *CreatePoint(int x, int y);
} // namespace ScriptStructHelpers

} // namespace AGS3

#endif
