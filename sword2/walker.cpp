/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

// WALKER.CPP by James (14nov96)

// Script functions for moving megas about the place & also for keeping tabs
// on them

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"

namespace Sword2 {

/**
 * Walk mega to (x,y,dir). Set RESULT to 0 if it succeeded. Otherwise, set
 * RESULT to 1.
 */

int32 Logic::fnWalk(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord
	//		6 target direction (8 means end walk on ANY direction)

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);
	ObjectGraphic *ob_graph = (ObjectGraphic *) _vm->_memory->decodePtr(params[1]);
	ObjectMega *ob_mega  = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

	int16 target_x = (int16) params[4];
	int16 target_y = (int16) params[5];
	uint8 target_dir = (uint8) params[6];

	ObjectWalkdata *ob_walkdata;

	// If this is the start of the walk, calculate the route.

	if (!ob_logic->looping) {
		// If we're already there, don't even bother allocating
		// memory and calling the router, just quit back & continue
		// the script! This avoids an embarassing mega stand frame
		// appearing for one cycle when we're already in position for
		// an anim eg. repeatedly clicking on same object to repeat
		// an anim - no mega frame will appear in between runs of the
		// anim.
		
		if (ob_mega->feet_x == target_x && ob_mega->feet_y == target_y && ob_mega->current_dir == target_dir) {
			_scriptVars[RESULT] = 0;
			return IR_CONT;
		}

		assert(params[6] >= 0 && params[6] <= 8);

		ob_walkdata = (ObjectWalkdata *) _vm->_memory->decodePtr(params[3]);

		ob_mega->walk_pc = 0;

		// Set up mem for _walkData in route_slots[] & set mega's
		// 'route_slot_id' accordingly

		_router->allocateRouteMem();

		int32 route = _router->routeFinder(ob_mega, ob_walkdata, target_x, target_y, target_dir);

		// 0 = can't make route to target
		// 1 = created route
		// 2 = zero route but may need to turn

		if (route == 1 || route == 2) {
			// so script fnWalk loop continues until end of
			// walk-anim

			ob_logic->looping = 1;

			// need to animate the route now, so don't set result
			// or return yet!

			// started walk
			ob_mega->currently_walking = 1;

			// (see fnGetPlayerSaveData() in save_rest.cpp
		} else {
			_router->freeRouteMem();
			_scriptVars[RESULT] = 1;
			return IR_CONT;
		}

		// Walk is about to start, so set the mega's graphic resource
		ob_graph->anim_resource = ob_mega->megaset_res;
	} else if (_scriptVars[EXIT_FADING] && _vm->_graphics->getFadeStatus() == RDFADE_BLACK) {
		// Double clicked an exit so quit the walk when screen is black
		// ok, thats it - back to script and change screen

		ob_logic->looping = 0;
		_router->freeRouteMem();

		// Must clear in-case on the new screen there's a walk
		// instruction (which would get cut short)
		_scriptVars[EXIT_CLICK_ID] = 0;

		// finished walk
		ob_mega->currently_walking = 0;

		// see fnGetPlayerSaveData() in save_rest.cpp

		_scriptVars[RESULT] = 0;

		// continue the script so that RESULT can be checked!
		return IR_CONT;
	}

	// get pointer to walkanim & current frame position

	WalkData *walkAnim = _router->getRouteMem();
	int32 walk_pc = ob_mega->walk_pc;

	// If stopping the walk early, overwrite the next step with a
	// slow-out, then finish

	if (checkEventWaiting()) {
		if (walkAnim[walk_pc].step == 0 && walkAnim[walk_pc + 1].step == 1) {
			// At the beginning of a step
			ob_walkdata = (ObjectWalkdata *) _vm->_memory->decodePtr(params[3]);
			_router->earlySlowOut(ob_mega, ob_walkdata);
		}
	}

	// Get new frame of walk

	ob_graph->anim_pc = walkAnim[walk_pc].frame;
	ob_mega->current_dir = walkAnim[walk_pc].dir;
	ob_mega->feet_x = walkAnim[walk_pc].x;
	ob_mega->feet_y = walkAnim[walk_pc].y;

	// Check if NEXT frame is in fact the end-marker of the walk sequence
	// so we can return to script just as the final (stand) frame of the
	// walk is set - so that if followed by an anim, the anim's first
	// frame replaces the final stand-frame of the walk (see below)

	// '512' is end-marker
	if (walkAnim[walk_pc + 1].frame == 512) {
		ob_logic->looping = 0;
		_router->freeRouteMem();

		// finished walk
		ob_mega->currently_walking = 0;

		// (see fnGetPlayerSaveData() in save_rest.cpp

		// if George's walk has been interrupted to run a new action
		// script for instance or Nico's walk has been interrupted by
		// player clicking on her to talk

		// There used to be code here for checking if two megas were
		// colliding, but that code had been commented out, and it
		// was only run if a function that always returned zero
		// returned non-zero.

		if (checkEventWaiting()) {
			startEvent();
			_scriptVars[RESULT] = 1;
			return IR_TERMINATE;
		} else {
			_scriptVars[RESULT] = 0;

			// CONTINUE the script so that RESULT can be checked!
			// Also, if an anim command follows the fnWalk command,
			// the 1st frame of the anim (which is always a stand
			// frame itself) can replace the final stand frame of
			// the walk, to hide the slight difference between the
			// shrinking on the mega frames and the pre-shrunk anim
			// start-frame.

			return IR_CONT;
		}
	}

	// Increment the walkanim frame number and come back next cycle

	ob_mega->walk_pc++;
	return IR_REPEAT;
}

/**
 * Walk mega to start position of anim
 */

int32 Logic::fnWalkToAnim(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 anim resource id

	int32 pars[7];

	// Walkdata is needed for earlySlowOut if player clicks elsewhere
	// during the walk.

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the walk, read anim file to get start coords

	if (!ob_logic->looping) {
		byte *anim_file = _vm->_resman->openResource(params[4]);
		AnimHeader *anim_head = _vm->fetchAnimHeader( anim_file );

		pars[4] = anim_head->feetStartX;
		pars[5] = anim_head->feetStartY;
		pars[6] = anim_head->feetStartDir;

		_vm->_resman->closeResource(params[4]);

		// If start coords not yet set in anim header, use the standby
		// coords (which should be set beforehand in the script).

		if (pars[4] == 0 && pars[5] == 0) {
			pars[4] = _standbyX;
			pars[5] = _standbyY;
			pars[6] = _standbyDir;

			debug(3, "WARNING: fnWalkToAnim(%s) used standby coords", _vm->fetchObjectName(params[4]));
		}

		assert(pars[6] >= 0 && pars[6] <= 7);
	}

	return fnWalk(pars);
}

/**
 * Turn mega to <direction>. Just needs to call fnWalk() with current feet
 * coords, so router can produce anim of turn frames.
 */

int32 Logic::fnTurn(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target direction

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the turn, get the mega's current feet
	// coords + the required direction

	if (!ob_logic->looping) {
		assert(params[4] >= 0 && params[4] <= 7);

		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = params[4];
	}

	return fnWalk(pars);
}

/**
 * Stand mega at (x,y,dir)
 * Sets up the graphic object, but also needs to set the new 'current_dir' in
 * the mega object, so the router knows in future
 */

int32 Logic::fnStandAt(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target x-coord
	//		3 target y-coord
	//		4 target direction

	assert(params[4] >= 0 && params[4] <= 7);

	ObjectGraphic *ob_graph = (ObjectGraphic *) _vm->_memory->decodePtr(params[0]);
	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[1]);

	// set up the stand frame & set the mega's new direction

	ob_mega->feet_x = params[2];
	ob_mega->feet_y = params[3];
	ob_mega->current_dir = params[4];

	// mega-set animation file
	ob_graph->anim_resource	= ob_mega->megaset_res;

	// dir + first stand frame (always frame 96)
	ob_graph->anim_pc = params[4] + 96;

	return IR_CONT;
}

/**
 * Stand mega in <direction> at current feet coords
 * Just needs to call fnStandAt() with current feet coords
 */

int32 Logic::fnStand(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target direction

	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[1]);

	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = ob_mega->feet_x;
	pars[3] = ob_mega->feet_y;
	pars[4] = params[2];

	return fnStandAt(pars);
}

/**
 * stand mega at end position of anim
 */

int32 Logic::fnStandAfterAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	byte *anim_file = _vm->_resman->openResource(params[2]);
	AnimHeader *anim_head = _vm->fetchAnimHeader(anim_file);

	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetEndX;
	pars[3] = anim_head->feetEndY;
	pars[4] = anim_head->feetEndDir;

	// If start coords not available either use the standby coords (which
	// should be set beforehand in the script)

	if (pars[2] == 0 && pars[3] == 0) {
		pars[2] = _standbyX;
		pars[3] = _standbyY;
		pars[4] = _standbyDir;

		debug(3, "WARNING: fnStandAfterAnim(%s) used standby coords", _vm->fetchObjectName(params[2]));
	}

	assert(pars[4] >= 0 && pars[4] <= 7);

	_vm->_resman->closeResource(params[2]);
	return fnStandAt(pars);
}

/**
 * Stand mega at start position of anim
 */

int32 Logic::fnStandAtAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	byte *anim_file = _vm->_resman->openResource(params[2]);
	AnimHeader *anim_head = _vm->fetchAnimHeader(anim_file);

	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetStartX;
	pars[3] = anim_head->feetStartY;
	pars[4] = anim_head->feetStartDir;

	// If start coords not available use the standby coords (which should
	// be set beforehand in the script)

	if (pars[2] == 0 && pars[3] == 0) {
		pars[2] = _standbyX;
		pars[3] = _standbyY;
		pars[4] = _standbyDir;

		debug(3, "WARNING: fnStandAtAnim(%s) used standby coords", _vm->fetchObjectName(params[2]));
	}

	assert(pars[4] >= 0 && pars[4] <= 7);

	_vm->_resman->closeResource(params[2]);
	return fnStandAt(pars);
}

/**
 * Work out direction from start to dest.
 */

// Used in whatTarget(); not valid for all megas
#define	diagonalx 36
#define	diagonaly 8

int Logic::whatTarget(int startX, int startY, int destX, int destY) {
	int deltaX = destX - startX;
	int deltaY = destY - startY;

	// 7 0 1
	// 6   2
	// 5 4 3

	// Flat route

	if (ABS(deltaY) * diagonalx < ABS(deltaX) * diagonaly / 2)
		return (deltaX > 0) ? 2 : 6;

	// Vertical route

	if (ABS(deltaY) * diagonalx / 2 > ABS(deltaX) * diagonaly)
		return (deltaY > 0) ? 4 : 0;

	// Diagonal route

	if (deltaX > 0)
		return (deltaY > 0) ? 3 : 1;

	return (deltaY > 0) ? 5 : 7;
}

/**
 * Turn mega to face point (x,y) on the floor
 * Just needs to call fnWalk() with current feet coords & direction computed
 * by whatTarget()
 */

int32 Logic::fnFaceXY(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the turn, get the mega's current feet
	// coords + the required direction

	if (!ob_logic->looping) {
		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);
	
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = whatTarget(ob_mega->feet_x, ob_mega->feet_y, params[4], params[5]);
	}

	return fnWalk(pars);
}

int32 Logic::fnFaceMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the walk, decide where to walk to.

	if (!ob_logic->looping) {
		StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[4]);

		assert(head->fileType == GAME_OBJECT);

		// Call the base script. This is the graphic/mouse service
		// call, and will set _engineMega to the ObjectMega of mega we
		// want to turn to face.

		char *raw_script_ad = (char *) head;
		uint32 null_pc = 3;

		runScript(raw_script_ad, raw_script_ad, &null_pc);

		_vm->_resman->closeResource(params[4]);

		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

		pars[3] = params[3];
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = whatTarget(ob_mega->feet_x, ob_mega->feet_y, _engineMega.feet_x, _engineMega.feet_y);
	}

	return fnWalk(pars);
}

/**
 * Route to the left or right hand side of target id, if possible.
 */

int32 Logic::fnWalkToTalkToMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face
	//		5 distance

	int32 pars[7];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	ObjectLogic *ob_logic = (ObjectLogic *) _vm->_memory->decodePtr(params[0]);

	// If this is the start of the walk, calculate the route.

	if (!ob_logic->looping)	{
		StandardHeader *head = (StandardHeader *) _vm->_resman->openResource(params[4]);

		assert(head->fileType == GAME_OBJECT);

		// Call the base script. This is the graphic/mouse service
		// call, and will set _engineMega to the ObjectMega of mega we
		// want to route to.

		char *raw_script_ad = (char *) head;
		uint32 null_pc = 3;

		runScript(raw_script_ad, raw_script_ad, &null_pc);

		_vm->_resman->closeResource(params[4]);

		// Stand exactly beside the mega, ie. at same y-coord
		pars[5] = _engineMega.feet_y;

		ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[2]);

		// Apply scale factor to walk distance. Ay+B gives 256 * scale
		// ie. 256 * 256 * true_scale for even better accuracy, ie.
		// scale = (Ay + B) / 256

		int scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b) / 256;
		int mega_separation = (params[5] * scale) / 256;

		debug(4, "Target is at (%d, %d), separation %d", _engineMega.feet_x, _engineMega.feet_y, mega_separation);

		if (_engineMega.feet_x < ob_mega->feet_x) {
			// Target is left of us, so aim to stand to their
			// right. Face down_left

			pars[4] = _engineMega.feet_x + mega_separation;
			pars[6] = 5;
		} else {
			// Ok, must be right of us so aim to stand to their
			// left. Face down_right.

			pars[4] = _engineMega.feet_x - mega_separation;
			pars[6] = 3;
		}
	}

	return fnWalk(pars);
}

int32 Logic::fnSetWalkGrid(int32 *params) {
	// params:	none

	error("fnSetWalkGrid() is no longer a valid opcode");
	return IR_CONT;
}

/**
 * Add this walkgrid resource to the list of those used for routing in this
 * location. Note that this is ignored if the resource is already in the list.
 */

int32 Logic::fnAddWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	// All objects that add walkgrids must be restarted whenever we
	// re-enter a location.

	// DON'T EVER KILL GEORGE!
	if (_scriptVars[ID] != 8) {
		// Need to call this in case it wasn't called in script!
		fnAddToKillList(NULL);
	}

	_router->addWalkGrid(params[0]);
	fnPreLoad(params);
	return IR_CONT;
}

/**
 * Remove this walkgrid resource from the list of those used for routing in
 * this location. Note that this is ignored if the resource isn't actually
 * in the list.
 */

int32 Logic::fnRemoveWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	_router->removeWalkGrid(params[0]);
	return IR_CONT;
}

int32 Logic::fnRegisterWalkGrid(int32 *params) {
	// params:	none

	error("fnRegisterWalkGrid() is no longer a valid opcode");
	return IR_CONT;
}

int32 Logic::fnSetScaling(int32 *params) {
	// params:	0 pointer to object's mega structure
	//		1 scale constant A
	//		2 scale constant B

	// 256 * s = A * y + B

	// Where s is system scale, which itself is (256 * actual_scale) ie.
	// s == 128 is half size

 	ObjectMega *ob_mega = (ObjectMega *) _vm->_memory->decodePtr(params[0]);

	ob_mega->scale_a = params[1];
	ob_mega->scale_b = params[2];

	return IR_CONT;
}

/**
 * Set the standby walk coords to be used by fnWalkToAnim() and
 * fnStandAfterAnim() when the anim header's start/end coords are zero.
 * Useful during development; can stay in final game anyway.
 */

int32 Logic::fnSetStandbyCoords(int32 *params) {
	// params:	0 x-coord
	//		1 y-coord
	//		2 direction (0..7)

	assert(params[2] >= 0 && params[2] <= 7);

	_standbyX = (int16) params[0];
	_standbyY = (int16) params[1];
	_standbyDir = (uint8) params[2];

	return IR_CONT;
}

} // End of namespace Sword2
