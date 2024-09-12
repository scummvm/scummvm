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
#include "ags/shared/util/stream.h"

namespace AGS3 {

struct ScriptUserObject final : AGSCCDynamicObject {
public:
	static const char *TypeName;

	ScriptUserObject() = default;

protected:
	virtual ~ScriptUserObject();

public:
	static ScriptUserObject *CreateManaged(size_t size);
	void Create(const uint8_t *data, AGS::Shared::Stream *in, size_t size);

	// return the type name of the object
	const char *GetType() override;
	int  Dispose(void *address, bool force) override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

	// Support for reading and writing object values by their relative offset
	void	*GetFieldPtr(void *address, intptr_t offset) override;
	void 	Read(void *address, intptr_t offset, uint8_t *dest, size_t size) override;
	uint8_t ReadInt8(void *address, intptr_t offset) override;
	int16_t ReadInt16(void *address, intptr_t offset) override;
	int32_t ReadInt32(void *address, intptr_t offset) override;
	float	ReadFloat(void *address, intptr_t offset) override;
	void	Write(void *address, intptr_t offset, const uint8_t *src, size_t size) override;
	void	WriteInt8(void *address, intptr_t offset, uint8_t val) override;
	void	WriteInt16(void *address, intptr_t offset, int16_t val) override;
	void	WriteInt32(void *address, intptr_t offset, int32_t val) override;
	void	WriteFloat(void *address, intptr_t offset, float val) override;

private:
	// NOTE: we use signed int for Size at the moment, because the managed
	// object interface's Serialize() function requires the object to return
	// negative value of size in case the provided buffer was not large
	// enough. Since this interface is also a part of Plugin API, we would
	// need more significant change to program before we could use different
	// approach.
	int32_t  _size = 0;
	uint8_t *_data = nullptr;

	// Savegame serialization
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(void *address) override;
	// Write object data into the provided stream
	void Serialize(void *address, AGS::Shared::Stream *out) override;
};


// Helper functions for setting up custom managed structs based on ScriptUserObject.
namespace ScriptStructHelpers {
// Creates a managed Point object, represented as a pair of X and Y coordinates.
ScriptUserObject *CreatePoint(int x, int y);
} // namespace ScriptStructHelpers

} // namespace AGS3

#endif
