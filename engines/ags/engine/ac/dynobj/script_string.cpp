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

namespace AGS3 {

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

int ScriptString::Serialize(const char *address, char *buffer, int bufsize) {
	StartSerialize(buffer);

	auto toSerialize = text ? text : "";

	auto len = strlen(toSerialize);
	SerializeInt(len);
	strcpy(&serbuffer[bytesSoFar], toSerialize);
	bytesSoFar += len + 1;

	return EndSerialize();
}

void ScriptString::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	int textsize = UnserializeInt();
	text = (char *)malloc(textsize + 1);
	strcpy(text, &serializedData[bytesSoFar]);
	ccRegisterUnserializedObject(index, text, this);
}

ScriptString::ScriptString() {
	text = nullptr;
}

ScriptString::ScriptString(const char *fromText) {
	text = (char *)malloc(strlen(fromText) + 1);
	strcpy(text, fromText);
}

} // namespace AGS3
