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

#include "ags/engine/ac/dynobj/script_date_time.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;

int ScriptDateTime::Dispose(void *address, bool force) {
	// always dispose a DateTime
	delete this;
	return 1;
}

const char *ScriptDateTime::GetType() {
	return "DateTime";
}

size_t ScriptDateTime::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t) * 7;
}

void ScriptDateTime::Serialize(const void *address, Stream *out) {
	out->WriteInt32(year);
	out->WriteInt32(month);
	out->WriteInt32(day);
	out->WriteInt32(hour);
	out->WriteInt32(minute);
	out->WriteInt32(second);
	out->WriteInt32(rawUnixTime);
}

void ScriptDateTime::Unserialize(int index, Stream *in, size_t data_sz) {
	year = in->ReadInt32();
	month = in->ReadInt32();
	day = in->ReadInt32();
	hour = in->ReadInt32();
	minute = in->ReadInt32();
	second = in->ReadInt32();
	rawUnixTime = in->ReadInt32();
	ccRegisterUnserializedObject(index, this, this);
}

ScriptDateTime::ScriptDateTime() {
	year = month = day = 0;
	hour = minute = second = 0;
	rawUnixTime = 0;
}

} // namespace AGS3
