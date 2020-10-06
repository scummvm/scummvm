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

#ifndef ICB_MAP_MARKERR
#define ICB_MAP_MARKERR

#include "engines/icb/common/px_array.h"
#include "engines/icb/common/px_features.h"
#include "engines/icb/p4_generic.h"

namespace ICB {

#define MARKER_NAME_LEN 32

// this must be same as MAX_voxel_list
#define MAX_markers 33

// The real map_marker struct for in use in game engine
typedef struct {
	char name[MARKER_NAME_LEN]; // cut down name equivalent
	PXreal x, y, z;             // Reference point for the prop.
	_feature_type type;         // The type of the prop.
	PXfloat pan;                // 0 - 99 (maybe use -1 to indicate no direction)
} _map_marker;

// The map_marker struct used for saving to disc
typedef struct {
	char name[MARKER_NAME_LEN]; // cut down name equivalent
	float x, y, z;              // Reference point for the prop.
	_feature_type type;         // The type of the prop.
	float pan;                  // 0 - 99 (maybe use -1 to indicate no direction)
} _file_map_marker;

class _marker {
public:
	void ___init();
	void Write_markers();
	_map_marker *Fetch_marker_by_object_name(const char *name);
	_map_marker *Create_new_marker(const char *name);
	uint32 num_markers;
	_map_marker marks[MAX_markers];
};

} // End of namespace ICB

#endif
