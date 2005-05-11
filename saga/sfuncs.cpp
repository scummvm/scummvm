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
#include "saga/events.h"
#include "saga/font.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/itedata.h"
#include "saga/render.h"
#include "saga/sound.h"
#include "saga/sndres.h"

#include "saga/script.h"
#include "saga/objectmap.h"

#include "saga/scene.h"
#include "saga/isomap.h"

namespace Saga {

#define OPCODE(x) {&Script::x, #x}

void Script::setupScriptFuncList(void) {
	static const ScriptFunctionDescription scriptFunctionsList[SCRIPT_FUNCTION_MAX] = {
		OPCODE(sfPutString),
		OPCODE(sfWait),
		OPCODE(sfTakeObject),
		OPCODE(sfIsCarried),
		OPCODE(sfStatusBar),
		OPCODE(sfMainMode),
		OPCODE(sfScriptWalkTo),
		OPCODE(sfScriptDoAction),
		OPCODE(sfSetActorFacing),
		OPCODE(sfStartBgdAnim),
		OPCODE(sfStopBgdAnim),
		OPCODE(sfLockUser),
		OPCODE(sfPreDialog),
		OPCODE(sfKillActorThreads),
		OPCODE(sfFaceTowards),
		OPCODE(sfSetFollower),
		OPCODE(sfScriptGotoScene),
		OPCODE(sfSetObjImage),
		OPCODE(sfSetObjName),
		OPCODE(sfGetObjImage),
		OPCODE(SF_getNumber),
		OPCODE(sfScriptOpenDoor),
		OPCODE(sfScriptCloseDoor),
		OPCODE(sfSetBgdAnimSpeed),
		OPCODE(SF_cycleColors),
		OPCODE(sfDoCenterActor),
		OPCODE(sfStartBgdAnimSpeed),
		OPCODE(sfScriptWalkToAsync),
		OPCODE(sfEnableZone),
		OPCODE(sfSetActorState),
		OPCODE(sfScriptMoveTo),
		OPCODE(sfSceneEq),
		OPCODE(sfDropObject),
		OPCODE(sfFinishBgdAnim),
		OPCODE(sfSwapActors),
		OPCODE(sfSimulSpeech),
		OPCODE(sfScriptWalk),
		OPCODE(sfCycleFrames),
		OPCODE(sfSetFrame),
		OPCODE(sfSetPortrait),
		OPCODE(sfSetProtagPortrait),
		OPCODE(sfChainBgdAnim),
		OPCODE(sfScriptSpecialWalk),
		OPCODE(sfPlaceActor),
		OPCODE(sfCheckUserInterrupt),
		OPCODE(sfScriptWalkRelative),
		OPCODE(sfScriptMoveRelative),
		OPCODE(SF_simulSpeech2),
		OPCODE(sfPlacard),
		OPCODE(sfPlacardOff),
		OPCODE(sfSetProtagState),
		OPCODE(sfResumeBgdAnim),
		OPCODE(SF_throwActor),
		OPCODE(sfWaitWalk),
		OPCODE(sfScriptSceneID),
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
		OPCODE(sfGetDeltaFrame),
		OPCODE(SF_showProtect),
		OPCODE(SF_protectResult),
		OPCODE(sfRand),
		OPCODE(SF_fadeMusic),
		OPCODE(SF_playVoice),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub),
		OPCODE(SF_stub)
	};
	_scriptFunctionsList = scriptFunctionsList;
}

// Script function #0 (0x00)
// Print a debugging message
void Script::sfPutString(SCRIPTFUNC_PARAMS) {
	const char *str;
	str = thread->_strings->getString(thread->pop());

	_vm->_console->DebugPrintf("sfPutString: %s\n",str);
	debug(0, "sfPutString: %s", str);
}

// Script function #1 (0x01) blocking
// Param1: time in ticks
void Script::sfWait(SCRIPTFUNC_PARAMS) {
	int16 time;
	time = thread->pop();

	if (!_skipSpeeches) {
		thread->waitDelay(ticksToMSec(time)); // put thread to sleep
	}
}

// Script function #2 (0x02)
void Script::sfTakeObject(SCRIPTFUNC_PARAMS) {
	uint16 objectId = thread->pop();
	ObjectData *obj;
	obj = _vm->_actor->getObj(objectId);
	if (obj->sceneNumber != ITE_SCENE_INV) {
		obj->sceneNumber = ITE_SCENE_INV;
		//4debug for (int j=0;j<17;j++)
		_vm->_interface->addToInventory(objectId);
	}
}

// Script function #3 (0x03)
// Check if an object is carried.
void Script::sfIsCarried(SCRIPTFUNC_PARAMS) {
	uint16 objectId = thread->pop();
	ObjectData *obj;
	obj = _vm->_actor->getObj(objectId);

	thread->_returnValue = (obj->sceneNumber == ITE_SCENE_INV) ? 1 : 0;
}

// Script function #4 (0x04) nonblocking
// Set the command display to the specified text string
// Param1: dialogue index of string
void Script::sfStatusBar(SCRIPTFUNC_PARAMS) {
	int16 stringIndex = thread->pop();

	_vm->_interface->setStatusText(thread->_strings->getString(stringIndex));
}

// Script function #5 (0x05)
void Script::sfMainMode(SCRIPTFUNC_PARAMS) {
	_vm->_actor->_centerActor = _vm->_actor->_protagonist;
	showVerb();		
	_vm->_interface->activate();
	_vm->_interface->setMode(kPanelInventory);
	setPointerVerb();
}

// Script function #6 (0x06) blocking
// Param1: actor id
// Param2: actor x
// Param3: actor y
void Script::sfScriptWalkTo(SCRIPTFUNC_PARAMS) {
	uint16 actorId;
	Location actorLocation;
	ActorData *actor;

	actorId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();
	
	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;
	
	actor->flags &= ~kFollower;

	if (_vm->_actor->actorWalkTo(actorId, actorLocation)) {
		thread->waitWalk(actor);
	}
}

// Script function #7 (0x07)
// Param1: actor id
// Param2: action
// Param3: theObject
// Param4: withObject
void Script::sfScriptDoAction(SCRIPTFUNC_PARAMS) {
	uint16 objectId;
	uint16 action;
	uint16 theObject;
	uint16 withObject;
	int16 scriptEntryPointNumber;
	int16 moduleNumber;
	ActorData *actor;
	ObjectData *obj;
	const HitZone *hitZone;
	EVENT event;

	objectId = thread->pop();
	action = thread->pop();
	theObject = thread->pop();
	withObject = thread->pop();

	switch (objectTypeId(objectId)) {
		case kGameObjectObject:
			obj = _vm->_actor->getObj(objectId);
			scriptEntryPointNumber = obj->scriptEntrypointNumber;
			if (scriptEntryPointNumber <= 0) {
				return;
			}
			moduleNumber = 0; 
			break;
		case kGameObjectActor: 
			actor = _vm->_actor->getActor(objectId);			
			scriptEntryPointNumber = actor->scriptEntrypointNumber;
			if (scriptEntryPointNumber <= 0) {
				return;
			}
			if (actor->flags & (kProtagonist | kFollower)) {
				moduleNumber = 0; 
			} else {
				moduleNumber = _vm->_scene->getScriptModuleNumber();
			}
			break;
		case kGameObjectHitZone:
		case kGameObjectStepZone:
			if (objectTypeId(objectId) == kGameObjectHitZone) {
				hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(objectId));
			} else {
				hitZone = _vm->_scene->_actionMap->getHitZone(objectIdToIndex(objectId));
			}
			scriptEntryPointNumber = hitZone->getScriptNumber();
			moduleNumber = _vm->_scene->getScriptModuleNumber();			
			break;
		default:
			error("Script::sfScriptDoAction wrong object type");
	}

	event.type = ONESHOT_EVENT;
	event.code = SCRIPT_EVENT;
	event.op = EVENT_EXEC_NONBLOCKING;
	event.time = 0;
	event.param = moduleNumber;
	event.param2 = scriptEntryPointNumber;
	event.param3 = action;		// Action
	event.param4 = theObject;	// Object
	event.param5 = theObject;	// With Object
	event.param6 = objectId;

	_vm->_events->queue(&event);
}

// Script function #8 (0x08) nonblocking
// Param1: actor id
// Param2: actor orientation
void Script::sfSetActorFacing(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int actorDirection;
	ActorData *actor;

	actorId = thread->pop();
	actorDirection =  thread->pop();

	actor = _vm->_actor->getActor(actorId);
	actor->facingDirection = actor->actionDirection = actorDirection;
	actor->targetObject = ID_NOTHING;
}

// Script function #9 (0x09)
void Script::sfStartBgdAnim(SCRIPTFUNC_PARAMS) {
	int16 animId = thread->pop();
	int16 cycles = thread->pop();

	_vm->_anim->setCycles(animId, cycles);
	_vm->_anim->play(animId, kRepeatSpeed);

	debug(1, "sfStartBgdAnim(%d, %d)", animId, cycles);
}

// Script function #10 (0x0A)
void Script::sfStopBgdAnim(SCRIPTFUNC_PARAMS) {
	int16 animId = thread->pop();

	_vm->_anim->stop(animId);

	debug(1, "sfStopBgdAnim(%d)", animId);
}

// Script function #11 (0x0B) nonblocking
// If the parameter is true, the user interface is disabled while script
// continues to run. If the parameter is false, the user interface is 
// reenabled.
// Param1: boolean
void Script::sfLockUser(SCRIPTFUNC_PARAMS) {
	int16 lock;

	lock = thread->pop();

	if (lock) {
		_vm->_interface->deactivate();
	} else {
		_vm->_interface->activate();
	}

}

// Script function #12 (0x0C)
// Disables mouse input, etc.
void Script::sfPreDialog(SCRIPTFUNC_PARAMS) {
	_vm->_interface->deactivate();
	_vm->_interface->converseClear();
	if (_vm->_interface->isInMainMode())
		_vm->_interface->setMode(kPanelConverse);
	else
		_vm->_interface->converseDisplayText();
		
	_vm->_interface->setMode(kPanelNull);
}

// Script function #13 (0x0D)
void Script::sfKillActorThreads(SCRIPTFUNC_PARAMS) {
	ScriptThread *anotherThread;
	ScriptThreadList::iterator threadIterator;
	int16 actorId;

	actorId = thread->pop();


	for (threadIterator = _threadList.begin(); threadIterator != _threadList.end(); ++threadIterator) {
		anotherThread = threadIterator.operator->();
		if ((anotherThread != thread) && (anotherThread->_threadVars[kThreadVarActor] == actorId)) {
			anotherThread->_flags &= ~kTFlagWaiting;
			anotherThread->_flags |= kTFlagAborted;
		}
	}
}

// Script function #14 (0x0E)
// Param1: actor id
// Param2: object id
void Script::sfFaceTowards(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int16 targetObject;
	ActorData *actor;

	actorId = thread->pop();
	targetObject = thread->pop();

	actor = _vm->_actor->getActor(actorId);
	actor->targetObject = targetObject;
}

// Script function #15 (0x0F)
// Param1: actor id
// Param2: target object
void Script::sfSetFollower(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int16 targetObject;

	ActorData *actor;

	actorId = thread->pop();
	targetObject = thread->pop();

	debug(1, "sfSetFollower(%d, %d) [%d]", actorId, targetObject, _vm->_actor->actorIdToIndex(actorId));
	
	actor = _vm->_actor->getActor(actorId);
	actor->targetObject = targetObject;
	if (targetObject != ID_NOTHING) {
		actor->flags |= kFollower;
		actor->actorFlags &= ~kActorNoFollow;
	} else {
		actor->flags &= ~kFollower;
	}
}

static struct SceneSubstitutes {
    int sceneId;
    const char *message;
    const char *name;
    const char *image;
} sceneSubstitutes[] = {
    { 
		7,
		"Tycho says he knows much about the northern lands. Can Rif convince "
		"the Dog to share this knowledge?",
		"The Home of Tycho Northpaw",
		"tycho.bbm"
	},

    {
		27,
		"The scene of the crime may hold many clues, but will the servants of "
		"the Sanctuary trust Rif?",
		"The Sanctuary of the Orb",
		"sanctuar.bbm"
	},

    {
		5,
		"The Rats hold many secrets that could guide Rif on his quest -- assuming "
		"he can get past the doorkeeper.",
		"The Rat Complex",
		"ratdoor.bbm"
	},

    {
		2,
		"The Ferrets enjoy making things and have the materials to do so. How can "
		"that help Rif?",
		"The Ferret Village",
		"ferrets.bbm"
	},

    {
		67,
		"What aid can the noble King of the Elks provide to Rif and his companions?",
		"The Realm of the Forest King",
		"elkenter.bbm"
	},

    {
		3,
		"The King holds Rif's sweetheart hostage. Will the Boar provide any "
		"assistance to Rif?",
		"The Great Hall of the Boar King",
		"boarhall.bbm"
	}
};

// Script function #16 (0x10)
void Script::sfScriptGotoScene(SCRIPTFUNC_PARAMS) {
	int16 sceneNumber;
	int16 entrance;

	sceneNumber = thread->pop();
	entrance = thread->pop();
	if (sceneNumber < 0) {
		//TODO: quit from game at all
	}

	// This is used for latter demos where all places on world map except
	// Tent Faire are substituted with LBM picture and short description
	// TODO: implement
	for (int i = 0; i < ARRAYSIZE(sceneSubstitutes); i++) {
	
		if (sceneSubstitutes[i].sceneId == sceneNumber) {
			debug(0, "Scene %d substitute exists", sceneNumber);
		}
	}

	_vm->_scene->changeScene(sceneNumber, entrance, (sceneNumber == RID_ITE_ENDCREDIT_SCENE_1) ? kTransitionFade : kTransitionNoFade);

	//TODO: placard stuff
	_pendingVerb = kVerbNone;
	_currentObject[0] = _currentObject[1] = ID_NOTHING;
	showVerb();
}

// Script function #17 (0x11)
// Param1: object id
// Param2: sprite index
void Script::sfSetObjImage(SCRIPTFUNC_PARAMS) {
	uint16 objectId;
	uint16 spriteId;
	ObjectData *obj;

	objectId = thread->pop();
	spriteId = thread->pop();

	obj = _vm->_actor->getObj(objectId);
	obj->spriteListResourceId = OBJ_SPRITE_BASE + spriteId;
	_vm->_interface->refreshInventory();
}

// Script function #18 (0x12)
// Param1: object id
// Param2: name index
void Script::sfSetObjName(SCRIPTFUNC_PARAMS) {
	uint16 objectId;
	uint16 nameIdx;
	ObjectData *obj;

	objectId = thread->pop();
	nameIdx = thread->pop();

	obj = _vm->_actor->getObj(objectId);
	obj->nameIndex = nameIdx;
}

// Script function #19 (0x13)
// Param1: object id
void Script::sfGetObjImage(SCRIPTFUNC_PARAMS) {
	uint16 objectId;
	ObjectData *obj;

	objectId = thread->pop();

	obj = _vm->_actor->getObj(objectId);
	thread->_returnValue = obj->spriteListResourceId - OBJ_SPRITE_BASE;
}

// Script function #20 (0x14)
void Script::SF_getNumber(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_getNumber(), %d args", nArgs);
}

// Script function #21 (0x15)
// Param1: door #
void Script::sfScriptOpenDoor(SCRIPTFUNC_PARAMS) {
	int16 doorNumber;
	doorNumber = thread->pop();

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->setTileDoorState(doorNumber, 1);
	} else {
		_vm->_scene->setDoorState(doorNumber, 0);
	}
}

// Script function #22 (0x16)
// Param1: door #
void Script::sfScriptCloseDoor(SCRIPTFUNC_PARAMS) {
	int16 doorNumber;
	doorNumber = thread->pop();

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->setTileDoorState(doorNumber, 0);
	} else {
		_vm->_scene->setDoorState(doorNumber, 0xff);
	}
}

// Script function #23 (0x17)
void Script::sfSetBgdAnimSpeed(SCRIPTFUNC_PARAMS) {
	int16 animId = thread->pop();
	int16 speed = thread->pop();

	_vm->_anim->setFrameTime(animId, ticksToMSec(speed));
	debug(1, "sfSetBgdAnimSpeed(%d, %d)", animId, speed);
}

// Script function #24 (0x18)
void Script::SF_cycleColors(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_cycleColors(), %d args", nArgs);
}

// Script function #25 (0x19)
// Param1: actor id
void Script::sfDoCenterActor(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	actorId = thread->pop();

	_vm->_actor->_centerActor = _vm->_actor->getActor(actorId);
}

// Script function #26 (0x1A) nonblocking
// Starts the specified animation 
void Script::sfStartBgdAnimSpeed(SCRIPTFUNC_PARAMS) {
	int16 animId = thread->pop();
	int16 cycles = thread->pop();
	int16 speed = thread->pop();

	_vm->_anim->setCycles(animId, cycles);
	_vm->_anim->play(animId, ticksToMSec(speed));

	debug(1, "sfStartBgdAnimSpeed(%d, %d, %d)", animId, cycles, speed);
}

// Script function #27 (0x1B) nonblocking
// Param1: actor id
// Param2: actor x
// Param3: actor y
void Script::sfScriptWalkToAsync(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	Location actorLocation;
	ActorData *actor;

	actorId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();

	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;

	actor->flags &= ~kFollower;

	_vm->_actor->actorWalkTo(actorId, actorLocation);
}

// Script function #28 (0x1C)
void Script::sfEnableZone(SCRIPTFUNC_PARAMS) {
	uint16 objectId = thread->pop();
	int16 flag = thread->pop();
	HitZone *hitZone;

	debug(0, "sfEnableZone(%d, %d)", objectId, flag);
	if (objectTypeId(objectId) == kGameObjectHitZone) {
		hitZone = _vm->_scene->_objectMap->getHitZone(objectIdToIndex(objectId));
	} else {
		hitZone = _vm->_scene->_actionMap->getHitZone(objectIdToIndex(objectId));
	}

	if (flag) {
		hitZone->setFlag(kHitZoneEnabled);
	} else {
		hitZone->clearFlag(kHitZoneEnabled);
		_vm->_actor->_protagonist->lastZone = NULL;
	}
}

// Script function #29 (0x1D)
// Param1: actor id
// Param2: current action
void Script::sfSetActorState(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int currentAction;
	ActorData *actor;

	actorId = thread->pop();
	currentAction = thread->pop();

	actor = _vm->_actor->getActor(actorId);

	if ((currentAction >= kActionWalkToPoint) && (currentAction <= kActionWalkToPoint)) {
		wakeUpActorThread(kWaitTypeWalk, actor);
	}
	actor->currentAction = currentAction;
	actor->actorFlags &= ~kActorBackwards;
}

// Script function #30 (0x1E) nonblocking
// Param1: actor id
// Param2: actor pos x
// Param3: actor pos y
void Script::sfScriptMoveTo(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	Location actorLocation;
	ActorData *actor;

	actorId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();

	actor = _vm->_actor->getActor(actorId);

	actor->location.x = actorLocation.x;
	actor->location.y = actorLocation.y;
}

// Script function #31 (0x21)
// Param1: sceneNumber
void Script::sfSceneEq(SCRIPTFUNC_PARAMS) {
	int16 sceneNumber = thread->pop();

	if (_vm->_scene->getSceneResourceId(sceneNumber) == _vm->_scene->currentSceneResourceId())
		thread->_returnValue = 1;
	else 
		thread->_returnValue = 0;
}

// Script function #32 (0x20)
void Script::sfDropObject(SCRIPTFUNC_PARAMS) {
	uint16 objectId;
	uint16 spriteId;
	int16 x;
	int16 y;
	ObjectData *obj;

	objectId = thread->pop();
	spriteId = thread->pop();
	x = thread->pop();
	y = thread->pop();

	obj = _vm->_actor->getObj(objectId);

	if (obj->sceneNumber == ITE_SCENE_INV) {
		_vm->_interface->removeFromInventory(objectId);
	}

	obj->sceneNumber = _vm->_scene->currentSceneNumber();
	obj->spriteListResourceId = OBJ_SPRITE_BASE + spriteId;
	obj->location.x = x;
	obj->location.y = y;
}

// Script function #33 (0x21)
void Script::sfFinishBgdAnim(SCRIPTFUNC_PARAMS) {
	int16 animId = thread->pop();

	_vm->_anim->finish(animId);

	debug(1, "sfFinishBgdAnim(%d)", animId);
}

// Script function #34 (0x22)
// Param1: actor id 1
// Param2: actor id 2
void Script::sfSwapActors(SCRIPTFUNC_PARAMS) {
	int16 actorId1;
	int16 actorId2;
	ActorData *actor1;
	ActorData *actor2;

	actorId1 = thread->pop();
	actorId2 = thread->pop();

	actor1 = _vm->_actor->getActor(actorId1);
	actor2 = _vm->_actor->getActor(actorId2);

	SWAP(actor1->location, actor2->location);

	if (actor1->flags & kProtagonist) {
		actor1->flags &= ~kProtagonist;
		actor2->flags |= kProtagonist;
		_vm->_actor->_protagonist = _vm->_actor->_centerActor = actor2;
	} else if (actor2->flags & kProtagonist) {
		actor2->flags &= ~kProtagonist;
		actor1->flags |= kProtagonist;
		_vm->_actor->_protagonist = _vm->_actor->_centerActor = actor1;
	}
}

// Script function #35 (0x23)
// Param1: string rid
// Param2: actorscount
// Param3: actor id1
///....
// Param3: actor idN
void Script::sfSimulSpeech(SCRIPTFUNC_PARAMS) {
	int16 stringId;
	int16 actorsCount;
	int i;
	uint16 actorsIds[ACTOR_SPEECH_ACTORS_MAX];
	const char *string;

	stringId = thread->pop();
	actorsCount = thread->pop();

	if (actorsCount > ACTOR_SPEECH_ACTORS_MAX)
		error("sfSimulSpeech actorsCount=0x%X exceed ACTOR_SPEECH_ACTORS_MAX", actorsCount);

	for (i = 0; i < actorsCount; i++)
		actorsIds[i] = thread->pop();
	
	string = thread->_strings->getString(stringId);

	_vm->_actor->simulSpeech(string, actorsIds, actorsCount, 0);
}

// Script function #36 (0x24) ?
// Param1: actor id
// Param2: actor x
// Param3: actor y
// Param4: actor walk flag
void Script::sfScriptWalk(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	Location actorLocation;
	ActorData *actor;
	uint16 walkFlags;

	actorId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();
	walkFlags = thread->pop();

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
}

// Script function #37 (0x25) nonblocking
// Param1: actor id
// Param2: flags telling how to cycle the frames
// Param3: cycle frame number
// Param4: cycle delay
void Script::sfCycleFrames(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int flags;
	int cycleFrameSequence;
	int cycleDelay;
	ActorData *actor;

	actorId = thread->pop();
	flags = thread->pop();
	cycleFrameSequence = thread->pop();
	cycleDelay =  thread->pop();

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

}

// Script function #38 (0x26) nonblocking
// Param1: actor id
// Param2: frame type
// Param3: frame offset
void Script::sfSetFrame(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int frameType;
	int frameOffset;
	ActorData *actor;
	ActorFrameRange *frameRange;

	actorId = thread->pop();
	frameType = thread->pop();
	frameOffset = thread->pop();

	actor = _vm->_actor->getActor(actorId);

	frameRange = _vm->_actor->getActorFrameRange(actorId, frameType);

	if (frameRange->frameCount <= frameOffset) {
		frameRange = _vm->_actor->getActorFrameRange(actorId, frameType);

		error("Wrong frameOffset 0x%X", frameOffset);
	}
	actor->frameNumber = frameRange->frameIndex + frameOffset;

	if (actor->currentAction != kActionFall) {
		actor->currentAction = kActionFreeze;
	}
}

// Script function #39 (0x27)
// Sets the right-hand portrait
void Script::sfSetPortrait(SCRIPTFUNC_PARAMS) {
	int16 param = thread->pop();

	_vm->_interface->setRightPortrait(param);
}

// Script function #40 (0x28)
// Sets the left-hand portrait
void Script::sfSetProtagPortrait(SCRIPTFUNC_PARAMS) {
	int16 param = thread->pop();

	_vm->_interface->setLeftPortrait(param);
}

// Script function #41 (0x29) nonblocking
// Links the specified animations for playback

// Param1: ?
// Param2: total linked frame count
// Param3: animation id link target
// Param4: animation id link source
void Script::sfChainBgdAnim(SCRIPTFUNC_PARAMS) {
	int16 animId1 = thread->pop();
	int16 animId = thread->pop();
	int16 cycles = thread->pop();
	int16 speed = thread->pop();

	if (speed >= 0) {
		_vm->_anim->setCycles(animId, cycles);
		_vm->_anim->stop(animId);
		_vm->_anim->setFrameTime(animId, ticksToMSec(speed));
	}

	_vm->_anim->link(animId1, animId);
	debug(1, "sfChainBgdAnim(%d, %d, %d, %d)", animId1, animId, cycles, speed);
}

// Script function #42 (0x2A)
// Param1: actor id
// Param2: actor x
// Param3: actor y
// Param4: frame seq
void Script::sfScriptSpecialWalk(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int16 walkFrameSequence;
	Location actorLocation;
	ActorData *actor;

	actorId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();
	walkFrameSequence =  thread->pop();

	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;

	_vm->_actor->actorWalkTo(actorId, actorLocation);

	actor->walkFrameSequence = walkFrameSequence;	
}

// Script function #43 (0x2B) nonblocking
// Param1: actor id
// Param2: actor x
// Param3: actor y
// Param4: actor direction
// Param5: actor action
// Param6: actor frame number
void Script::sfPlaceActor(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	Location actorLocation;
	int actorDirection;
	int frameType;
	int frameOffset;
	ActorData *actor;
	ActorFrameRange *frameRange;

	actorId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();
	actorDirection =  thread->pop();
	frameType =  thread->pop();
	frameOffset =  thread->pop();

	debug(1, "sfPlaceActor(%d, %d, %d, %d, %d, %d)", actorId, actorLocation.x, 
		  actorLocation.y, actorDirection, frameType, frameOffset);

	if (_vm->getGameType() == GType_IHNM) {
		warning("Actors aren't implemented for IHNM yet");
		return;
	}

	actor = _vm->_actor->getActor(actorId);
	actor->location.x = actorLocation.x;
	actor->location.y = actorLocation.y;
	actor->facingDirection = actor->actionDirection = actorDirection;

	if (frameType >= 0) {
		frameRange = _vm->_actor->getActorFrameRange(actorId, frameType);
	
		if (frameRange->frameCount <= frameOffset) {
			error("Wrong frameOffset 0x%X", frameOffset);
		}

		actor->frameNumber = frameRange->frameIndex + frameOffset;
		actor->currentAction = kActionFreeze;
	} else {
		actor->currentAction = kActionWait;
	}

	actor->targetObject = ID_NOTHING;

}

// Script function #44 (0x2C) nonblocking
// Checks to see if the user has interrupted a currently playing 
// game cinematic. Pushes a zero or positive value if the game 
// has not been interrupted.
void Script::sfCheckUserInterrupt(SCRIPTFUNC_PARAMS) {
	thread->_returnValue = (_skipSpeeches == true);

}

// Script function #45 (0x2D)
// Param1: actor id
// Param2: object id
// Param3: actor x
// Param4: actor y
// Param5: actor walk flag
void Script::sfScriptWalkRelative(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int16 objectId;
	uint16 walkFlags;
	Location actorLocation;
	ActorData *actor;

	actorId = thread->pop();
	objectId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();
	walkFlags =  thread->pop();

	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;

	_vm->_actor->realLocation(actorLocation, objectId, walkFlags);

	actor->flags &= ~kFollower;

	if (_vm->_actor->actorWalkTo(actorId, actorLocation) && !(walkFlags & kWalkAsync)) {
		thread->waitWalk(actor);
	}

	if (walkFlags & kWalkBackPedal) {
		actor->actorFlags |= kActorBackwards;
	}

	actor->actorFlags = (actor->actorFlags & ~kActorFacingMask) | (walkFlags & kActorFacingMask);	
}

// Script function #46 (0x2E)
// Param1: actor id
// Param2: object id
// Param3: actor x
// Param4: actor y
// Param5: actor walk flag
void Script::sfScriptMoveRelative(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	int16 objectId;
	uint16 walkFlags;
	Location actorLocation;
	ActorData *actor;

	actorId = thread->pop();
	objectId = thread->pop();
	actorLocation.x = thread->pop();
	actorLocation.y = thread->pop();
	walkFlags =  thread->pop();

	actor = _vm->_actor->getActor(actorId);
	actorLocation.z = actor->location.z;

	_vm->_actor->realLocation(actorLocation, objectId, walkFlags);


	actor->location = actorLocation;
	actor->actorFlags = (actor->actorFlags & ~kActorFacingMask) | (walkFlags & kActorFacingMask);
}

// Script function #47 (0x2F)
void Script::SF_simulSpeech2(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_simulSpeech2(), %d args", nArgs);
}

static TEXTLIST_ENTRY *placardTextEntry;

// Script function #48 (0x30)
// Param1: string rid
void Script::sfPlacard(SCRIPTFUNC_PARAMS) {
	int stringId;
	SURFACE *back_buf = _vm->_gfx->getBackBuffer();
	static PALENTRY cur_pal[PAL_ENTRIES];
	PALENTRY *pal;
	EVENT event;
	EVENT *q_event;
	
	thread->wait(kWaitTypePlacard);

	_vm->_interface->rememberMode();
	_vm->_interface->setMode(kPanelPlacard);

	stringId = thread->pop();

	event.type = ONESHOT_EVENT;
	event.code = CURSOR_EVENT;
	event.op = EVENT_HIDE;

	q_event = _vm->_events->queue(&event);

	_vm->_gfx->getCurrentPal(cur_pal);

	event.type = IMMEDIATE_EVENT;
	event.code = PAL_EVENT;
	event.op = EVENT_PALTOBLACK;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = cur_pal;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = INTERFACE_EVENT;
	event.op = EVENT_CLEAR_STATUS;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = GRAPHICS_EVENT;
	event.op = EVENT_SETFLAG;
	event.param = RF_PLACARD;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = GRAPHICS_EVENT;
	event.op = EVENT_FILL_RECT;
	event.data = back_buf;
	event.param = 138;
	event.param2 = 0;
	event.param3 = _vm->getSceneHeight();
	event.param4 = 0;
	event.param5 = _vm->getDisplayWidth();

	q_event = _vm->_events->chain(q_event, &event);

	// Put the text in the center of the viewport, assuming it will fit on
	// one line. If we cannot make that assumption we'll need to extend
	// the text drawing function so that it can center text around a point.
	// It doesn't end up in exactly the same spot as the original did it,
	// but it's close enough for now at least.

	TEXTLIST_ENTRY text_entry;

	text_entry.color = kITEColorBrightWhite;
	text_entry.effect_color = kITEColorBlack;
	text_entry.text_x = _vm->getDisplayWidth() / 2;
	text_entry.text_y = (_vm->getSceneHeight() - _vm->_font->getHeight(MEDIUM_FONT_ID)) / 2;
	text_entry.font_id = MEDIUM_FONT_ID;
	text_entry.flags = FONT_OUTLINE | FONT_CENTERED;
	text_entry.string = thread->_strings->getString(stringId);

	placardTextEntry = _vm->textAddEntry(_vm->_scene->_textList, &text_entry);

	event.type = ONESHOT_EVENT;
	event.code = TEXT_EVENT;
	event.op = EVENT_DISPLAY;
	event.data = placardTextEntry;

	q_event = _vm->_events->chain(q_event, &event);

	_vm->_scene->getBGPal(&pal);

	event.type = IMMEDIATE_EVENT;
	event.code = PAL_EVENT;
	event.op = EVENT_BLACKTOPAL;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = pal;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = SCRIPT_EVENT;
	event.op = EVENT_THREAD_WAKE;
	event.param = kWaitTypePlacard;

	q_event = _vm->_events->chain(q_event, &event);

}

// Script function #49 (0x31)
void Script::sfPlacardOff(SCRIPTFUNC_PARAMS) {
	static PALENTRY cur_pal[PAL_ENTRIES];
	PALENTRY *pal;
	EVENT event;
	EVENT *q_event;

	thread->wait(kWaitTypePlacard);

	_vm->_interface->restoreMode();

	_vm->_gfx->getCurrentPal(cur_pal);

	event.type = IMMEDIATE_EVENT;
	event.code = PAL_EVENT;
	event.op = EVENT_PALTOBLACK;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = cur_pal;

	q_event = _vm->_events->queue(&event);

	event.type = ONESHOT_EVENT;
	event.code = GRAPHICS_EVENT;
	event.op = EVENT_CLEARFLAG;
	event.param = RF_PLACARD;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = TEXT_EVENT;
	event.op = EVENT_REMOVE;
	event.data = placardTextEntry;

	q_event = _vm->_events->chain(q_event, &event);

	_vm->_scene->getBGPal(&pal);

	event.type = IMMEDIATE_EVENT;
	event.code = PAL_EVENT;
	event.op = EVENT_BLACKTOPAL;
	event.time = 0;
	event.duration = kNormalFadeDuration;
	event.data = pal;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = CURSOR_EVENT;
	event.op = EVENT_SHOW;

	q_event = _vm->_events->chain(q_event, &event);

	event.type = ONESHOT_EVENT;
	event.code = SCRIPT_EVENT;
	event.op = EVENT_THREAD_WAKE;
	event.param = kWaitTypePlacard;

	q_event = _vm->_events->chain(q_event, &event);

}

// Script function #50 (0x32)
void Script::sfSetProtagState(SCRIPTFUNC_PARAMS) {
	int protagState = thread->pop();

	_vm->_actor->setProtagState(protagState);
}

// Script function #51 (0x33)
void Script::sfResumeBgdAnim(SCRIPTFUNC_PARAMS) {
	int16 animId = thread->pop();
	int16 cycles = thread->pop();

	_vm->_anim->resume(animId, cycles);
	debug(1, "sfResumeBgdAnimSpeed(%d, %d)", animId, cycles);

}

// Script function #52 (0x34)
void Script::SF_throwActor(SCRIPTFUNC_PARAMS) {
	int param1, param2, param3, param4, param5, param6;

	param1 = thread->pop();
	param2 = thread->pop();
	param3 = thread->pop();
	param4 = thread->pop();
	param5 = thread->pop();
	param6 = thread->pop();

	debug(0, "STUB: SF_throwActor(%d, %d, %d, %d, %d, %d)", param1, param2, param3, param4, param5, param6);
}

// Script function #53 (0x35)
// Param1: actor id
// Param2: target object
void Script::sfWaitWalk(SCRIPTFUNC_PARAMS) {
	int16 actorId;
	ActorData *actor;

	actorId = thread->pop();
	actor = _vm->_actor->getActor(actorId);

	if ((actor->currentAction == kActionWalkToPoint) ||
		(actor->currentAction == kActionWalkToLink) ||
		(actor->currentAction == kActionFall)) {
			thread->waitWalk(actor);
	}
}

// Script function #54 (0x36)
void Script::sfScriptSceneID(SCRIPTFUNC_PARAMS) {
	thread->_returnValue = _vm->_scene->currentSceneNumber();
}

// Script function #55 (0x37)
void Script::SF_changeActorScene(SCRIPTFUNC_PARAMS) {
	int param1 = thread->pop();
	int param2 = thread->pop();

	debug(0, "STUB: SF_changeActorScene(%d, %d)", param1, param2);
}

// Script function #56 (0x38)
void Script::SF_climb(SCRIPTFUNC_PARAMS) {
	int param1 = thread->pop();
	int param2 = thread->pop();
	int param3 = thread->pop();
	int param4 = thread->pop();

	debug(0, "STUB: SF_climb(%d, %d, %d, %d)", param1, param2, param3, param4);
}

// Script function #57 (0x39)
// Param1: door #
// Param2: door state
void Script::sfSetDoorState(SCRIPTFUNC_PARAMS) {
	int16 doorNumber;
	int16 doorState;
	doorNumber = thread->pop();
	doorState = thread->pop();

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->setTileDoorState(doorNumber, doorState);
	} else {
		_vm->_scene->setDoorState(doorNumber, doorState);
	}
}

// Script function #58 (0x3A)
void Script::SF_setActorZ(SCRIPTFUNC_PARAMS) {
	int param1 = thread->pop();
	int param2 = thread->pop();

	debug(0, "STUB: SF_setActorZ(%d, %d)", param1, param2);
}

// Script function #59 (0x3B)
void Script::SF_text(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_text(), %d args", nArgs);
}

// Script function #60 (0x3C)
void Script::SF_getActorX(SCRIPTFUNC_PARAMS) {
	int16 param = thread->pop();

	debug(0, "STUB: SF_getActorX(%d)", param);
}

// Script function #61 (0x3D)
void Script::SF_getActorY(SCRIPTFUNC_PARAMS) {
	int16 param = thread->pop();

	debug(0, "STUB: SF_getActorY(%d)", param);
}

// Script function #62 (0x3E)
void Script::SF_eraseDelta(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_eraseDelta(), %d args", nArgs);
}

// Script function #63 (0x3F)
void Script::sfPlayMusic(SCRIPTFUNC_PARAMS) {
	if (_vm->getGameType() == GType_ITE) {
		int16 param = thread->pop() + 9;

		if (param >= 9 && param <= 34)
			_vm->_music->play(param);
		else
			_vm->_music->stop();
	} else {
		int16 param1 = thread->pop();
		int16 param2 = thread->pop();

		debug(0, "STUB: sfPlayMusic(%d, %d)", param1, param2);
	}

}

// Script function #64 (0x40)
void Script::SF_pickClimbOutPos(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_pickClimbOutPos(), %d args", nArgs);
}

// Script function #65 (0x41)
void Script::SF_tossRif(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_tossRif(), %d args", nArgs);
}

// Script function #66 (0x42)
void Script::SF_showControls(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_showControls(), %d args", nArgs);
}

// Script function #67 (0x43)
void Script::SF_showMap(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_showMap(), %d args", nArgs);
}

// Script function #68 (0x44)
void Script::SF_puzzleWon(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_puzzleWon(), %d args", nArgs);
}

// Script function #69 (0x45)
void Script::sfEnableEscape(SCRIPTFUNC_PARAMS) {
	if (thread->pop())
		_abortEnabled = true;
	else {
		_skipSpeeches = false;
		_abortEnabled = false;
	}
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
void Script::sfPlaySound(SCRIPTFUNC_PARAMS) {
	int16 param = thread->pop();
	int res;

	if (param < ARRAYSIZE(sfxTable)) {
		res = sfxTable[param].res;
		if (_vm->getFeatures() & GF_CD_FX)
			res -= 14;

		_vm->_sndRes->playSound(res, sfxTable[param].vol, false);
	} else {
		_vm->_sound->stopSound();
	}

}

// Script function #71 (0x47)
void Script::SF_playLoopedSound(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_playLoopedSound(), %d args", nArgs);
}

// Script function #72 (0x48)
void Script::sfGetDeltaFrame(SCRIPTFUNC_PARAMS) {
	uint16 animId = (uint16)thread->pop();

	thread->_returnValue = _vm->_anim->getCurrentFrame(animId);
}

// Script function #73 (0x49)
void Script::SF_showProtect(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_showProtect(), %d args", nArgs);
}

// Script function #74 (0x4A)
void Script::SF_protectResult(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_protectResult(), %d args", nArgs);
}

// Script function #75 (0x4b)
void Script::sfRand(SCRIPTFUNC_PARAMS) {
	int16 param;

	if (_vm->getGameType() == GType_IHNM) {
		// I don't know what this function does in IHNM, but apparently
		// it can take three parameters.

		debug(0, "STUB: sfRand()");

		for (int i = 0; i < nArgs; i++) {
			thread->pop();
		}

		thread->_returnValue = 0;
	} else {
		param = thread->pop();
		thread->_returnValue = _vm->_rnd.getRandomNumber(param - 1);
	}
}

// Script function #76 (0x4c)
void Script::SF_fadeMusic(SCRIPTFUNC_PARAMS) {
	debug(0, "STUB: SF_fadeMusic()");
}

// Script function #77 (0x4d)
void Script::SF_playVoice(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_playVoice(), %d args", nArgs);
}

void Script::finishDialog(int replyID, int flags, int bitOffset) {
	byte *addr;

	if (_conversingThread) {
		_vm->_interface->setMode(kPanelNull);

		_conversingThread->_flags &= ~kTFlagWaiting;

		_conversingThread->push(replyID);

		if (flags & kReplyOnce) {
			addr = _conversingThread->_staticBase + (bitOffset >> 3);
			*addr |=  (1 << (bitOffset & 7));
		}
	}

	_conversingThread = NULL;
	wakeUpThreads(kWaitTypeDialogBegin);
}

void Script::SF_stub(SCRIPTFUNC_PARAMS) {
	for (int i = 0; i < nArgs; i++)
		thread->pop();

	debug(0, "STUB: SF_stub(), %d args", nArgs);
}

} // End of namespace Saga
