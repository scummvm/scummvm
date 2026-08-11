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

#include "ags/shared/ac/mouse_cursor.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"
#include "common/util.h"

namespace AGS3 {

using namespace AGS::Shared;

void MouseCursor::clear() {
	pic = 0;
	hotx = hoty = 0;
	view = -1;
	name.Empty();
	flags = 0;
}

void MouseCursor::ReadFromFile(Stream *in) {
	pic = in->ReadInt32();
	hotx = in->ReadInt16();
	hoty = in->ReadInt16();
	view = in->ReadInt16();
	StrUtil::ReadCStrCount(legacy_name, in, LEGACY_MAX_CURSOR_NAME_LENGTH);
	flags = in->ReadInt8();
	in->Seek(3); // alignment padding to int32

	name = legacy_name;
}

void MouseCursor::WriteToFile(Stream *out) {
	out->WriteInt32(pic);
	out->WriteInt16(hotx);
	out->WriteInt16(hoty);
	out->WriteInt16(view);
	out->Write(legacy_name, LEGACY_MAX_CURSOR_NAME_LENGTH);
	out->WriteInt8(flags);
	out->WriteByteCount(0, 3); // alignment padding to int32
}

void MouseCursor::ReadFromSavegame(Stream *in, int cmp_ver) {
	pic = in->ReadInt32();
	hotx = static_cast<int16_t>(in->ReadInt32());
	hoty = static_cast<int16_t>(in->ReadInt32());
	view = static_cast<int16_t>(in->ReadInt32());
	flags = static_cast<int8_t>(in->ReadInt32());
	if (cmp_ver >= kCursorSvgVersion_36016)
		animdelay = in->ReadInt32();
}

void MouseCursor::WriteToSavegame(Stream *out) const {
	out->WriteInt32(pic);
	out->WriteInt32(hotx);
	out->WriteInt32(hoty);
	out->WriteInt32(view);
	out->WriteInt32(flags);
	out->WriteInt32(animdelay);
}

} // namespace AGS3
