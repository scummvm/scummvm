/* Copyright (C) 1994-2003 Revolution Software Ltd
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

// script functions for moving megas about the place & also for keeping tabs
// on them

#include "stdafx.h"
#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/defs.h"
#include "sword2/events.h"
#include "sword2/function.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/object.h"
#include "sword2/protocol.h"
#include "sword2/router.h"
#include "sword2/sync.h"

namespace Sword2 {

/**
 * Walk mega to (x,y,dir)
 */

int32 Logic::fnWalk(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord
	//		6 target direction

	Object_logic *ob_logic;
	Object_graphic *ob_graph;
	Object_mega *ob_mega;
	Object_walkdata *ob_walkdata;
	int16 target_x;
	int16 target_y;
	uint8 target_dir;
	int8 route;
	int32 walk_pc;
	_walkData *walkAnim;

	// get the parameters

	ob_logic = (Object_logic *) params[0];
	ob_graph = (Object_graphic *) params[1];
	ob_mega	 = (Object_mega *) params[2];

	target_x = (int16) params[4];
	target_y = (int16) params[5];
	target_dir = (uint8) params[6];

	// if this is the start of the walk, calculate route

	if (ob_logic->looping == 0) {
		// If we're already there, don't even bother allocating
		// memory and calling the router, just quit back & continue
		// the script! This avoids an embarassing mega stand frame
		// appearing for one cycle when we're already in position for
		// an anim eg. repeatedly clicking on same object to repeat
		// an anim - no mega frame will appear in between runs of the
		// anim.
		
		if (ob_mega->feet_x == target_x && ob_mega->feet_y == target_y && ob_mega->current_dir == target_dir) {
			RESULT = 0;	// 0 means ok - finished walk
			return IR_CONT;	// may as well continue the script
		}

		// invalid direction (NB. '8' means end walk on ANY direction)
		if (params[6] < 0 || params[6] > 8)
			error("Invalid direction (%d) in fnWalk", params[6]);

		ob_walkdata = (Object_walkdata *) params[3];

		ob_mega->walk_pc = 0;	// always

		// set up mem for _walkData in route_slots[] & set mega's
		// 'route_slot_id' accordingly

		_router->allocateRouteMem();

		route = (int8) _router->routeFinder(ob_mega, ob_walkdata, target_x, target_y, target_dir);

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
			// free up the walkdata mem block
			_router->freeRouteMem();

			// 1 means error, no walk created
			RESULT = 1;

			// may as well continue the script
			return IR_CONT;
		}

		// ok, walk is about to start, so set the mega's graphic
		// resource

		ob_graph->anim_resource = ob_mega->megaset_res;
	} else if (EXIT_FADING && g_display->getFadeStatus() == RDFADE_BLACK) {
		// double clicked an exit so quit the walk when screen is black

		// ok, thats it - back to script and change screen

		ob_logic->looping = 0;	// so script loop stops
		_router->freeRouteMem();	// free up the walkdata mem block

		// must clear in-case on the new screen there's a walk
		// instruction (which would get cut short)
		EXIT_CLICK_ID = 0;

		// this will be reset when we change screens, so we can use
		// it in script to check if a 2nd-click came along
		// EXIT_FADING = 0;

		// finished walk
		ob_mega->currently_walking = 0;

		// (see fnGetPlayerSaveData() in save_rest.cpp

		RESULT = 0;		// 0 means ok

		// continue the script so that RESULT can be checked!
		return IR_CONT;
	}

	// get pointer to walkanim & current frame position

	// lock the _walkData array
	walkAnim = _router->lockRouteMem();
	walk_pc = ob_mega->walk_pc;

	// if stopping the walk early, overwrite the next step with a
	// slow-out, then finish

	if (checkEventWaiting()) {
		if (walkAnim[walk_pc].step == 0 && walkAnim[walk_pc + 1].step == 1) {
			// at the beginning of a step
			ob_walkdata = (Object_walkdata *) params[3];
			_router->earlySlowOut(ob_mega, ob_walkdata);
		}
	}

	// get new frame of walk

	ob_graph->anim_pc = walkAnim[walk_pc].frame;
	ob_mega->current_dir = walkAnim[walk_pc].dir;
	ob_mega->feet_x = walkAnim[walk_pc].x;
	ob_mega->feet_y = walkAnim[walk_pc].y;

	// check if NEXT frame is in fact the end-marker of the walk sequence
	// so we can return to script just as the final (stand) frame of the
	// walk is set - so that if followed by an anim, the anim's first
	// frame replaces the final stand-frame of the walk (see below)

	// '512' is end-marker
	if (walkAnim[walk_pc + 1].frame == 512) {
		ob_logic->looping = 0;	// so script loop stops
		_router->freeRouteMem();	// free up the walkdata mem block

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
			RESULT = 1;		// 1 means didn't finish walk
			return IR_TERMINATE;
		} else {
			RESULT = 0;		// 0 means ok - finished walk

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

	// increment the walkanim frame number, float the walkanim & come
	// back next cycle

	ob_mega->walk_pc++;

	// allow _walkData array to float about memory again
	_router->floatRouteMem();

	// stop the script, but repeat this call next cycle
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

	Object_logic *ob_logic;
	uint8 *anim_file;
	_animHeader *anim_head;
	int32 pars[7];

	// if this is the start of the walk, read anim file to get start coords

	ob_logic = (Object_logic *) params[0];

	if (ob_logic->looping == 0) {
		// open anim file
		anim_file = res_man->openResource(params[4]);

		// point to animation header
		anim_head = _vm->fetchAnimHeader( anim_file );

		pars[4] = anim_head->feetStartX;	// target_x
		pars[5] = anim_head->feetStartY;	// target_y
		pars[6] = anim_head->feetStartDir;	// target_dir

		// close anim file
		res_man->closeResource(params[4]);

		// if start coords not yet set in anim header, use the standby
		// coords (which should be set beforehand in the script)

		if (pars[4] == 0 && pars[5] == 0) {
			pars[4] = _standbyX;
			pars[5] = _standbyY;
			pars[6] = _standbyDir;

			debug(5, "WARNING: fnWalkToAnim(%s) used standby coords", _vm->fetchObjectName(params[4]));
		}

		if (pars[6] < 0 || pars[6] > 7)
			error("Invalid direction (%d) in fnWalkToAnim", pars[6]);
	}

	// set up the rest of the parameters for fnWalk()

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	// walkdata (param 3) is needed for earlySlowOut if player clicks
	// elsewhere during the walk

	// call fnWalk() with target coords set to anim start position
	return fnWalk(pars);
}

/**
 * turn mega to <direction>
 * just needs to call fnWalk() with current feet coords, so router can
 * produce anim of turn frames
 */

int32 Logic::fnTurn(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target direction

	Object_logic *ob_logic;
	Object_mega *ob_mega;
	int32 pars[7];

	// if this is the start of the turn, get the mega's current feet
	// coords + the required direction

	ob_logic = (Object_logic *) params[0];

	if (ob_logic->looping == 0) {
		if (params[4] < 0 || params[4] > 7)
			error("Invalid direction (%d) in fnTurn", params[4]);

	 	ob_mega = (Object_mega *) params[2];
	
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = params[4];		// DIRECTION to turn to
	}

	// set up the rest of the parameters for fnWalk()

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	// call fnWalk() with target coords set to feet coords
	return fnWalk(pars);
}

/**
 * stand mega at (x,y,dir)
 * sets up the graphic object, but also needs to set the new 'current_dir' in
 * the mega object, so the router knows in future
 */

int32 Logic::fnStandAt(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target x-coord
	//		3 target y-coord
	//		4 target direction

	Object_mega *ob_mega;
	Object_graphic *ob_graph;

	// check for invalid direction

	if (params[4] < 0 || params[4] > 7)
		error("Invalid direction (%d) in fnStandAt", params[4]);

	// set up pointers to the graphic & mega structure

	ob_graph = (Object_graphic *) params[0];
	ob_mega = (Object_mega *) params[1];

	// set up the stand frame & set the mega's new direction

	// mega-set animation file
	ob_graph->anim_resource	= ob_mega->megaset_res;

	ob_mega->feet_x = params[2];
	ob_mega->feet_y = params[3];

	// dir + first stand frame (always frame 96)
	ob_graph->anim_pc = params[4] + 96;

	ob_mega->current_dir = params[4];

	return IR_CONT;
}

// stand mega in <direction> at current feet coords
// just needs to call fnStandAt() with current feet coords

int32 Logic::fnStand(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 target direction

	Object_mega *ob_mega = (Object_mega *) params[1];
	int32 pars[5];

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = ob_mega->feet_x;
	pars[3] = ob_mega->feet_y;
	pars[4] = params[2];		// DIRECTION to stand in

	// call fnStandAt() with target coords set to feet coords
	return fnStandAt(pars);
}

/**
 * stand mega at end position of anim
 */

int32 Logic::fnStandAfterAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	uint8 *anim_file;
	_animHeader *anim_head;
	int32 pars[5];

	// open the anim file & set up a pointer to the animation header

	// open anim file
	anim_file = res_man->openResource(params[2]);
	anim_head = _vm->fetchAnimHeader(anim_file);

	// set up the parameter list for fnWalkTo()

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetEndX;
	pars[3] = anim_head->feetEndY;
	pars[4] = anim_head->feetEndDir;

	// if start coords not available either use the standby coords (which
	// should be set beforehand in the script)

	if (pars[2] == 0 && pars[3] == 0) {
		pars[2] = _standbyX;
		pars[3] = _standbyY;
		pars[4] = _standbyDir;

		debug(5, "WARNING: fnStandAfterAnim(%s) used standby coords", _vm->fetchObjectName(params[2]));
	}

	if (pars[4] < 0 || pars[4] > 7)
		error("Invalid direction (%d) in fnStandAfterAnim", pars[4]);

	// close the anim file
	res_man->closeResource(params[2]);

	// call fnStandAt() with target coords set to anim end position
	return fnStandAt(pars);
}

// stand mega at start position of anim

int32 Logic::fnStandAtAnim(int32 *params) {
	// params:	0 pointer to object's graphic structure
	//		1 pointer to object's mega structure
	//		2 anim resource id

	uint8 *anim_file;
	_animHeader *anim_head;
	int32 pars[5];

	// open the anim file & set up a pointer to the animation header

	// open anim file
	anim_file = res_man->openResource(params[2]);
	anim_head = _vm->fetchAnimHeader(anim_file);

	// set up the parameter list for fnWalkTo()

	pars[0] = params[0];
	pars[1] = params[1];

	pars[2] = anim_head->feetStartX;
	pars[3] = anim_head->feetStartY;
	pars[4] = anim_head->feetStartDir;

	// if start coords not available use the standby coords (which should
	// be set beforehand in the script)

	if (pars[2] == 0 && pars[3] == 0) {
		pars[2] = _standbyX;
		pars[3] = _standbyY;
		pars[4] = _standbyDir;

		debug(5, "WARNING: fnStandAtAnim(%s) used standby coords", _vm->fetchObjectName(params[2]));
	}

	if (pars[4] < 0 || pars[4] > 7)
		error("Invalid direction (%d) in fnStandAfterAnim", pars[4]);

	// close the anim file
	res_man->closeResource(params[2]);

	// call fnStandAt() with target coords set to anim end position
	return fnStandAt(pars);
}

// Code to workout direction from start to dest

// used in what_target not valid for all megas
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
 * turn mega to face point (x,y) on the floor
 * just needs to call fnWalk() with current feet coords & direction computed
 * by whatTarget()
 */

int32 Logic::fnFaceXY(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 target x-coord
	//		5 target y-coord

	Object_logic *ob_logic;
	Object_mega *ob_mega;
 	int32 pars[7];

	// if this is the start of the turn, get the mega's current feet
	// coords + the required direction

	ob_logic = (Object_logic *) params[0];

	if (ob_logic->looping == 0) {
	 	ob_mega = (Object_mega *) params[2];
	
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = whatTarget(ob_mega->feet_x, ob_mega->feet_y, params[4], params[5]);
	}

	// set up the rest of the parameters for fnWalk()

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	// call fnWalk() with target coords set to feet coords
	return fnWalk(pars);
}

int32 Logic::fnFaceMega(int32 *params) {
	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face

	uint32 null_pc = 3;	// get ob_mega
	char *raw_script_ad;
	int32 pars[7];
	Object_logic *ob_logic;
	Object_mega *ob_mega;
	_standardHeader	*head;

	ob_mega = (Object_mega *) params[2];
	ob_logic = (Object_logic *) params[0];

	if (ob_logic->looping == 0) {
		// get targets info
		head = (_standardHeader*) res_man->openResource(params[4]);

		if (head->fileType != GAME_OBJECT)
			error("fnFaceMega %d not an object", params[4]);

		raw_script_ad = (char *) head;

		//call the base script - this is the graphic/mouse service call
		runScript(raw_script_ad, raw_script_ad, &null_pc);

		res_man->closeResource(params[4]);

		// engineMega is now the Object_mega of mega we want to turn
		// to face

		pars[3] = params[3];
		pars[4] = ob_mega->feet_x;
		pars[5] = ob_mega->feet_y;
		pars[6] = whatTarget(ob_mega->feet_x, ob_mega->feet_y, _vm->_engineMega.feet_x, _vm->_engineMega.feet_y);
	}

	pars[0] = params[0];
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];

	// call fnWalk() with target coords set to feet coords
	return fnWalk(pars);
}

int32 Logic::fnWalkToTalkToMega(int32 *params) {
	// we route to left or right hand side of target id if possible
	// target is a shrinking mega

	// params:	0 pointer to object's logic structure
	//		1 pointer to object's graphic structure
	//		2 pointer to object's mega structure
	//		3 pointer to object's walkdata structure
	//		4 id of target mega to face
	//		5 distance

	Object_mega *ob_mega;
	Object_logic *ob_logic;

	uint32 null_pc = 3;	// 4th script - get mega
	char *raw_script_ad;
	int32 pars[7];
	int scale;
	int mega_separation = params[5];
	_standardHeader	*head;

	ob_logic = (Object_logic*) params[0];
	ob_mega = (Object_mega*) params[2];

	pars[0] = params[0];	// standard stuff
	pars[1] = params[1];
	pars[2] = params[2];
	pars[3] = params[3];	// walkdata

	// not been here before so decide where to walk-to
	if (!ob_logic->looping)	{
		// first request the targets info
		head = (_standardHeader*) res_man->openResource(params[4]);

		if (head->fileType != GAME_OBJECT)
			error("fnWalkToTalkToMega %d not an object", params[4]);

		raw_script_ad = (char *) head;

		// call the base script - this is the graphic/mouse service
		// call
		runScript(raw_script_ad, raw_script_ad, &null_pc);

		res_man->closeResource(params[4]);

		// engineMega is now the Object_mega of mega we want to
		// route to

		// stand exactly beside the mega, ie. at same y-coord
		pars[5] = _vm->_engineMega.feet_y;

		// apply scale factor to walk distance
		// Ay+B gives 256 * scale ie. 256 * 256 * true_scale for even
		// better accuracy, ie. scale = (Ay + B) / 256

		scale = (ob_mega->scale_a * ob_mega->feet_y + ob_mega->scale_b) / 256;

		mega_separation= (mega_separation * scale) / 256;

		debug(5, "separation %d", mega_separation);
		debug(5, " target x %d, y %d", _vm->_engineMega.feet_x, _vm->_engineMega.feet_y);

		if (_vm->_engineMega.feet_x < ob_mega->feet_x)
		{
			// Target is left of us, so aim to stand to their
			// right. Face down_left

			pars[4] = _vm->_engineMega.feet_x + mega_separation;
			pars[6] = 5;
		} else {
			// Ok, must be right of us so aim to stand to their
			// left. Face down_right.

			pars[4] = _vm->_engineMega.feet_x - mega_separation;
			pars[6] = 3;
		}
	}

  	// first cycle builds the route - thereafter merely follows it

	// Call fnWalk() with target coords set to feet coords. RESULT will
	// be 1 when it finishes, or 0 if it failed to build route.
	return fnWalk(pars);
}

int32 Logic::fnSetWalkGrid(int32 *params) {
	// params:	none

	error("fnSetWalkGrid no longer valid");
	return IR_CONT;
}

// add this walkgrid resource to the list of those used for routing in this
// location - note this is ignored in the resource is already in the list

int32 Logic::fnAddWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	// all objects that add walkgrids must be restarted whenever we
	// re-enter a location

	// DON'T EVER KILL GEORGE!
	if (ID != 8) {
		// need to call this in case it wasn't called in script!
		// ('params' just used as dummy param)
		fnAddToKillList(params);
	}

	_router->addWalkGrid(params[0]);

	// Touch the grid, getting it into memory.
	res_man->openResource(params[0]);
	res_man->closeResource(params[0]);

	return IR_CONT;
}

// remove this walkgrid resource from the list of those used for routing in
// this location - note that this is ignored if the resource isn't actually
// in the list

int32 Logic::fnRemoveWalkGrid(int32 *params) {
	// params:	0 id of walkgrid resource

	_router->removeWalkGrid(params[0]);
	return IR_CONT;
}

int32 Logic::fnRegisterWalkGrid(int32 *params) {
	// params:	none

	error("fnRegisterWalkGrid no longer valid");
	return IR_CONT;
}

int32 Logic::fnSetScaling(int32 *params) {
	// params:	0 pointer to object's mega structure
	//		1 scale constant A
	//		2 scale constant B

	// 256 * s = A * y + B

	// where s is system scale, which itself is (256 * actual_scale) ie.
	// s == 128 is half size

 	Object_mega *ob_mega = (Object_mega *) params[0];

	ob_mega->scale_a = params[1];
	ob_mega->scale_b = params[2];

	return IR_CONT;
}

int32 Logic::fnSetStandbyCoords(int32 *params) {
	// set the standby walk coords to be used by fnWalkToAnim and
	// fnStandAfterAnim when the anim header's start/end coords are zero

	// useful during development; can stay in final game anyway

	// params:	0 x-coord
	//		1 y-coord
	//		2 direction (0..7)

	if (params[2] < 0 || params[2] > 7)
		error("Invalid direction (%d) in fnSetStandbyCoords", params[2]);

	_standbyX = (int16) params[0];
	_standbyY = (int16) params[1];
	_standbyDir = (uint8) params[2];

	return IR_CONT;
}

} // End of namespace Sword2
