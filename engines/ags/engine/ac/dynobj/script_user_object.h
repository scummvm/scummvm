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
// ScriptUserObject is a dynamic (managed) struct manager.
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

	struct Header {
		uint32_t Size = 0u;
		// NOTE: we use signed int for Size at the moment, because the managed
		// object interface's Serialize() function requires the object to return
		// negative value of size in case the provided buffer was not large
		// enough. Since this interface is also a part of Plugin API, we would
		// need more significant change to program before we could use different
		// approach.
	};

	ScriptUserObject() = default;
	~ScriptUserObject() = default;

	inline static const Header &GetHeader(const void *address) {
		return reinterpret_cast<const Header &>(*(static_cast<const uint8_t *>(address) - MemHeaderSz));
	}

	// Create managed struct object and return a pointer to the beginning of a buffer
	static DynObjectRef Create(size_t size);

	// return the type name of the object
	const char *GetType() override;
	int  Dispose(void *address, bool force) override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

private:
	// The size of the array's header in memory, prepended to the element data
	static const size_t MemHeaderSz = sizeof(Header);
	// The size of the serialized header
	static const size_t FileHeaderSz = sizeof(uint32_t) * 0; // no header serialized

	// Savegame serialization
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;
};

// Helper functions for setting up custom managed structs based on ScriptUserObject.
namespace ScriptStructHelpers {
// Creates a managed Point object, represented as a pair of X and Y coordinates.
ScriptUserObject *CreatePoint(int x, int y);
} // namespace ScriptStructHelpers

} // namespace AGS3

#endif
