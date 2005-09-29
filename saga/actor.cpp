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

#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/console.h"
#include "saga/script.h"
#include "saga/sndres.h"
#include "saga/sprite.h"
#include "saga/font.h"
#include "saga/sound.h"
#include "saga/scene.h"

#include "saga/isomap.h"
#include "saga/actor.h"
#include "saga/itedata.h"
#include "saga/stream.h"
#include "saga/interface.h"
#include "saga/events.h"
#include "saga/objectmap.h"
#include "saga/rscfile.h"
#include "saga/resnames.h"

#include "common/config-manager.h"

namespace Saga {

static int commonObjectCompare(const CommonObjectDataPointer& obj1, const CommonObjectDataPointer& obj2) {
	int p1 = obj1->_location.y - obj1->_location.z;
	int p2 = obj2->_location.y - obj2->_location.z;
	if (p1 == p2)
		return 0;
	if (p1 < p2)
		return -1;
	return 1;
}

static int tileCommonObjectCompare(const CommonObjectDataPointer& obj1, const CommonObjectDataPointer& obj2) {
	int p1 = -obj1->_location.u() - obj1->_location.v() - obj1->_location.z;
	int p2 = -obj2->_location.u() - obj2->_location.v() - obj2->_location.z;
	//TODO:  for kObjNotFlat obj Height*3 of sprite should be added to p1 and p2
	//if (validObjId(obj1->id)) {

	if (p1 == p2)
		return 0;
	if (p1 < p2)
		return -1;
	return 1;
}

// Lookup table to convert 8 cardinal directions to 4
static const int actorDirectectionsLUT[8] = {
	ACTOR_DIRECTION_BACK,	// kDirUp
	ACTOR_DIRECTION_RIGHT,	// kDirUpRight
	ACTOR_DIRECTION_RIGHT,	// kDirRight
	ACTOR_DIRECTION_RIGHT,	// kDirDownRight
	ACTOR_DIRECTION_FORWARD,// kDirDown
	ACTOR_DIRECTION_LEFT,	// kDirDownLeft
	ACTOR_DIRECTION_LEFT,	// kDirLeft
	ACTOR_DIRECTION_LEFT,	// kDirUpLeft
};

static const PathDirectionData pathDirectionLUT[8][3] = {
	{ { 0,  0, -1 }, { 7, -1, -1 }, { 4,  1, -1 } },
	{ { 1,  1,  0 }, { 4,  1, -1 }, { 5,  1,  1 } },
	{ { 2,  0,  1 }, { 5,  1,  1 }, { 6, -1,  1 } },
	{ { 3, -1,  0 }, { 6, -1,  1 }, { 7, -1, -1 } },
	{ { 0,  0, -1 }, { 1,  1,  0 }, { 4,  1, -1 } },
	{ { 1,  1,  0 }, { 2,  0,  1 }, { 5,  1,  1 } },
	{ { 2,  0,  1 }, { 3, -1,  0 }, { 6, -1,  1 } },
	{ { 3, -1,  0 }, { 0,  0, -1 }, { 7, -1, -1 } }
};

static const int pathDirectionLUT2[8][2] = {
	{  0, -1 },
	{  1,  0 },
	{  0,  1 },
	{ -1,  0 },
	{  1, -1 },
	{  1,  1 },
	{ -1,  1 },
	{ -1, -1 }
};

static const int angleLUT[16][2] = {
	{    0, -256 },
	{   98, -237 },
	{  181, -181 },
	{  237,  -98 },
	{  256,    0 },
	{  237,	  98 },
	{  181,  181 },
	{   98,  237 },
	{    0,  256 },
	{  -98,  237 },
	{ -181,  181 },
	{ -237,   98 },
	{ -256,    0 },
	{ -237,  -98 },
	{ -181, -181 },
	{  -98, -237 }
};

static const int directionLUT[8][2] = {
	{  0 * 2, -2 * 2 },
	{  2 * 2, -1 * 2 },
	{  3 * 2,  0 * 2 },
	{  2 * 2,  1 * 2 },
	{  0 * 2,  2 * 2 },
	{ -2 * 2,  1 * 2 },
	{ -4 * 2,  0 * 2 },
	{ -2 * 2, -1 * 2 }
};

static const int tileDirectionLUT[8][2] = {
	{  1,  1 },
	{  2,  0 },
	{  1, -1 },
	{  0, -2 },
	{ -1, -1 },
	{ -2,  0 },
	{ -1,  1 },
	{  0,  2 }
};

struct DragonMove {
	uint16 baseFrame;
	int16 offset[4][2];
};

static const DragonMove dragonMoveTable[12] = {
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{  0, { {  0,  0 }, {  0,  0 }, {   0,   0 }, {   0,   0 } } },
	{ 28, { { -0,  0 }, { -1,  6 }, {  -5,  11 }, { -10,  15 } } },
	{ 56, { {  0,  0 }, {  1,  6 }, {   5,  11 }, {  10,  15 } } },
	{ 40, { {  0,  0 }, {  6,  1 }, {  11,   5 }, {  15,  10 } } },
	{ 44, { {  0,  0 }, {  6, -1 }, {  11,  -5 }, {  15, -10 } } },
	{ 32, { { -0, -0 }, { -6, -1 }, { -11,  -5 }, { -15, -10 } } },
	{ 52, { { -0,  0 }, { -6,  1 }, { -11,   5 }, { -15,  10 } } },
	{ 36, { {  0, -0 }, {  1, -6 }, {   5, -11 }, {  10, -15 } } },
	{ 48, { { -0, -0 }, { -1, -6 }, {  -5, -11 }, { -10, -15 } } }
};

Actor::Actor(SagaEngine *vm) : _vm(vm) {
	int i;
	byte *stringsPointer;
	size_t stringsLength;
	ActorData *actor;
	ObjectData *obj;
	debug(9, "Actor::Actor()");
	_handleActionDiv = 15;

	_actors = NULL;
	_actorsCount = 0;

	_objs = NULL;
	_objsCount = 0;

#ifdef ACTOR_DEBUG
	_debugPoints = NULL;
	_debugPointsAlloced = _debugPointsCount = 0;
#endif

	_protagStates = 0;
	_protagStatesCount = 0;

	_pathNodeList = _newPathNodeList = NULL;
	_pathList = NULL;
	_pathDirectionList = NULL;
	_pathListAlloced = _pathNodeListAlloced = _newPathNodeListAlloced = 0;
	_pathListIndex = _pathNodeListIndex = _newPathNodeListIndex = -1;
	_pathDirectionListCount = 0;
	_pathDirectionListAlloced = 0;

	_centerActor = _protagonist = NULL;
	_protagState = 0;
	_lastTickMsec = 0;

	_yCellCount = _vm->getSceneHeight();
	_xCellCount = _vm->getDisplayWidth();

	_pathCell = (int8 *)malloc(_yCellCount * _xCellCount * sizeof(*_pathCell));

	_pathRect.left = 0;
	_pathRect.right = _vm->getDisplayWidth();
	_pathRect.top = _vm->getDisplayInfo().pathStartY;
	_pathRect.bottom = _vm->getSceneHeight();

	// Get actor resource file context
	_actorContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (_actorContext == NULL) {
		error("Actor::Actor() resource context not found");
	}

	if (_vm->getGameType() == GType_ITE) {

		_vm->_resource->loadResource(_actorContext, _vm->getResourceDescription()->actorsStringsResourceId, stringsPointer, stringsLength);

		_vm->loadStrings(_actorsStrings, stringsPointer, stringsLength);
		free(stringsPointer);
	} else {
		// TODO
	}

	if (_vm->getGameType() == GType_ITE) {
		_actorsCount = ITE_ACTORCOUNT;
		_actors = (ActorData **)malloc(_actorsCount * sizeof(*_actors));
		for (i = 0; i < _actorsCount; i++) {
			actor = _actors[i] = new ActorData();
			actor->_id = actorIndexToId(i);
			actor->_index = i;
			debug(9, "init actor id=%d index=%d", actor->_id, actor->_index);
			actor->_nameIndex = ITE_ActorTable[i].nameIndex;
			actor->_scriptEntrypointNumber = ITE_ActorTable[i].scriptEntrypointNumber;
			actor->_spriteListResourceId = ITE_ActorTable[i].spriteListResourceId;
			actor->_frameListResourceId = ITE_ActorTable[i].frameListResourceId;
			actor->_speechColor = ITE_ActorTable[i].speechColor;
			actor->_sceneNumber = ITE_ActorTable[i].sceneIndex;
			actor->_flags = ITE_ActorTable[i].flags;
			actor->_currentAction = ITE_ActorTable[i].currentAction;
			actor->_facingDirection = ITE_ActorTable[i].facingDirection;
			actor->_actionDirection = ITE_ActorTable[i].actionDirection;

			actor->_location.x = ITE_ActorTable[i].x;
			actor->_location.y = ITE_ActorTable[i].y;
			actor->_location.z = ITE_ActorTable[i].z;

			actor->_disabled = !loadActorResources(actor);
			if (actor->_disabled) {
				warning("Disabling actor Id=%d index=%d", actor->_id, actor->_index);
			}
		}
		_objsCount = ITE_OBJECTCOUNT;
		_objs = (ObjectData **)malloc(_objsCount * sizeof(*_objs));
		for (i = 0; i < _objsCount; i++) {
			obj = _objs[i] = new ObjectData();
			obj->_id = objIndexToId(i);
			obj->_index = i;
			debug(9, "init obj id=%d index=%d", obj->_id, obj->_index);
			obj->_nameIndex = ITE_ObjectTable[i].nameIndex;
			obj->_scriptEntrypointNumber = ITE_ObjectTable[i].scriptEntrypointNumber;
			obj->_spriteListResourceId = ITE_ObjectTable[i].spriteListResourceId;
			obj->_sceneNumber = ITE_ObjectTable[i].sceneIndex;
			obj->_interactBits = ITE_ObjectTable[i].interactBits;

			obj->_location.x = ITE_ObjectTable[i].x;
			obj->_location.y = ITE_ObjectTable[i].y;
			obj->_location.z = ITE_ObjectTable[i].z;
		}
	} else {
		// TODO. This is causing problems for SYMBIAN os as it doesn't like a static class here
		ActorData dummyActor;

		dummyActor._frames = NULL;
		dummyActor._walkStepsPoints = NULL;

		_protagonist = &dummyActor;
	}

	_dragonHunt = true;
}

Actor::~Actor() {
	debug(9, "Actor::~Actor()");

#ifdef ACTOR_DEBUG
	free(_debugPoints);
#endif
	free(_pathDirectionList);
	free(_pathNodeList);
	free(_newPathNodeList);
	free(_pathList);
	free(_pathCell);
	_actorsStrings.freeMem();
	//release resources
	freeActorList();
	freeObjList();
}

bool Actor::loadActorResources(ActorData *actor) {
	byte *resourcePointer;
	size_t resourceLength;
	int framesCount;
	ActorFrameSequence *framesPointer;
	int lastFrame = 0;
	int orient;
	int resourceId;
	bool gotSomething = false;

	if (actor->_frameListResourceId) {
		debug(9, "Loading frame resource id %d", actor->_frameListResourceId);
		_vm->_resource->loadResource(_actorContext, actor->_frameListResourceId, resourcePointer, resourceLength);

		framesCount = resourceLength / 16;
		debug(9, "Frame resource contains %d frames (res length is %d)", framesCount, resourceLength);

		framesPointer = (ActorFrameSequence *)malloc(sizeof(ActorFrameSequence) * framesCount);
		if (framesPointer == NULL && framesCount != 0) {
			memoryError("Actor::loadActorResources");
		}

		MemoryReadStreamEndian readS(resourcePointer, resourceLength, _actorContext->isBigEndian);

		for (int i = 0; i < framesCount; i++) {
			for (orient = 0; orient < ACTOR_DIRECTIONS_COUNT; orient++) {
				// Load all four orientations
				framesPointer[i].directions[orient].frameIndex = readS.readUint16();
				framesPointer[i].directions[orient].frameCount = readS.readSint16();
				if (framesPointer[i].directions[orient].frameCount < 0)
					warning("frameCount < 0 (%d)", framesPointer[i].directions[orient].frameCount);
				if (framesPointer[i].directions[orient].frameIndex > lastFrame) {
					lastFrame = framesPointer[i].directions[orient].frameIndex;
				}
			}
		}

		free(resourcePointer);

		actor->_frames = framesPointer;
		actor->_framesCount = framesCount;

		gotSomething = true;
	} else {
		warning("Frame List ID = 0 for actor index %d", actor->_index);

		if (_vm->getGameType() == GType_ITE)
			return true;
	}

	resourceId = actor->_spriteListResourceId;

	if (resourceId) {
		debug(9, "Loading sprite resource id %d", resourceId);

		_vm->_sprite->loadList(resourceId, actor->_spriteList);

		if (actor->_flags & kExtended) {
			while ((lastFrame >= actor->_spriteList.spriteCount)) {
				resourceId++;
				debug(9, "Appending to sprite list %d", resourceId);
				_vm->_sprite->loadList(resourceId, actor->_spriteList);
			}
		}

		gotSomething = true;
	} else {
		warning("Sprite List ID = 0 for actor index %d", actor->_index);
	}

	return gotSomething;
}

void Actor::freeActorList() {
	int i;
	ActorData *actor;
	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		delete actor;
	}
	free(_actors);
	_actors = NULL;
	_actorsCount = 0;
}

void Actor::loadActorList(int protagonistIdx, int actorCount, int actorsResourceID, int protagStatesCount, int protagStatesResourceID) {
	int i, j;
	ActorData *actor;
	byte* actorListData;
	size_t actorListLength;
	byte walk[128];
	byte acv[6];
	int movementSpeed;
	int walkStepIndex;
	int walkStepCount;

	freeActorList();
	
	_vm->_resource->loadResource(_actorContext, actorsResourceID, actorListData, actorListLength);
		
	_actorsCount = actorCount;

	if ((int)(actorListLength / ACTOR_INHM_SIZE) < _actorsCount) {
		error("Actor::loadActorList wrong actorlist length");
	}

	MemoryReadStream actorS(actorListData, actorListLength);
	
	_actors = (ActorData **)malloc(_actorsCount * sizeof(*_actors));
	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i] = new ActorData();
		actor->_id = actorIndexToId(i);
		actor->_index = i;
		debug(9, "init actor id=%d index=%d", actor->_id, actor->_index);
		actorS.readUint32LE(); //next displayed	
		actorS.readByte(); //type
		actor->_flags = actorS.readByte();
		actor->_nameIndex = actorS.readUint16LE();
		actor->_sceneNumber = actorS.readUint32LE();
		actor->_location.fromStream(actorS);
		actor->_screenPosition.x = actorS.readUint16LE();
		actor->_screenPosition.y = actorS.readUint16LE();
		actor->_screenScale = actorS.readUint16LE();
		actor->_screenDepth = actorS.readUint16LE();
		actor->_spriteListResourceId = actorS.readUint32LE();
		actor->_frameListResourceId = actorS.readUint32LE();
		debug(0, "%d: %d, %d", i, actor->_spriteListResourceId, actor->_frameListResourceId);
		actor->_scriptEntrypointNumber = actorS.readUint32LE();
		actorS.readByte();
		actorS.readByte();
		actorS.readByte();
		actorS.readByte();
		actorS.readUint16LE(); //LEFT
		actorS.readUint16LE(); //RIGHT
		actorS.readUint16LE(); //TOP
		actorS.readUint16LE(); //BOTTOM
		actor->_speechColor = actorS.readByte();
		actor->_currentAction = actorS.readByte();
		actor->_facingDirection = actorS.readByte();
		actor->_actionDirection = actorS.readByte();
		actor->_actionCycle = actorS.readUint16LE();
		actor->_frameNumber = actorS.readUint16LE();
		actor->_finalTarget.fromStream(actorS);
		actor->_partialTarget.fromStream(actorS);
		movementSpeed = actorS.readUint16LE(); //movement speed
		if (movementSpeed) {
			error("Actor::loadActorList movementSpeed != 0");
		}
		actorS.read(walk, 128);
		for (j = 0; j < 128; j++) {
			if (walk[j]) {
				error("Actor::loadActorList walk[128] != 0");
			}
		}
		//actorS.seek(128, SEEK_CUR);
		walkStepCount = actorS.readByte();//walkStepCount
		if (walkStepCount) {
			error("Actor::loadActorList walkStepCount != 0");
		}
		walkStepIndex = actorS.readByte();//walkStepIndex
		if (walkStepIndex) {
			error("Actor::loadActorList walkStepIndex != 0");
		}
		//no need to check pointers
		actorS.readUint32LE(); //sprites
		actorS.readUint32LE(); //frames
		actorS.readUint32LE(); //last zone
		actor->_targetObject = actorS.readUint16LE();
		actor->_actorFlags = actorS.readUint16LE();
		//no need to check pointers
		actorS.readUint32LE(); //next in scene
		actorS.read(acv, 6);
		for (j = 0; j < 6; j++) {
			if (acv[j]) {
				error("Actor::loadActorList acv[%d] != 0", j);
			}
		}
//		actorS.seek(6, SEEK_CUR); //action vars
	}
	free(actorListData);

	_actors[protagonistIdx]->_flags |= kProtagonist | kExtended;

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		if (actor->_flags & kExtended) {
			loadActorResources(actor);

			if (actor->_disabled) {
				warning("Disabling actor Id=%d index=%d", actor->_id, actor->_index);
			}
			break;
		}
	}

	_centerActor = _protagonist = _actors[protagonistIdx];
	_protagState = 0;

	if (protagStatesResourceID) {
		free(_protagStates);

		_protagStates = (ActorFrameSequence *)malloc(sizeof(ActorFrameSequence) * protagStatesCount);

		byte *resourcePointer;
		size_t resourceLength;

		_vm->_resource->loadResource(_actorContext, protagStatesResourceID,
									 resourcePointer, resourceLength);


		MemoryReadStream statesS(resourcePointer, resourceLength);
		
		for (i = 0; i < protagStatesCount; i++) {
			for (j = 0; j < ACTOR_DIRECTIONS_COUNT; j++) {
				_protagStates[i].directions[j].frameIndex = statesS.readUint16LE();
				_protagStates[i].directions[j].frameCount = statesS.readUint16LE();
			}
		}
		free(resourcePointer);

		_protagonist->_frames = &_protagStates[_protagState];
	}

	_protagStatesCount = protagStatesCount;
}

void Actor::freeObjList() {
	int i;
	ObjectData *object;
	for (i = 0; i < _objsCount; i++) {
		object = _objs[i];
		delete object;
	}
	free(_objs);
	_objs = NULL;
	_objsCount = 0;
}

void Actor::loadObjList(int objectCount, int objectsResourceID) {
	int i;
	int frameListResourceId;
	ObjectData *object;
	byte* objectListData;
	size_t objectListLength;
	freeObjList();
	
	_vm->_resource->loadResource(_actorContext, objectsResourceID, objectListData, objectListLength);
		
	_objsCount = objectCount;

	MemoryReadStream objectS(objectListData, objectListLength);
	
	_objs = (ObjectData **)malloc(_objsCount * sizeof(*_objs));
	for (i = 0; i < _objsCount; i++) {
		object = _objs[i] = new ObjectData();
		object->_id = objectIndexToId(kGameObjectObject, i);
		object->_index = i;
		debug(9, "init object id=%d index=%d", object->_id, object->_index);
		objectS.readUint32LE(); //next displayed	
		objectS.readByte(); //type
		object->_flags = objectS.readByte();
		object->_nameIndex = objectS.readUint16LE();
		object->_sceneNumber = objectS.readUint32LE();
		object->_location.fromStream(objectS);
		object->_screenPosition.x = objectS.readUint16LE();
		object->_screenPosition.y = objectS.readUint16LE();
		object->_screenScale = objectS.readUint16LE();
		object->_screenDepth = objectS.readUint16LE();
		object->_spriteListResourceId = objectS.readUint32LE();
		frameListResourceId = objectS.readUint32LE(); // object->_frameListResourceId
		if (frameListResourceId) {
			error("Actor::loadObjList frameListResourceId != 0");
		}
		object->_scriptEntrypointNumber = objectS.readUint32LE();
		objectS.readByte();
		objectS.readByte();
		objectS.readByte();
		objectS.readByte();
		objectS.readUint16LE(); //LEFT
		objectS.readUint16LE(); //RIGHT
		objectS.readUint16LE(); //TOP
		objectS.readUint16LE(); //BOTTOM
		object->_interactBits = objectS.readUint16LE();
	}
	free(objectListData);
}

void Actor::takeExit(uint16 actorId, const HitZone *hitZone) {
	ActorData *actor;
	actor = getActor(actorId);
	actor->_lastZone = NULL;

	_vm->_scene->changeScene(hitZone->getSceneNumber(), hitZone->getActorsEntrance(), kTransitionNoFade);
	if (_vm->_interface->getMode() != kPanelSceneSubstitute) {
		_vm->_script->setNoPendingVerb();
	}
}

void Actor::stepZoneAction(ActorData *actor, const HitZone *hitZone, bool exit, bool stopped) {
	Event event;

	if (actor != _protagonist) {
		return;
	}
	if (((hitZone->getFlags() & kHitZoneTerminus) && !stopped) || (!(hitZone->getFlags() & kHitZoneTerminus) && stopped)) {
		return;
	}

	if (!exit) {
		if (hitZone->getFlags() & kHitZoneAutoWalk) {
			actor->_currentAction = kActionWalkDir;
			actor->_actionDirection = actor->_facingDirection = hitZone->getDirection();
			actor->_walkFrameSequence = kFrameWalk;
			return;
		}
	} else if (!(hitZone->getFlags() & kHitZoneAutoWalk)) {
		return;
	}
	if (hitZone->getFlags() & kHitZoneExit) {
		takeExit(actor->_id, hitZone);
	} else if (hitZone->getScriptNumber() > 0) {
		event.type = kEvTOneshot;
		event.code = kScriptEvent;
		event.op = kEventExecNonBlocking;
		event.time = 0;
		event.param = _vm->_scene->getScriptModuleNumber(); // module number
		event.param2 = hitZone->getScriptNumber();			// script entry point number
		event.param3 = kVerbEnter;		// Action
		event.param4 = ID_NOTHING;		// Object
		event.param5 = ID_NOTHING;		// With Object
		event.param6 = ID_PROTAG;		// Actor

		_vm->_events->queue(&event);
	}
}

void Actor::realLocation(Location &location, uint16 objectId, uint16 walkFlags) {
	int angle;
	int distance;
	ActorData *actor;
	ObjectData *obj;
	debug (8, "Actor::realLocation objectId=%i", objectId);
	if (walkFlags & kWalkUseAngle) {
		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			angle = (location.x + 2) & 15;
			distance = location.y;

			location.u() = (angleLUT[angle][0] * distance) >> 8;
			location.v() = -(angleLUT[angle][1] * distance) >> 8;
		} else {
			angle = location.x & 15;
			distance = location.y;

			location.x = (angleLUT[angle][0] * distance) >> 6;
			location.y = (angleLUT[angle][1] * distance) >> 6;
		}
	}

	if (objectId != ID_NOTHING) {
		if (validActorId(objectId)) {
			actor = getActor(objectId);
			location.addXY(actor->_location);
		} else if (validObjId(objectId)) {
			obj = getObj(objectId);
			location.addXY(obj->_location);
		}
	}
}

void Actor::actorFaceTowardsPoint(uint16 actorId, const Location &toLocation) {
	ActorData *actor;
	Location delta;
	//debug (8, "Actor::actorFaceTowardsPoint actorId=%i", actorId);
	actor = getActor(actorId);

	toLocation.delta(actor->_location, delta);

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		if (delta.u() > 0) {
			actor->_facingDirection = (delta.v() > 0) ? kDirUp : kDirRight;
		} else {
			actor->_facingDirection = (delta.v() > 0) ? kDirLeft : kDirDown;
		}
	} else {
		if (ABS(delta.y) > ABS(delta.x * 2)) {
			actor->_facingDirection = (delta.y > 0) ? kDirDown : kDirUp;
		} else {
			actor->_facingDirection = (delta.x > 0) ? kDirRight : kDirLeft;
		}
	}
}

void Actor::actorFaceTowardsObject(uint16 actorId, uint16 objectId) {
	ActorData *actor;
	ObjectData *obj;

	if (validActorId(objectId)) {
		actor = getActor(objectId);
		actorFaceTowardsPoint(actorId, actor->_location);
	} else if (validObjId(objectId)) {
		obj = getObj(objectId);
		actorFaceTowardsPoint(actorId, obj->_location);
	}
}


ObjectData *Actor::getObj(uint16 objId) {
	ObjectData *obj;

	if (!validObjId(objId))
		error("Actor::getObj Wrong objId 0x%X", objId);

	obj = _objs[objIdToIndex(objId)];

	if (obj->_disabled)
		error("Actor::getObj disabled objId 0x%X", objId);

	return obj;
}

ActorData *Actor::getActor(uint16 actorId) {
	ActorData *actor;

	if (!validActorId(actorId)) {
		warning("Actor::getActor Wrong actorId 0x%X", actorId);
		assert(0);
	}

	if (actorId == ID_PROTAG) {
		if (_protagonist == NULL) {
			error("_protagonist == NULL");
		}
		return _protagonist;
	}

	actor = _actors[actorIdToIndex(actorId)];

	if (actor->_disabled)
		error("Actor::getActor disabled actorId 0x%X", actorId);

	return actor;
}

bool Actor::validFollowerLocation(const Location &location) {
	Point point;
	location.toScreenPointXY(point);

	if ((point.x < 5) || (point.x >= _vm->getDisplayWidth() - 5) ||
		(point.y < 0) || (point.y > _vm->getSceneHeight())) {
		return false;
	}

	return (_vm->_scene->canWalk(point));
}

void Actor::setProtagState(int state) {
	_protagState = state;

	if (_vm->getGameType() == GType_IHNM)
		_protagonist->_frames = &_protagStates[state];
}

void Actor::updateActorsScene(int actorsEntrance) {
	int i, j;
	int followerDirection;
	ActorData *actor;
	Location tempLocation;
	Location possibleLocation;
	Point delta;
	const SceneEntry *sceneEntry;

	if (_vm->_scene->currentSceneNumber() == 0) {
		error("Actor::updateActorsScene _vm->_scene->currentSceneNumber() == 0");
	}

	_vm->_sound->stopVoice();
	_activeSpeech.stringsCount = 0;
	_activeSpeech.playing = false;
	_protagonist = NULL;

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		actor->_inScene = false;
		if (actor->_disabled) {
			continue;
		}
		if ((actor->_flags & (kProtagonist | kFollower)) || (i == 0)) {
			if (actor->_flags & kProtagonist) {
				actor->_finalTarget = actor->_location;
				_centerActor = _protagonist = actor;
			} else if (_vm->getGameType() == GType_ITE && 
					   _vm->_scene->currentSceneResourceId() == RID_ITE_OVERMAP_SCENE) {
				continue;
			}

			actor->_sceneNumber = _vm->_scene->currentSceneNumber();
		}
		if (actor->_sceneNumber == _vm->_scene->currentSceneNumber()) {
			actor->_inScene = true;
			actor->_actionCycle = (_vm->_rnd.getRandomNumber(7) & 0x7) * 4; // 1/8th chance
		}
	}

	assert(_protagonist);

	if ((actorsEntrance >= 0) && (_vm->_scene->_entryList.entryListCount > 0)) {
		if (_vm->_scene->_entryList.entryListCount <= actorsEntrance) {
			actorsEntrance = 0; //OCEAN bug
		}

		sceneEntry = _vm->_scene->_entryList.getEntry(actorsEntrance);
		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			_protagonist->_location = sceneEntry->location;
		} else {
			_protagonist->_location.x = sceneEntry->location.x * ACTOR_LMULT;
			_protagonist->_location.y = sceneEntry->location.y * ACTOR_LMULT;
			_protagonist->_location.z = sceneEntry->location.z * ACTOR_LMULT;
		}
		_protagonist->_facingDirection = _protagonist->_actionDirection = sceneEntry->facing;
	}

	_protagonist->_currentAction = kActionWait;

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		//nothing?
	} else {
		_vm->_scene->initDoorsState(); //TODO: move to _scene
	}

	followerDirection = _protagonist->_facingDirection + 3;
	calcScreenPosition(_protagonist);

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		if (actor->_flags & (kFollower)) {
			actor->_facingDirection = actor->_actionDirection = _protagonist->_facingDirection;
			actor->_currentAction = kActionWait;
			actor->_walkStepsCount = actor->_walkStepIndex = 0;
			actor->_location.z = _protagonist->_location.z;


			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				_vm->_isoMap->placeOnTileMap(_protagonist->_location, actor->_location, 3, followerDirection & 0x07);
			} else {
				followerDirection &= 0x07;

				possibleLocation = _protagonist->_location;


				delta.x = directionLUT[followerDirection][0];
				delta.y = directionLUT[followerDirection][1];


				for (j = 0; j < 30; j++) {
					tempLocation = possibleLocation;
					tempLocation.x += delta.x;
					tempLocation.y += delta.y;

					if (validFollowerLocation( tempLocation)) {
						possibleLocation = tempLocation;
					} else {
						tempLocation = possibleLocation;
						tempLocation.x += delta.x;
						if (validFollowerLocation( tempLocation)) {
							possibleLocation = tempLocation;
						} else {
							tempLocation = possibleLocation;
							tempLocation.y += delta.y;
							if (validFollowerLocation( tempLocation)) {
								possibleLocation = tempLocation;
							} else {
								break;
							}
						}
					}
				}

				actor->_location = possibleLocation;
			}
			followerDirection += 2;
		}

	}

	handleActions(0, true);
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->adjustScroll(true);
	}
}

ActorFrameRange *Actor::getActorFrameRange(uint16 actorId, int frameType) {
	ActorData *actor;
	int fourDirection;
	static ActorFrameRange def = {0, 0};

	actor = getActor(actorId);
	if (actor->_disabled)
		error("Actor::getActorFrameRange Wrong actorId 0x%X", actorId);

	if (frameType >= actor->_framesCount) {
		warning("Actor::getActorFrameRange Wrong frameType 0x%X (%d) actorId 0x%X", frameType, actor->_framesCount, actorId);
		return &def;
	}

	if ((actor->_facingDirection < kDirUp) || (actor->_facingDirection > kDirUpLeft))
		error("Actor::getActorFrameRange Wrong direction 0x%X actorId 0x%X", actor->_facingDirection, actorId);

	fourDirection = actorDirectectionsLUT[actor->_facingDirection];
	return &actor->_frames[frameType].directions[fourDirection];
}

void Actor::handleSpeech(int msec) {
	int stringLength;
	int sampleLength;
	bool removeFirst;
	int i;
	ActorData *actor;
	int width, height, height2;
	Point posPoint;

	if (_activeSpeech.playing) {
		_activeSpeech.playingTime -= msec;
		stringLength = strlen(_activeSpeech.strings[0]);

		removeFirst = false;
		if (_activeSpeech.playingTime <= 0) {
			if (_activeSpeech.speechFlags & kSpeakSlow) {
				_activeSpeech.slowModeCharIndex++;
				if (_activeSpeech.slowModeCharIndex >= stringLength)
					removeFirst = true;
			} else {
				removeFirst = true;
			}
			_activeSpeech.playing = false;
			if (_activeSpeech.actorIds[0] != 0) {
				actor = getActor(_activeSpeech.actorIds[0]);
				if (!(_activeSpeech.speechFlags & kSpeakNoAnimate)) {
					actor->_currentAction = kActionWait;
				}
			}
		}

		if (removeFirst) {
			for (i = 1; i < _activeSpeech.stringsCount; i++) {
				_activeSpeech.strings[i - 1] = _activeSpeech.strings[i];
			}
			_activeSpeech.stringsCount--;
		}

		if (_vm->_script->_skipSpeeches) {
			_activeSpeech.stringsCount = 0;
			_vm->_script->wakeUpThreads(kWaitTypeSpeech);
			return;
		}

		if (_activeSpeech.stringsCount == 0) {
			_vm->_script->wakeUpThreadsDelayed(kWaitTypeSpeech, ticksToMSec(kScriptTimeTicksPerSecond / 3));
		}

		return;
	}

	if (_vm->_script->_skipSpeeches) {
		_activeSpeech.stringsCount = 0;
		_vm->_script->wakeUpThreads(kWaitTypeSpeech);
	}

	if (_activeSpeech.stringsCount == 0) {
		return;
	}

	stringLength = strlen(_activeSpeech.strings[0]);

	if (_activeSpeech.speechFlags & kSpeakSlow) {
		if (_activeSpeech.slowModeCharIndex >= stringLength)
			error("Wrong string index");

		warning("Slow string encountered!");
		_activeSpeech.playingTime = stringLength * 1000 / 4;

	} else {
		sampleLength = _vm->_sndRes->getVoiceLength(_activeSpeech.sampleResourceId);

		if (sampleLength < 0) {
			_activeSpeech.playingTime = stringLength * 1000 / 22;
			switch (_vm->_readingSpeed) {
			case 1:
				_activeSpeech.playingTime *= 2;
				break;
			case 2:
				_activeSpeech.playingTime *= 4;
				break;
			case 3:
				_activeSpeech.playingTime = 0x7fffff;
				break;
			}
		} else {
			_activeSpeech.playingTime = sampleLength;
		}
	}

	if (_activeSpeech.sampleResourceId != -1) {
		_vm->_sndRes->playVoice(_activeSpeech.sampleResourceId);
		_activeSpeech.sampleResourceId++;
	}

	if (_activeSpeech.actorIds[0] != 0) {
		actor = getActor(_activeSpeech.actorIds[0]);
		if (!(_activeSpeech.speechFlags & kSpeakNoAnimate)) {
			actor->_currentAction = kActionSpeak;
			actor->_actionCycle = _vm->_rnd.getRandomNumber(63);
		}
	}

	if (_activeSpeech.actorsCount == 1) {
		if (_speechBoxScript.width() > 0) {
			_activeSpeech.drawRect.left = _speechBoxScript.left;
			_activeSpeech.drawRect.right = _speechBoxScript.right;
			_activeSpeech.drawRect.top = _speechBoxScript.top;
			_activeSpeech.drawRect.bottom = _speechBoxScript.bottom;
		} else {
			width = _activeSpeech.speechBox.width();
			height = _vm->_font->getHeight((_vm->getGameType() == GType_ITE ? kMediumFont : kIHNMMainFont), _activeSpeech.strings[0], width - 2, _activeSpeech.getFontFlags(0)) + 1;

			if (height > 40 && width < _vm->getDisplayWidth() - 100) {
				width = _vm->getDisplayWidth() - 100;
				height = _vm->_font->getHeight((_vm->getGameType() == GType_ITE ? kMediumFont : kIHNMMainFont), _activeSpeech.strings[0], width - 2, _activeSpeech.getFontFlags(0)) + 1;
			}

			_activeSpeech.speechBox.setWidth(width);

			if (_activeSpeech.actorIds[0] != 0) {
				actor = getActor(_activeSpeech.actorIds[0]);
				_activeSpeech.speechBox.setHeight(height);

				if (_activeSpeech.speechBox.right > _vm->getDisplayWidth() - 10) {
					_activeSpeech.drawRect.left = _vm->getDisplayWidth() - 10 - width;
				} else {
					_activeSpeech.drawRect.left = _activeSpeech.speechBox.left;
				}

				height2 =  actor->_screenPosition.y - 50;
				_activeSpeech.speechBox.top = _activeSpeech.drawRect.top = MAX(10, (height2 - height) / 2);
			} else {
				_activeSpeech.drawRect.left = _activeSpeech.speechBox.left;
				_activeSpeech.drawRect.top = _activeSpeech.speechBox.top + (_activeSpeech.speechBox.height() - height) / 2;
			}
			_activeSpeech.drawRect.setWidth(width);
			_activeSpeech.drawRect.setHeight(height);
		}
	}

	_activeSpeech.playing = true;
}

void Actor::handleActions(int msec, bool setup) {
	int i;
	ActorData *actor;
	ActorFrameRange *frameRange;
	int state;
	int speed;
	int32 framesLeft;
	Location delta;
	Location addDelta;
	int hitZoneIndex;
	const HitZone *hitZone;
	Point hitPoint;
	Location pickLocation;

	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		if (!actor->_inScene)
			continue;

		if ((_vm->getGameType() == GType_ITE) && (i == ACTOR_DRAGON_INDEX)) {
			moveDragon(actor);
			continue;
		}

		if (_vm->getGameType() == GType_IHNM) {
			if (actor->_spriteList.spriteCount == 0)
				continue;
		}

		switch (actor->_currentAction) {
		case kActionWait:
			if (!setup && (actor->_flags & kFollower)) {
				followProtagonist(actor);
				if (actor->_currentAction != kActionWait)
					break;
			}

			if (actor->_targetObject != ID_NOTHING) {
				actorFaceTowardsObject(actor->_id, actor->_targetObject);
			}

			if (actor->_flags & kCycle) {
				frameRange = getActorFrameRange(actor->_id, kFrameStand);
				if (frameRange->frameCount > 0) {
					actor->_actionCycle++;
					actor->_actionCycle = (actor->_actionCycle) % frameRange->frameCount;
				} else {
					actor->_actionCycle = 0;
				}
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
				break;
			}

			if ((actor->_actionCycle & 3) == 0) {
				actor->cycleWrap(100);

				frameRange = getActorFrameRange(actor->_id, kFrameWait);
				if ((frameRange->frameCount < 1 || actor->_actionCycle > 33))
					frameRange = getActorFrameRange(actor->_id, kFrameStand);

				if (frameRange->frameCount) {
					actor->_frameNumber = frameRange->frameIndex + (uint16)_vm->_rnd.getRandomNumber(frameRange->frameCount - 1);
				} else {
					actor->_frameNumber = frameRange->frameIndex;
				}
			}
			actor->_actionCycle++;
			break;

		case kActionWalkToPoint:
		case kActionWalkToLink:
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_partialTarget.delta(actor->_location, delta);

				while ((delta.u() == 0) && (delta.v() == 0)) {

					if ((actor == _protagonist) && (_vm->mouseButtonPressed())) {
						_vm->_isoMap->screenPointToTileCoords(_vm->mousePos(), pickLocation);

						if (!actorWalkTo(_protagonist->_id, pickLocation)) {
							break;
						}
					} else if (!_vm->_isoMap->nextTileTarget(actor) && !actorEndWalk(actor->_id, true)) {
						break;
					}

					actor->_partialTarget.delta(actor->_location, delta);
					actor->_partialTarget.z = 0;
				}

				if (actor->_flags & kFastest) {
					speed = 8;
				} else if (actor->_flags & kFaster) {
					speed = 6;
				} else {
					speed = 4;
				}

				if (_vm->_scene->currentSceneResourceId() == RID_ITE_OVERMAP_SCENE) {
					speed = 2;
				}

				if ((actor->_actionDirection == 2) || (actor->_actionDirection == 6)) {
					speed = speed / 2;
				}

				if (ABS(delta.v()) > ABS(delta.u())) {
					addDelta.v() = clamp( -speed, delta.v(), speed );
					if (addDelta.v() == delta.v()) {
						addDelta.u() = delta.u();
					} else {
						addDelta.u() = delta.u() * addDelta.v();
						addDelta.u() += (addDelta.u() > 0) ? (delta.v() / 2) : (-delta.v() / 2);
						addDelta.u() /= delta.v();
					}
				} else {
					addDelta.u() = clamp( -speed, delta.u(), speed );
					if (addDelta.u() == delta.u()) {
						addDelta.v() = delta.v();
					} else {
						addDelta.v() = delta.v() * addDelta.u();
						addDelta.v() += (addDelta.v() > 0) ? (delta.u() / 2) : (-delta.u() / 2);
						addDelta.v() /= delta.u();
					}
				}

				actor->_location.add(addDelta);
			} else {
				actor->_partialTarget.delta(actor->_location, delta);

				while ((delta.x == 0) && (delta.y == 0)) {

					if (actor->_walkStepIndex >= actor->_walkStepsCount) {
						actorEndWalk(actor->_id, true);
						break;
					}

					actor->_partialTarget.fromScreenPoint(actor->_walkStepsPoints[actor->_walkStepIndex++]);
					if (actor->_partialTarget.x > 224 * 2 * ACTOR_LMULT) {
						actor->_partialTarget.x -= 256 * 2 * ACTOR_LMULT;
					}

					actor->_partialTarget.delta(actor->_location, delta);

					if (ABS(delta.y) > ABS(delta.x)) {
						actor->_actionDirection = delta.y > 0 ? kDirDown : kDirUp;
					} else {
						actor->_actionDirection = delta.x > 0 ? kDirRight : kDirLeft;
					}
				}

				speed = (ACTOR_LMULT * 2 * actor->_screenScale + 63) / 256;
				if (speed < 1) {
					speed = 1;
				}

				if ((actor->_actionDirection == kDirUp) || (actor->_actionDirection == kDirDown)) {
					addDelta.y = clamp(-speed, delta.y, speed);
					if (addDelta.y == delta.y) {
						addDelta.x = delta.x;
					} else {
						addDelta.x = delta.x * addDelta.y;
						addDelta.x += (addDelta.x > 0) ? (delta.y / 2) : (-delta.y / 2);
						addDelta.x /= delta.y;
						actor->_facingDirection = actor->_actionDirection;
					}
				} else {
					addDelta.x = clamp(-2 * speed, delta.x, 2 * speed);
					if (addDelta.x == delta.x) {
						addDelta.y = delta.y;
					} else {
						addDelta.y = delta.y * addDelta.x;
						addDelta.y += (addDelta.y > 0) ? (delta.x / 2) : (-delta.x / 2);
						addDelta.y /= delta.x;
						actor->_facingDirection = actor->_actionDirection;
					}
				}

				actor->_location.add(addDelta);
			}

			if (actor->_actorFlags & kActorBackwards) {
				actor->_facingDirection = (actor->_actionDirection + 4) & 7;
				actor->_actionCycle--;
			} else {
				actor->_actionCycle++;
			}

			frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);

			if (actor->_actionCycle < 0) {
				actor->_actionCycle = frameRange->frameCount - 1;
			} else if (actor->_actionCycle >= frameRange->frameCount) {
				actor->_actionCycle = 0;
			}

			actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			break;

		case kActionWalkDir:
			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_location.u() += tileDirectionLUT[actor->_actionDirection][0];
				actor->_location.v() += tileDirectionLUT[actor->_actionDirection][1];

				frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);

				actor->_actionCycle++;
				actor->cycleWrap(frameRange->frameCount);
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			} else {
				actor->_location.x += directionLUT[actor->_actionDirection][0] * 2;
				actor->_location.y += directionLUT[actor->_actionDirection][1] * 2;

				frameRange = getActorFrameRange(actor->_id, actor->_walkFrameSequence);
				actor->_actionCycle++;
				actor->cycleWrap(frameRange->frameCount);
				actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			}
			break;

		case kActionSpeak:
			actor->_actionCycle++;
			actor->cycleWrap(64);

			frameRange = getActorFrameRange(actor->_id, kFrameGesture);
			if (actor->_actionCycle >= frameRange->frameCount) {
				if (actor->_actionCycle & 1)
					break;
				frameRange = getActorFrameRange(actor->_id, kFrameSpeak);

				state = (uint16)_vm->_rnd.getRandomNumber(frameRange->frameCount);

				if (state == 0) {
					frameRange = getActorFrameRange(actor->_id, kFrameStand);
				} else {
					state--;
				}
			} else {
				state = actor->_actionCycle;
			}

			actor->_frameNumber = frameRange->frameIndex + state;
			break;

		case kActionAccept:
		case kActionStoop:
			break;

		case kActionCycleFrames:
		case kActionPongFrames:
			if (actor->_cycleTimeCount > 0) {
				actor->_cycleTimeCount--;
				break;
			}

			actor->_cycleTimeCount = actor->_cycleDelay;
			actor->_actionCycle++;

			frameRange = getActorFrameRange(actor->_id, actor->_cycleFrameSequence);

			if (actor->_currentAction == kActionPongFrames) {
				if (actor->_actionCycle >= frameRange->frameCount * 2 - 2) {
					if (actor->_actorFlags & kActorContinuous) {
						actor->_actionCycle = 0;
					} else {
						actor->_currentAction = kActionFreeze;
						break;
					}
				}

				state = actor->_actionCycle;
				if (state >= frameRange->frameCount) {
					state = frameRange->frameCount * 2 - 2 - state;
				}
			} else {
				if (actor->_actionCycle >= frameRange->frameCount) {
					if (actor->_actorFlags & kActorContinuous) {
						actor->_actionCycle = 0;
					} else {
						actor->_currentAction = kActionFreeze;
						break;
					}
				}
				state = actor->_actionCycle;
			}

			if (frameRange->frameCount && (actor->_actorFlags & kActorRandom)) {
				state = _vm->_rnd.getRandomNumber(frameRange->frameCount - 1);
			}

			if (actor->_actorFlags & kActorBackwards) {
				actor->_frameNumber = frameRange->frameIndex + frameRange->frameCount - 1 - state;
			} else {
				actor->_frameNumber = frameRange->frameIndex + state;
			}
			break;

		case kActionFall:
			if (actor->_actionCycle > 0) {
				framesLeft = actor->_actionCycle--;
				actor->_finalTarget.delta(actor->_location, delta);
				delta.x /= framesLeft;
				delta.y /= framesLeft;
				actor->_location.addXY(delta);
				actor->_fallVelocity += actor->_fallAcceleration;
				actor->_fallPosition += actor->_fallVelocity;
				actor->_location.z = actor->_fallPosition >> 4;
			} else {
				actor->_location = actor->_finalTarget;
				actor->_currentAction = kActionFreeze;
				_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
			}
			break;

		case kActionClimb:
			actor->_cycleDelay++;
			if (actor->_cycleDelay & 3) {
				break;
			}

			if (actor->_location.z >= actor->_finalTarget.z + ACTOR_CLIMB_SPEED) {
				actor->_location.z -= ACTOR_CLIMB_SPEED;
				actor->_actionCycle--;
			} else if (actor->_location.z <= actor->_finalTarget.z - ACTOR_CLIMB_SPEED) {
				actor->_location.z += ACTOR_CLIMB_SPEED;
				actor->_actionCycle++;
			} else {
				actor->_location.z = actor->_finalTarget.z;
				actor->_currentAction = kActionFreeze;
				_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
			}

			frameRange = getActorFrameRange(actor->_id, actor->_cycleFrameSequence);

			if (actor->_actionCycle < 0) {
				actor->_actionCycle = frameRange->frameCount - 1;
			}
			actor->cycleWrap(frameRange->frameCount);
			actor->_frameNumber = frameRange->frameIndex + actor->_actionCycle;
			break;
		}

		if ((actor->_currentAction >= kActionWalkToPoint) && (actor->_currentAction <= kActionWalkDir)) {
			hitZone = NULL;

			if (_vm->_scene->getFlags() & kSceneFlagISO) {
				actor->_location.toScreenPointUV(hitPoint);
			} else {
				actor->_location.toScreenPointXY(hitPoint);
			}
			hitZoneIndex = _vm->_scene->_actionMap->hitTest(hitPoint);
			if (hitZoneIndex != -1) {
				hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
			}

			if (hitZone != actor->_lastZone) {
				if (actor->_lastZone)
					stepZoneAction( actor, actor->_lastZone, true, false);
				actor->_lastZone = hitZone;
				if (hitZone)
					stepZoneAction( actor, hitZone, false, false);
			}
		}
	}
}

void Actor::direct(int msec) {

	if (_vm->_scene->_entryList.entryListCount == 0) {
		return;
	}

	if (_vm->_interface->_statusTextInput) {
		return;
	}

	// FIXME: HACK. This should be turned into cycle event.
	_lastTickMsec += msec;

	if (_lastTickMsec > 1000 / _handleActionDiv) {
		_lastTickMsec = 0;
		//process actions
		handleActions(msec, false);
	}

//process speech
	handleSpeech(msec);
}


bool Actor::calcScreenPosition(CommonObjectData *commonObjectData) {
	int beginSlope, endSlope, middle;
	bool result;
	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		_vm->_isoMap->tileCoordsToScreenPoint(commonObjectData->_location, commonObjectData->_screenPosition);
		commonObjectData->_screenScale = 256;
	} else {
		middle = _vm->getSceneHeight() - commonObjectData->_location.y / ACTOR_LMULT;

		_vm->_scene->getSlopes(beginSlope, endSlope);

		commonObjectData->_screenDepth = (14 * middle) / endSlope + 1;

		if (middle <= beginSlope) {
			commonObjectData->_screenScale = 256;
		} else if (middle >= endSlope) {
			commonObjectData->_screenScale = 1;
		} else {
			middle -= beginSlope;
			endSlope -= beginSlope;
			commonObjectData->_screenScale = 256 - (middle * 256) / endSlope;
		}

		commonObjectData->_location.toScreenPointXYZ(commonObjectData->_screenPosition);
	}

	result = commonObjectData->_screenPosition.x > -64 &&
			commonObjectData->_screenPosition.x < _vm->getDisplayWidth() + 64 &&
			commonObjectData->_screenPosition.y > -64 &&
			commonObjectData->_screenPosition.y < _vm->getSceneHeight() + 64;

	return result;
}

uint16 Actor::hitTest(const Point &testPoint, bool skipProtagonist) {
	// We can only interact with objects or actors that are inside the
	// scene area. While this is usually the entire upper part of the
	// screen, it could also be an inset. Note that other kinds of hit
	// areas may be outside the inset, and that those are still perfectly
	// fine to interact with. For example, the door entrance at the glass
	// makers's house in ITE's ferret village.

	if (!_vm->_scene->getSceneClip().contains(testPoint))
		return ID_NOTHING;

	CommonObjectOrderList::iterator drawOrderIterator;
	CommonObjectDataPointer drawObject;
	int frameNumber;
	SpriteList *spriteList;

	createDrawOrderList();

	for (drawOrderIterator = _drawOrderList.begin(); drawOrderIterator != _drawOrderList.end(); ++drawOrderIterator) {
		drawObject = drawOrderIterator.operator*();
		if (skipProtagonist && (drawObject == _protagonist)) {
			continue;
		}
		if (!getSpriteParams(drawObject, frameNumber, spriteList)) {
			continue;
		}
		if (_vm->_sprite->hitTest(*spriteList, frameNumber, drawObject->_screenPosition, drawObject->_screenScale, testPoint)) {
			return drawObject->_id;
		}
	}
	return ID_NOTHING;
}

void Actor::createDrawOrderList() {
	int i;
	ActorData *actor;
	ObjectData *obj;
	CommonObjectOrderList::CompareFunction compareFunction;

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		compareFunction = &tileCommonObjectCompare;
	} else {
		compareFunction = &commonObjectCompare;
	}

	_drawOrderList.clear();
	for (i = 0; i < _actorsCount; i++) {
		actor = _actors[i];
		if (!actor->_inScene)
			continue;

		if (calcScreenPosition(actor)) {
			_drawOrderList.pushBack(actor, compareFunction);
		}
	}

	for (i = 0; i < _objsCount; i++) {
		obj = _objs[i];
		if (obj->_disabled)
			continue;

		if (obj->_sceneNumber != _vm->_scene->currentSceneNumber())
			 continue;

		if (calcScreenPosition(obj)) {
			_drawOrderList.pushBack(obj, compareFunction);
		}
	}
}

bool Actor::getSpriteParams(CommonObjectData *commonObjectData, int &frameNumber, SpriteList *&spriteList) {
	if (_vm->_scene->currentSceneResourceId() == RID_ITE_OVERMAP_SCENE) {
		if (!(commonObjectData->_flags & kProtagonist)){
//			warning("not protagonist");
			return false;
		}
		frameNumber = 8;
		spriteList = &_vm->_sprite->_mainSprites;
	} else if (validActorId(commonObjectData->_id)) {
		spriteList = &((ActorData *)commonObjectData)->_spriteList;
		frameNumber = ((ActorData *)commonObjectData)->_frameNumber;
	} else if (validObjId(commonObjectData->_id)) {
		spriteList = &_vm->_sprite->_mainSprites;
		frameNumber = commonObjectData->_spriteListResourceId;
	}

	if ((frameNumber < 0) || (spriteList->spriteCount <= frameNumber)) {
		debug(1, "Actor::getSpriteParams frameNumber invalid for %s id 0x%X (%d)",
				validObjId(commonObjectData->_id) ? "object" : "actor",
				commonObjectData->_id, frameNumber);
		return false;
	}
	return true;
}

void Actor::drawActors() {
	if (_vm->_scene->currentSceneNumber() <= 0) {
		return;
	}

	if (_vm->_scene->_entryList.entryListCount == 0) {
		return;
	}

	CommonObjectOrderList::iterator drawOrderIterator;
	CommonObjectDataPointer drawObject;
	int frameNumber;
	SpriteList *spriteList;

	Surface *backBuffer;

	backBuffer = _vm->_gfx->getBackBuffer();

	createDrawOrderList();

	for (drawOrderIterator = _drawOrderList.begin(); drawOrderIterator != _drawOrderList.end(); ++drawOrderIterator) {
		drawObject = drawOrderIterator.operator*();

		if (!getSpriteParams(drawObject, frameNumber, spriteList)) {
			continue;
		}

		if (_vm->_scene->getFlags() & kSceneFlagISO) {
			_vm->_isoMap->drawSprite(backBuffer, *spriteList, frameNumber, drawObject->_location, drawObject->_screenPosition, drawObject->_screenScale);
		} else {
			_vm->_sprite->drawOccluded(backBuffer, _vm->_scene->getSceneClip(),*spriteList, frameNumber, drawObject->_screenPosition, drawObject->_screenScale, drawObject->_screenDepth);
		}
	}

	drawSpeech();
}

void Actor::drawSpeech(void) {
	if (!isSpeaking() || !_activeSpeech.playing || _vm->_script->_skipSpeeches
		|| (!_vm->_subtitlesEnabled && (_vm->getFeatures() & GF_CD_FX)))
		return;

	int i;
	Point textPoint;
	ActorData *actor;
	int width, height;
	char oneChar[2];
	oneChar[1] = 0;
	const char *outputString;
	Surface *backBuffer;

	backBuffer = _vm->_gfx->getBackBuffer();

	if (_activeSpeech.speechFlags & kSpeakSlow) {
		outputString = oneChar;
		oneChar[0] = _activeSpeech.strings[0][_activeSpeech.slowModeCharIndex];
	} else {
		outputString = _activeSpeech.strings[0];
	}

	if (_activeSpeech.actorsCount > 1) {
		height = _vm->_font->getHeight((_vm->getGameType() == GType_ITE ? kMediumFont : kIHNMMainFont));
		width = _vm->_font->getStringWidth((_vm->getGameType() == GType_ITE ? kMediumFont : kIHNMMainFont), _activeSpeech.strings[0], 0, kFontNormal);

		for (i = 0; i < _activeSpeech.actorsCount; i++) {
			actor = getActor(_activeSpeech.actorIds[i]);
			calcScreenPosition(actor);

			textPoint.x = clamp( 10, actor->_screenPosition.x - width / 2, _vm->getDisplayWidth() - 10 - width);
			textPoint.y = clamp( 10, actor->_screenPosition.y - 58, _vm->getSceneHeight() - 10 - height);

			_vm->_font->textDraw((_vm->getGameType() == GType_ITE ? kMediumFont : kIHNMMainFont), backBuffer, _activeSpeech.strings[0], textPoint,
				_activeSpeech.speechColor[i], _activeSpeech.outlineColor[i], _activeSpeech.getFontFlags(i));
		}
	} else {
		_vm->_font->textDrawRect((_vm->getGameType() == GType_ITE ? kMediumFont : kIHNMMainFont), backBuffer, _activeSpeech.strings[0], _activeSpeech.drawRect, _activeSpeech.speechColor[0],
			_activeSpeech.outlineColor[0], _activeSpeech.getFontFlags(0));
	}
}

bool Actor::followProtagonist(ActorData *actor) {
	Location protagonistLocation;
	Location newLocation;
	Location delta;
	int protagonistBGMaskType;
	Point prefer1;
	Point prefer2;
	Point prefer3;
	int16 prefU;
	int16 prefV;
	int16 newU;
	int16 newV;

	assert(_protagonist);

	actor->_flags &= ~(kFaster | kFastest);
	protagonistLocation = _protagonist->_location;
	calcScreenPosition(_protagonist);

	if (_vm->_scene->getFlags() & kSceneFlagISO) {
		prefU = 60;
		prefV = 60;


		actor->_location.delta(protagonistLocation, delta);

		if (actor->_id == actorIndexToId(2)) {
			prefU = prefV = 48;
		}

		if ((delta.u() > prefU) || (delta.u() < -prefU) || (delta.v() > prefV) || (delta.v() < -prefV)) {

			if ((delta.u() > prefU * 2) || (delta.u() < -prefU * 2) || (delta.v() > prefV * 2) || (delta.v() < -prefV * 2)) {
				actor->_flags |= kFaster;

				if ((delta.u() > prefU * 3) || (delta.u() < -prefU*3) || (delta.v() > prefV * 3) || (delta.v() < -prefV * 3)) {
					actor->_flags |= kFastest;
				}
			}

			prefU /= 2;
			prefV /= 2;

			newU = clamp( -prefU, delta.u(), prefU ) + protagonistLocation.u();
			newV = clamp( -prefV, delta.v(), prefV ) + protagonistLocation.v();

			newLocation.u() = newU + _vm->_rnd.getRandomNumber(prefU - 1) - prefU / 2;
			newLocation.v() = newV + _vm->_rnd.getRandomNumber(prefV - 1) - prefV / 2;
			newLocation.z = 0;

			return actorWalkTo(actor->_id, newLocation);
		}

	} else {
		prefer1.x = (100 * _protagonist->_screenScale) >> 8;
		prefer1.y = (50 * _protagonist->_screenScale) >> 8;

		if (_protagonist->_currentAction == kActionWalkDir) {
			prefer1.x /= 2;
		}

		if (prefer1.x < 8) {
			prefer1.x = 8;
		}

		if (prefer1.y < 8) {
			prefer1.y = 8;
		}

		prefer2.x = prefer1.x * 2;
		prefer2.y = prefer1.y * 2;
		prefer3.x = prefer1.x + prefer1.x / 2;
		prefer3.y = prefer1.y + prefer1.y / 2;

		actor->_location.delta(protagonistLocation, delta);

		protagonistBGMaskType = 0;
		if (_vm->_scene->isBGMaskPresent() && _vm->_scene->validBGMaskPoint(_protagonist->_screenPosition)) {
			protagonistBGMaskType = _vm->_scene->getBGMaskType(_protagonist->_screenPosition);
		}

		if ((_vm->_rnd.getRandomNumber(7) & 0x7) == 0) // 1/8th chance
			actor->_actorFlags &= ~kActorNoFollow;

		if (actor->_actorFlags & kActorNoFollow) {
			return false;
		}

		if ((delta.x > prefer2.x) || (delta.x < -prefer2.x) ||
			(delta.y > prefer2.y) || (delta.y < -prefer2.y) ||
			((_protagonist->_currentAction == kActionWait) &&
			(delta.x * 2 < prefer1.x) && (delta.x * 2 > -prefer1.x) &&
			(delta.y < prefer1.y) && (delta.y > -prefer1.y))) {

				if (ABS(delta.x) > ABS(delta.y)) {

					delta.x = (delta.x > 0) ? prefer3.x : -prefer3.x;

					newLocation.x = delta.x + protagonistLocation.x;
					newLocation.y = clamp(-prefer2.y, delta.y, prefer2.y) + protagonistLocation.y;
				} else {
					delta.y = (delta.y > 0) ? prefer3.y : -prefer3.y;

					newLocation.x = clamp(-prefer2.x, delta.x, prefer2.x) + protagonistLocation.x;
					newLocation.y = delta.y + protagonistLocation.y;
				}
				newLocation.z = 0;

				if (protagonistBGMaskType != 3) {
					newLocation.x += _vm->_rnd.getRandomNumber(prefer1.x - 1) - prefer1.x / 2;
					newLocation.y += _vm->_rnd.getRandomNumber(prefer1.y - 1) - prefer1.y / 2;
				}

				newLocation.x = clamp(-31*4, newLocation.x, (_vm->getDisplayWidth() + 31) * 4); //fixme

				return actorWalkTo(actor->_id, newLocation);
			}
	}
	return false;
}

bool Actor::actorEndWalk(uint16 actorId, bool recurse) {
	bool walkMore = false;
	ActorData *actor;
	const HitZone *hitZone;
	int hitZoneIndex;
	Point testPoint;

	actor = getActor(actorId);
	actor->_actorFlags &= ~kActorBackwards;

	if (actor->_location.distance(actor->_finalTarget) > 8 && (actor->_flags & kProtagonist) && recurse && !(actor->_actorFlags & kActorNoCollide)) {
		actor->_actorFlags |= kActorNoCollide;
		return actorWalkTo(actorId, actor->_finalTarget);
	}

	actor->_currentAction = kActionWait;
	if (actor->_actorFlags & kActorFinalFace) {
		actor->_facingDirection = actor->_actionDirection = (actor->_actorFlags >> 6) & 0x07; //?
	}

	actor->_actorFlags &= ~(kActorNoCollide | kActorCollided | kActorFinalFace | kActorFacingMask);
	actor->_flags &= ~(kFaster | kFastest);

	if (actor == _protagonist) {
		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
		if (_vm->_script->_pendingVerb == kVerbWalkTo) {
			actor->_location.toScreenPointUV(testPoint);
			hitZoneIndex = _vm->_scene->_actionMap->hitTest(testPoint);
			if (hitZoneIndex != -1) {
				hitZone = _vm->_scene->_actionMap->getHitZone(hitZoneIndex);
				stepZoneAction(actor, hitZone, false, true);
			} else {
				_vm->_script->setNoPendingVerb();
			}
		} else if (_vm->_script->_pendingVerb != kVerbNone) {
			_vm->_script->doVerb();
		}
	} else {
		if (recurse && (actor->_flags & kFollower))
			walkMore = followProtagonist(actor);

		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
	}
	return walkMore;
}

bool Actor::actorWalkTo(uint16 actorId, const Location &toLocation) {
	ActorData *actor;
	ActorData *anotherActor;
	int	i;

	Rect testBox;
	Rect testBox2;
	Point anotherActorScreenPosition;
	Point collision;
	Point pointFrom, pointTo, pointBest, pointAdd;
	Point delta, bestDelta;
	Point tempPoint;
	bool extraStartNode;
	bool extraEndNode;

	actor = getActor(actorId);

	if (actor == _protagonist) {
		_vm->_scene->setDoorState(2, 0xff);
		_vm->_scene->setDoorState(3, 0);
	} else {
		_vm->_scene->setDoorState(2, 0);
		_vm->_scene->setDoorState(3, 0xff);
	}

	if (_vm->_scene->getFlags() & kSceneFlagISO) {

		if ((_vm->getGameType() == GType_ITE) && (actor->_index == ACTOR_DRAGON_INDEX)) {
			return false;
		}

		actor->_finalTarget = toLocation;
		actor->_walkStepsCount = 0;
		_vm->_isoMap->findTilePath(actor, actor->_location, toLocation);


		if ((actor->_walkStepsCount == 0) && (actor->_flags & kProtagonist)) {
			actor->_actorFlags |= kActorNoCollide;
			_vm->_isoMap->findTilePath(actor, actor->_location, toLocation);
		}

		actor->_walkStepIndex = 0;
		if (_vm->_isoMap->nextTileTarget(actor)) {
			actor->_currentAction = kActionWalkToPoint;
			actor->_walkFrameSequence = kFrameWalk;
		} else {
			actorEndWalk( actorId, false);
			return false;
		}
	} else {

		actor->_location.toScreenPointXY(pointFrom);
		pointFrom.x &= ~1;

		extraStartNode = _vm->_scene->offscreenPath(pointFrom);

		toLocation.toScreenPointXY(pointTo);
		pointTo.x &= ~1;

		extraEndNode = _vm->_scene->offscreenPath(pointTo);

		if (_vm->_scene->isBGMaskPresent()) {

			if ((((actor->_currentAction >= kActionWalkToPoint) &&
				(actor->_currentAction <= kActionWalkDir)) || (actor == _protagonist)) &&
				!_vm->_scene->canWalk(pointFrom)) {
				for (i = 1; i < 8; i++) {
					pointAdd = pointFrom;
					pointAdd.y += i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					pointAdd = pointFrom;
					pointAdd.y -= i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					pointAdd = pointFrom;
					pointAdd.x += i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
					pointAdd = pointFrom;
					pointAdd.x -= i;
					if (_vm->_scene->canWalk(pointAdd)) {
						pointFrom = pointAdd;
						break;
					}
				}
			}

			_barrierCount = 0;
			if (!(actor->_actorFlags & kActorNoCollide)) {
				collision.x = ACTOR_COLLISION_WIDTH * actor->_screenScale / (256 * 2);
				collision.y = ACTOR_COLLISION_HEIGHT * actor->_screenScale / (256 * 2);


				for (i = 0; (i < _actorsCount) && (_barrierCount < ACTOR_BARRIERS_MAX); i++) {
					anotherActor = _actors[i];
					if (!anotherActor->_inScene) continue;
					if (anotherActor == actor ) continue;


					anotherActorScreenPosition = anotherActor->_screenPosition;
					testBox.left = (anotherActorScreenPosition.x - collision.x) & ~1;
					testBox.right = (anotherActorScreenPosition.x + collision.x) & ~1 + 1;
					testBox.top = anotherActorScreenPosition.y - collision.y;
					testBox.bottom = anotherActorScreenPosition.y + collision.y + 1;
					testBox2 = testBox;
					testBox2.right += 2;
					testBox2.left -= 2;
					testBox2.top -= 1;
					testBox2.bottom += 1;

					if (testBox2.contains(pointFrom)) {
						if (pointFrom.x > anotherActorScreenPosition.x + 4) {
							testBox.right = pointFrom.x - 1;
						} else if (pointFrom.x < anotherActorScreenPosition.x - 4) {
							testBox.left = pointFrom.x + 2;
						} else if (pointFrom.y > anotherActorScreenPosition.y) {
							testBox.bottom = pointFrom.y;
						} else {
							testBox.top = pointFrom.y + 1 ;
						}
					}

					if ((testBox.width() > 0) && (testBox.height() > 0)) {
						_barrierList[_barrierCount++] = testBox;
					}
				}
			}


			pointBest = pointTo;
			actor->_walkStepsCount = 0;
			findActorPath(actor, pointFrom, pointTo);

			if (actor->_walkStepsCount == 0) {
				error("actor->_walkStepsCount == 0");
			}

			if (extraStartNode) {
				actor->_walkStepIndex = 0;
			} else {
				actor->_walkStepIndex = 1;
			}

			if (extraEndNode) {
				toLocation.toScreenPointXY(tempPoint);
				actor->_walkStepsCount--;
				actor->addWalkStepPoint(tempPoint);
			}


			pointBest = actor->_walkStepsPoints[actor->_walkStepsCount - 1];

			pointBest.x &= ~1;
			delta.x = ABS(pointFrom.x - pointTo.x);
			delta.y = ABS(pointFrom.y - pointTo.y);

			bestDelta.x = ABS(pointBest.x - pointTo.x);
			bestDelta.y = ABS(pointBest.y - pointTo.y);

			if ((delta.x + delta.y <= bestDelta.x + bestDelta.y) && (actor->_flags & kFollower)) {
				actor->_actorFlags |= kActorNoFollow;
			}

			if (pointBest == pointFrom) {
				actor->_walkStepsCount = 0;
			}
		} else {
			actor->_walkStepsCount = 0;
			actor->addWalkStepPoint(pointTo);
			actor->_walkStepIndex = 0;
		}

		actor->_partialTarget = actor->_location;
		actor->_finalTarget = toLocation;
		if (actor->_walkStepsCount == 0) {
			actorEndWalk(actorId, false);
			return false;
		} else {
			if (actor->_flags & kProtagonist) {
				_actors[1]->_actorFlags &= ~kActorNoFollow; // TODO: mark all actors with kFollower flag, not only 1 and 2
				_actors[2]->_actorFlags &= ~kActorNoFollow;
			}
			actor->_currentAction = (actor->_walkStepsCount >= ACTOR_MAX_STEPS_COUNT) ? kActionWalkToLink : kActionWalkToPoint;
			actor->_walkFrameSequence = kFrameWalk;
		}

	}
	return true;
}

void Actor::actorSpeech(uint16 actorId, const char **strings, int stringsCount, int sampleResourceId, int speechFlags) {
	ActorData *actor;
	int i;
	int16 dist;

	actor = getActor(actorId);
	calcScreenPosition(actor);
	for (i = 0; i < stringsCount; i++) {
		_activeSpeech.strings[i] = strings[i];
	}

	_activeSpeech.stringsCount = stringsCount;
	_activeSpeech.speechFlags = speechFlags;
	_activeSpeech.actorsCount = 1;
	_activeSpeech.actorIds[0] = actorId;
	_activeSpeech.speechColor[0] = actor->_speechColor;
	_activeSpeech.outlineColor[0] = kITEColorBlack;
	_activeSpeech.sampleResourceId = sampleResourceId;
	_activeSpeech.playing = false;
	_activeSpeech.slowModeCharIndex = 0;

	dist = MIN(actor->_screenPosition.x - 10, _vm->getDisplayWidth() - 10 - actor->_screenPosition.x );
	dist = clamp( 60, dist, 150 );

	_activeSpeech.speechBox.left = actor->_screenPosition.x - dist;
	_activeSpeech.speechBox.right = actor->_screenPosition.x + dist;

	if (_activeSpeech.speechBox.left < 10) {
		_activeSpeech.speechBox.right += 10 - _activeSpeech.speechBox.left;
		_activeSpeech.speechBox.left = 10;
	}
	if (_activeSpeech.speechBox.right > _vm->getDisplayWidth() - 10) {
		_activeSpeech.speechBox.left -= _activeSpeech.speechBox.right - _vm->getDisplayWidth() - 10;
		_activeSpeech.speechBox.right = _vm->getDisplayWidth() - 10;
	}
}

void Actor::nonActorSpeech(const Common::Rect &box, const char **strings, int stringsCount, int sampleResourceId, int speechFlags) {
	int i;

	_vm->_script->wakeUpThreads(kWaitTypeSpeech);

	for (i = 0; i < stringsCount; i++) {
		_activeSpeech.strings[i] = strings[i];
	}
	_activeSpeech.stringsCount = stringsCount;
	_activeSpeech.speechFlags = speechFlags;
	_activeSpeech.actorsCount = 1;
	_activeSpeech.actorIds[0] = 0;
	if (!(_vm->getFeatures() & GF_CD_FX))
		_activeSpeech.sampleResourceId = -1;
	else
		_activeSpeech.sampleResourceId = sampleResourceId;
	_activeSpeech.playing = false;
	_activeSpeech.slowModeCharIndex = 0;
	_activeSpeech.speechBox = box;
}

void Actor::simulSpeech(const char *string, uint16 *actorIds, int actorIdsCount, int speechFlags, int sampleResourceId) {
	int i;

	for (i = 0; i < actorIdsCount; i++) {
		ActorData *actor;

		actor = getActor(actorIds[i]);
		_activeSpeech.actorIds[i] = actorIds[i];
		_activeSpeech.speechColor[i] = actor->_speechColor;
		_activeSpeech.outlineColor[i] = 0; // disable outline
	}
	_activeSpeech.actorsCount = actorIdsCount;
	_activeSpeech.strings[0] = string;
	_activeSpeech.stringsCount = 1;
	_activeSpeech.speechFlags = speechFlags;
	_activeSpeech.sampleResourceId = sampleResourceId;
	_activeSpeech.playing = false;
	_activeSpeech.slowModeCharIndex = 0;

	// caller should call thread->wait(kWaitTypeSpeech) by itself
}

void Actor::abortAllSpeeches() {
	abortSpeech();

	if (_vm->_script->_abortEnabled)
		_vm->_script->_skipSpeeches = true;

	for (int i = 0; i < 10; i++)
		_vm->_script->executeThreads(0);
}

void Actor::abortSpeech() {
	_vm->_sound->stopVoice();
	_activeSpeech.playingTime = 0;
}

void Actor::moveDragon(ActorData *actor) {
	int16 dir0, dir1, dir2, dir3;
	int16 moveType;
	Event event;
	const DragonMove *dragonMove;

	if ((actor->_actionCycle < 0) ||
		((actor->_actionCycle == 0) && (actor->_dragonMoveType >= ACTOR_DRAGON_TURN_MOVES))) {

		moveType = kDragonMoveInvalid;
		if (actor->_location.distance(_protagonist->_location) < 24) {
			if (_dragonHunt && (_protagonist->_currentAction != kActionFall)) {
				event.type = kEvTOneshot;
				event.code = kScriptEvent;
				event.op = kEventExecNonBlocking;
				event.time = 0;
				event.param = _vm->_scene->getScriptModuleNumber(); // module number
				event.param2 = ACTOR_EXP_KNOCK_RIF;			// script entry point number
				event.param3 = -1;		// Action
				event.param4 = -1;		// Object
				event.param5 = -1;		// With Object
				event.param6 = -1;		// Actor

				_vm->_events->queue(&event);
				_dragonHunt = false;
			}
		} else {
			_dragonHunt = true;
		}

		if (actor->_walkStepIndex + 2 > actor->_walkStepsCount) {

			_vm->_isoMap->findDragonTilePath(actor, actor->_location, _protagonist->_location, actor->_actionDirection);

			if (actor->_walkStepsCount == 0) {
				_vm->_isoMap->findDragonTilePath(actor, actor->_location, _protagonist->_location, 0);
			}

			if (actor->_walkStepsCount < 2) {
				return;
			}

			actor->_partialTarget = actor->_location;
			actor->_finalTarget = _protagonist->_location;
			actor->_walkStepIndex = 0;
		}

		dir0 = actor->_actionDirection;
		dir1 = actor->_tileDirections[actor->_walkStepIndex++];
		dir2 = actor->_tileDirections[actor->_walkStepIndex];
		dir3 = actor->_tileDirections[actor->_walkStepIndex + 1];

		if (dir0 != dir1){
			actor->_actionDirection = dir0 = dir1;
		}

		actor->_location = actor->_partialTarget;

		if ((dir1 != dir2) && (dir1 == dir3)) {
			switch (dir1) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				moveType = kDragonMoveUpLeft;
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				moveType = kDragonMoveDownLeft;
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				moveType = kDragonMoveDownRight;
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				moveType = kDragonMoveUpRight;
				break;
			}

			switch (dir2) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				break;
			}

			actor->_walkStepIndex++;
		} else {
			switch (dir1) {
			case kDirUpLeft:
				actor->_partialTarget.v() += 16;
				switch (dir2) {
				case kDirDownLeft:
					moveType = kDragonMoveUpLeft_Left;
					actor->_partialTarget.u() -= 16;
					break;
				case kDirUpLeft:
					moveType = kDragonMoveUpLeft;
					break;
				case kDirUpRight:
					actor->_partialTarget.u() += 16;
					moveType = kDragonMoveUpLeft_Right;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirDownLeft:
				actor->_partialTarget.u() -= 16;
				switch (dir2) {
				case kDirDownRight:
					moveType = kDragonMoveDownLeft_Left;
					actor->_partialTarget.v() -= 16;
					break;
				case kDirDownLeft:
					moveType = kDragonMoveDownLeft;
					break;
				case kDirUpLeft:
					moveType = kDragonMoveDownLeft_Right;
					actor->_partialTarget.v() += 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirDownRight:
				actor->_partialTarget.v() -= 16;
				switch (dir2) {
				case kDirUpRight:
					moveType = kDragonMoveDownRight_Left;
					actor->_partialTarget.u() += 16;
					break;
				case kDirDownRight:
					moveType = kDragonMoveDownRight;
					break;
				case kDirDownLeft:
					moveType = kDragonMoveDownRight_Right;
					actor->_partialTarget.u() -= 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;
			case kDirUpRight:
				actor->_partialTarget.u() += 16;
				switch (dir2) {
				case kDirUpLeft:
					moveType = kDragonMoveUpRight_Left;
					actor->_partialTarget.v() += 16;
					break;
				case kDirUpRight:
					moveType = kDragonMoveUpRight;
					break;
				case kDirDownRight:
					moveType = kDragonMoveUpRight_Right;
					actor->_partialTarget.v() -= 16;
					break;
				default:
					actor->_actionDirection = dir1;
					actor->_walkStepsCount = 0;
					break;
				}
				break;

			default:
				actor->_actionDirection = dir1;
				actor->_walkStepsCount = 0;
				break;
			}
		}

		actor->_dragonMoveType = moveType;

		if (moveType >= ACTOR_DRAGON_TURN_MOVES) {
			actor->_dragonStepCycle = 0;
			actor->_actionCycle = 4;
			actor->_walkStepIndex++;
		} else {
			actor->_actionCycle = 4;
		}
	}

	actor->_actionCycle--;

	if ((actor->_walkStepsCount < 1) || (actor->_actionCycle < 0)) {
		return;
	}

	if (actor->_dragonMoveType < ACTOR_DRAGON_TURN_MOVES) {

		actor->_dragonStepCycle++;
		if (actor->_dragonStepCycle >= 7) {
			actor->_dragonStepCycle = 0;
		}

		actor->_dragonBaseFrame = actor->_dragonMoveType * 7;

		if (actor->_location.u() > actor->_partialTarget.u() + 3) {
			actor->_location.u() -= 4;
		} else if (actor->_location.u() < actor->_partialTarget.u() - 3) {
			actor->_location.u() += 4;
		} else {
			actor->_location.u() = actor->_partialTarget.u();
		}

		if (actor->_location.v() > actor->_partialTarget.v() + 3) {
			actor->_location.v() -= 4;
		} else if (actor->_location.v() < actor->_partialTarget.v() - 3) {
			actor->_location.v() += 4;
		} else {
			actor->_location.v() = actor->_partialTarget.v();
		}
	} else {
		dragonMove = &dragonMoveTable[actor->_dragonMoveType];
		actor->_dragonBaseFrame = dragonMove->baseFrame;


		actor->_location.u() = actor->_partialTarget.u() - dragonMove->offset[actor->_actionCycle][0];
		actor->_location.v() = actor->_partialTarget.v() - dragonMove->offset[actor->_actionCycle][1];

		actor->_dragonStepCycle++;
		if (actor->_dragonStepCycle >= 3) {
			actor->_dragonStepCycle = 3;
		}
	}

	actor->_frameNumber = actor->_dragonBaseFrame + actor->_dragonStepCycle;
}

void Actor::findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point iteratorPoint;
	Point bestPoint;
	int maskType;
	int i;
	Rect intersect;

#ifdef ACTOR_DEBUG
	_debugPointsCount = 0;
#endif

	actor->_walkStepsCount = 0;
	if (fromPoint == toPoint) {
		actor->addWalkStepPoint(toPoint);
		return;
	}

	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (_vm->_scene->validBGMaskPoint(iteratorPoint)) {
				maskType = _vm->_scene->getBGMaskType(iteratorPoint);
				setPathCell(iteratorPoint, _vm->_scene->getDoorState(maskType) ? kPathCellBarrier : kPathCellEmpty);
			} else {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

	for (i = 0; i < _barrierCount; i++) {
		intersect.left = MAX(_pathRect.left, _barrierList[i].left);
		intersect.top = MAX(_pathRect.top, _barrierList[i].top);
		intersect.right = MIN(_pathRect.right, _barrierList[i].right);
		intersect.bottom = MIN(_pathRect.bottom, _barrierList[i].bottom);

		for (iteratorPoint.y = intersect.top; iteratorPoint.y < intersect.bottom; iteratorPoint.y++) {
			for (iteratorPoint.x = intersect.left; iteratorPoint.x < intersect.right; iteratorPoint.x++) {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}

#ifdef ACTOR_DEBUG
	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			if (getPathCell(iteratorPoint) == kPathCellBarrier) {
				addDebugPoint(iteratorPoint, 24);
			}
		}
	}
#endif

	if (scanPathLine(fromPoint, toPoint)) {
		actor->addWalkStepPoint(fromPoint);
		actor->addWalkStepPoint(toPoint);
		return;
	}

	i = fillPathArray(fromPoint, toPoint, bestPoint);

	if (fromPoint == bestPoint) {
		actor->addWalkStepPoint(bestPoint);
		return;
	}

	if (i == 0) {
		error("fillPathArray returns zero");
	}

	setActorPath(actor, fromPoint, bestPoint);
}

bool Actor::scanPathLine(const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	bool interchange = false;
	Point fDelta;
	int errterm;
	int s1;
	int s2;
	int i;

	point = point1;
	delta.x = ABS(point1.x - point2.x);
	delta.y = ABS(point1.y - point2.y);
	s1 = integerCompare(point2.x, point1.x);
	s2 = integerCompare(point2.y, point1.y);

	if (delta.y > delta.x) {
		SWAP(delta.y, delta.x);
		interchange = true;
	}

	fDelta.x = delta.x * 2;
	fDelta.y = delta.y * 2;

	errterm = fDelta.y - delta.x;

	for (i = 0; i < delta.x; i++) {
		while (errterm >= 0) {
			if (interchange) {
				point.x += s1;
			} else {
				point.y += s2;
			}
			errterm -= fDelta.x;
		}

		if (interchange)
			point.y += s2;
		else
			point.x += s1;

		errterm += fDelta.y;

		if (!validPathCellPoint(point)) {
			return false;
		}
		if (getPathCell(point) == kPathCellBarrier) {
			return false;
		}
	}
	return true;
}

int Actor::fillPathArray(const Point &fromPoint, const Point &toPoint, Point &bestPoint) {
	int bestRating;
	int currentRating;
	int i;
	Point bestPath;
	int pointCounter;
	int startDirection;
	PathDirectionData *pathDirection;
	PathDirectionData *newPathDirection;
	const PathDirectionData *samplePathDirection;
	Point nextPoint;
	int directionCount;

	_pathDirectionListCount = 0;
	pointCounter = 0;
	bestRating = quickDistance(fromPoint, toPoint);
	bestPath = fromPoint;

	for (startDirection = 0; startDirection < 4; startDirection++) {
		newPathDirection = addPathDirectionListData();
		newPathDirection->x = fromPoint.x;
		newPathDirection->y = fromPoint.y;
		newPathDirection->direction = startDirection;
	}

	if (validPathCellPoint(fromPoint)) {
		setPathCell(fromPoint, kDirUp);

#ifdef ACTOR_DEBUG
		addDebugPoint(fromPoint, 24+36);
#endif
	}

	i = 0;

	do {
		pathDirection = &_pathDirectionList[i];
		for (directionCount = 0; directionCount < 3; directionCount++) {
			samplePathDirection = &pathDirectionLUT[pathDirection->direction][directionCount];
			nextPoint.x = samplePathDirection->x + pathDirection->x;
			nextPoint.y = samplePathDirection->y + pathDirection->y;

			if (!validPathCellPoint(nextPoint)) {
				continue;
			}

			if (getPathCell(nextPoint) != kPathCellEmpty) {
				continue;
			}

			setPathCell(nextPoint, samplePathDirection->direction);

#ifdef ACTOR_DEBUG
			addDebugPoint(nextPoint, samplePathDirection->direction + 96);
#endif
			newPathDirection = addPathDirectionListData();
			newPathDirection->x = nextPoint.x;
			newPathDirection->y = nextPoint.y;
			newPathDirection->direction = samplePathDirection->direction;
			++pointCounter;
			if (nextPoint == toPoint) {
				bestPoint = toPoint;
				return pointCounter;
			}
			currentRating = quickDistance(nextPoint, toPoint);
			if (currentRating < bestRating) {
				bestRating = currentRating;
				bestPath = nextPoint;
			}
			pathDirection = &_pathDirectionList[i];
		}
		++i;
	} while (i < _pathDirectionListCount);

	bestPoint = bestPath;
	return pointCounter;
}

void Actor::setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point nextPoint;
	int8 direction;
	int i;

	_pathListIndex = -1;
	addPathListPoint(toPoint);
	nextPoint = toPoint;

	while ( !(nextPoint == fromPoint)) {
		direction = getPathCell(nextPoint);
		if ((direction < 0) || (direction >= 8)) {
			error("Actor::setActorPath error direction 0x%X", direction);
		}
		nextPoint.x -= pathDirectionLUT2[direction][0];
		nextPoint.y -= pathDirectionLUT2[direction][1];
		addPathListPoint(nextPoint);

#ifdef ACTOR_DEBUG
		addDebugPoint(nextPoint, 0x8a);
#endif
	}

	pathToNode();
	removeNodes();
	nodeToPath();
	removePathPoints();

	for (i = 0; i <= _pathNodeListIndex; i++) {
		actor->addWalkStepPoint(_pathNodeList[i].point);
	}
}

void Actor::pathToNode() {
	Point point1, point2, delta;
	int direction;
	int i;
	Point *point;

	point= &_pathList[_pathListIndex];
	direction = 0;

	_pathNodeListIndex = -1;
	addPathNodeListPoint(*point);

	for (i = _pathListIndex; i > 0; i--) {
		point1 = *point;
		--point;
		point2 = *point;
		if (direction == 0) {
			delta.x = integerCompare(point2.x, point1.x);
			delta.y = integerCompare(point2.y, point1.y);
			direction++;
		}
		if ((point1.x + delta.x != point2.x) || (point1.y + delta.y != point2.y)) {
			addPathNodeListPoint(point1);
			direction--;
			i++;
			point++;
		}
	}
	addPathNodeListPoint(*_pathList);
}

int pathLine(Point *pointList, const Point &point1, const Point &point2) {
	Point point;
	Point delta;
	Point tempPoint;
	int s1;
	int s2;
	bool interchange = false;
	int errterm;
	int i;

	delta.x = abs(point2.x - point1.x);
	delta.y = abs(point2.y - point1.y);
	point = point1;
	s1 = integerCompare(point2.x, point1.x);
	s2 = integerCompare(point2.y, point1.y);

	if (delta.y > delta.x) {
		SWAP(delta.y, delta.x);
		interchange = true;
	}

	tempPoint.x = delta.x * 2;
	tempPoint.y = delta.y * 2;

	errterm = tempPoint.y - delta.x;

	for (i = 0; i < delta.x; i++) {
		while (errterm >= 0) {
			if (interchange) {
				point.x += s1;
			} else {
				point.y += s2;
			}
			errterm -= tempPoint.x;
		}
		if (interchange) {
			point.y += s2;
		} else {
			point.x += s1;
		}
		errterm += tempPoint.y;

		pointList[i] = point;
	}
	return delta.x;
}

void Actor::nodeToPath() {
	int i;
	Point point1, point2;
	PathNode *node;
	Point *point;

	for (i = 0, point = _pathList; i < _pathListAlloced; i++, point++) {
		point->x = point->y = PATH_NODE_EMPTY;
	}

	_pathListIndex = 1;
	_pathList[0] = _pathNodeList[0].point;
	_pathNodeList[0].link = 0;
	for (i = 0, node = _pathNodeList; i < _pathNodeListIndex; i++) {
		point1 = node->point;
		node++;
		point2 = node->point;
		_pathListIndex += pathLine(&_pathList[_pathListIndex], point1, point2);
		node->link = _pathListIndex - 1;
	}
	_pathListIndex--;
	_pathNodeList[_pathNodeListIndex].link = _pathListIndex;

}

void Actor::removeNodes() {
	int i, j, k;
	PathNode *iNode, *jNode, *kNode, *fNode;
	fNode = &_pathNodeList[_pathNodeListIndex];

	if (scanPathLine(_pathNodeList[0].point, fNode->point)) {
		_pathNodeList[1] = *fNode;
		_pathNodeListIndex = 1;
	}

	if (_pathNodeListIndex < 4) {
		return;
	}

	for (i = _pathNodeListIndex - 1, iNode = fNode-1; i > 1 ; i--, iNode--) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(_pathNodeList[0].point, iNode->point)) {
			for (j = 1, jNode = _pathNodeList + 1; j < i; j++, jNode++) {
				jNode->point.x = PATH_NODE_EMPTY;
			}
		}
	}

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex - 1; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}

		if (scanPathLine(fNode->point, iNode->point)) {
			for (j = i + 1, jNode = iNode + 1; j < _pathNodeListIndex; j++, jNode++) {
				jNode->point.x = PATH_NODE_EMPTY;
			}
		}
	}
	condenseNodeList();

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex - 1; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			continue;
		}
		for (j = i + 2, jNode = iNode + 2; j < _pathNodeListIndex; j++, jNode++) {
			if (jNode->point.x == PATH_NODE_EMPTY) {
				continue;
			}

			if (scanPathLine(iNode->point, jNode->point)) {
				for (k = i + 1,kNode = iNode + 1; k < j; k++, kNode++) {
					kNode->point.x = PATH_NODE_EMPTY;
				}
			}
		}
	}
	condenseNodeList();
}

void Actor::condenseNodeList() {
	int i, j, count;
	PathNode *iNode, *jNode;

	count = _pathNodeListIndex;

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeListIndex; i++, iNode++) {
		if (iNode->point.x == PATH_NODE_EMPTY) {
			j = i + 1;
			jNode = iNode + 1;
			while ( jNode->point.x == PATH_NODE_EMPTY ) {
				j++;
				jNode++;
			}
			*iNode = *jNode;
			count = i;
			jNode->point.x = PATH_NODE_EMPTY;
			if (j == _pathNodeListIndex) {
				break;
			}
		}
	}
	_pathNodeListIndex = count;
}

void Actor::removePathPoints() {
	int i, j, k, l;
	PathNode *node;
	int start;
	int end;
	Point point1, point2;

	if (_pathNodeListIndex < 2)
		return;

	_newPathNodeListIndex = -1;
	addNewPathNodeListPoint(_pathNodeList[0]);

	for (i = 1, node = _pathNodeList + 1; i < _pathNodeListIndex; i++, node++) {
		addNewPathNodeListPoint(*node);

		for (j = 5; j > 0; j--) {
			start = node->link - j;
			end = node->link + j;

			if (start < 0 || end > _pathListIndex) {
				continue;
			}

			point1 = _pathList[start];
			point2 = _pathList[end];
			if ((point1.x == PATH_NODE_EMPTY) || (point2.x == PATH_NODE_EMPTY)) {
				continue;
			}

			if (scanPathLine(point1, point2)) {
				for (l = 1; l <= _newPathNodeListIndex; l++) {
					if (start <= _newPathNodeList[l].link) {
						_newPathNodeListIndex = l;
						_newPathNodeList[_newPathNodeListIndex].point = point1;
						_newPathNodeList[_newPathNodeListIndex].link = start;
						incrementNewPathNodeListIndex();
						break;
					}
				}
				_newPathNodeList[_newPathNodeListIndex].point = point2;
				_newPathNodeList[_newPathNodeListIndex].link = end;

				for (k = start + 1; k < end; k++) {
					_pathList[k].x = PATH_NODE_EMPTY;
				}
				break;
			}
		}
	}

	addNewPathNodeListPoint(_pathNodeList[_pathNodeListIndex]);

	for (i = 0, j = 0; i <= _newPathNodeListIndex; i++) {
		if (_newPathNodeListIndex == i || (_newPathNodeList[i].point != _newPathNodeList[i+1].point)) {
			_pathNodeList[j++] = _newPathNodeList[i];
		}
	}
	_pathNodeListIndex = j - 1;
}

void Actor::drawPathTest() {
#ifdef ACTOR_DEBUG
	int i;
	Surface *surface;
	surface = _vm->_gfx->getBackBuffer();
	if (_debugPoints == NULL) {
		return;
	}

	for (i = 0; i < _debugPointsCount; i++) {
		*((byte *)surface->pixels + (_debugPoints[i].point.y * surface->pitch) + _debugPoints[i].point.x) = _debugPoints[i].color;
	}
#endif
}

void Actor::saveState(Common::OutSaveFile *out) {
	uint16 i;

	out->writeSint16LE(getProtagState());

	for (i = 0; i < _actorsCount; i++) {
		ActorData *a = _actors[i];
		a->saveState(out);
	}

	for (i = 0; i < _objsCount; i++) {
		ObjectData *o = _objs[i];
		o->saveState(out);
	}
}

void Actor::loadState(Common::InSaveFile *in) {
	int32 i;

	setProtagState(in->readSint16LE());

	for (i = 0; i < _actorsCount; i++) {
		ActorData *a = _actors[i];
		a->loadState(_vm->getCurrentLoadVersion(), in);

		// Fix bug #1258633 "ITE: Second Rif appears in wall of dog castle prison"
		// For some reason in some cases actor position is all wrong, so Rif
		// crawls to his original poition
		if (i == 122 && _vm->getGameType() == GType_ITE) {
			a->_location.x = 130;
			a->_location.y = 55;
		}
	}

	for (i = 0; i < _objsCount; i++) {
		ObjectData *o = _objs[i];
		o->loadState(in);
	}
}

// Console wrappers - must be safe to run

void Actor::cmdActorWalkTo(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Location location;
	Point movePoint;

	movePoint.x = atoi(argv[2]);
	movePoint.y = atoi(argv[3]);

	location.fromScreenPoint(movePoint);

	if (!validActorId(actorId)) {
		_vm->_console->DebugPrintf("Actor::cmActorWalkTo Invalid actorId 0x%X.\n", actorId);
		return;
	}

	actorWalkTo(actorId, location);
}

} // End of namespace Saga
