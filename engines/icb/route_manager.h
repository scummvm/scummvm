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

#ifndef ICB_ROUTE_MANAGER_H
#define ICB_ROUTE_MANAGER_H

#include "engines/icb/common/px_rcutypes.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/graphic_prims.h" //_point etc.
#include "engines/icb/animation_mega_set.h"

namespace ICB {

// note MAX_final_route must be a multiple of 4 to get alignment
#define MAX_final_route 16

enum _route_type { ROUTE_find_shortest_direct_floor_route, ROUTE_return_all_floor_routes, ROUTE_points_only };

enum __route_request_error { __ROUTE_REQUEST_PRIM_FAILED, __ROUTE_REQUEST_ROOMS_FAILED, __RR_NO_ROUTE_REQUIRED, __ROUTE_REQUEST_OK };

#define POST_INTERACTION                                                                                                                                                           \
	M->actor_xyz.x = M->target_xyz.x;                                                                                                                                          \
	M->actor_xyz.z = M->target_xyz.z;                                                                                                                                          \
	L->cur_anim_type = __STAND;                                                                                                                                                \
	M->reverse_route = FALSE8;                                                                                                                                                 \
	L->anim_pc = 0;

typedef struct {
	_route_type rtype;

	PXreal initial_x;
	PXreal initial_z;
	PXreal dest_x;
	PXreal dest_z;

	PXreal character_y;

	uint32 initial_floor;
	uint32 dest_floor;

	__mega_set_names anim_type; // walk or run - this declares the route movement type

	__route_request_error error;

	bool8 finish_on_stand; // when the route is complete the manager will attempt to slow out and stand the character
	bool8 finish_on_null_stand; // when the route is complete the manager will attempt to slow out and stand the character WITHOUT MOVEMENT ON FINAL ANIM
	uint8 padding1;
	uint8 padding2;
} _route_request;

class _route_description {
public:
	void ___init();

	_route_request request_form;

	_point prim_route[MAX_final_route];

	uint32 total_points; // total number of point junctions in the route
	uint32 current_position; // current point number
	PXreal dist_left; // unit distance left to travel on current line

	//      barriers for nethack diagnostics
	_point *diag_bars;
	uint32 number_of_diag_bars;

	bool8 arrived; // set when route done so we know we're into the optional slow out phase
	uint8 padding1;
	uint8 padding2;
	uint8 padding3;
};

enum _route_phase { RM_NONE, RM_MAIN, RM_ALT };

} // End of namespace ICB

#endif
