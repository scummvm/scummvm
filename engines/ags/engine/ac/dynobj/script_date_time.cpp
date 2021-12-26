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

namespace AGS3 {

int ScriptDateTime::Dispose(const char *address, bool force) {
	// always dispose a DateTime
	delete this;
	return 1;
}

const char *ScriptDateTime::GetType() {
	return "DateTime";
}

int ScriptDateTime::Serialize(const char *address, char *buffer, int bufsize) {
	StartSerialize(buffer);
	SerializeInt(year);
	SerializeInt(month);
	SerializeInt(day);
	SerializeInt(hour);
	SerializeInt(minute);
	SerializeInt(second);
	SerializeInt(rawUnixTime);
	return EndSerialize();
}

void ScriptDateTime::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	year = UnserializeInt();
	month = UnserializeInt();
	day = UnserializeInt();
	hour = UnserializeInt();
	minute = UnserializeInt();
	second = UnserializeInt();
	rawUnixTime = UnserializeInt();
	ccRegisterUnserializedObject(index, this, this);
}

ScriptDateTime::ScriptDateTime() {
	year = month = day = 0;
	hour = minute = second = 0;
	rawUnixTime = 0;
}

} // namespace AGS3
