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

#include "ags/shared/ac/dynobj/cc_guiobject.h"
#include "ags/shared/ac/dynobj/scriptgui.h"
#include "ags/shared/gui/guimain.h"
#include "ags/shared/gui/guiobject.h"

namespace AGS3 {

using AGS::Shared::GUIObject;

// return the type name of the object
const char *CCGUIObject::GetType() {
	return "GUIObject";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int CCGUIObject::Serialize(const char *address, char *buffer, int bufsize) {
	GUIObject *guio = (GUIObject *)address;
	StartSerialize(buffer);
	SerializeInt(guio->ParentId);
	SerializeInt(guio->Id);
	return EndSerialize();
}

void CCGUIObject::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	int guinum = UnserializeInt();
	int objnum = UnserializeInt();
	ccRegisterUnserializedObject(index, guis[guinum].GetControl(objnum), this);
}

} // namespace AGS3
