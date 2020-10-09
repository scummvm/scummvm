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

#ifndef ICB_BARRIERS_H
#define ICB_BARRIERS_H

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_linkeddatafile.h"
#include "engines/icb/common/px_route_barriers.h"

namespace ICB {

#define MAX_slices 9

//+1 for dummy top floor ceiling
#define MAX_heights (MAX_slices + 1)

#define MAX_floors 48

// this is the master number of objects -
// The +3 & ~3 - means it is rounded up to be a multiple of 4
#define MAX_props ((116 + 3) & ~3)

#define MAX_parents_per_anim_slice 24
//#define   MAX_bars_per_parent 96
#define MAX_props_per_parent 16

#define MAX_animating_props 24
#define MAX_bars_per_prop 60

#define MAX_child_groups_per_parent 16

#define MAX_prop_abars (MAX_animating_props * MAX_bars_per_prop)

//--------------------------------------------------------------------------------------
class _animating_parent { // contains all of the abarriers that lie within this parent space - for each we record its associated prop and
	                  // state so we know when each is in scope
public:
	uint8 num_props;
	uint8 prop_number[MAX_props_per_parent];
};

class _animating_barrier_slice { // contains a list of parents that lie within the slice
	                         // and a list of props within the slice
public:
	_animating_parent *anim_parents[MAX_parents_per_anim_slice];

	uint8 num_props_in_slice;
	uint8 prop_list[MAX_props];
};

class _anim_prop_info { // contains a list of all the abars for the prop
public:
	uint8 barriers_per_state;
	uint8 total_states;   // temp - could be removed
	uint16 *barrier_list; // pointer into 'prop_abar_table'
};

class _barrier_handler {

public:
	void ___init();

	void Form_route_barrier_list(PXreal x, PXreal y, PXreal z, PXreal x2, PXreal z2);
	void Form_parent_barrier_list(PXreal x, PXreal y, PXreal z);

	_parent_box *Fetch_parent_box_for_xyz(PXreal x, PXreal y, PXreal z, uint32 &par_num, uint32 &slice_num);
	_parent_box *Fetch_parent_num_on_slice_y(uint32 requested_parent, PXreal y);
	uint32 Fetch_number_of_child_boxes(_parent_box *parent);
	_child_group *Fetch_child_box(_parent_box *parent, uint32 child);
	_route_barrier *Fetch_barrier(uint32 num);
	uint32 Fetch_total_barriers();
	_linked_data_file *Get_barrier_pointer() const { return raw_barriers; }
	void Prepare_animating_barriers();
	uint32 Get_anim_barriers(uint32 n, uint32 *oThisCubesBarriers, uint32 slice);

	void Set_route_barrier_mask(int32 left, int32 right, int32 top, int32 bottom);
	void Clear_route_barrier_mask();

	_animating_barrier_slice anim_slices[MAX_slices];

	_anim_prop_info anim_prop_info[MAX_props];

	uint16 prop_abar_table[MAX_animating_props * MAX_bars_per_prop];

	uint8 parents_used;                              // count how many of table are used
	_animating_parent anim_parent_table[MAX_floors]; // storage

	// raw barriers
	_linked_data_file *raw_barriers; // raw route barriers used for routing/line of sight and maybe shadow geometry

	uint32 total_barriers;

	// route barrier wrapper file
	_linked_data_file *route_wrapper;

	uint32 total_slices; // useful out of file

	bool8 barrier_mask;
	DXrect mask;
};

inline void _barrier_handler::Set_route_barrier_mask(int32 left, int32 right, int32 top, int32 bottom) {
	// certain route building will provide an inner rect that barriers must lie within
	barrier_mask = TRUE8;

	mask.left = left;
	mask.right = right;
	mask.top = top;
	mask.bottom = bottom;
}

inline void _barrier_handler::Clear_route_barrier_mask() {
	// cancel inner route barrier mask

	barrier_mask = FALSE8;
}

inline uint32 _barrier_handler::Fetch_number_of_child_boxes(_parent_box *parent) { return (parent->num_childgroups); }

inline _child_group *_barrier_handler::Fetch_child_box(_parent_box *parent, uint32 child) { return ((_child_group *)(((uint8 *)parent) + parent->childgroups[child])); }

inline uint32 _barrier_handler::Fetch_total_barriers() { return (total_barriers); }

} // End of namespace ICB

#endif
