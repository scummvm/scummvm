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
#include "saga/sound.h"
#include "saga/sndres.h"

#include "saga/script.h"
#include "saga/sdata.h"

#include "saga/scene.h"

namespace Saga {

#define OPCODE(x) &Script::x

void Script::setupScriptFuncList(void) {
	static const SFUNC_ENTRY SFuncList[SFUNC_NUM] = {
		{0, 0, NULL},
		{1, 1, OPCODE(SF_sleep)},
		{2, 1, OPCODE(SF_takeObject)},
		{3, 1, OPCODE(SF_objectIsCarried)},
		{4, 1, OPCODE(SF_setStatusText)},
		{5, 0, OPCODE(SF_commandMode)},
		{6, 3, OPCODE(SF_actorWalkTo)},
		{7, 4, OPCODE(SF_doAction)},
		{8, 2, OPCODE(SF_setFacing)},
		{9, 2, OPCODE(SF_startBgdAnim)},
		{10, 1, OPCODE(SF_stopBgdAnim)},
		{11, 1, OPCODE(SF_freezeInterface)},
		{12, 0, OPCODE(SF_dialogMode)},
		{13, 0, NULL},
		{14, 2, OPCODE(SF_faceTowards)},
		{15, 2, OPCODE(SF_setFollower)},
		{16, 2, OPCODE(SF_gotoScene)},
		{17, 0, NULL},
		{18, 0, NULL},
		{19, 0, NULL},
		{20, 0, NULL},
		{21, 0, NULL},
		{22, 0, NULL},
		{23, 2, OPCODE(SF_setBgdAnimSpeed)},
		{24, 0, NULL},
		{25, 1, OPCODE(SF_centerActor)},
		{26, 3, OPCODE(SF_startAnim)},
		{27, 3, OPCODE(SF_actorWalkToAsync)},
		{28, 0, NULL},
		{29, 2, OPCODE(SF_setActorState)},
		{30, 3, OPCODE(SF_moveTo)},
		{31, 1, OPCODE(SF_sceneEq)},
		{32, 0, NULL},
		{33, 1, OPCODE(SF_finishBgdAnim)},
		{34, 2, OPCODE(SF_swapActors)},
		{35, 0, NULL},
		{36, 4, OPCODE(SF_actorWalk)},
		{37, 4, OPCODE(SF_cycleActorFrames)},
		{38, 3, OPCODE(SF_setFrame)},
		{39, 1, OPCODE(SF_setRightPortrait)},
		{40, 1, OPCODE(SF_setLeftPortrait)},
		{41, 4, OPCODE(SF_linkAnim)},
		{42, 4, OPCODE(SF_scriptSpecialWalk)},
		{43, 6, OPCODE(SF_placeActor)},
		{44, 0, OPCODE(SF_checkUserInterrupt)},
		{45, 5, OPCODE(SF_walkRelative)},
		{46, 5, OPCODE(SF_moveRelative)},
		{47, 0, NULL},
		{48, 0, OPCODE(SF_placard)},
		{49, 0, OPCODE(SF_placardOff)},
		{50, 0, NULL},
		{51, 0, NULL},
		{52, 6, OPCODE(SF_throwActor)},
		{53, 1, OPCODE(SF_waitWalk)},
		{54, 0, NULL},
		{55, 2, OPCODE(SF_changeActorScene)},
		{56, 4, OPCODE(SF_climb)},
		{57, 0, NULL},
		{58, 2, OPCODE(SF_setActorZ)},
		{59, 0, NULL},
		{60, 1, OPCODE(SF_getActorX)},
		{61, 1, OPCODE(SF_getActorY)},
		{62, 0, NULL},
		{63, 1, OPCODE(SF_playMusic)},
		{64, 0, NULL},
		{65, 0, NULL},
		{66, 0, NULL},
		{67, 0, NULL},
		{68, 0, NULL},
		{69, 1, OPCODE(SF_enableEscape)},
		{70, 1, OPCODE(SF_playSound)},
		{71, 0, NULL},
		{72, 0, NULL},
		{73, 0, NULL},
		{74, 0, NULL},
		{75, 1, OPCODE(SF_rand)},
		{76, 0, OPCODE(SF_fadeMusic)},
		{77, 0, NULL}
	};
	_SFuncList = SFuncList;
}

// Script function #1 (0x01) blocking
// Suspends thread execution for the specified time period
int Script::SF_sleep(SCRIPTFUNC_PARAMS) {
	SDataWord_T time_param;
	long time;

	if (!_skipSpeeches) {
		time_param = thread->pop();
		time = _vm->_sdata->readWordU(time_param);
		time = time * 10;  // 72.8 ticks per second
		thread->flags |= kTFlagWaiting;	// put thread to sleep
		thread->waitType = kTWaitDelay;
	}
	return SUCCESS;
}

// Script function #2 (0x02)
int Script::SF_takeObject(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_takeObject(%d)", param);
	return SUCCESS;
}

// Script function #3 (0x03)
// Unknown function; pops a parameter and pushes a return value
// Param1: unknown
int Script::SF_objectIsCarried(SCRIPTFUNC_PARAMS) {
	// INCOMPLETE
	SDataWord_T param1;
	param1 = thread->pop();

	// FIXME: Incomplete, but returning 0 assures that the fair start
	// script will run completely.

	thread->retVal = 0;

	return SUCCESS;
}

// Script function #4 (0x04) nonblocking
// Set the command display to the specified text string
// Param1: dialogue index of string
int Script::SF_setStatusText(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	return _vm->_interface->setStatusText(currentScript()->diag->str[param]);
}

// Script function #5 (0x05)
int Script::SF_commandMode(SCRIPTFUNC_PARAMS) {
	return _vm->_interface->setMode(kPanelCommand);
}

// Script function #6 (0x06) blocking
// Commands the specified actor to walk to the given position
// Param1: actor id
// Param2: actor destination x
// Param3: actor destination y
int Script::SF_actorWalkTo(SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	int actor_id;
	int actor_idx;
	Point pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.", actor_id);
		return FAILURE;
	}

	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);

	_vm->_actor->walkTo(actor_idx, &pt, 0, &thread->sem);

	return SUCCESS;
}

// Script function #7 (0x07)
int Script::SF_doAction(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();
	SDataWord_T param3 = thread->pop();
	SDataWord_T param4 = thread->pop();

	debug(1, "stub: SF_doAction(%d, %d, %d, %d)", param1, param2, param3, param4);
	return SUCCESS;
}

// Script function #8 (0x08) nonblocking
// Sets the orientation of the specified actor.
// Param1: actor id
// Param2: actor orientation
int Script::SF_setFacing(SCRIPTFUNC_PARAMS) {
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
		return FAILURE;
	}

	_vm->_actor->setOrientation(actor_idx, orientation);
	return SUCCESS;
}

// Script function #9 (0x09)
int Script::SF_startBgdAnim(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_startBgdAnim(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #10 (0x0A)
int Script::SF_stopBgdAnim(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_stopBgdAnim(%d)", param);
	return SUCCESS;
}

// Script function #11 (0x0B) nonblocking
// If the parameter is true, the user interface is disabled while script
// continues to run. If the parameter is false, the user interface is 
// reenabled.
// Param1: boolean
int Script::SF_freezeInterface(SCRIPTFUNC_PARAMS) {
	SDataWord_T b_param;

	b_param = thread->pop();

	if (b_param) {
		_vm->_interface->deactivate();
	} else {
		_vm->_interface->activate();
	}

	return SUCCESS;
}

// Script function #12 (0x0C)
// Disables mouse input, etc.
int Script::SF_dialogMode(SCRIPTFUNC_PARAMS) {
	return _vm->_interface->setMode(kPanelDialogue);
}

// Script function #14 (0x0E)
int Script::SF_faceTowards(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_faceTowards(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #15 (0x0F)
int Script::SF_setFollower(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_setFollower(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #16 (0x10)
int Script::SF_gotoScene(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_gotoScene(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #23 (0x17)
int Script::SF_setBgdAnimSpeed(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_setBgdAnimSpeed(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #25 (0x19)
int Script::SF_centerActor(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_centerActor(%d)", param);
	return SUCCESS;
}

// Script function #26 (0x1A) nonblocking
// Starts the specified animation 
// Param1: ?
// Param2: frames of animation to play or -1 to loop
// Param3: animation id
int Script::SF_startAnim(SCRIPTFUNC_PARAMS) {
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

	if (_vm->_anim->play(anim_id, 0) != SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.26: Anim::play() failed. Anim id: %u\n", anim_id);
		return FAILURE;
	}

	return SUCCESS;
}

// Script function #27 (0x1B) nonblocking
// Commands the specified actor to walk to the given position
// Param1: actor id
// Param2: actor destination x
// Param3: actor destination y
int Script::SF_actorWalkToAsync(SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	int actor_id;
	int actor_idx;
	Point pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	actor_idx = _vm->_actor->getActorIndex(actor_id);
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.08: Actor id 0x%X not found.",
		    actor_id);
		return FAILURE;
	}

	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);
	_vm->_actor->walkTo(actor_idx, &pt, 0, NULL);

	return SUCCESS;
}

// Script function #29 (0x1D)
int Script::SF_setActorState(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_setActorState(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #30 (0x1E) nonblocking
// Positions an actor at the specified location; actor is created if the 
// actor does not already exist.
// Param1: actor id
// Param2: actor pos x
// Param3: actor pos y
int Script::SF_moveTo(SCRIPTFUNC_PARAMS) {
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	int actor_id;
	int actor_idx;
	int result;
	Point pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();

	actor_id = _vm->_sdata->readWordS(actor_parm);
	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);

	if (!_vm->_actor->actorExists(actor_id)) {
		result = _vm->_actor->create(actor_id, pt.x, pt.y);
		if (result != SUCCESS) {
			_vm->_console->print(S_WARN_PREFIX "SF.30: Couldn't create actor 0x%X.", actor_id);
			return FAILURE;
		}
	} else {
		actor_idx = _vm->_actor->getActorIndex(actor_id);
		_vm->_actor->move(actor_idx, &pt);
	}

	return SUCCESS;
}

// Script function #31 (0x21)
int Script::SF_sceneEq(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	if (_vm->_scene->getSceneLUT(param) == _vm->_scene->currentSceneNumber())
		thread->retVal = 1;
	else 
		thread->retVal = 0;
	return SUCCESS;
}


// Script function #33 (0x21)
int Script::SF_finishBgdAnim(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_finishBgdAnim(%d)", param);
	return SUCCESS;
}

// Script function #34 (0x22)
int Script::SF_swapActors(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_swapActors(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #36 (0x24) ?
// Commands the specified actor to walk to the given position
// Param1: actor id
// Param2: actor destination x
// Param3: actor destination y
// Param4: unknown
int Script::SF_actorWalk(SCRIPTFUNC_PARAMS) {
	// INCOMPLETE
	SDataWord_T actor_parm;
	SDataWord_T x_parm;
	SDataWord_T y_parm;
	SDataWord_T unk_parm;
	int actor_idx;
	Point pt;

	actor_parm = thread->pop();
	x_parm = thread->pop();
	y_parm = thread->pop();
	unk_parm = thread->pop();

	actor_idx = _vm->_actor->getActorIndex(_vm->_sdata->readWordS(actor_parm));
	if (actor_idx < 0) {
		_vm->_console->print(S_WARN_PREFIX "SF.36: Actor id 0x%X not found.", (int)actor_parm);
		return FAILURE;
	}

	pt.x = _vm->_sdata->readWordS(x_parm);
	pt.y = _vm->_sdata->readWordS(y_parm);

#if 1
	_vm->_actor->walkTo(actor_idx, &pt, 0, NULL);
#else
	_vm->_actor->walkTo(actor_idx, &pt, 0, &thread->sem);
#endif

	return SUCCESS;
}

// Script function #37 (0x25) nonblocking
// Sets an actor to the specified action state
// Param1: actor id
// Param2: unknown
// Param3: actor action state
// Param4: unknown
int Script::SF_cycleActorFrames(SCRIPTFUNC_PARAMS) {
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

	if (_vm->_actor->setAction(actor_idx, action, ACTION_NONE) != SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.37: Actor::setAction() failed.");
		return FAILURE;
	}

	return SUCCESS;
}

// Script function #38 (0x26) nonblocking
// Sets an actor to the specified action state
// Param1: actor id
// Param2: actor action state
// Param3: unknown
int Script::SF_setFrame(SCRIPTFUNC_PARAMS) {
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

	if (_vm->_actor->setAction(actor_idx, action, ACTION_NONE) != SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.38: Actor::setAction() failed.");
		return FAILURE;
	}

	return SUCCESS;
}

// Script function #39 (0x27)
// Sets the right-hand portrait
int Script::SF_setRightPortrait(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	return _vm->_interface->setRightPortrait(param);
}

// Script function #40 (0x28)
// Sets the left-hand portrait
int Script::SF_setLeftPortrait(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	return _vm->_interface->setLeftPortrait(param);
}

// Script function #41 (0x29) nonblocking
// Links the specified animations for playback

// Param1: ?
// Param2: total linked frame count
// Param3: animation id link target
// Param4: animation id link source
int Script::SF_linkAnim(SCRIPTFUNC_PARAMS) {
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

	if (_vm->_anim->link(anim_id1, anim_id2) != SUCCESS) {
		_vm->_console->print(S_WARN_PREFIX "SF.41: Anim::link() failed. (%u->%u)\n", anim_id1, anim_id2);
		return FAILURE;
	}

	return SUCCESS;
}

// Script function #42 (0x2A)
int Script::SF_scriptSpecialWalk(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();
	SDataWord_T param3 = thread->pop();
	SDataWord_T param4 = thread->pop();

	debug(1, "stub: SF_scriptSpecialWalk(%d, %d, %d, %d)", param1, param2, param3, param4);
	return SUCCESS;
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
int Script::SF_placeActor(SCRIPTFUNC_PARAMS) {
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
	Point pt;

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
		if (result != SUCCESS) {
			_vm->_console->print(S_WARN_PREFIX "SF.43: Couldn't create actor 0x%X.", actor_id);
			return FAILURE;
		}
	} else {
		actor_idx = _vm->_actor->getActorIndex(actor_id);
		_vm->_actor->move(actor_idx, &pt);
	}

	actor_idx = _vm->_actor->getActorIndex(actor_id);
	_vm->_actor->setDefaultAction(actor_idx, action_state, ACTION_NONE);
	_vm->_actor->setAction(actor_idx, action_state, ACTION_NONE);

	return SUCCESS;
}

// Script function #44 (0x2C) nonblocking
// Checks to see if the user has interrupted a currently playing 
// game cinematic. Pushes a zero or positive value if the game 
// has not been interrupted.
int Script::SF_checkUserInterrupt(SCRIPTFUNC_PARAMS) {
	thread->retVal = (_skipSpeeches == true);

	return SUCCESS;
}

// Script function #45 (0x2D)
int Script::SF_walkRelative(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();
	SDataWord_T param3 = thread->pop();
	SDataWord_T param4 = thread->pop();
	SDataWord_T param5 = thread->pop();

	debug(1, "stub: SF_walkRelative(%d, %d, %d, %d, %d)", param1, param2, param3, param4, param5);
	return SUCCESS;
}

// Script function #46 (0x2E)
int Script::SF_moveRelative(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();
	SDataWord_T param3 = thread->pop();
	SDataWord_T param4 = thread->pop();
	SDataWord_T param5 = thread->pop();

	debug(1, "stub: SF_moveRelative(%d, %d, %d, %d, %d)", param1, param2, param3, param4, param5);
	return SUCCESS;
}

// Script function #48 (0x30)
int Script::SF_placard(SCRIPTFUNC_PARAMS) {
	debug(1, "stub: SF_placard()");
	return SUCCESS;
}

// Script function #49 (0x31)
int Script::SF_placardOff(SCRIPTFUNC_PARAMS) {
	debug(1, "stub: SF_placardOff()");
	return SUCCESS;
}

// Script function #52 (0x34)
int Script::SF_throwActor(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();
	SDataWord_T param3 = thread->pop();
	SDataWord_T param4 = thread->pop();
	SDataWord_T param5 = thread->pop();
	SDataWord_T param6 = thread->pop();

	debug(1, "stub: SF_throwActor(%d, %d, %d, %d, %d, %d)", param1, param2, param3, param4, param5, param6);
	return SUCCESS;
}

// Script function #53 (0x35)
int Script::SF_waitWalk(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_waitWalk(%d)", param);
	return SUCCESS;
}

// Script function #55 (0x37)
int Script::SF_changeActorScene(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_changeActorScene(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #56 (0x38)
int Script::SF_climb(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();
	SDataWord_T param3 = thread->pop();
	SDataWord_T param4 = thread->pop();

	debug(1, "stub: SF_climb(%d, %d, %d, %d)", param1, param2, param3, param4);
	return SUCCESS;
}

// Script function #58 (0x3A)
int Script::SF_setActorZ(SCRIPTFUNC_PARAMS) {
	SDataWord_T param1 = thread->pop();
	SDataWord_T param2 = thread->pop();

	debug(1, "stub: SF_setActorZ(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #60 (0x3C)
int Script::SF_getActorX(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_getActorX(%d)", param);
	return SUCCESS;
}

// Script function #61 (0x3D)
int Script::SF_getActorY(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	debug(1, "stub: SF_getActorY(%d)", param);
	return SUCCESS;
}

// Script function #63 (0x3F)
int Script::SF_playMusic(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop() + 9;

	if (param >= 9 && param <= 34)
		_vm->_music->play(param);
	else
		_vm->_music->stop();

	return SUCCESS;
}

// Script function #69
int Script::SF_enableEscape(SCRIPTFUNC_PARAMS) {
	if (thread->pop())
		_abortEnabled = true;
	else {
		_skipSpeeches = false;
		_abortEnabled = false;
	}
	
	return SUCCESS;
}

static struct {
	int res;
	int vol;
} sfxTable[] = {
	{ FX_DOOR_OPEN,    127 },
	{ FX_DOOR_CLOSE,   127 },
	{ FX_RUSH_WATER,    63 }, // Floppy volume: 127
	{ FX_RUSH_WATER,    26 }, // Floppy volume: 40
	{ FX_CRICKET,       64 },
	{ FX_PORTICULLIS,   84 }, // Floppy volume: 127
	{ FX_CLOCK_1,       64 },
	{ FX_CLOCK_2,       64 },
	{ FX_DAM_MACHINE,   64 },
	{ FX_DAM_MACHINE,   40 },
	{ FX_HUM1,          64 },
	{ FX_HUM2,          64 },
	{ FX_HUM3,          64 },
	{ FX_HUM4,          64 },
	{ FX_WATER_LOOP_S,  32 }, // Floppy volume: 64
	{ FX_SURF,          42 }, // Floppy volume: 127
	{ FX_SURF,          32 }, // Floppy volume: 64
	{ FX_FIRELOOP,      64 }, // Floppy volume: 96
	{ FX_SCRAPING,      84 }, // Floppy volume: 127
	{ FX_BEE_SWARM,     64 }, // Floppy volume: 96
	{ FX_BEE_SWARM,     26 }, // Floppy volume: 40
	{ FX_SQUEAKBOARD,   64 },
	{ FX_KNOCK,        127 },
	{ FX_COINS,         32 }, // Floppy volume: 48
	{ FX_STORM,         84 }, // Floppy volume: 127
	{ FX_DOOR_CLOSE_2,  84 }, // Floppy volume: 127
	{ FX_ARCWELD,       84 }, // Floppy volume: 127
	{ FX_RETRACT_ORB,  127 },
	{ FX_DRAGON,       127 },
	{ FX_SNORES,       127 },
	{ FX_SPLASH,       127 },
	{ FX_LOBBY_DOOR,   127 },
	{ FX_CHIRP_LOOP,    26 }, // Floppy volume: 40
	{ FX_DOOR_CREAK,    96 },
	{ FX_SPOON_DIG,     64 },
	{ FX_CROW,          96 },
	{ FX_COLDWIND,      42 }, // Floppy volume: 64
	{ FX_TOOL_SND_1,    96 },
	{ FX_TOOL_SND_2,   127 },
	{ FX_TOOL_SND_3,    64 },
	{ FX_DOOR_METAL,    96 },
	{ FX_WATER_LOOP_S,  32 },
	{ FX_WATER_LOOP_L,  32 }, // Floppy volume: 64
	{ FX_DOOR_OPEN_2,  127 },
	{ FX_JAIL_DOOR,     64 },
	{ FX_KILN_FIRE,     53 }, // Floppy volume: 80

	// Only in the CD version
	{ FX_CROWD_01,      64 },
	{ FX_CROWD_02,      64 },
	{ FX_CROWD_03,      64 },
	{ FX_CROWD_04,      64 },
	{ FX_CROWD_05,      64 },
	{ FX_CROWD_06,      64 },
	{ FX_CROWD_07,      64 },
	{ FX_CROWD_08,      64 },
	{ FX_CROWD_09,      64 },
	{ FX_CROWD_10,      64 },
	{ FX_CROWD_11,      64 },
	{ FX_CROWD_12,      64 },
	{ FX_CROWD_13,      64 },
	{ FX_CROWD_14,      64 },
	{ FX_CROWD_15,      64 },
	{ FX_CROWD_16,      64 },
	{ FX_CROWD_17,      64 }
};

// Script function #70 (0x46)
int Script::SF_playSound(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop() - 13;

	if (/* param >= 0 && */ param < ARRAYSIZE(sfxTable))
		_vm->_sndRes->playSound(sfxTable[param].res, sfxTable[param].vol);
	else
		_vm->_sound->stopSound();

	return SUCCESS;
}

// Script function #75 (0x4d)
int Script::SF_rand(SCRIPTFUNC_PARAMS) {
	SDataWord_T param = thread->pop();

	thread->retVal = (_vm->_rnd.getRandomNumber(param));

	return SUCCESS;
}

// Script function #76 (0x4c)
int Script::SF_fadeMusic(SCRIPTFUNC_PARAMS) {
	debug(1, "stub: SF_fadeMusic()");
	return SUCCESS;
}

} // End of namespace Saga
