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

#include "ac/mousecursor.h"
#include "util/stream.h"

namespace AGS3 {

using AGS::Shared::Stream;

MouseCursor::MouseCursor() {
	pic = 2054;
	hotx = 0;
	hoty = 0;
	name[0] = 0;
	flags = 0;
	view = -1;
}

void MouseCursor::ReadFromFile(Stream *in) {
	pic = in->ReadInt32();
	hotx = in->ReadInt16();
	hoty = in->ReadInt16();
	view = in->ReadInt16();
	in->Read(name, 10);
	flags = in->ReadInt8();
}

void MouseCursor::WriteToFile(Stream *out) {
	out->WriteInt32(pic);
	out->WriteInt16(hotx);
	out->WriteInt16(hoty);
	out->WriteInt16(view);
	out->Write(name, 10);
	out->WriteInt8(flags);
}

void MouseCursor::ReadFromSavegame(Stream *in) {
	pic = in->ReadInt32();
	hotx = in->ReadInt32();
	hoty = in->ReadInt32();
	view = in->ReadInt32();
	flags = in->ReadInt32();
}

void MouseCursor::WriteToSavegame(Stream *out) const {
	out->WriteInt32(pic);
	out->WriteInt32(hotx);
	out->WriteInt32(hoty);
	out->WriteInt32(view);
	out->WriteInt32(flags);
}

} // namespace AGS3
