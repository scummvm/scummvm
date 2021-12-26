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

#include "ags/engine/ac/dynobj/script_set.h"

namespace AGS3 {

int ScriptSetBase::Dispose(const char *address, bool force) {
	Clear();
	delete this;
	return 1;
}

const char *ScriptSetBase::GetType() {
	return "StringSet";
}

int ScriptSetBase::Serialize(const char *address, char *buffer, int bufsize) {
	size_t total_sz = CalcSerializeSize() + sizeof(int32_t) * 2;
	if (bufsize < 0 || total_sz > (size_t)bufsize) {
		// buffer not big enough, ask for a bigger one
		return -((int)total_sz);
	}
	StartSerialize(buffer);
	SerializeInt(IsSorted());
	SerializeInt(IsCaseSensitive());
	SerializeContainer();
	return EndSerialize();
}

void ScriptSetBase::Unserialize(int index, const char *serializedData, int dataSize) {
	// NOTE: we expect sorted/case flags are read by external reader;
	// this is awkward, but I did not find better design solution atm
	StartUnserialize(serializedData, dataSize);
	UnserializeContainer(serializedData);
	ccRegisterUnserializedObject(index, this, this);
}

} // namespace AGS3
