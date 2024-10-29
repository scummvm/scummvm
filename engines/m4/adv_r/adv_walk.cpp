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

#include "m4/adv_r/adv_walk.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/graphics/gr_series.h"
#include "m4/wscript/wst_regs.h"
#include "m4/vars.h"

namespace M4 {

void set_walker_scaling(SceneDef *rdef) {
	_G(globals)[GLB_MIN_Y] = rdef->back_y << 16;
	_G(globals)[GLB_MAX_Y] = rdef->front_y << 16;
	_G(globals)[GLB_MIN_SCALE] = FixedDiv(rdef->back_scale << 16, 100 << 16);
	_G(globals)[GLB_MAX_SCALE] = FixedDiv(rdef->front_scale << 16, 100 << 16);
	if (_G(globals)[GLB_MIN_Y] == _G(globals)[GLB_MAX_Y])
		_G(globals)[GLB_SCALER] = 0;
	else
		_G(globals)[GLB_SCALER] = FixedDiv(_G(globals)[GLB_MAX_SCALE] - _G(globals)[GLB_MIN_SCALE], _G(globals)[GLB_MAX_Y] - _G(globals)[GLB_MIN_Y]);
}

static void ws_walkto_node(machine *myWalker, railNode *destNode, bool firstTime) {
	frac16	x, y, s;

	// Parameter verification
	if (!myWalker) {
		error_show(FL, 'W:-(');
		return;
	}
	if (!destNode) {
		error_show(FL, 'WNDN');
		return;
	}

	// Calculate the destination values x, y, s
	x = destNode->x << 16;
	y = destNode->y << 16;
	s = _G(globals)[GLB_MIN_SCALE] + FixedMul(y - _G(globals)[GLB_MIN_Y], _G(globals)[GLB_SCALER]);

	// Plug in the destination x, y, and s
	_G(globals)[GLB_TEMP_1] = x;
	_G(globals)[GLB_TEMP_2] = y;
	_G(globals)[GLB_TEMP_3] = s;

	// Final direction (GLB_TEMP_4) and trigger (GLB_TEMP_5) are set in ws_walk() below
	if (firstTime) {
		if (_G(completeWalk)) {
			_G(globals)[GLB_TEMP_6] = 0x10000;	// so the feet will come together
		} else {
			_G(globals)[GLB_TEMP_6] = 0;		// so the walker will freeze when he reaches the last node
		}
		sendWSMessage(STARTWALK << 16, 0, myWalker, 0, nullptr, 1);
	} else {
		sendWSMessage(WALKSEQ << 16, 0, myWalker, 0, nullptr, 1);
	}
}

bool walker_has_walk_finished(machine *sender) {
	railNode *tempNode;

	// Parameter verification
	if ((!sender) || (!sender->myAnim8)) {
		error_show(FL, 'W:-(');
		return false;
	}

	// Remove the node we just arrived at from the sender's walkPath
	if (sender->walkPath) {
		tempNode = sender->walkPath;
		sender->walkPath = sender->walkPath->shortPath;
		mem_free((void *)tempNode);
	}

	// If no more nodes to traverse (a canadian word), check if he's standing.
	// If not standing, let him finish standing, then when he finishes standing
	if (!sender->walkPath) {
		return true;
	} else {
		// Else there are more nodes, so keep walking
		ws_walkto_node(sender, sender->walkPath, false);
		return false;
	}
}

/**
 * Called by player_walk 
 */
void ws_walk(machine *myWalker, int32 x, int32 y, GrBuff **, int16 trigger, int32 finalFacing, bool complete_walk) {
	int8 directions[14] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9 };
	int32 currX, currY;
	int32 currNodeID, destNodeID;
	bool result;

	if (!myWalker || !myWalker->myAnim8)
		error_show(FL, 'W:-(');

	// Get walker's current location
	currX = myWalker->myAnim8->myRegs[IDX_X] >> 16;
	currY = myWalker->myAnim8->myRegs[IDX_Y] >> 16;

	// Add the walker's current location and the destination to the rail nodes...
	Buffer *walkerCodes = nullptr;
	if (_G(screenCodeBuff))
		walkerCodes = _G(screenCodeBuff)->get_buffer();
	if ((currNodeID = AddRailNode(currX, currY, walkerCodes, true)) < 0) {
		error_show(FL, 'WCAN', "Walker's curr posn: %d %d", currX, currY);
	}
	if ((destNodeID = AddRailNode(x, y, walkerCodes, true)) < 0) {
		error_show(FL, 'WCAN', "Trying to walk to: %d %d", x, y);
	}

	// Dispose of the current path myWalker is following
	if (myWalker->walkPath) {
		DisposePath(myWalker->walkPath);
	}

	// Find the shortest path between currNodeID, and destNodeID
	result = GetShortestPath(currNodeID, destNodeID, &(myWalker->walkPath));

	// Now that a path has been found, remove the two extra added nodes
	RemoveRailNode(currNodeID, walkerCodes, true);
	RemoveRailNode(destNodeID, walkerCodes, true);
	if (_G(screenCodeBuff))
		_G(screenCodeBuff)->release();

	// Check the success of GetShortestPath
	if (!result) {
		term_message("Player: Can't walk there!!!");
		_G(player).waiting_for_walk = false;
		return;
	}

	// If the result was true, but no path was returned, we are already there
	if (!myWalker->walkPath) {
		_G(player).need_to_walk = false;

		//we can only turn to face the final direction
		ws_turn_to_face(myWalker, finalFacing, trigger);
	} else {
		// Otherwise we have a path to follow, so get going...

		// Verify that finalFacing is valid or set -1
		if (finalFacing > 0 && finalFacing < 13) {
			_G(globals)[GLB_TEMP_4] = directions[finalFacing] << 16;
		} else {
			_G(globals)[GLB_TEMP_4] = (frac16)-1 & ~0xffff;
		}

		// Set the trigger to be returned when the walk is finished
		_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);

		_G(completeWalk) = complete_walk;
		ws_walkto_node(myWalker, myWalker->walkPath, true);
	}

	if (_G(hyperwalk))
		adv_hyperwalk_to_final_destination(nullptr, nullptr);
}

bool adv_walker_path_exists(machine *myWalker, int32 x, int32 y) {
	int32 currX, currY;
	int32 currNodeID, destNodeID;
	bool result;

	if (!myWalker || !myWalker->myAnim8) {
		error_show(FL, 'W:-(');
		return false;
	}

	// Get walker's current location
	currX = myWalker->myAnim8->myRegs[IDX_X] >> 16;
	currY = myWalker->myAnim8->myRegs[IDX_Y] >> 16;

	// Add the walker's current location and the destination to the rail nodes...
	Buffer *walkerCodes = nullptr;
	if (_G(screenCodeBuff)) {
		walkerCodes = _G(screenCodeBuff)->get_buffer();
	}
	if ((currNodeID = AddRailNode(currX, currY, walkerCodes, true)) < 0) {
		error_show(FL, 'WCAN', "Walker's curr posn: %d %d", currX, currY);
	}
	if ((destNodeID = AddRailNode(x, y, walkerCodes, true)) < 0) {
		error_show(FL, 'WCAN', "Trying to walk to: %d %d", x, y);
	}

	// Dispose of the current path myWalker is following
	if (myWalker->walkPath) {
		DisposePath(myWalker->walkPath);
	}

	// Find the shortest path between currNodeID, and destNodeID
	result = GetShortestPath(currNodeID, destNodeID, &(myWalker->walkPath));

	// Now that a path has been attempted, remove the two extra added nodes
	RemoveRailNode(currNodeID, walkerCodes, true);
	RemoveRailNode(destNodeID, walkerCodes, true);
	if (_G(screenCodeBuff))
		_G(screenCodeBuff)->release();

	return result;
}


void ws_custom_walk(machine *myWalker, int32 finalFacing, int32 trigger, bool complete_walk) {
	const int8 directions[14] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9 };

	// Verify parameters
	if ((!myWalker) || (!myWalker->walkPath)) {
		return;
	}

	// Verify that finalFacing is valid or set -1
	if (finalFacing > 0 && finalFacing < 13) {
		_G(globals)[GLB_TEMP_4] = directions[finalFacing] << 16;
	} else {
		_G(globals)[GLB_TEMP_4] = (frac16)-1 & ~0xffff;
	}

	// Set the trigger to be returned when the walk is finished
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);

	// Begin the walk...
	_G(completeWalk) = complete_walk;
	ws_walkto_node(myWalker, myWalker->walkPath, true);
}


void ws_demand_facing(machine *myWalker, int32 facing) {
	const int8 directions[13] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9 };

	if ((!myWalker) || (!myWalker->myAnim8)) {
		term_message("demand facing, but no walker");
		return;
	}

	if (facing > 0 && facing < 13) {
		_G(globals)[GLB_TEMP_4] = directions[facing] << 16;
		sendWSMessage(DEMAND_FACING << 16, 0, myWalker, 0, nullptr, 1);
	}
}

void ws_demand_location(machine *myWalker, int32 x, int32 y, int facing) {
	frac16 s;

	if (!myWalker || !myWalker->myAnim8) {
		term_message("demand locn, no walker");
		return;
	}

	s = _G(globals)[GLB_MIN_SCALE] + FixedMul((y << 16) - _G(globals)[GLB_MIN_Y], _G(globals)[GLB_SCALER]);

	_G(globals)[GLB_TEMP_1] = x << 16;
	_G(globals)[GLB_TEMP_2] = y << 16;
	_G(globals)[GLB_TEMP_3] = s;

	sendWSMessage(DEMAND_LOCATION << 16, 0, myWalker, 0, nullptr, 1);

	if (facing != -1)
		ws_demand_facing(myWalker, facing);
}

static void ws_demand_location_and_facing(machine *myWalker, int32 x, int32 y, int32 facing) {
	frac16 s;

	if ((!myWalker) || (!myWalker->myAnim8)) {
		term_message("demand f & l, no walker");
		return;
	}

	s = _G(globals)[GLB_MIN_SCALE] + FixedMul((y << 16) - _G(globals)[GLB_MIN_Y], _G(globals)[GLB_SCALER]);

	_G(globals)[GLB_TEMP_1] = x << 16;
	_G(globals)[GLB_TEMP_2] = y << 16;
	_G(globals)[GLB_TEMP_3] = s;

	if (facing > 0 && facing < 13)
		// WORKAROUND: The original's hyperwalk didn't work. By doing
		// the facing set separately, this is fixed
		ws_demand_facing(facing);

	sendWSMessage(DEMAND_LOCATION << 16, 0, myWalker, 0, nullptr, 1);
	_G(player).waiting_for_walk = false; // lets parse code get called when there is no facing set (from scenedit)
}

void ws_turn_to_face(machine *myWalker, int32 facing, int32 trigger) {
	int8 directions[13] = { 0, 0, 1, 2, 3, 4, 4, 5, 6, 7, 8, 9, 9 };

	if (!myWalker || !myWalker->myAnim8) {
		error_show(FL, 'W:-(', "demand facing: %d", facing);
		return;
	}

	// Verify that facing is valid or set -1
	if (facing > 0 && facing < 13) {
		_G(globals)[GLB_TEMP_4] = directions[facing] << 16;
	} else {
		_G(globals)[GLB_TEMP_4] = (frac16)-1 & ~0xffff;
	}

	// Set the trigger to be returned when the walk is finished
	_G(globals)[GLB_TEMP_5] = kernel_trigger_create(trigger);

	// Make sure the _G(completeWalk) flag is set
	_G(globals)[GLB_TEMP_6] = 0x10000;

	sendWSMessage(TURN_TO_FACE << 16, 0, myWalker, 0, nullptr, 1);
}

void ws_demand_location(int32 x, int32 y, int facing) {
	ws_demand_location(_G(my_walker), x, y, facing);
}

void ws_hide_walker(machine *myWalker) {
	if (!myWalker) {
		error_show(FL, 'W:-(');
		return;
	}

	_G(player).walker_visible = false;
	sendWSMessage(PLAYER_HIDE << 16, 0, myWalker, 0, nullptr, 1);
}

void ws_unhide_walker(machine *myWalker) {
	if (!myWalker) {
		error_show(FL, 'W:-(');
		return;
	}

	_G(player).walker_visible = true;
	sendWSMessage(PLAYER_UNHIDE << 16, 0, myWalker, 0, nullptr, 1);
}

void ws_demand_facing(int32 newFacing) {
	ws_demand_facing(_G(my_walker), newFacing);
}

void ws_turn_to_face(int32 facing, int32 trigger) {
	ws_turn_to_face(_G(my_walker), facing, trigger);
}

void ws_hide_walker() {
	ws_hide_walker(_G(my_walker));
}

void ws_unhide_walker() {
	ws_unhide_walker(_G(my_walker));
}

void ws_walk(int32 x, int32 y, GrBuff **buffer, int16 trigger, int32 finalFacing, bool complete_walk) {
	ws_walk(_G(my_walker), x, y, buffer, trigger, finalFacing, complete_walk);
}

void ws_get_walker_info(machine *myWalker, int32 *x, int32 *y, int32 *s, int32 *layer, int32 *facing) {
	Anim8 *myAnim8;
	const int8 facings[10] = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11 };

	if (!myWalker || !myWalker->myAnim8) {
		error_show(FL, 'W:-(');
		return;
	}

	myAnim8 = myWalker->myAnim8;

	if (x) {
		*x = myAnim8->myRegs[IDX_X] >> 16;
	}
	if (y) {
		*y = myAnim8->myRegs[IDX_Y] >> 16;
	}
	if (s) {
		*s = MulSF16(100 << 16, myAnim8->myRegs[IDX_S]) >> 16;
	}
	if (layer) {
		*layer = myAnim8->myRegs[IDX_LAYER] >> 16;
	}
	if (facing) {
		int index = myAnim8->myRegs[IDX_CELS_HASH] >> 24;

		// WORKAROUND: At the very least, Mei Chen in Riddle room 201
		// produces a negative index value. This ensures indexes are valid
		if (index < 0 || index > 9)
			index = 0;

		if (myAnim8->myRegs[IDX_W] < 0)
			index = 9 - index;

		*facing = facings[index];
	}
}


bool ws_walk_init_system() {
	// Initialize walker
	_G(globals)[GLB_MIN_Y] = _G(currentSceneDef).back_y << 16;
	_G(globals)[GLB_MAX_Y] = _G(currentSceneDef).front_y << 16;
	_G(globals)[GLB_MIN_SCALE] = FixedDiv(_G(currentSceneDef).back_scale << 16, 100 << 16);
	_G(globals)[GLB_MAX_SCALE] = FixedDiv(_G(currentSceneDef).front_scale << 16, 100 << 16);
	if (_G(globals)[GLB_MIN_Y] == _G(globals)[GLB_MAX_Y]) {
		_G(globals)[GLB_SCALER] = 0;
	} else {
		_G(globals)[GLB_SCALER] = FixedDiv(_G(globals)[GLB_MAX_SCALE] - _G(globals)[GLB_MIN_SCALE], _G(globals)[GLB_MAX_Y] - _G(globals)[GLB_MIN_Y]);
	}

	_G(my_walker) = _GW().walk_initialize_walker();

	if (!_G(my_walker)) {
		error_show(FL, 'W:-(');
		return false;
	}
	return true;
}

bool ws_walk_load_series(const int16 *dir_array, const char *name_array[], bool shadow_flag, bool load_palette) {
	int32 i = 0;

	while (dir_array[i] >= 0) {
		int32 result = AddWSAssetCELS(name_array[i], dir_array[i],
			(load_palette && !shadow_flag) ? _G(master_palette) : nullptr);
		if (result < 0) {
			error_show(FL, 'W:-(');
			return false;
		}
		i++;
	}

	return true;
}

bool ws_walk_load_walker_series(const int16 *dir_array, const char *name_array[], bool load_palette) {
	return (ws_walk_load_series(dir_array, name_array, false, load_palette));
}

bool ws_walk_load_shadow_series(const int16 *dir_array, const char *name_array[]) {
	return (ws_walk_load_series(dir_array, name_array, true, false));
}

void ws_walk_dump_series(int16 num_directions, int16 start_hash) {
	int32 i;

	for (i = 0; i < num_directions; i++) {
		series_unload(start_hash++);
	}
}

void adv_get_walker_destination(machine *my_walker, int32 *x, int32 *y, int32 *final_facing) {
	railNode *current_node;
	int32 face;
	int8 directions[11] = { 1, 2, 3, 4, 5, 7, 8, 9, 10, 11 };

	// If there is no walker, or the walker is not on a walk path, return 
	if (!my_walker || !my_walker->walkPath) {
		*x = 0;
		*y = 0;
		*final_facing = 0;
		return;
	}

	// Find the end of the path
	current_node = my_walker->walkPath;
	while (current_node->shortPath) {
		current_node = current_node->shortPath;
	}

	// Set the destination coords
	*x = current_node->x;
	*y = current_node->y;

	// Get final facing from l.v.6 = myRegs[6 + IDX_COUNT]
	face = my_walker->myAnim8->myRegs[6 + IDX_COUNT] >> 16;
	*final_facing = directions[face];
}


void adv_hyperwalk_to_final_destination(void *, void *) {
	int32 x, y;
	int32 facing;

	_G(i_just_hyperwalked) = true;

	// Make sure we have a walker, that it can walk in this scene, and that we can hyperwalk
	if ((!_G(my_walker)) || (!_G(player).walker_in_this_scene) || _G(player).disable_hyperwalk) {
		return;
	}

	// If the walker is not currently walking anywhere, return
	if (!_G(my_walker)->walkPath) {
		return;
	}

	//get the final direction and facing
	adv_get_walker_destination(_G(my_walker), &x, &y, &facing);

	// Nuke the rail node path
	DisposePath(_G(my_walker)->walkPath);
	_G(my_walker)->walkPath = nullptr;

	// This will make player goto x,y,facing. when that happens, trigger will return
	ws_demand_location_and_facing(_G(my_walker), x, y, facing);
}

} // End of namespace M4
