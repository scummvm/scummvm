/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Scripting module script function component

#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/console.h"
#include "saga/interface.h"
#include "saga/music.h"

#include "saga/script.h"
#include "saga/sdata.h"

namespace Saga {

#define OPCODE(x) &Script::x

void Script::setupScriptFuncList(void) {
	static const R_SFUNC_ENTRY SFuncList[R_SFUNC_NUM] = {
		{0, 0, NULL},
		{1, 1, OPCODE(SF_sleep)},
		{2, 0, NULL},
		{3, 1, OPCODE(SF_3)},
		{4, 1, OPCODE(SF_setCommandText)},
		{5, 0, NULL},
		{6, 3, OPCODE(SF_actorWalkTo)},
		{7, 0, OPCODE(SF_doAction)},
		{8, 2, OPCODE(SF_setFacing)},
		{9, 0, NULL},
		{10, 0, NULL},
		{11, 1, OPCODE(SF_freezeInterface)},
		{12, 0, NULL},
		{13, 0, NULL},
		{14, 0, OPCODE(SF_faceTowards)},
		{15, 0, OPCODE(SF_setFollower)},
		{16, 0, NULL},
		{17, 0, NULL},
		{18, 0, NULL},
		{19, 0, NULL},
		{20, 0, NULL},
		{21, 0, NULL},
		{22, 0, NULL},
		{23, 0, NULL},
		{24, 0, NULL},
		{25, 0, OPCODE(SF_centerActor)},
		{26, 3, OPCODE(SF_startAnim)},
		{27, 3, OPCODE(SF_actorWalkToAsync)},
		{28, 0, NULL},
		{29, 0, OPCODE(SF_setActorState)},
		{30, 3, OPCODE(SF_moveTo)},
		{31, 0, NULL},
		{32, 0, NULL},
		{33, 0, NULL},
		{34, 0, OPCODE(SF_swapActors)},
		{35, 0, NULL},
		{36, 4, OPCODE(SF_actorWalk)},
		{37, 4, OPCODE(SF_cycleActorFrames)},
		{38, 3, OPCODE(SF_setFrame)},
		{39, 0, NULL},
		{40, 0, NULL},
		{41, 4, OPCODE(SF_linkAnim)},
		{42, 0, OPCODE(SF_scriptSpecialWalk)},
		{43, 6, OPCODE(SF_placeActor)},
		{44, 0, OPCODE(SF_checkUserInterrupt)},
		{45, 0, OPCODE(SF_walkRelative)},
		{46, 0, OPCODE(SF_moveRelative)},
		{47, 0, NULL},
		{48, 0, NULL},
		{49, 0, NULL},
		{50, 0, NULL},
		{51, 0, NULL},
		{52, 0, OPCODE(SF_throwActor)},
		{53, 0, OPCODE(SF_waitWalk)},
		{54, 0, NULL},
		{55, 0, OPCODE(SF_changeActorScene)},
		{56, 0, OPCODE(SF_climb)},
		{57, 0, NULL},
		{58, 0, OPCODE(SF_setActorZ)},
		{59, 0, NULL},
		{60, 0, OPCODE(SF_getActorX)},
		{61, 0, OPCODE(SF_getActorY)},
		{62, 0, NULL},
		{63, 0, NULL},
		{64, 0, NULL},
		{65, 0, NULL},
		{66, 0, NULL},
		{67, 0, NULL},
		{68, 0, NULL},
		{69, 0, NULL},
		{70, 0, NULL},
		{71, 0, NULL},
		{72, 0, NULL},
		{73, 0, NULL},
		{74, 0, NULL},
		{75, 0, NULL},
		{76, 0, NULL},
		{77, 0, NULL}
	};
	_SFuncList = SFuncList;
}

// Script function #1 (0x01) blocking
// Suspends thread execution for the specified time period
// Param1: time to suspend ( units? )
int Script::SF_sleep(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T time_param;
	int time;

	time_param = thread->pop();
	time = _vm->_sdata->readWordU(time_param);
	thread->sleep_time = time * 10;
	return R_SUCCESS;
}

// Script function #3 (0x03)
// Unknown function; pops a parameter and pushes a return value
// Param1: unknown
int Script::SF_3(R_SCRIPTFUNC_PARAMS) {
	// INCOMPLETE
	SDataWord_T param1;
	param1 = thread->pop();
	thread->push(0);	// push for now to allow intro faire 
									// setup to run completely

	return R_SUCCESS;
}

// Script function #4 (0x04) nonblocking
// Set the command display to the specified text string
// Param1: dialogue index of string
int Script::SF_setCommandText(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T s_idx_parm;

	s_idx_parm = thread->pop();
	// INCOMPLETE

	return R_SUCCESS;
}

// Script function #6 (0x06) blocking
// Commands the specified actor to walk to the given position
// Param1: actor id
// Param2: actor destination x
// Param3: actor destination y
int Script::SF_actorWalkTo(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	int actor_id;
	int actor_idx;
	R_POINT pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.", actor_id);
		return R_FAILURE;
	}

	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);

	_vm->_actor->walkTo(actor_idx, &pt, 0, &thread->sem);

	return R_SUCCESS;
}

// Script function #7
int Script::SF_doAction(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #8 (0x08) nonblocking
// Sets the orientation of the specified actor.
// Param1: actor id
// Param2: actor orientation
int Script::SF_setFacing(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T orient_parm;
	int actor_id;
	int actor_idx;
	int orientation;

	actor_parm = thread->pop();
	orient_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	orientation = _vm->_sdata->readWordS(orient_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.", actor_id);
		return R_FAILURE;
	}

	_vm->_actor->setOrientation(actor_idx, orientation);
	return R_SUCCESS;
}

// Script function #11 (0x0B) nonblocking
// If the parameter is true, the user interface is disabled while script
// continues to run. If the parameter is false, the user interface is 
// reenabled.
// Param1: boolean
int Script::SF_freezeInterface(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T b_param;

	b_param = thread->pop();

	if (b_param) {
		_vm->_interface->deactivate();
	} else {
		_vm->_interface->activate();
	}

	return R_SUCCESS;
}

// Script function #14
int Script::SF_faceTowards(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #15
int Script::SF_setFollower(R_SCRIPTFUNC_PARAMS) {
	thread->pop();
	thread->pop();
	return R_SUCCESS;
}

// Script function #25
int Script::SF_centerActor(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #26 (0x1A) nonblocking
// Starts the specified animation 
// Param1: ?
// Param2: frames of animation to play or -1 to loop
// Param3: animation id
int Script::SF_startAnim(R_SCRIPTFUNC_PARAMS) {
// FIXME: implementation is wrong. Should link animation
	SDataWord_T unk_parm;
	SDataWord_T frame_parm;
	SDataWord_T anim_id_parm;
	int frame_count;
	int anim_id;

	anim_id_parm = thread->pop();
	frame_parm = thread->pop();
	unk_parm = thread->pop();

	frame_count = _vm->_sdata->readWordS(frame_parm);
	anim_id = _vm->_sdata->readWordS(anim_id_parm);

	if (_vm->_anim->play(anim_id, 0) != R_SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.26: Anim::play() failed. Anim id: %u\n", anim_id);
		return R_FAILURE;
	}

	return R_SUCCESS;
}

// Script function #27 (0x1B) nonblocking
// Commands the specified actor to walk to the given position
// Param1: actor id
// Param2: actor destination x
// Param3: actor destination y
int Script::SF_actorWalkToAsync(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	int actor_id;
	int actor_idx;
	R_POINT pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.",
		    actor_id);
		return R_FAILURE;
	}

	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);
	_vm->_actor->walkTo(actor_idx, &pt, 0, NULL);

	return R_SUCCESS;
}

// Script function #29
int Script::SF_setActorState(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #30 (0x1E) nonblocking
// Positions an actor at the specified location; actor is created if the 
// actor does not already exist.
// Param1: actor id
// Param2: actor pos x
// Param3: actor pos y
int Script::SF_moveTo(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	int actor_id;
	int actor_idx;
	int result;
	R_POINT pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);

	if (!_vm->_actor->actorExists(actor_id)) {
		result = _vm->_actor->create(actor_id, pt.x, pt.y);
		if (result != R_SUCCESS) {
			_vm->_console->print(S_WARN_PREFIX "SF.30: Couldn't create actor 0x%X.", actor_id);
			return R_FAILURE;
		}
	} else {
		actor_idx = _vm->_actor->getActorIndex(actor_id);
		_vm->_actor->move(actor_idx, &pt);
	}

	return R_SUCCESS;
}

// Script function #34
int Script::SF_swapActors(R_SCRIPTFUNC_PARAMS) {
	thread->pop();
	thread->pop();
	return R_SUCCESS;
}

// Script function #36 (0x24) ?
// Commands the specified actor to walk to the given position
// Param1: actor id
// Param2: actor destination x
// Param3: actor destination y
// Param4: unknown
int Script::SF_actorWalk(R_SCRIPTFUNC_PARAMS) {
	// INCOMPLETE
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	SDataWord_T unk_parm;
	int actor_idx;
	R_POINT pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();
	unk_parm = thread->pop();

	actor_idx = _vm->_actor->getActorIndex(_vm->_sdata->readWordS(actor_parm));
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.36: Actor id 0x%X not found.", (int)actor_parm);
		return R_FAILURE;
	}

	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);

#if 1
	_vm->_actor->walkTo(actor_idx, &pt, 0, NULL);
#else
	_vm->_actor->walkTo(actor_idx, &pt, 0, &thread->sem);
#endif

	return R_SUCCESS;
}

// Script function #37 (0x25) nonblocking
// Sets an actor to the specified action state
// Param1: actor id
// Param2: unknown
// Param3: actor action state
// Param4: unknown
int Script::SF_cycleActorFrames(R_SCRIPTFUNC_PARAMS) {
	// INCOMPLETE
	SDataWord_T actor_parm;
	SDataWord_T unk1_parm;
	SDataWord_T unk2_parm;
	SDataWord_T action_parm;
	int actor_id;
	int actor_idx;
	int action;
	//uint16 flags;

	actor_parm = thread->pop();
	unk1_parm = thread->pop();
	action_parm = thread->pop();
	unk2_parm = thread->pop();
	actor_id = _vm->_sdata->readWordS(actor_parm);
	action = _vm->_sdata->readWordS(action_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);

	if (_vm->_actor->setAction(actor_idx, action, ACTION_NONE) != R_SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.37: Actor::setAction() failed.");
		return R_FAILURE;
	}

	return R_SUCCESS;
}

// Script function #38 (0x26) nonblocking
// Sets an actor to the specified action state
// Param1: actor id
// Param2: actor action state
// Param3: unknown
int Script::SF_setFrame(R_SCRIPTFUNC_PARAMS) {
	// INCOMPLETE

	SDataWord_T actor_parm;
	SDataWord_T unk1_parm;
	SDataWord_T action_parm;

	int actor_id;
	int actor_idx;
	int action;
	//uint16 flags;

	actor_parm = thread->pop();
	action_parm = thread->pop();
	unk1_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	action = _vm->_sdata->readWordS(action_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);

	if (_vm->_actor->setAction(actor_idx, action, ACTION_NONE) != R_SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.38: Actor::setAction() failed.");
		return R_FAILURE;
	}

	return R_SUCCESS;
}

// Script function #41 (0x29) nonblocking
// Links the specified animations for playback

// Param1: ?
// Param2: total linked frame count
// Param3: animation id link target
// Param4: animation id link source
int Script::SF_linkAnim(R_SCRIPTFUNC_PARAMS) {
	SDataWord_T unk_parm;
	SDataWord_T tframes_parm;
	SDataWord_T anim1_parm;
	SDataWord_T anim2_parm;
	int tframes;
	uint16 anim_id1;
	uint16 anim_id2;

	anim1_parm = thread->pop();
	anim2_parm = thread->pop();
	tframes_parm = thread->pop();
	unk_parm = thread->pop();
	tframes = _vm->_sdata->readWordS(tframes_parm);
	anim_id1 = _vm->_sdata->readWordU(anim1_parm);
	anim_id2 = _vm->_sdata->readWordU(anim2_parm);

	if (_vm->_anim->link(anim_id1, anim_id2) != R_SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.41: Anim::link() failed. (%u->%u)\n", anim_id1, anim_id2);
		return R_FAILURE;
	}

	return R_SUCCESS;
}

// Script function #42
int Script::SF_scriptSpecialWalk(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #43 (0x2B) nonblocking
// Positions an actor at the specified location; actor is created if the 
//  actor does not already exist.
// Param1: actor id
// Param2: actor pos x
// Param3: actor pos y
// Param4: ?
// Param5: actor action
// Param6: ?
int Script::SF_placeActor(R_SCRIPTFUNC_PARAMS) {
	// INCOMPLETE
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	SDataWord_T action_parm;
	SDataWord_T unknown_parm;
	int actor_id;
	int actor_idx;
	int action_state;
	int result;
	R_POINT pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();
	unknown_parm = thread->pop();
	action_parm = thread->pop();
	unknown_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);
	action_state = _vm->_sdata->readWordU(action_parm);

	if (!_vm->_actor->actorExists(actor_id)) {
		result = _vm->_actor->create(actor_id, pt.x, pt.y);
		if (result != R_SUCCESS) {
			_vm->_console->print(S_WARN_PREFIX "SF.43: Couldn't create actor 0x%X.", actor_id);
			return R_FAILURE;
		}
	} else {
		actor_idx = _vm->_actor->getActorIndex(actor_id);
		_vm->_actor->move(actor_idx, &pt);
	}

	actor_idx = _vm->_actor->getActorIndex(actor_id);
	_vm->_actor->setDefaultAction(actor_idx, action_state, ACTION_NONE);
	_vm->_actor->setAction(actor_idx, action_state, ACTION_NONE);

	return R_SUCCESS;
}

// Script function #44 (0x2C) nonblocking
// Checks to see if the user has interrupted a currently playing 
// game cinematic. Pushes a zero or positive value if the game 
// has not been interrupted.
int Script::SF_checkUserInterrupt(R_SCRIPTFUNC_PARAMS) {
	thread->push(0);

	// INCOMPLETE

	return R_SUCCESS;
}

// Script function #45
int Script::SF_walkRelative(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #46
int Script::SF_moveRelative(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #52
int Script::SF_throwActor(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #53
int Script::SF_waitWalk(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #55
int Script::SF_changeActorScene(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #56
int Script::SF_climb(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #58
int Script::SF_setActorZ(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #60
int Script::SF_getActorX(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

// Script function #61
int Script::SF_getActorY(R_SCRIPTFUNC_PARAMS) {
	return R_SUCCESS;
}

} // End of namespace Saga
