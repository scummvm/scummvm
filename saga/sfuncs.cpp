/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/objectdata.h"
#include "saga/render.h"
#include "saga/sound.h"
#include "saga/sndres.h"

#include "saga/script.h"

#include "saga/scene.h"

namespace Saga {

#define OPCODE(x) {&Script::x, #x}

void Script::setupScriptFuncList(void) {
	static const ScriptFunctionDescription scriptFunctionsList[SCRIPT_FUNCTION_MAX] = {
		OPCODE(SF_putString),
		OPCODE(sfWait),
		OPCODE(SF_takeObject),
		OPCODE(SF_objectIsCarried),
		OPCODE(sfStatusBar),
		OPCODE(SF_mainMode),
		OPCODE(sfScriptWalkTo),
		OPCODE(SF_doAction),
		OPCODE(sfSetActorFacing),
		OPCODE(sfStartBgdAnim),
		OPCODE(sfStopBgdAnim),
		OPCODE(sfLockUser),
		OPCODE(SF_preDialog),
		OPCODE(SF_killActorThreads),
		OPCODE(SF_faceTowards),
		OPCODE(sfSetFollower),
		OPCODE(SF_gotoScene),
		OPCODE(SF_setObjImage),
		OPCODE(SF_setObjName),
		OPCODE(SF_getObjImage),
		OPCODE(SF_getNumber),
		OPCODE(sfScriptOpenDoor),
		OPCODE(sfScriptCloseDoor),
		OPCODE(sfSetBgdAnimSpeed),
		OPCODE(SF_cycleColors),
		OPCODE(sfDoCenterActor),
		OPCODE(sfStartBgdAnimSpeed),
		OPCODE(sfScriptWalkToAsync),
		OPCODE(SF_enableZone),
		OPCODE(sfSetActorState),
		OPCODE(scriptMoveTo),
		OPCODE(SF_sceneEq),
		OPCODE(SF_dropObject),
		OPCODE(sfFinishBgdAnim),
		OPCODE(sfSwapActors),
		OPCODE(sfSimulSpeech),
		OPCODE(sfScriptWalk),
		OPCODE(sfCycleFrames),
		OPCODE(sfSetFrame),
		OPCODE(sfSetPortrait),
		OPCODE(sfSetProtagPortrait),
		OPCODE(sfChainBgdAnim),
		OPCODE(SF_scriptSpecialWalk),
		OPCODE(sfPlaceActor),
		OPCODE(SF_checkUserInterrupt),
		OPCODE(SF_walkRelative),
		OPCODE(SF_moveRelative),
		OPCODE(SF_simulSpeech2),
		OPCODE(sfPlacard),
		OPCODE(sfPlacardOff),
		OPCODE(SF_setProtagState),
		OPCODE(sfResumeBgdAnim),
		OPCODE(SF_throwActor),
		OPCODE(SF_waitWalk),
		OPCODE(SF_sceneID),
		OPCODE(SF_changeActorScene),
		OPCODE(SF_climb),
		OPCODE(sfSetDoorState),
		OPCODE(SF_setActorZ),
		OPCODE(SF_text),
		OPCODE(SF_getActorX),
		OPCODE(SF_getActorY),
		OPCODE(SF_eraseDelta),
		OPCODE(sfPlayMusic),
		OPCODE(SF_pickClimbOutPos),
		OPCODE(SF_tossRif),
		OPCODE(SF_showControls),
		OPCODE(SF_showMap),
		OPCODE(SF_puzzleWon),
		OPCODE(sfEnableEscape),
		OPCODE(sfPlaySound),
		OPCODE(SF_playLoopedSound),
		OPCODE(SF_getDeltaFrame),
		OPCODE(SF_showProtect),
		OPCODE(SF_protectResult),
		OPCODE(sfRand),
		OPCODE(SF_fadeMusic),
		OPCODE(SF_playVoice)
	};
	_scriptFunctionsList = scriptFunctionsList;
}

// Script function #0 (0x00)
// Print a debugging message
int Script::SF_putString(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	_vm->_console->DebugPrintf(getString(param));
	return SUCCESS;
}

// Script function #1 (0x01) blocking
// Param1: time in ticks
int Script::sfWait(SCRIPTFUNC_PARAMS) {
	int time;
	time = getUWord(thread->pop());

	if (!_skipSpeeches) {
		thread->waitDelay(ticksToMSec(time)); // put thread to sleep
	}
	return SUCCESS;
}

// Script function #2 (0x02)
int Script::SF_takeObject(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();
	int index = param & 0x1FFF;

	if (index >= ARRAYSIZE(ObjectTable)) {
		return FAILURE;
	}

	if (ObjectTable[index].sceneIndex != -1) {
		ObjectTable[index].sceneIndex = -1;
		_vm->_interface->addToInventory(index);
	}

	return SUCCESS;
}

// Script function #3 (0x03)
// Check if an object is carried.
int Script::SF_objectIsCarried(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();
	int index = param & 0x1FFF;

	if (index >= ARRAYSIZE(ObjectTable)) {
		thread->retVal = 0;
		return FAILURE;
	}

	thread->retVal = (ObjectTable[index].sceneIndex == -1) ? 1 : 0;
	return SUCCESS;
}

// Script function #4 (0x04) nonblocking
// Set the command display to the specified text string
// Param1: dialogue index of string
int Script::sfStatusBar(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	return _vm->_interface->setStatusText(getString(param));
}

// Script function #5 (0x05)
int Script::SF_mainMode(SCRIPTFUNC_PARAMS) {
	; // center actor
	; // show verb
	_vm->_interface->activate();
	//_vm->_interface->setMode(kPanelInventory);
	; // set pointer verb

	return SUCCESS;
}

// Script function #6 (0x06) blocking
// Param1: actor id
// Param2: actor x
// Param3: actor y
int Script::sfScriptWalkTo(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	ActorLocation actorLocation;
	ActorData *actor;

	actorId = getSWord(thread->pop());
	actorLocation.x = getSWord(thread->pop());
	actorLocation.y = getSWord(thread->pop());
	
	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;
	
	actor->flags &= ~kFollower;

	if (_vm->_actor->actorWalkTo(actorId, actorLocation)) {
		thread->waitWalk(actor);
	}
	return SUCCESS;
}

// Script function #7 (0x07)
int Script::SF_doAction(SCRIPTFUNC_PARAMS) {
	ScriptDataWord actor_parm = thread->pop();
	ScriptDataWord action_parm = thread->pop();
	ScriptDataWord obj_parm = thread->pop();
	ScriptDataWord withobj_parm = thread->pop();

	// The parameters correspond with the thread variables.

	debug(1, "stub: SF_doAction(%d, %d, %d, %d)", actor_parm, action_parm, obj_parm, withobj_parm);
	return SUCCESS;
}

// Script function #8 (0x08) nonblocking
// Param1: actor id
// Param2: actor orientation
int Script::sfSetActorFacing(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	int actorDirection;
	ActorData *actor;

	actorId = getSWord(thread->pop());
	actorDirection =  getSWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);
	actor->facingDirection = actor->actionDirection = actorDirection;
	actor->targetObject = ID_NOTHING;

	return SUCCESS;
}

// Script function #9 (0x09)
int Script::sfStartBgdAnim(SCRIPTFUNC_PARAMS) {
	int animId = getSWord(thread->pop());
	int cycles = getSWord(thread->pop());

	_vm->_anim->setCycles(animId, cycles);
	_vm->_anim->play(animId, kRepeatSpeed);

	debug(1, "sfStartBgdAnim(%d, %d)", animId, cycles);
	return SUCCESS;
}

// Script function #10 (0x0A)
int Script::sfStopBgdAnim(SCRIPTFUNC_PARAMS) {
	ScriptDataWord animId = getSWord(thread->pop());

	_vm->_anim->stop(animId);

	debug(1, "sfStopBgdAnim(%d)", animId);
	return SUCCESS;
}

// Script function #11 (0x0B) nonblocking
// If the parameter is true, the user interface is disabled while script
// continues to run. If the parameter is false, the user interface is 
// reenabled.
// Param1: boolean
int Script::sfLockUser(SCRIPTFUNC_PARAMS) {
	ScriptDataWord b_param;

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
int Script::SF_preDialog(SCRIPTFUNC_PARAMS) {
	_vm->_interface->deactivate();
	; // clear converse text
	if (_vm->_interface->isInMainMode())
		_vm->_interface->setMode(kPanelConverse);
	else
		; // display zero text
	_vm->_interface->setMode(kPanelNull);

	debug(1, "stub: SF_preDialog()");
	return SUCCESS;
}

// Script function #13 (0x0D)
int Script::SF_killActorThreads(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_killActorThreads(), %d args", nArgs);
	return SUCCESS;
}

// Script function #14 (0x0E)
int Script::SF_faceTowards(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();

	debug(1, "stub: SF_faceTowards(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #15 (0x0F)
// Param1: actor id
// Param2: target object
int Script::sfSetFollower(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int16 targetObject;

	ActorData *actor;

	actorId = getSWord(thread->pop());
	targetObject = getSWord(thread->pop());

	debug(1, "sfSetFollower(%d, %d) [%d]", actorId, targetObject, ACTOR_ID_TO_INDEX(actorId));
	
	actor = _vm->_actor->getActor(actorId);
	actor->targetObject = targetObject;
	if (targetObject != ID_NOTHING) {
		actor->flags |= kFollower;
		actor->actorFlags &= ~kActorNoFollow;
	} else {
		actor->flags &= ~kFollower;
	}
	
	return SUCCESS;
}

// Script function #16 (0x10)
int Script::SF_gotoScene(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();

	debug(1, "stub: SF_gotoScene(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #17 (0x11)
int Script::SF_setObjImage(SCRIPTFUNC_PARAMS) {
	ScriptDataWord obj_param = thread->pop();
	ScriptDataWord sprite_param = thread->pop();

	int index = obj_param & 0x1FFF;

	if (index >= ARRAYSIZE(ObjectTable)) {
		return FAILURE;
	}

	ObjectTable[index].spritelistRn = sprite_param + 9;
	_vm->_interface->draw();

	return SUCCESS;
}

// Script function #18 (0x12)
int Script::SF_setObjName(SCRIPTFUNC_PARAMS) {
	ScriptDataWord obj_param = thread->pop();
	ScriptDataWord name_param = thread->pop();

	int index = obj_param & 0x1FFF;

	if (index >= ARRAYSIZE(ObjectTable)) {
		return FAILURE;
	}

	ObjectTable[index].nameIndex = name_param;
	return SUCCESS;
}

// Script function #19 (0x13)
int Script::SF_getObjImage(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();
	int index = param & 0x1FFF;

	if (index >= ARRAYSIZE(ObjectTable)) {
		thread->retVal = 0;
		return FAILURE;
	}

	thread->retVal = ObjectTable[index].spritelistRn;
	return SUCCESS;
}

// Script function #20 (0x14)
int Script::SF_getNumber(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_getNumber(), %d args", nArgs);
	return SUCCESS;
}

// Script function #21 (0x15)
// Param1: door #
int Script::sfScriptOpenDoor(SCRIPTFUNC_PARAMS) {
	int doorNumber;
	doorNumber = getUWord(thread->pop());

	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
		_vm->_scene->setDoorState(doorNumber, 0);
	}
	return SUCCESS;
}

// Script function #22 (0x16)
// Param1: door #
int Script::sfScriptCloseDoor(SCRIPTFUNC_PARAMS) {
	int doorNumber;
	doorNumber = getUWord(thread->pop());

	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
		_vm->_scene->setDoorState(doorNumber, 0xff);
	}
	return SUCCESS;
}

// Script function #23 (0x17)
int Script::sfSetBgdAnimSpeed(SCRIPTFUNC_PARAMS) {
	int animId = getSWord(thread->pop());
	int speed = getSWord(thread->pop());

	_vm->_anim->setFrameTime(animId, ticksToMSec(speed));
	debug(1, "sfSetBgdAnimSpeed(%d, %d)", animId, speed);

	return SUCCESS;
}

// Script function #24 (0x18)
int Script::SF_cycleColors(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_cycleColors(), %d args", nArgs);
	return SUCCESS;
}

// Script function #25 (0x19)
// Param1: actor id
int Script::sfDoCenterActor(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	actorId = getSWord(thread->pop());

	_vm->_actor->_centerActor = _vm->_actor->getActor(actorId);
	return SUCCESS;
}

// Script function #26 (0x1A) nonblocking
// Starts the specified animation 
int Script::sfStartBgdAnimSpeed(SCRIPTFUNC_PARAMS) {
	int animId = getSWord(thread->pop());
	int cycles = getSWord(thread->pop());
	int speed = getSWord(thread->pop());

	_vm->_anim->setCycles(animId, cycles);
	_vm->_anim->play(animId, ticksToMSec(speed));

	debug(1, "sfStartBgdAnimSpeed(%d, %d, %d)", animId, cycles, speed);
	return SUCCESS;
}

// Script function #27 (0x1B) nonblocking
// Param1: actor id
// Param2: actor x
// Param3: actor y
int Script::sfScriptWalkToAsync(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	ActorLocation actorLocation;
	ActorData *actor;

	actorId = getSWord(thread->pop());
	actorLocation.x = getSWord(thread->pop());
	actorLocation.y = getSWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;

	actor->flags &= ~kFollower;

	_vm->_actor->actorWalkTo(actorId, actorLocation);
	return SUCCESS;
}

// Script function #28 (0x1C)
int Script::SF_enableZone(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_enableZone(), %d args", nArgs);
	return SUCCESS;
}

// Script function #29 (0x1D)
// Param1: actor id
// Param2: current action
int Script::sfSetActorState(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	int currentAction;
	ActorData *actor;

	actorId = getSWord(thread->pop());
	currentAction = getSWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);

	if ((currentAction >= kActionWalkToPoint) && (currentAction <= kActionWalkToPoint)) {
		wakeUpActorThread(kWaitTypeWalk, actor);
	}
	actor->currentAction = currentAction;
	actor->actorFlags &= ~kActorBackwards;

	return SUCCESS;
}

// Script function #30 (0x1E) nonblocking
// Param1: actor id
// Param2: actor pos x
// Param3: actor pos y
int Script::scriptMoveTo(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	ActorLocation actorLocation;
	ActorData *actor;

	actorId = getSWord(thread->pop());
	actorLocation.x = getSWord(thread->pop());
	actorLocation.y = getSWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);

	actor->location.x = actorLocation.x;
	actor->location.y = actorLocation.y;

	return SUCCESS;
}

// Script function #31 (0x21)
int Script::SF_sceneEq(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	if (_vm->_scene->getSceneLUT(param) == _vm->_scene->currentSceneNumber())
		thread->retVal = 1;
	else 
		thread->retVal = 0;
	return SUCCESS;
}

// Script function #32 (0x20)
int Script::SF_dropObject(SCRIPTFUNC_PARAMS) {
	ScriptDataWord obj_param = thread->pop();
	ScriptDataWord sprite_param = thread->pop();
	ScriptDataWord x_param = thread->pop();
	ScriptDataWord y_param = thread->pop();

	int index = obj_param & 0x1FFF;

	if (index >= ARRAYSIZE(ObjectTable)) {
		return FAILURE;
	}

	if (ObjectTable[index].sceneIndex == -1) {
		_vm->_interface->removeFromInventory(index);
	}

	ObjectTable[index].sceneIndex = _vm->_scene->currentSceneNumber();
	ObjectTable[index].spritelistRn = 9 + sprite_param;
	ObjectTable[index].x = x_param;
	ObjectTable[index].y = y_param;

	return SUCCESS;
}

// Script function #33 (0x21)
int Script::sfFinishBgdAnim(SCRIPTFUNC_PARAMS) {
	ScriptDataWord animId = getSWord(thread->pop());

	_vm->_anim->finish(animId);

	debug(1, "sfFinishBgdAnim(%d)", animId);
	return SUCCESS;
}

// Script function #34 (0x22)
// Param1: actor id 1
// Param2: actor id 2
int Script::sfSwapActors(SCRIPTFUNC_PARAMS) {
	uint16 actorId1;
	uint16 actorId2;
	ActorData *actor1;
	ActorData *actor2;
	ActorLocation location;

	actorId1 = getSWord(thread->pop());
	actorId2 = getSWord(thread->pop());

	actor1 = _vm->_actor->getActor(actorId1);
	actor2 = _vm->_actor->getActor(actorId2);
	location = actor1->location;
	actor1->location = actor2->location;
	actor2->location = location;
	

	if (actor1->flags & kProtagonist) {
		actor1->flags &= ~kProtagonist;
		actor2->flags |= kProtagonist;
		_vm->_actor->_protagonist = _vm->_actor->_centerActor = actor2;
	} else {
		if (actor2->flags & kProtagonist) {
		actor2->flags &= ~kProtagonist;
		actor1->flags |= kProtagonist;
		_vm->_actor->_protagonist = _vm->_actor->_centerActor = actor1;
		}
	}


	return SUCCESS;
}

// Script function #35 (0x23)
// Param1: string rid
// Param2: actorscount
// Param3: actor id1
///....
// Param3: actor idN
int Script::sfSimulSpeech(SCRIPTFUNC_PARAMS) {
	ScriptDataWord stringId;
	int actorsCount;
	int i;
	uint16 actorsIds[ACTOR_SPEECH_ACTORS_MAX];
	const char *string;

	stringId = thread->pop();
	actorsCount = getSWord(thread->pop());

	if (actorsCount > ACTOR_SPEECH_ACTORS_MAX)
		error("sfSimulSpeech actorsCount=0x%X exceed ACTOR_SPEECH_ACTORS_MAX", actorsCount);

	for (i = 0; i < actorsCount; i++)
		actorsIds[i] = getSWord(thread->pop());
	
	string = getString(stringId);

	_vm->_actor->simulSpeech(string, actorsIds, actorsCount, 0);
	return SUCCESS;
}

// Script function #36 (0x24) ?
// Param1: actor id
// Param2: actor x
// Param3: actor y
// Param4: actor walk flag
int Script::sfScriptWalk(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	ActorLocation actorLocation;
	ActorData *actor;
	uint16 walkFlags;

	actorId = getSWord(thread->pop());
	actorLocation.x = getSWord(thread->pop());
	actorLocation.y = getSWord(thread->pop());
	walkFlags = getUWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;

	_vm->_actor->realLocation(actorLocation, ID_NOTHING, walkFlags);

	actor->flags &= ~kFollower;

	if (_vm->_actor->actorWalkTo(actorId, actorLocation) && !(walkFlags & kWalkAsync)) {
		thread->waitWalk(actor);
	}

	if (walkFlags & kWalkBackPedal) {
		actor->actorFlags |= kActorBackwards;
	}

	actor->actorFlags = (actor->actorFlags & ~kActorFacingMask) | (walkFlags & kActorFacingMask);
	return SUCCESS;
}

// Script function #37 (0x25) nonblocking
// Param1: actor id
// Param2: flags telling how to cycle the frames
// Param3: cycle frame number
// Param4: cycle delay
int Script::sfCycleFrames(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	int flags;
	int cycleFrameSequence;
	int cycleDelay;
	ActorData *actor;

	actorId = getSWord(thread->pop());
	flags = getUWord(thread->pop());
	cycleFrameSequence = getUWord(thread->pop());
	cycleDelay =  getUWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);

	if (flags & kCyclePong) {
		actor->currentAction = kActionPongFrames;
	} else {
		actor->currentAction = kActionCycleFrames;
	}

	actor->actorFlags &= ~(kActorContinuous | kActorRandom | kActorBackwards);

	if (!(flags & kCycleOnce)) {
		actor->actorFlags |= kActorContinuous;
	}
	if (flags & kCycleRandom) {
		actor->actorFlags |= kActorRandom;
	}
	if (flags & kCycleReverse) {
		actor->actorFlags |= kActorBackwards;
	}

	actor->cycleFrameSequence	= cycleFrameSequence;
	actor->cycleTimeCount = 0;
	actor->cycleDelay = cycleDelay;
	actor->actionCycle = 0;

	return SUCCESS;
}

// Script function #38 (0x26) nonblocking
// Param1: actor id
// Param2: frame type
// Param3: frame offset
int Script::sfSetFrame(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	int frameType;
	int frameOffset;
	ActorData *actor;
	ActorFrameRange *frameRange;

	actorId = getSWord(thread->pop());
	frameType = getSWord(thread->pop());
	frameOffset = getSWord(thread->pop());

	actor = _vm->_actor->getActor(actorId);

	frameRange = _vm->_actor->getActorFrameRange(actorId, frameType);

	if (frameRange->frameCount <= frameOffset) {
	//	frameRange = _vm->_actor->getActorFrameRange(actorId, frameType);

		error("Wrong frameOffset 0x%X", frameOffset);
	}
	actor->frameNumber = frameRange->frameIndex + frameOffset;

	if (actor->currentAction != kActionFall) {
		actor->currentAction = kActionFreeze;
	}

	return SUCCESS;
}

// Script function #39 (0x27)
// Sets the right-hand portrait
int Script::sfSetPortrait(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	return _vm->_interface->setRightPortrait(param);
}

// Script function #40 (0x28)
// Sets the left-hand portrait
int Script::sfSetProtagPortrait(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	return _vm->_interface->setLeftPortrait(param);
}

// Script function #41 (0x29) nonblocking
// Links the specified animations for playback

// Param1: ?
// Param2: total linked frame count
// Param3: animation id link target
// Param4: animation id link source
int Script::sfChainBgdAnim(SCRIPTFUNC_PARAMS) {
	int animId1 = getSWord(thread->pop());
	int animId = getSWord(thread->pop());
	int cycles = getSWord(thread->pop());
	int speed = getSWord(thread->pop());

	if (speed >= 0) {
		_vm->_anim->setCycles(animId, cycles);
		_vm->_anim->stop(animId);
		_vm->_anim->setFrameTime(animId, ticksToMSec(speed));
	}

	_vm->_anim->link(animId1, animId);
	debug(1, "sfChainBgdAnim(%d, %d, %d, %d)", animId1, animId, cycles, speed);
	return SUCCESS;
}

// Script function #42 (0x2A)
int Script::SF_scriptSpecialWalk(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();
	ScriptDataWord param3 = thread->pop();
	ScriptDataWord param4 = thread->pop();

	debug(1, "stub: SF_scriptSpecialWalk(%d, %d, %d, %d)", param1, param2, param3, param4);
	return SUCCESS;
}

// Script function #43 (0x2B) nonblocking
// Param1: actor id
// Param2: actor x
// Param3: actor y
// Param4: actor direction
// Param5: actor action
// Param6: actor frame number
int Script::sfPlaceActor(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	ActorLocation actorLocation;
	int actorDirection;
	int frameType;
	int frameOffset;
	ActorData *actor;
	ActorFrameRange *frameRange;

	actorId = getSWord(thread->pop());
	actorLocation.x = getSWord(thread->pop());
	actorLocation.y = getSWord(thread->pop());
	actorDirection =  getSWord(thread->pop());
	frameType =  getSWord(thread->pop());
	frameOffset =  getSWord(thread->pop());

	debug(1, "sfPlaceActor(%d, %d, %d, %d, %d, %d)", actorId, actorLocation.x, 
		  actorLocation.y, actorDirection, frameType, frameOffset);

	actor = _vm->_actor->getActor(actorId);
	actor->location.x = actorLocation.x;
	actor->location.y = actorLocation.y;
	actor->facingDirection = actor->actionDirection = actorDirection;

	if (frameType >= 0) {
		frameRange = _vm->_actor->getActorFrameRange(actorId, frameType);
	
		if (frameRange->frameCount <= frameOffset)
			error("Wrong frameOffset 0x%X", frameOffset);

		actor->frameNumber = frameRange->frameIndex + frameOffset;
		actor->currentAction = kActionFreeze;
	} else {
		actor->currentAction = kActionWait;
	}

	actor->targetObject = ID_NOTHING;

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
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();
	ScriptDataWord param3 = thread->pop();
	ScriptDataWord param4 = thread->pop();
	ScriptDataWord param5 = thread->pop();

	debug(1, "stub: SF_walkRelative(%d, %d, %d, %d, %d)", param1, param2, param3, param4, param5);
	return SUCCESS;
}

// Script function #46 (0x2E)
int Script::SF_moveRelative(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();
	ScriptDataWord param3 = thread->pop();
	ScriptDataWord param4 = thread->pop();
	ScriptDataWord param5 = thread->pop();

	debug(1, "stub: SF_moveRelative(%d, %d, %d, %d, %d)", param1, param2, param3, param4, param5);
	return SUCCESS;
}

// Script function #47 (0x2F)
int Script::SF_simulSpeech2(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_simulSpeech2(), %d args", nArgs);
	return SUCCESS;
}

// Script function #48 (0x30)
// Param1: string rid
int Script::sfPlacard(SCRIPTFUNC_PARAMS) {
	int stringId;
	const char *string;
	GAME_DISPLAYINFO disp;
	SURFACE *back_buf = _vm->_gfx->getBackBuffer();
	PALENTRY cur_pal[PAL_ENTRIES];
	PALENTRY *pal;
	
	stringId = thread->pop();
	string = getString(stringId);

	_vm->getDisplayInfo(&disp);

	_vm->_gfx->showCursor(false);
	_vm->_gfx->getCurrentPal(cur_pal);
	_vm->_gfx->palToBlackWait(back_buf, cur_pal, kNormalFadeDuration);

	_vm->_interface->setStatusText("");

	Rect rect(disp.logical_w, disp.scene_h);
	drawRect(back_buf, &rect, 138);

	// Put the text in the center of the viewport, assuming it will fit on
	// one line. If we cannot make that assumption we'll need to extend
	// the text drawing function so that it can center text around a point.
	// It doesn't end up in exactly the same spot as the original did it,
	// but it's close enough for now at least.

	_vm->textDraw(MEDIUM_FONT_ID, back_buf, string,
		disp.logical_w / 2,
		(disp.scene_h - _vm->_font->getHeight(MEDIUM_FONT_ID)) / 2,
		_vm->_gfx->getWhite(), _vm->_gfx->getBlack(),
		FONT_OUTLINE | FONT_CENTERED);

	_vm->_render->setFlag(RF_PLACARD);
	_vm->_render->drawScene();

	_vm->_scene->getBGPal(&pal);
	_vm->_gfx->blackToPalWait(back_buf, pal, kNormalFadeDuration);

	_vm->_interface->rememberMode();
	_vm->_interface->setMode(kPanelPlacard);

	return SUCCESS;
}

// Script function #49 (0x31)
int Script::sfPlacardOff(SCRIPTFUNC_PARAMS) {
	SURFACE *back_buf = _vm->_gfx->getBackBuffer();
	PALENTRY cur_pal[PAL_ENTRIES];
	PALENTRY *pal;

	// Fade down
	_vm->_gfx->getCurrentPal(cur_pal);
	_vm->_gfx->palToBlackWait(back_buf, cur_pal, kNormalFadeDuration);

	_vm->_render->clearFlag(RF_PLACARD);
	_vm->_render->drawScene();

	// Fade up
	_vm->_scene->getBGPal(&pal);

	_vm->_gfx->showCursor(true);
	_vm->_gfx->blackToPalWait(back_buf, pal, kNormalFadeDuration);

	_vm->_interface->restoreMode();

	return SUCCESS;
}

// Script function #50 (0x32)
int Script::SF_setProtagState(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_setProtagState(), %d args", nArgs);
	return SUCCESS;
}

// Script function #51 (0x33)
int Script::sfResumeBgdAnim(SCRIPTFUNC_PARAMS) {
	int animId = getSWord(thread->pop());
	int cycles = getSWord(thread->pop());

	_vm->_anim->resume(animId, cycles);
	debug(1, "sfResumeBgdAnimSpeed(%d, %d)", animId, cycles);

	return SUCCESS;
}

// Script function #52 (0x34)
int Script::SF_throwActor(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();
	ScriptDataWord param3 = thread->pop();
	ScriptDataWord param4 = thread->pop();
	ScriptDataWord param5 = thread->pop();
	ScriptDataWord param6 = thread->pop();

	debug(1, "stub: SF_throwActor(%d, %d, %d, %d, %d, %d)", param1, param2, param3, param4, param5, param6);
	return SUCCESS;
}

// Script function #53 (0x35)
int Script::SF_waitWalk(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	debug(1, "stub: SF_waitWalk(%d)", param);
	return SUCCESS;
}

// Script function #54 (0x36)
int Script::SF_sceneID(SCRIPTFUNC_PARAMS) {
	thread->retVal = _vm->_scene->currentSceneNumber();
	return SUCCESS;
}

// Script function #55 (0x37)
int Script::SF_changeActorScene(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();

	debug(1, "stub: SF_changeActorScene(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #56 (0x38)
int Script::SF_climb(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();
	ScriptDataWord param3 = thread->pop();
	ScriptDataWord param4 = thread->pop();

	debug(1, "stub: SF_climb(%d, %d, %d, %d)", param1, param2, param3, param4);
	return SUCCESS;
}

// Script function #57 (0x39)
// Param1: door #
// Param2: door state
int Script::sfSetDoorState(SCRIPTFUNC_PARAMS) {
	int doorNumber;
	int doorState;
	doorNumber = getUWord(thread->pop());
	doorState = getUWord(thread->pop());

	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
		_vm->_scene->setDoorState(doorNumber, doorState);
	}
	return SUCCESS;
}

// Script function #58 (0x3A)
int Script::SF_setActorZ(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param1 = thread->pop();
	ScriptDataWord param2 = thread->pop();

	debug(1, "stub: SF_setActorZ(%d, %d)", param1, param2);
	return SUCCESS;
}

// Script function #59 (0x3B)
int Script::SF_text(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_text(), %d args", nArgs);
	return SUCCESS;
}

// Script function #60 (0x3C)
int Script::SF_getActorX(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	debug(1, "stub: SF_getActorX(%d)", param);
	return SUCCESS;
}

// Script function #61 (0x3D)
int Script::SF_getActorY(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	debug(1, "stub: SF_getActorY(%d)", param);
	return SUCCESS;
}

// Script function #62 (0x3E)
int Script::SF_eraseDelta(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_eraseDelta(), %d args", nArgs);
	return SUCCESS;
}

// Script function #63 (0x3F)
int Script::sfPlayMusic(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop() + 9;

	if (param >= 9 && param <= 34)
		_vm->_music->play(param);
	else
		_vm->_music->stop();

	return SUCCESS;
}

// Script function #64 (0x40)
int Script::SF_pickClimbOutPos(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_pickClimbOutPos(), %d args", nArgs);
	return SUCCESS;
}

// Script function #65 (0x41)
int Script::SF_tossRif(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_tossRif(), %d args", nArgs);
	return SUCCESS;
}

// Script function #66 (0x42)
int Script::SF_showControls(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_showControls(), %d args", nArgs);
	return SUCCESS;
}

// Script function #67 (0x43)
int Script::SF_showMap(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_showMap(), %d args", nArgs);
	return SUCCESS;
}

// Script function #68 (0x44)
int Script::SF_puzzleWon(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_puzzleWon(), %d args", nArgs);
	return SUCCESS;
}

// Script function #69 (0x45)
int Script::sfEnableEscape(SCRIPTFUNC_PARAMS) {
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
int Script::sfPlaySound(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop() - 13;

	if (/* param >= 0 && */ param < ARRAYSIZE(sfxTable))
		_vm->_sndRes->playSound(sfxTable[param].res, sfxTable[param].vol, false);
	else
		_vm->_sound->stopSound();

	return SUCCESS;
}

// Script function #71 (0x47)
int Script::SF_playLoopedSound(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_playLoopedSound(), %d args", nArgs);
	return SUCCESS;
}

// Script function #72 (0x48)
int Script::SF_getDeltaFrame(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_getDeltaFrame(), %d args", nArgs);
	return SUCCESS;
}

// Script function #73 (0x49)
int Script::SF_showProtect(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_showProtect(), %d args", nArgs);
	return SUCCESS;
}

// Script function #74 (0x4A)
int Script::SF_protectResult(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_protectResult(), %d args", nArgs);
	return SUCCESS;
}

// Script function #75 (0x4d)
int Script::sfRand(SCRIPTFUNC_PARAMS) {
	ScriptDataWord param = thread->pop();

	thread->retVal = (_vm->_rnd.getRandomNumber(param));

	return SUCCESS;
}

// Script function #76 (0x4c)
int Script::SF_fadeMusic(SCRIPTFUNC_PARAMS) {
	debug(1, "stub: SF_fadeMusic()");
	return SUCCESS;
}

// Script function #77 (0x4d)
int Script::SF_playVoice(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(1, "stub: SF_playVoice(), %d args", nArgs);
	return SUCCESS;
}

} // End of namespace Saga
