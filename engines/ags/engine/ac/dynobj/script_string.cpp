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

namespace AGS3 {

using namespace AGS::Shared;

DynObjectRef ScriptString::CreateString(const char *fromText) {
	return CreateNewScriptStringObj(fromText);
}

int ScriptString::Dispose(void * /*address*/, bool force) {
	// always dispose
	if (_text) {
		free(_text);
		_text = nullptr;
	}
	delete this;
	return 1;
}

const char *ScriptString::GetType() {
	return "String";
}

size_t ScriptString::CalcSerializeSize(const void * /*address*/) {
	return _len + 1 + sizeof(int32_t);
}

void ScriptString::Serialize(const void * /*address*/, Stream *out) {
	const auto *cstr = _text ? _text : "";
	out->WriteInt32(_len);
	out->Write(cstr, _len + 1);
}

void ScriptString::Unserialize(int index, Stream *in, size_t /*data_sz*/) {
	_len = in->ReadInt32();
	_text = (char *)malloc(_len + 1);
	in->Read(_text, _len + 1);
	_text[_len] = 0; // for safety
	ccRegisterUnserializedObject(index, _text, this);
}

ScriptString::ScriptString(const char *text) {
	_len = strlen(text);
	_text = (char *)malloc(_len + 1);
	memcpy(_text, text, _len + 1);
}

ScriptString::ScriptString(char *text, bool take_ownership) {
	_len = strlen(text);
	if (take_ownership) {
		_text = text;
	} else {
		_text = (char *)malloc(_len + 1);
		memcpy(_text, text, _len + 1);
	}
}

} // namespace AGS3
