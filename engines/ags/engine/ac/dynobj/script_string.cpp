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
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/string.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;

DynObjectRef ScriptString::CreateString(const char *fromText) {
	return CreateNewScriptStringObj(fromText);
}

int ScriptString::Dispose(const char *address, bool force) {
	// always dispose
	if (text) {
		free(text);
		text = nullptr;
	}
	delete this;
	return 1;
}

const char *ScriptString::GetType() {
	return "String";
}

size_t ScriptString::CalcSerializeSize() {
	return _len + 1 + sizeof(int32_t);
}

void ScriptString::Serialize(const char *address, Stream *out) {
	const auto *cstr = text ? text : "";
	out->WriteInt32(_len);
	out->Write(cstr, _len + 1);
}

void ScriptString::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	_len = UnserializeInt();
	text = (char *)malloc(_len + 1);
	strcpy(text, &serializedData[bytesSoFar]);
	ccRegisterUnserializedObject(index, text, this);
}

ScriptString::ScriptString() {
	text = nullptr;
	_len = 0;
}

ScriptString::ScriptString(const char *fromText) {
	_len = strlen(fromText);
	text = (char *)malloc(_len + 1);
	memcpy(text, fromText, _len + 1);
}

} // namespace AGS3
