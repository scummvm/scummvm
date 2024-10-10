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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPT_STRING_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPT_STRING_H

#include "ags/engine/ac/dynobj/cc_ags_dynamic_object.h"

namespace AGS3 {

struct ScriptString final : AGSCCDynamicObject {
public:
	struct Header {
		uint32_t Length = 0u;
	};

	ScriptString() = default;
	~ScriptString() = default;

	inline static const Header &GetHeader(const void *address) {
		return reinterpret_cast<const Header &>(*(static_cast<const uint8_t *>(address) - MemHeaderSz));
	}

	// Create a new script string by copying the given text
	static DynObjectRef Create(const char *text) { return CreateImpl(text, -1); }
	// Create a new script string with a buffer of at least the given text length
	static DynObjectRef Create(size_t buf_len) { return CreateImpl(nullptr, buf_len); }

	const char *GetType() override;
	int Dispose(void *address, bool force) override;
	void Unserialize(int index, AGS::Shared::Stream *in, size_t data_sz) override;

private:
	// The size of the array's header in memory, prepended to the element data
	static const size_t MemHeaderSz = sizeof(Header);
	// The size of the serialized header
	static const size_t FileHeaderSz = sizeof(uint32_t);

	static DynObjectRef CreateImpl(const char *text, size_t buf_len);

	// Savegame serialization
	// Calculate and return required space for serialization, in bytes
	size_t CalcSerializeSize(const void *address) override;
	// Write object data into the provided stream
	void Serialize(const void *address, AGS::Shared::Stream *out) override;
};

} // namespace AGS3

#endif
