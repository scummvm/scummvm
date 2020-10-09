/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#define FORBIDDEN_SYMBOL_EXCEPTION_strncasecmp

#include "engines/icb/common/px_common.h"
#include "engines/icb/map_marker.h"
#include "engines/icb/debug.h"
#include "engines/icb/mission.h"
#include "engines/icb/global_objects.h"

namespace ICB {

_map_marker *_marker::Create_new_marker(const char *name) {
	Zdebug("marker %d", num_markers);

	if (num_markers == MAX_markers)
		Fatal_error("no room left for more map markers"); // shouldnt happen

	// set markers name
	strncpy(marks[num_markers].name, name, MARKER_NAME_LEN);

	Zdebug("marker name [%s]", marks[num_markers].name);

	// strncpy doesn't null terminate if the source string is larger or equal to the dest.
	marks[num_markers].name[MARKER_NAME_LEN - 1] = 0;

	// reset pan
	marks[num_markers].pan = ZERO_TURN;

	// return pointer to new marker object
	return (&marks[num_markers++]);
}

_map_marker *_marker::Fetch_marker_by_object_name(const char *name) {
	// find an item in the map_marker file
	// return a pointer to it or PXNULL if not found
	uint32 j;

	if (num_markers) {
		for (j = 0; j < num_markers; j++) {
			if (!scumm_strnicmp(marks[j].name, name, MARKER_NAME_LEN - 1))
				return (&marks[j]);
		}
	}

	// not found
	return (NULL);
}

} // End of namespace ICB
