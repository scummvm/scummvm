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

#include "ags/engine/ac/dynobj/script_view_frame.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {

using namespace AGS::Shared;

int ScriptViewFrame::Dispose(void * /*address*/, bool force) {
	// always dispose a ViewFrame
	delete this;
	return 1;
}

const char *ScriptViewFrame::GetType() {
	return "ViewFrame";
}

size_t ScriptViewFrame::CalcSerializeSize(const void * /*address*/) {
	return sizeof(int32_t) * 3;
}

void ScriptViewFrame::Serialize(const void * /*address*/, Stream *out) {
	out->WriteInt32(view);
	out->WriteInt32(loop);
	out->WriteInt32(frame);
}

void ScriptViewFrame::Unserialize(int index, Stream *in, size_t data_sz) {
	view = in->ReadInt32();
	loop = in->ReadInt32();
	frame = in->ReadInt32();
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

} // namespace AGS3
