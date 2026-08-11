/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/map_marker.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/string_vest.h"
#include "engines/icb/res_man.h"

namespace ICB {

void _marker::___init() {
	// read in the session markers file - this is the engine written file not the max Nico file
	// read data into a structure as it needs to be modifiable and saveable which isn't possible with res_man files
	int32 len;

	Zdebug("\n\n\n-------------*** Init marker file ***-------------");

	// build file name - file lives in session directory
	char buf[] = "markers";
	uint32 buf_hash = NULL_HASH;
	uint32 cluster_hash = MS->Fetch_session_cluster_hash();
	uint8 *file_marker = rs_anims->Res_open((const char *)buf, buf_hash, MS->Fetch_session_cluster(), cluster_hash, 0, &len);

	// We have access to the data, now to copy it to the markers block
	num_markers = len / sizeof(_map_marker);

	if (num_markers >= MAX_markers)
		Fatal_error("too many map markers");

	if (len) {
		memcpy(&marks[0], file_marker, len);
	}
}

} // End of namespace ICB
