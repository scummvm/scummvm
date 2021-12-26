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

#include "ags/engine/ac/dynobj/cc_gui.h"
#include "ags/engine/ac/dynobj/script_gui.h"
#include "ags/globals.h"

namespace AGS3 {

// return the type name of the object
const char *CCGUI::GetType() {
	return "GUI";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCGUI::Serialize(const char *address, char *buffer, int bufsize) {
	const ScriptGUI *shh = (const ScriptGUI *)address;
	StartSerialize(buffer);
	SerializeInt(shh->id);
	return EndSerialize();
}

void CCGUI::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	int num = UnserializeInt();
	ccRegisterUnserializedObject(index, &_G(scrGui)[num], this);
}

} // namespace AGS3
