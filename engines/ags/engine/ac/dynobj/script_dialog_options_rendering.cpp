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

#include "ags/engine/ac/dynobj/script_dialog_options_rendering.h"

namespace AGS3 {

// return the type name of the object
const char *ScriptDialogOptionsRendering::GetType() {
	return "DialogOptionsRendering";
}

// serialize the object into BUFFER (which is BUFSIZE bytes)
// return number of bytes used
int ScriptDialogOptionsRendering::Serialize(const char *address, char *buffer, int bufsize) {
	return 0;
}

void ScriptDialogOptionsRendering::Unserialize(int index, const char *serializedData, int dataSize) {
	ccRegisterUnserializedObject(index, this, this);
}

void ScriptDialogOptionsRendering::Reset() {
	x = 0;
	y = 0;
	width = 0;
	height = 0;
	hasAlphaChannel = false;
	parserTextboxX = 0;
	parserTextboxY = 0;
	parserTextboxWidth = 0;
	dialogID = 0;
	surfaceToRenderTo = nullptr;
	surfaceAccessed = false;
	activeOptionID = -1;
	chosenOptionID = -1;
	needRepaint = false;
}

ScriptDialogOptionsRendering::ScriptDialogOptionsRendering() {
	Reset();
}

} // namespace AGS3
