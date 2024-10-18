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

#include "ags/engine/ac/dynobj/script_dict.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"

namespace AGS3 {

int ScriptDictBase::Dispose(void *address, bool force) {
	Clear();
	delete this;
	return 1;
}

const char *ScriptDictBase::GetType() {
	return "StringDictionary";
}

size_t ScriptDictBase::CalcSerializeSize(const void * /*address*/) {
	return CalcContainerSize();
}

void ScriptDictBase::Serialize(const void *address, Stream *out) {
	out->WriteInt32(IsSorted());
	out->WriteInt32(IsCaseSensitive());
	SerializeContainer(out);
}

void ScriptDictBase::Unserialize(int index, Stream *in, size_t data_sz) {
	// NOTE: we expect sorted/case flags are read by external reader;
	// this is awkward, but I did not find better design solution atm
	UnserializeContainer(in);
	ccRegisterUnserializedObject(index, this, this);
}

} // namespace AGS3
