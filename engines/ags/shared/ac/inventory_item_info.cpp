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

#include "ags/shared/ac/inventory_item_info.h"
#include "ags/shared/util/stream.h"
#include "ags/shared/util/string_utils.h"

namespace AGS3 {

using namespace AGS::Shared;

void InventoryItemInfo::ReadFromFile(Stream *in) {
	name.ReadCount(in, LEGACY_MAX_INVENTORY_NAME_LENGTH);
	in->Seek(3); // alignment padding to int32
	pic = in->ReadInt32();
	cursorPic = in->ReadInt32();
	hotx = in->ReadInt32();
	hoty = in->ReadInt32();
	in->ReadArrayOfInt32(reserved, 5);
	flags = in->ReadInt8();
	in->Seek(3); // alignment padding to int32
}

void InventoryItemInfo::WriteToFile(Stream *out) {
	name.WriteCount(out, LEGACY_MAX_INVENTORY_NAME_LENGTH);
	out->WriteByteCount(0, 3); // alignment padding to int32
	out->WriteInt32(pic);
	out->WriteInt32(cursorPic);
	out->WriteInt32(hotx);
	out->WriteInt32(hoty);
	out->WriteArrayOfInt32(reserved, 5);
	out->WriteInt8(flags);
	out->WriteByteCount(0, 3); // alignment padding to int32
}

void InventoryItemInfo::ReadFromSavegame(Stream *in) {
	name = StrUtil::ReadString(in);
	pic = in->ReadInt32();
	cursorPic = in->ReadInt32();
}

void InventoryItemInfo::WriteToSavegame(Stream *out) const {
	StrUtil::WriteString(name, out);
	out->WriteInt32(pic);
	out->WriteInt32(cursorPic);
}

} // namespace AGS3
