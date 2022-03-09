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

//=============================================================================
//
// Script Editor run-time engine component (c) 1998 Chris Jones
// script chunk format:
// 00h  1 dword  version - should be 2
// 04h  1 dword  sizeof(scriptblock)
// 08h  1 dword  number of ScriptBlocks
// 0Ch  n STRUCTs ScriptBlocks
//
//=============================================================================

#include "ags/lib/std/utility.h"
#include "ags/engine/script/cc_instance.h"
#include "ags/shared/script/cc_error.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

namespace AGS {
namespace Shared {
class RoomStruct;
} // namespace Shared
} // namespace AGS

using namespace AGS::Shared;

std::pair<String, String> cc_error_at_line(const char *error_msg) {
	ccInstance *sci = ccInstance::GetCurrentInstance();
	if (!sci) {
		return std::make_pair(String::FromFormat("Error (line %d): %s", _G(currentline), error_msg), String());
	} else {
		return std::make_pair(String::FromFormat("Error: %s\n", error_msg), ccInstance::GetCurrentInstance()->GetCallStack(5));
	}
}

String cc_error_without_line(const char *error_msg) {
	return String::FromFormat("Runtime error: %s", error_msg);
}

} // namespace AGS3
