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

#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

const char *ScriptString::GetType() {
	return "String";
}

int ScriptString::Dispose(void *address, bool /*force*/) {
	delete[] (static_cast<uint8_t *>(address) - MemHeaderSz);
	return 1;
}

size_t ScriptString::CalcSerializeSize(const void *address) {
	const Header &hdr = GetHeader(address);
	return hdr.Length + 1 + FileHeaderSz;
}

void ScriptString::Serialize(const void *address, Stream *out) {
	const Header &hdr = GetHeader(address);
	out->WriteInt32(hdr.Length);
	out->Write(address, hdr.Length + 1); // it was writing trailing 0 for some reason
}

void ScriptString::Unserialize(int index, Stream *in, size_t /*data_sz*/) {
	size_t len = in->ReadInt32();
	uint8_t *buf = new uint8_t[len + 1 + MemHeaderSz];
	Header &hdr = reinterpret_cast<Header &>(*buf);
	hdr.Length = len;
	char *text_ptr = reinterpret_cast<char *>(buf + MemHeaderSz);
	in->Read(text_ptr, len + 1); // it was writing trailing 0 for some reason
	text_ptr[len] = 0;           // for safety
	ccRegisterUnserializedObject(index, text_ptr, this);
}

DynObjectRef ScriptString::CreateImpl(const char *text, size_t buf_len) {
	size_t len = text ? strlen(text) : buf_len;
	uint8_t *buf = new uint8_t[len + 1 + MemHeaderSz];
	Header &hdr = reinterpret_cast<Header &>(*buf);
	hdr.Length = len;
	char *text_ptr = reinterpret_cast<char *>(buf + MemHeaderSz);
	if (text)
		memcpy(text_ptr, text, len + 1);
	int32_t handle = ccRegisterManagedObject(text_ptr, &_GP(myScriptStringImpl));
	if (handle == 0) {
		delete[] buf;
		return DynObjectRef();
	}
	return DynObjectRef(handle, text_ptr, &_GP(myScriptStringImpl));
}

} // namespace AGS3
