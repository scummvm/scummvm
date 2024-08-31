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

#ifndef ICB_FLOORS_H
#define ICB_FLOORS_H

#include "engines/icb/common/px_common.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/prim_route_builder.h"
#include "engines/icb/common/px_floor_map.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/barriers.h"

namespace ICB {

class _floor_world {

public:
	_floor_world();
	~_floor_world();
	void ___init();

	uint32 Locate_floor_rect(PXreal x, PXreal z, PXreal y, _floor **rct);
	uint32 Return_floor_rect(PXreal x, PXreal z, PXreal y, uint32 rubber);
	bool8 Point_on_rubber_floor(PXreal x, PXreal z, PXreal y, uint32 rubber, uint32 rect_num);

	void Set_floor_rect_flag(_logic *log);
	uint32 Return_non_rubber_floor_no(_logic *log, uint32 cur_rubber_floor);

	// floor router
	void Pick_floor_route(uint32 start, uint32 dest);
	void Find_shortest_floor_route(uint32 start, uint32 dest, uint32 x, uint32 z);
	void Recurse_to_shortest(uint32 adjoining_floor, uint32 level, uint32 dest);
	uint32 Calculate_floor_route_length(uint32 total_floors);

	PXreal Fetch_height(uint32 height);
	int32 Fetch_total_heights();
	uint32 Fetch_total_floors();

	PXreal Gravitise_y(PXreal y);

	PXreal Floor_safe_gravitise_y(PXreal fY);

	void Align_with_floor(_mega *mega);
	PXreal Return_true_y(PXreal y);
	bool8 On_a_floor(_mega *mega);

	// for external routines such as fn_functions
	_floor *Fetch_named_floor(const char *name);
	_floor *Fetch_floor_number(uint32 num);
	PXreal Fetch_floors_volume_height(uint32 num);
	uint32 Fetch_floor_number_by_name(const char *name);
	uint32 Fetch_number_of_floors() const { return total_floors; }

	int32 Project_point_down_through_floors(int32 nX, int32 nY, int32 nZ);

	LinkedDataFile *floors; // the floor definition file

private:
	uint32 total_floors; // total number of individual floors

	uint32 total_heights;        // how many unique heights
	PXreal heights[MAX_heights]; // list of actual heights

	PXreal floor_y_volume[MAX_floors]; // table of depths for floors
};

inline PXreal _floor_world::Fetch_floors_volume_height(uint32 num) { return floor_y_volume[num]; }

inline PXreal _floor_world::Fetch_height(uint32 height) {
	if (height >= total_heights) {
		Fatal_error("can't get %d height of %d", height, total_heights);
	}

	return (heights[height]);
}

inline int32 _floor_world::Fetch_total_heights() { return (total_heights); }

inline uint32 _floor_world::Fetch_total_floors() { return (total_floors); }

inline _floor *_floor_world::Fetch_named_floor(const char *name) {
	// return a pointer to a named floor to an external routine - most likely a fn_function
	return ((_floor *)LinkedDataObject::Fetch_item_by_name(floors, name));
}

inline _floor *_floor_world::Fetch_floor_number(uint32 num) {
	// return a pointer to a named floor to an external routine - most likely a fn_function
	return ((_floor *)LinkedDataObject::Fetch_item_by_number(floors, num));
}

} // End of namespace ICB

#endif
