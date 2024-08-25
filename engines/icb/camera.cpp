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
#include "engines/icb/common/px_game_object.h"
#include "engines/icb/common/px_scriptengine.h"
#include "engines/icb/common/ptr_util.h"
#include "engines/icb/session.h"
#include "engines/icb/object_structs.h"
#include "engines/icb/debug.h"
#include "engines/icb/player.h"
#include "engines/icb/global_objects.h"
#include "engines/icb/global_switches.h"
#include "engines/icb/res_man.h"
#include "engines/icb/floors.h"
#include "engines/icb/mission.h"

namespace ICB {

#define RUBBER 30
#define REAL_RUBBER (30 * REAL_ONE)

PXvector *posi;
uint32 this_rect;
_floor *obfloor;

mcodeFunctionReturnCodes fn_floor_and_floor_camera_linked(int32 &result, int32 *params) { return (MS->fn_floor_and_floor_camera_linked(result, params)); }

mcodeFunctionReturnCodes fn_switch_to_manual_camera(int32 &result, int32 *params) { return (MS->fn_switch_to_manual_camera(result, params)); }

mcodeFunctionReturnCodes fn_cancel_manual_camera(int32 &result, int32 *params) { return (MS->fn_cancel_manual_camera(result, params)); }

mcodeFunctionReturnCodes fn_is_current_camera(int32 &result, int32 *params) { return (MS->fn_is_current_camera(result, params)); }

mcodeFunctionReturnCodes fn_is_current_location(int32 &result, int32 *params) { return (MS->fn_is_current_location(result, params)); }

mcodeFunctionReturnCodes _game_session::fn_floor_and_floor_camera_linked(int32 &, int32 *params) {
	// params    0       ascii name of home floor
	//			1      ascii name of viewable floor name

	// search for a camera of this name
	// confirm floor exists
	// add floor number to cameras extra floor list

	uint32 home_floor_num;
	uint32 floor_num;
	const char *home_floor_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *floor_name = (const char *)MemoryUtil::resolvePtr(params[1]);

	home_floor_num = floor_def->Fetch_floor_number_by_name(home_floor_name);
	if (home_floor_num == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("fn_floor_and_floor_camera_linked can't find floor [%s]", home_floor_name);

	floor_num = floor_def->Fetch_floor_number_by_name(floor_name);
	if (floor_num == PX_LINKED_DATA_FILE_ERROR)
		Fatal_error("fn_floor_and_floor_camera_linked can't find floor [%s]", floor_name);

	if (home_floor_num == floor_num)
		Fatal_error("fn_floor_and_floor_camera_linked finds [%s] and [%s] are same floor!", home_floor_name, floor_name);

	// add  viewable floor number to the camera that displays our primary home floor

	uint32 index1 = floor_to_camera_index[home_floor_num];
	uint32 index2 = floor_to_camera_index[floor_num];

	cam_floor_list[index1].extra_floors[cam_floor_list[index1].num_extra_floors++] = floor_num;
	cam_floor_list[index2].extra_floors[cam_floor_list[index2].num_extra_floors++] = home_floor_num;

	if (cam_floor_list[index1].num_extra_floors == MAX_extra_floors)
		Fatal_error("fn_floor_and_floor_camera_linked too many extra floors");

	return IR_CONT;
}

void _game_session::Build_camera_table() {
	// read each floor to note the name of the camera
	// each new camera name is put into a list of names
	// the floor number gets the numer of that camerea in the list
	uint32 j, k;
	_floor *floor;
	uint32 tot;

	Zdebug("\n***********building camera table**************");

	num_cameras = 0; // reset

	// reset extra cams table
	for (j = 0; j < MAX_floors; j++)
		cam_floor_list[j].num_extra_floors = 0;

	tot = floor_def->Fetch_total_floors();
	if (!tot)
		Fatal_error("Build_camera_table finds no floors?");

	for (j = 0; j < tot; j++) {
		floor = (_floor *)floor_def->Fetch_floor_number(j);
		if (floor->camera_name_offset) {
			// is this camera name already been assigned a slot?
			k = 0;
			while ((k < num_cameras) && (strcmp(camera_name_list[k], (((char *)floor) + floor->camera_name_offset))))
				++k;

			if (k == num_cameras) { // didn't find this camera
				Zdebug(" new camera %d [%s] [%s]", num_cameras, (((char *)floor) + floor->camera_name_offset), floor->camera_cluster);
				camera_name_list[num_cameras] = (((char *)floor) + floor->camera_name_offset); // add camera name to list of names
				camera_cluster_list[num_cameras] = floor->camera_cluster;
				floor_to_camera_index[j] = num_cameras; // set this floors camera number

				num_cameras++;
			} else {
				floor_to_camera_index[j] = k; // set this floors camera number
			}

			Zdebug(" floor %d gets camera number %d", j, floor_to_camera_index[j]);
		} else {
			// camera name is missing
			floor_to_camera_index[j] = NO_CAMERA_CHOSEN;
			Zdebug("floor %d camera missing!!!!!!!!!!!", j);
		}
	}

	Zdebug("***********building camera table**************\n");
}

bool8 _game_session::Object_visible_to_camera(uint32 id) {
	// is id visible to current camera

	uint32 j, num_extra;
#define FLOOR_NO logic_structs[id]->owner_floor_rect
#define MM logic_structs[id]->mega

	// Don't want to see held objects
	if (logic_structs[id]->ob_status == OB_STATUS_HELD)
		return (FALSE8);

	// high level user clip
	if ((MM) && (!MM->display_me))
		return FALSE8;

	// check for no camera chosen - 1st cycle
	if (cur_camera_number == NO_CAMERA_CHOSEN)
		return (FALSE8);

	// if following the player then always TRUE8 - need this because of rubber band
	if ((!g_mission->camera_follow_id_overide) && (id == player.Fetch_player_id()))
		return TRUE8;

	if (floor_to_camera_index[FLOOR_NO] == cur_camera_number)
		return (TRUE8);

	// now check if the current floor is registered as an extra floor viewable from current camera
	num_extra = cam_floor_list[cur_camera_number].num_extra_floors;
	for (j = 0; j < num_extra; j++)
		if (cam_floor_list[cur_camera_number].extra_floors[j] == FLOOR_NO)
			return (TRUE8);

	// definitely not on screen
	return (FALSE8);
}

void _game_session::Reset_camera_director() {
	// by masking this off we force a new camera to be initialised in current view mode
	cur_camera_number = NO_CAMERA_CHOSEN;
	manual_camera = FALSE8;
	wa_camera = FALSE8;
	wa_tied_to_pin = FALSE8;
	wa_tied_to_exit_pin = FALSE8;
	this_rect = 0; // will be legal floor which is all that matters
}

int32 Contains(int32 x1, int32 y1, int32 x2, int32 y2, int32 mx, int32 my) {
	int32 tmp, x3;

	x1 = x1 << 1;
	y1 = y1 << 1;
	x2 = x2 << 1;
	y2 = y2 << 1;
	mx = (mx << 1) + 1;
	my = (my << 1) + 1;

	if (((y1 < my) && (y2 > my)) || ((y1 > my) && (y2 < my))) {
		if (x1 > x2) {
			tmp = x1;
			x1 = x2;
			x2 = tmp;
			tmp = y1;
			y1 = y2;
			y2 = tmp;
		}
		if (y1 < y2)
			x3 = x1 + (my - y1) * (x2 - x1) / (y2 - y1);
		else
			x3 = x2 - (my - y2) * (x2 - x1) / (y1 - y2);
		if (x3 < mx)
			return 1;
	}
	return 0;
}

void _game_session::Camera_director() {
	// check which object the camera is linked to
	// check which floor rect that object is on
	// if no camera run a script
	// else if new camera switch it in

	const __aWalkArea *wa;
	uint32 k;
	uint32 hit;
	PXreal sub1, sub2, len, y;

	Prepare_camera_floors();

	// a manual script camera completely overides the floor and WA system cameras
	if ((manual_camera) || (camera_lock))
		return;

	// do walkarea stuff first
	// are we currently on one?
	// if so are we still on it
	// if not are we on one in the list
	// if so start it
	// else go through normal floor system

	if (wa_camera) {
		// we are currently using a WA camera

		wa = MS->wa_list[wa_number];

		y = floor_def->Return_true_y((PXreal)wa->y);

		if ((y >= obfloor->base_height) && (y < (floor_def->Fetch_floors_volume_height(this_rect)))) {

			if (wa_tied_to_pin) {
				// we are still tied to the pin point
				// if we are within the stretch distance from the pin then we remain on the wa camera - even if we are now
				// actually outside of it

				sub1 = (PXreal)posi->x - wa_pin_x;
				sub2 = (PXreal)posi->z - wa_pin_z;

				// dist
				len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

				Tdebug("rubber.txt", "len %3.2f   pos %3.2f, %3.2f pin %3.2f, %3.2f", len, posi->x, posi->z, wa_pin_x, wa_pin_z);

				if (len < (PXreal)(REAL_RUBBER * REAL_RUBBER))
					return; // still constrained by the band

				// band is broken
				wa_tied_to_pin = FALSE8; // release
			}

			if ((posi->x > wa->x) && (posi->x < (wa->x + wa->w)) && (posi->z > wa->z) && (posi->z < (wa->z + wa->h))) {
				// we're inside the rectangle
				// are we within the polygon

				hit = 0; // result is 0, miss

				for (k = 0; k < (wa->noPoints - 1); k++) {
					if (Contains(wa->points[k].x, wa->points[k].z, wa->points[k + 1].x, wa->points[k + 1].z, (uint32)posi->x, (uint32)posi->z))
						hit = 1 - hit;
				}

				if (hit) { // hurrah - still hitting
					cur_camera_number = floor_to_camera_index[this_rect];
					return;
				}
			}
		}

		// only stick an off pin in if we're not wandering straight onto another
		if (!Process_wa_list()) {
			// not hitting anymore
			if (!g_mission->camera_follow_id_overide)
				this_rect = floor_def->Return_non_rubber_floor_no(logic_structs[player.Fetch_player_id()], this_rect);
			else
				this_rect = floor_def->Return_non_rubber_floor_no(logic_structs[g_mission->camera_follow_id_overide], this_rect);

			// stick a pin in that we must leave before hitting a wa again
			wa_pin_x = posi->x;
			wa_pin_y = posi->y;
			wa_pin_z = posi->z;
			wa_tied_to_exit_pin = TRUE8;

			cur_camera_number = NO_CAMERA_CHOSEN; // force a choose
			wa_camera = FALSE8;                   // not any more
		}
	}

	// not on one so check em all to see if we are
	// but first check to see if we are tied to a get-off-wa pin

	if (wa_tied_to_exit_pin) {

		if (wa_pin_y != posi->y) {            // as soon as we change y the pin breaks
			wa_tied_to_exit_pin = FALSE8; // off
		} else {
			sub1 = (PXreal)posi->x - wa_pin_x;
			sub2 = (PXreal)posi->z - wa_pin_z;

			// dist
			len = (PXreal)((sub1 * sub1) + (sub2 * sub2));

			if (len > (PXreal)(REAL_RUBBER * REAL_RUBBER)) {
				// band is broken
				wa_tied_to_exit_pin = FALSE8; // release
			}
		}
	} else {
		Process_wa_list();
	}

	// not on a WA camera so check via the floor rects

	if (this_rect == PXNULL) { // object not on a rect??
		// we can't pick a camera set view
		// if a set is set up then that will continue to be used
		// if there isn't a set at all (this is first cycle) then engine will switch to nethack mode
		return;
	}

	// has player changed camera - work this out from primary camera
	if (cur_camera_number != floor_to_camera_index[this_rect]) {
		// we are now on a floor with a different camera
		// BUT
		// we allow some elastic on the old one before switching

		// get floor that we last changed camera on
		// if we are within its bounds then remain

		if (cur_camera_number != NO_CAMERA_CHOSEN) {
			_floor *floor;
			floor = (_floor *)floor_def->Fetch_floor_number(anchor_floor);

			if ((posi->y == (PXreal)(floor->base_height)) && (posi->x >= (PXreal)(floor->rect.x1 - RUBBER)) && (posi->x <= (PXreal)(floor->rect.x2 + RUBBER)) &&
			    (posi->z >= (PXreal)(floor->rect.z1 - RUBBER)) && (posi->z <= (PXreal)(floor->rect.z2 + RUBBER)))
				return; // still within the rubber banded camera
		}

		if (floor_to_camera_index[this_rect] == 0xffffffff) { // no named camera so its a more complex logic switch
			// ok, lets assume that there was no special camera!
			g_px->display_mode = TEMP_NETHACK; // stick us into temporary nethack mode which will bounce out again if it can
			Zdebug("no named camera! - entering TEMP_NETHACK");
		} else { // ok, there is a named camera! and we know its different from current
			// different from current
			Zdebug(" make cam=%s %s", camera_name_list[floor_to_camera_index[this_rect]], camera_cluster_list[floor_to_camera_index[this_rect]]);

			// set camera number
			cur_camera_number = floor_to_camera_index[this_rect];
			anchor_floor = this_rect;

			// if we're not in NETHACK mode then switch in the set
			if (g_px->display_mode != NETHACK) {
				g_px->display_mode = THREED; // force back in-case we were in TEMP_NETHACK

				// init the new set - Initialise_set will record the name of the new camera
				// it will also handle missing set files and bung us into TEMP_NETHACK mode if it has to
				Initialise_set(camera_name_list[floor_to_camera_index[this_rect]], camera_cluster_list[floor_to_camera_index[this_rect]]); // name);

				// force in the anims of megas not playing stand - techs and the like
				MS->One_logic_cycle();
			}
		}
	}
}

void _game_session::Prepare_camera_floors() {
	// get floor numbers from followed object
	// get a mega class objects world position - can be player or other mega
	if (!g_mission->camera_follow_id_overide) {
		if (!player.Player_exists())
			Fatal_error("camera director can't choose a scene as player object has been shut down");

		if (logic_structs[player.Fetch_player_id()]->ob_status == OB_STATUS_HELD)
			Fatal_error("camera director can't choose a scene as player object has been shut down");

		posi = (PXvector *)&logic_structs[player.Fetch_player_id()]->mega->actor_xyz;

		this_rect = logic_structs[player.Fetch_player_id()]->owner_floor_rect;
	} else {
		// following another mega character

		posi = (PXvector *)&logic_structs[g_mission->camera_follow_id_overide]->mega->actor_xyz;

		this_rect = logic_structs[g_mission->camera_follow_id_overide]->owner_floor_rect;
	}

	// fetch the floor
	obfloor = (_floor *)floor_def->Fetch_floor_number(this_rect);
}

bool8 _game_session::Process_wa_list() {
	const __aWalkArea *wa;
	uint32 j, k;
	uint32 hit;
	char *name;
	PXreal y;

	for (j = 0; j < MS->total_was; j++) {
		wa = MS->wa_list[j];

		y = floor_def->Return_true_y((PXreal)wa->y);

		if ((y >= obfloor->base_height) && (y < (floor_def->Fetch_floors_volume_height(this_rect)))) {
			if ((posi->x > wa->x) && (posi->x < (wa->x + wa->w)) && (posi->z > wa->z) && (posi->z < (wa->z + wa->h))) {
				// we're inside the rectangle
				// are we within the polygon

				hit = 0; // result is 0, miss

				for (k = 0; k < (wa->noPoints - 1); k++) {
					if (Contains(wa->points[k].x, wa->points[k].z, wa->points[k + 1].x, wa->points[k + 1].z, (uint32)posi->x, (uint32)posi->z))
						hit = 1 - hit;
				}

				if (hit) { // hey we hit the closed poly
					name = (char *)const_cast<ICB::__point *>(&wa->points[wa->noPoints]);

					Tdebug("cam_changes.txt", " WA camera name %s cluster %s", name, wa->cameraCluster);

					Initialise_set(name, wa->cameraCluster); // name, clusterVersion of the name

					wa_camera = TRUE8;
					wa_number = j;
					wa_pin_x = posi->x;
					wa_pin_z = posi->z;
					wa_tied_to_pin = TRUE8;

					// in case we are coming here afresh - from a reset camera director - i.e. from nethack mode
					// find the floor and set the current camera
					// in other words we fake the system to think that the floor camera is chosen - thats how we decide
					// who is on camera.

					if (!g_mission->camera_follow_id_overide) {
						this_rect = floor_def->Return_floor_rect(posi->x, posi->z, posi->y, 0);
					} else {
						this_rect = floor_def->Return_non_rubber_floor_no(logic_structs[g_mission->camera_follow_id_overide], this_rect);
					}

					// set camera number
					cur_camera_number = floor_to_camera_index[this_rect];

					Tdebug("cam_changes.txt", "  floor %d", cur_camera_number);

					return TRUE8;
				}
			}
		}
	}

	return FALSE8;
}

mcodeFunctionReturnCodes _game_session::fn_switch_to_manual_camera(int32 &, int32 *params) {
	// switch in a manual camera

	//	params   0   stub name of room - CORRIDOR\pc\camera
	//				1 stub name of camera - corridor\pc\CAMERA
	//				2 name of primary floor
	uint32 floor_num;
	uint32 len;
	char h_buf[8];
	const char *room_name = (const char *)MemoryUtil::resolvePtr(params[0]);
	const char *camera_name = (const char *)MemoryUtil::resolvePtr(params[1]);
	const char *floor_name = (const char *)MemoryUtil::resolvePtr(params[2]);

	// get primary floor number
	floor_num = floor_def->Fetch_floor_number_by_name(floor_name);

	// set camera number
	cur_camera_number = floor_to_camera_index[floor_num];

	manual_camera = TRUE8;

	len = Common::sprintf_s(manual_camera_name, "%s\\pc\\%s", room_name, camera_name);
	if (len > ENGINE_STRING_LEN)
		Fatal_error("fn_switch_to_manual_camera string len error");

	HashFile(manual_camera_name, h_buf);

	Tdebug("cam_changes.txt", " built name %s %s", (const char *)temp_buf, h_buf);
	Initialise_set(manual_camera_name, h_buf); // name);

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_current_camera(int32 &result, int32 *params) {
	// check passed string against current camera
	//	params   0   ascii name
	const char *camera_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// check for not set initialised yet - 1st game cycle
	if (!set.OK()) {
		result = 0;
		return IR_CONT;
	}

	if (!strstr(set.GetSetName(), camera_name))
		result = 0; // no
	else
		result = 1;

	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_is_current_location(int32 &result, int32 *params) {
	char h_buf[8];
	uint32 len;
	const char *location_name = (const char *)MemoryUtil::resolvePtr(params[0]);

	// First we need to know which location the player is in, because the information level
	// for this is automatically at full.
	//uint32 nPlayerFloorIndex = MS->logic_structs[MS->player.Fetch_player_id()]->owner_floor_rect;

	Message_box("is %s current location?", location_name);

	len = Common::sprintf_s(manual_camera_name, "%s\\pc\\%s", location_name, set.GetSetName());
	if (len > ENGINE_STRING_LEN)
		Fatal_error("fn_is_current_location string len error");

	HashFile(manual_camera_name, h_buf);
	result = 1;
	// continue script
	return IR_CONT;
}

mcodeFunctionReturnCodes _game_session::fn_cancel_manual_camera(int32 &, int32 *) {
	// cancel the manual camera
	Tdebug("cam_changes.txt", "Releasing manual camera");

	if (!manual_camera)
		return IR_CONT;

	Reset_camera_director();

	return (IR_CONT);
}

// Compute if mega's are on or off current camera and send events to them to say "ON_CAMERA" / "OFF_CAMERA"
void _game_session::UpdateOnOffCamera() {
	PXcamera &camera = GetCamera();
	_logic *log;

	for (uint32 j = 0; j < number_of_voxel_ids; j++) {
		// fetch the logic structure for the game object that has a voxel image to render
		log = logic_structs[voxel_id_list[j]];

		// shift the this cycle camera flag to last cycle camera flag
		log->mega->ShiftViewState();

		// person owned by current camera floor? TEMP check
		if (Object_visible_to_camera(voxel_id_list[j])) {
			bool8 result = TRUE8;
			PXvector filmPosition;
			PXWorldToFilm(log->mega->actor_xyz, camera, result, filmPosition);

			// i.e. his feet are visible !
			if (result && (filmPosition.z < -g_actor_hither_plane))
				log->mega->SetThisViewState(ON_CAMERA);
		}

		// Now test the view state flags and send an appropriate event
		// Just walked on camera
		if (log->mega->viewState == OFF_ON_CAMERA) {
			g_oEventManager->PostNamedEvent(EVENT_ON_CAMERA, voxel_id_list[j]);
		}
		// Just walked off camera
		else if (log->mega->viewState == ON_OFF_CAMERA) {
			g_oEventManager->PostNamedEvent(EVENT_OFF_CAMERA, voxel_id_list[j]);
		}
	}
}

} // End of namespace ICB
