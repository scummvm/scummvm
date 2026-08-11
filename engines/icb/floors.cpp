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

#include "engines/icb/p4.h"
#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/mission.h"
#include "engines/icb/session.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/debug.h"
#include "engines/icb/floors.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/res_man.h"

namespace ICB {

_floor_world::_floor_world() {}

void _floor_world::___init() {
	// init the floor and exit object
	// works out how many floor levels are in the map and makes a list of them

	// sets  total_floors
	//			total_heights
	// fills heights[]

	uint32 buf_hash = NULL_HASH;
	uint32 j, k, len;
	PXreal temp;
	_floor *floor;

	// load the file for this session
	// When clustered the session files have the base stripped
	len = Common::sprintf_s(temp_buf, "%s", PX_FILENAME_FLOOR_MAP);
	if (len > ENGINE_STRING_LEN)
		Fatal_error("_floor_world::___init string len error");

	uint32 cluster_hash = MS->Fetch_session_cluster_hash();
	floors = (LinkedDataFile *)private_session_resman->Res_open(temp_buf, buf_hash, MS->Fetch_session_cluster(), cluster_hash);

	// Check the file schema
	if (LinkedDataObject::GetHeaderVersion(floors) != VERSION_PXWGFLOORS)
		Fatal_error("Incorrect version number for floor data [%s] - file has %d, engine has %d", temp_buf, LinkedDataObject::GetHeaderVersion(floors), VERSION_PXWGFLOORS);

	// set this for convenience
	total_floors = LinkedDataObject::Fetch_number_of_items(floors);
	Tdebug("floors.txt", "##total floors %d", total_floors);

	// check for no floors
	if (!total_floors) // no floors :O
		Fatal_error("session has no floors - engine cannot proceed");

	if (total_floors > MAX_floors) // general legality check to catch corrupt files
		Fatal_error("engine stopping due to suspicious PxWGFloors file - has %d floors", total_floors);

	// get some useful stats
	total_heights = 0; // set to 0

	int32 nMissing = 0;
	for (j = 0; j < total_floors; j++) {
		floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

		if (total_heights) {
			// see if this height is already defined
			for (k = 0; k < total_heights; k++)
				if (heights[k] == floor->base_height)
					break; // already here

			if (k == total_heights) {
				// not found so register the new height
				heights[total_heights++] = floor->base_height;

				if (total_heights > MAX_slices)
					Fatal_error("_floor_world::___init has run out of slices - %d found, %d allowed", total_heights, MAX_slices);
			}
		} else {
			// register the first height
			heights[0] = floor->base_height;
			total_heights = 1;
		}
	}

	if (nMissing > 0) {
		Fatal_error("%d missing cameras : Game must terminate", nMissing);
	}

	// now sort the heights
	if (total_heights > 1) {
		for (j = 0; j < total_heights; j++) {
			for (k = 0; k < total_heights - 1; k++) {
				if (heights[k] > heights[k + 1]) {
					temp = heights[k + 1];
					heights[k + 1] = heights[k];
					heights[k] = temp;
				}
			}
		}
	}

	// create a dummy top floor figure for floor_y_volume creation
	heights[total_heights] = REAL_LARGE;

	Tdebug("floors.txt", "\n\n\n\n%d different heights", total_heights);
	for (j = 0; j < total_heights; j++)
		Tdebug("floors.txt", " %3.1f", heights[j]);

	Tdebug("floors.txt", "\n\n\ncreating floor y volume table\n");

	// in-case second time around

	// create room height table

	for (j = 0; j < total_floors; j++) {
		floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

		for (k = 0; k < total_heights; k++) {
			if (floor->base_height == heights[k]) {
				floor_y_volume[j] = (heights[k + 1] - REAL_ONE);
				Tdebug("floors.txt", "floor %d, base %3.2f, top %3.2f", j, floor->base_height, floor_y_volume[j]);
			}
		}
	}
}

bool8 _floor_world::On_a_floor(_mega *mega) {
	// is a mega on a floor
	// used to dismiss gunshots when on stairs, etc.
	uint32 j;

	for (j = 0; j < total_heights; j++)
		if (mega->actor_xyz.y == heights[j])
			return TRUE8;

	return FALSE8;
}

void _floor_world::Align_with_floor(_mega *mega) {
	uint32 j;

	// check against actual heights
	for (j = 0; j < total_heights; j++) {
		if (mega->actor_xyz.y == heights[j])
			return;
	}

	// not on one so align with one if we are pretty near
	for (j = 0; j < total_heights; j++) {
		if (PXfabs(mega->actor_xyz.y - heights[j]) < (REAL_ONE * 15)) {
			mega->actor_xyz.y = heights[j];
			return;
		}
	}
}

PXreal _floor_world::Return_true_y(PXreal y) {
	// snap a y coordinate up or down to the floor it is meant to be
	// used by walk area camera director
	uint32 j;

	// check against actual heights
	for (j = 0; j < total_heights; j++)
		if (y == heights[j])
			return y;

	// not on one so align with one if we are pretty near
	for (j = 0; j < total_heights; j++)
		if (PXfabs(y - heights[j]) < (REAL_ONE * 15)) {
			y = heights[j];
			return y;
		}

	return y;
}

_floor_world::~_floor_world() {
	//_floor_world destructor
	Zdebug("*_floor_world destructing*");
}

uint32 _floor_world::Fetch_floor_number_by_name(const char *name) {
	// return a pointer to a named floor to an external routine - most likely a fn_function
	return (LinkedDataObject::Fetch_item_number_by_name(floors, name));
}

uint32 _floor_world::Return_floor_rect(PXreal x, PXreal z, PXreal y, uint32 rubber) {
	// find the floor LRECT that point x,y,z lies within
	// returns   rect number and pointer to _rect
	//				or PXNULL
	uint32 j;

	// search through all floors
	for (j = 0; j < total_floors; j++) {
		_floor *floor;

		floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

		if (floor->base_height == (int32)y) {
			// this floor is in our view level

			// check our x,z against all the rects

			// if hit then return floor number
			if ((x >= (PXreal)(floor->rect.x1 - rubber)) && (x <= (PXreal)(floor->rect.x2 + rubber)) && (z >= (PXreal)(floor->rect.z1 - rubber)) &&
			    (z <= (PXreal)(floor->rect.z2 + rubber)))
				return (j);
		}
	}

	// point is not on any floor rect
	return (PXNULL);
}

bool8 _floor_world::Point_on_rubber_floor(PXreal x, PXreal z, PXreal y, uint32 rubber, uint32 rect_num) {
	_floor *floor;

	floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, rect_num);

	if (floor->base_height == (int32)y) {
		// if  hit then return floor number
		if ((x >= (PXreal)(floor->rect.x1 - rubber)) && (x <= (PXreal)(floor->rect.x2 + rubber)) && (z >= (PXreal)(floor->rect.z1 - rubber)) &&
		    (z <= (PXreal)(floor->rect.z2 + rubber)))
			return TRUE8;
	}

	// point is not on floor rect
	return FALSE8;
}

uint32 _floor_world::Locate_floor_rect(PXreal x, PXreal z, PXreal y, _floor **rct) {
	// find the floor RECT that point x,y,z lies within

	// returns   rect number and pointer to _rect
	//				or PXNULL
	uint32 j;

	for (j = 0; j < total_floors; j++) {
		_floor *floor;

		floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

		if (floor->base_height == (int32)y) {
			// this floor is in our view level

			// check our x,z against all the rects

			// if hit then return floor number
			if ((x >= (PXreal)floor->rect.x1) && (x <= (PXreal)floor->rect.x2) && (z >= (PXreal)floor->rect.z1) && (z <= (PXreal)floor->rect.z2)) {
				*rct = floor;
				return (j);
			}
		}
	}

	// point is not on any floor rect
	Message_box("no floor");
	return (PXNULL);
}

void _floor_world::Set_floor_rect_flag(_logic *log) {
	// find the floor RECT that character belongs to and fill in the owner_floor_rect flag

	// note - there are ways to speed this up. We could record the rect and then only do a full search if the object
	//			moves outside the recorded rect again
	uint32 j;
	_floor *floor;
	PXreal y;

#define FLOOR_RUBBER (20 * REAL_ONE)

	// y locking
	if (log->mega->y_locked)
		y = log->mega->y_lock;
	else
		y = log->mega->actor_xyz.y;
	// ylocking

	// first see if we're one same one as last time
	floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, log->owner_floor_rect);
	if ((y >= (floor->base_height - (0 * REAL_ONE))) && ((y <= (floor_y_volume[log->owner_floor_rect] - (0 * REAL_ONE))))) // this floor is in our view level
		if ((log->mega->actor_xyz.x >= (floor->rect.x1 - FLOOR_RUBBER)) && (log->mega->actor_xyz.x <= (floor->rect.x2 + FLOOR_RUBBER)) &&
		    (log->mega->actor_xyz.z >= (floor->rect.z1 - FLOOR_RUBBER)) && (log->mega->actor_xyz.z <= (floor->rect.z2 + FLOOR_RUBBER))) {
			Zdebug("[%s]still on %d", MS->Fetch_object_name(MS->Fetch_cur_id()), log->owner_floor_rect);
			return; // yup, still hitting!
		}

	// search through all floors
	for (j = 0; j < total_floors; j++) {
		floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

		if ((y >= (floor->base_height - (0 * REAL_ONE))) && ((y <= (floor_y_volume[j] - (0 * REAL_ONE))))) {
			// this floor is in our view level
			// if hit then return floor number
			if ((log->mega->actor_xyz.x >= floor->rect.x1) && (log->mega->actor_xyz.x <= floor->rect.x2) && (log->mega->actor_xyz.z >= floor->rect.z1) &&
			    (log->mega->actor_xyz.z <= floor->rect.z2)) {
				log->owner_floor_rect = j;
				return;
			}
		}
	}

	// point is not on any floor rect
	// hmmm, well, hold previous value i guess

	Tdebug("warning.txt", "Set_floor_rect_flag; %s has no floor", MS->Fetch_object_name(MS->Fetch_cur_id()));
}

uint32 _floor_world::Return_non_rubber_floor_no(_logic *log, uint32 cur_rubber_floor) {
	// return exact box
	// used by camera director when leaving WA's

	uint32 j;
	_floor *floor;

	// first see if we're one same one as last time
	floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, cur_rubber_floor);
	if ((log->mega->actor_xyz.y >= floor->base_height) && ((log->mega->actor_xyz.y <= floor_y_volume[log->owner_floor_rect]))) // this floor is in our view level
		if ((log->mega->actor_xyz.x >= (floor->rect.x1)) && (log->mega->actor_xyz.x <= (floor->rect.x2)) && (log->mega->actor_xyz.z >= (floor->rect.z1)) &&
		    (log->mega->actor_xyz.z <= (floor->rect.z2))) {
			return cur_rubber_floor; // yup, still hitting!
		}

	// search through all floors
	for (j = 0; j < total_floors; j++) {
		floor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

		if ((log->mega->actor_xyz.y >= floor->base_height) && ((log->mega->actor_xyz.y <= floor_y_volume[j]))) {
			// this floor is in our view level
			// if hit then return floor number
			if ((log->mega->actor_xyz.x >= floor->rect.x1) && (log->mega->actor_xyz.x <= floor->rect.x2) && (log->mega->actor_xyz.z >= floor->rect.z1) &&
			    (log->mega->actor_xyz.z <= floor->rect.z2)) {
				return j;
			}
		}
	}

	// point is not on any floor rect
	// hmmm, well, hold previous value i guess

	return cur_rubber_floor;
}

PXreal _floor_world::Gravitise_y(PXreal y) {
	// pull a y coordinate back to a floor height

	int32 j;

	for (j = total_heights - 1; j != -1; j--) { // 4 heights == j=3 == [0][1][2][3]
		if (y >= heights[j]) {
			return (heights[j]);
		}
	}

	Zdebug("\n\nGravitise_y %3.2f", y);

	for (j = 0; j < (int32)total_heights; j++)
		Zdebug("%d [%3.2f]", j, heights[j]);

	Fatal_error("Gravitise_y finds major height problem - %s", MS->Fetch_object_name(MS->Fetch_cur_id()));

	return (y);
}

PXreal _floor_world::Floor_safe_gravitise_y(PXreal fY) {
	int32 i;

	// This function does the same as Gravitise_y() but does not Fatal_error if it
	// falls out of the bottom of the game world.  This is to correct faults in the
	// art (surprise, surprise) that were causing megas on ladders to briefly have a
	// y-coordinate lower than the floor the ladder bottom is on.
	for (i = total_heights - 1; i != -1; --i) {
		if (fY >= heights[i])
			return (heights[i]);
	}

	// Simply return the lowest floor height.
	return (heights[0]);
}

int32 _floor_world::Project_point_down_through_floors(int32 nX, int32 nY, int32 nZ) {
	int32 nSliceIndex;
	uint32 j;
	_floor *pFloor;

	// Do what the normal Gravitise_y() does to place the point on the slice height below it.
	nSliceIndex = total_heights - 1;
	while ((nSliceIndex > -1) && (nY < (int32)heights[nSliceIndex]))
		--nSliceIndex;

	// See which loop condition failed.
	if (nSliceIndex == -1) {
		// Fell out of the bottom of the floor world, but this is not an error in this function.
		return (-1);
	}

	// Right, we have put the point on a slice.  While there are slices still to go
	// beneath the current point, we check if the point lies within a floor rectangle
	// on that height.
	while (nSliceIndex > -1) {
		nY = (int32)heights[nSliceIndex];

		for (j = 0; j < total_floors; ++j) {
			pFloor = (_floor *)LinkedDataObject::Fetch_item_by_number(floors, j);

			if (pFloor->base_height == nY) {
				// Floor at this height, so check its position.
				if ((nX >= pFloor->rect.x1) && (nX <= pFloor->rect.x2) && (nZ >= pFloor->rect.z1) && (nZ <= pFloor->rect.z2)) {
					return (nSliceIndex);
				}
			}
		}

		// Right, the point hit nothing on that level.  Move to the slice below.
		--nSliceIndex;
	}

	// If we fell out, it is not an error.  It simply means there is no floor beneath
	// the point we are checking.
	return (-1);
}

} // End of namespace ICB
