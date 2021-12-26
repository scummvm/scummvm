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

#ifndef AGS_SHARED_AC_INVENTORY_ITEM_INFO_H
#define AGS_SHARED_AC_INVENTORY_ITEM_INFO_H

#include "ags/shared/core/types.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class Stream;
} // namespace Shared
} // namespace AGS

using namespace AGS; // FIXME later

#define IFLG_STARTWITH 1
struct InventoryItemInfo {
	char name[25];
	int  pic;
	int  cursorPic, hotx, hoty;
	int32_t reserved[5];
	int8 flags;

	void ReadFromFile(Shared::Stream *in);
	void WriteToFile(Shared::Stream *out);
	void ReadFromSavegame(Shared::Stream *in);
	void WriteToSavegame(Shared::Stream *out) const;
};

} // namespace AGS3

#endif
