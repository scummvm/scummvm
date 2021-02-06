/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "ags/engine/ac/dynobj/cc_region.h"
#include "ags/engine/ac/dynobj/scriptregion.h"
#include "ags/shared/ac/common_defines.h"
#include "ags/shared/game/roomstruct.h"

namespace AGS3 {

extern ScriptRegion scrRegion[MAX_ROOM_REGIONS];

// return the type name of the object
const char *CCRegion::GetType() {
	return "Region";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCRegion::Serialize(const char *address, char *buffer, int bufsize) {
	ScriptRegion *shh = (ScriptRegion *)const_cast<char *>(address);
	StartSerialize(buffer);
	SerializeInt(shh->id);
	return EndSerialize();
}

void CCRegion::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	int num = UnserializeInt();
	ccRegisterUnserializedObject(index, &scrRegion[num], this);
}

} // namespace AGS3
