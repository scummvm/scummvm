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

#include "ac/dynobj/scriptviewframe.h"

int ScriptViewFrame::Dispose(const char *address, bool force) {
	// always dispose a ViewFrame
	delete this;
	return 1;
}

const char *ScriptViewFrame::GetType() {
	return "ViewFrame";
}

int ScriptViewFrame::Serialize(const char *address, char *buffer, int bufsize) {
	StartSerialize(buffer);
	SerializeInt(view);
	SerializeInt(loop);
	SerializeInt(frame);
	return EndSerialize();
}

void ScriptViewFrame::Unserialize(int index, const char *serializedData, int dataSize) {
	StartUnserialize(serializedData, dataSize);
	view = UnserializeInt();
	loop = UnserializeInt();
	frame = UnserializeInt();
	ccRegisterUnserializedObject(index, this, this);
}

ScriptViewFrame::ScriptViewFrame(int p_view, int p_loop, int p_frame) {
	view = p_view;
	loop = p_loop;
	frame = p_frame;
}

ScriptViewFrame::ScriptViewFrame() {
	view = -1;
	loop = -1;
	frame = -1;
}
