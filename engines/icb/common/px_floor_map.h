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

#ifndef ICB_PX_FLOOR_MAP_H_INCLUDED
#define ICB_PX_FLOOR_MAP_H_INCLUDED

#include "engines/icb/common/px_common.h"

namespace ICB {

// Should put this in a nice header file somewhere
typedef float PXreal;
typedef float PXfloat;

// This defines the length of the hashed camera name on the PSX.
#define HASH_CAMERA_LEN 8

// These define the filenames for files containing floor maps.
#define PX_FILENAME_FLOOR_MAP "pxwgfloors"
#ifndef PC_EXT_LINKED
#define PC_EXT_LINKED "linked"
#endif

#ifndef PSX_EXT_LINKED
#define PSX_EXT_LINKED "PSXlinked"
#endif

// Update this whenever the format of the data changes, so engine can check it is in sync with data.
#define VERSION_PXWGFLOORS 300

// This holds one of the rectangles making up the floor.  The coordinates are held looking DOWN on the floor, and the
// coordinate system used is our system (y is up/down, x and z axes are in the horizontal plane).
typedef struct {
	PXreal x1, z1; // X, Z of top-left of the floor rectangle (looking DOWN on the floor).
	PXreal x2, z2; // Ditto for bottom-right corner.
} _rect;

// struct _neighbour_map_entry
// This holds one of the entries in the exit map.
typedef struct {
	uint32 neighbour;              // Index of neighbour (can be passed directly into _linked_data_file.Fetch_item_by_number()).
	uint32 offset_exit_descriptor; // File position of the exit descriptor for this.
} _neighbour_map_entry;

// struct _floor
// This is the top structure in a floor entry.  All the other structures hang off this one.
typedef struct {
	PXreal base_height;
	_rect rect;
	char camera_cluster[HASH_CAMERA_LEN];
	uint32 camera_name_offset;
	uint32 map_location_hash;
	uint32 num_neighbours;
	_neighbour_map_entry neighbour_map[1];
} _floor;

// struct _exit_descriptor
// This holds an exit descriptor would you believe?
typedef struct {
	uint32 num_waypoints;    // Number of waypoints for the exit.
	uint32 offset_waypoints; // File offset of the list of waypoints for this exit.
	uint32 offset_propname;  // File offset of the ASCII name of a prop associated with this exit.
} _exit_descriptor;

// struct _floor_waypoint
// This holds a waypoint for an exit.
typedef struct {
	PXreal x, z;       // Physical position of waypoint on floor.
	PXfloat direction; // A direction (0-360?) for the waypoint.  Might be used to face a mega a certain way.
} _floor_waypoint;

} // End of namespace ICB

#endif // #ifndef _PX_FLOOR_MAP_H_INCLUDED
