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
#include "saga/rscfile_mod.h"
#include "saga/script.h"
#include "saga/sndres.h"
#include "saga/sprite.h"
#include "saga/font.h"
#include "saga/text.h"
#include "saga/sound.h"
#include "saga/scene.h"
#include "saga/actionmap.h"

#include "saga/actor.h"
#include "saga/actordata.h"
#include "saga/stream.h"
#include "saga/interface.h"
#include "common/config-manager.h"

namespace Saga {

static int actorCompare(const ActorDataPointer& actor1, const ActorDataPointer& actor2) {
	if (actor1->location.y == actor2->location.y) {
		return 0;
	} else if (actor1->location.y < actor2->location.y) {
		return -1;
	} else {
		return 1;
	}
}

// Lookup table to convert 8 cardinal directions to 4
int actorDirectectionsLUT[8] = {
	ACTOR_DIRECTION_BACK,	// kDirUp
	ACTOR_DIRECTION_RIGHT,	// kDirUpRight
	ACTOR_DIRECTION_RIGHT,	// kDirRight
	ACTOR_DIRECTION_RIGHT,	// kDirDownRight
	ACTOR_DIRECTION_FORWARD,// kDirDown
	ACTOR_DIRECTION_LEFT,	// kDirDownLeft
	ACTOR_DIRECTION_LEFT,	// kDirLeft
	ACTOR_DIRECTION_LEFT,	// kDirUpLeft
};

PathDirectionData pathDirectionLUT[8][3] = {
	{{0,  0, -1}, {7, -1, -1}, {4,  1, -1}},
	{{1,  1,  0}, {4,  1, -1}, {5,  1,  1}},
	{{2,  0,  1}, {5,  1,  1}, {6, -1,  1}},
	{{3, -1,  0}, {6, -1,  1}, {7, -1, -1}},
	{{0,  0, -1}, {1,  1,  0}, {4,  1, -1}},
	{{1,  1,  0}, {2,  0,  1}, {5,  1,  1}},
	{{2,  0,  1}, {3, -1,  0}, {6, -1,  1}},
	{{3, -1,  0}, {0,  0, -1}, {7, -1, -1}}
};

int pathDirectionLUT2[8][2] =  {
	{ 0, -1},
	{ 1,  0},
	{ 0,  1},
	{-1,  0},
	{ 1, -1},
	{ 1,  1},
	{-1,  1},
	{-1, -1}
};

int angleLUT[16][2] = {
	{   0, -256},
	{  98, -237},
	{ 181, -181},
	{ 237,  -98},
	{ 256,    0},
	{ 237,	 98},
	{ 181,  181},
	{  98,  237},
	{   0,  256},
	{ -98,  237},
	{-181,  181},
	{-237,   98},
	{-256,    0},
	{-237,  -98},
	{-181, -181},
	{ -98, -237}
};

int directionLUT[8][2] = {
	{ 0*2, -2*2},
	{ 2*2, -1*2},
	{ 3*2,  0*2},
	{ 2*2,  1*2},
	{ 0*2,  2*2},
	{-2*2,  1*2},
	{-4*2,  0*2},
	{-2*2, -1*2}
};

int tileDirectionLUT[8][2] = {
	{ 1,  1},
	{ 2,  0},
	{ 1, -1},
	{ 0, -2},
	{-1, -1},
	{-2,  0},
	{-1,  1},
	{ 0,  2}
};

Actor::Actor(SagaEngine *vm) : _vm(vm) {
	int i;
	ActorData *actor;
	debug(9, "Actor::Actor()");

	if (_vm->_gameType == GType_IHNM) {
		warning("Actors aren't implemented for IHNM yet");
		return;
	}

	_centerActor = _protagonist = NULL;
	_lastTickMsec = 0;

	_yCellCount = _vm->getStatusYOffset() + 1;
	_xCellCount = _vm->getDisplayWidth();

	_pathCellCount = _yCellCount * _xCellCount;
	_pathCell = (int*)malloc(_pathCellCount * sizeof(*_pathCell));
	

	_pathRect.left = 0;
	_pathRect.right = _vm->getDisplayWidth() - 1;
	_pathRect.top = _vm->getPathYOffset();
	_pathRect.bottom = _vm->getStatusYOffset();

	// Get actor resource file context
	_actorContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_actorContext == NULL) {
		error("Actor::Actor(): Couldn't load actor module resource context.");
	}

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		actor->actorId = ACTOR_INDEX_TO_ID(i);
		actor->index = i;
		debug(9, "init actorId=%d index=%d", actor->actorId, actor->index);
		actor->nameIndex = ActorTable[i].nameIndex;
		actor->spriteListResourceId = ActorTable[i].spriteListResourceId;
		actor->frameListResourceId = ActorTable[i].frameListResourceId;
		actor->speechColor = ActorTable[i].speechColor;
		actor->sceneNumber = ActorTable[i].sceneIndex;
		actor->flags = ActorTable[i].flags;
		actor->currentAction = ActorTable[i].currentAction;
		actor->facingDirection = ActorTable[i].facingDirection;
		actor->actionDirection = ActorTable[i].actionDirection;
		actor->frameNumber = 0;
		actor->targetObject = ID_NOTHING;
		actor->actorFlags = 0;

		actor->location.x = ActorTable[i].x;
		actor->location.y = ActorTable[i].y;
		actor->location.z = ActorTable[i].z;

		actor->disabled = !loadActorResources(actor);
		if (actor->disabled) {
			warning("Disabling actorId=%d index=%d", actor->actorId, actor->index);
		} 
	}
}

Actor::~Actor() {
	int i;
	ActorData *actor;

	debug(9, "Actor::~Actor()");
	free(_pathCell);
	//release resources
	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		free(actor->frames);
		_vm->_sprite->freeSprite(actor->spriteList);
	}
}

bool Actor::loadActorResources(ActorData *actor) {
	byte *resourcePointer;
	size_t resourceLength;
	int framesCount;
	ActorFrameSequence *framesPointer;
	int lastFrame;
	int i, orient;

	if (actor->frameListResourceId == 0) {
		warning("Frame List ID = 0 for actor index %d", actor->index);
		return true;
	}

	debug(9, "Loading frame resource id %d", actor->frameListResourceId);
	if (RSC_LoadResource(_actorContext, actor->frameListResourceId, &resourcePointer, &resourceLength) != SUCCESS) {
		warning("Couldn't load sprite action index resource");
		return false;
	}

	framesCount = resourceLength / 16;
	debug(9, "Frame resource contains %d frames", framesCount);
	
	framesPointer = (ActorFrameSequence *)malloc(sizeof(ActorFrameSequence) * framesCount);
	if (framesPointer == NULL) {
		error("Couldn't allocate memory for sprite frames");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);

	lastFrame = 0;

	for (i = 0; i < framesCount; i++) {
		for (orient = 0; orient < ACTOR_DIRECTIONS_COUNT; orient++) {
			// Load all four orientations
			framesPointer[i].directions[orient].frameIndex = readS.readUint16();
			framesPointer[i].directions[orient].frameCount = readS.readSint16();
			if (framesPointer[i].directions[orient].frameCount < 0)
				warning("frameCount < 0", framesPointer[i].directions[orient].frameCount);
			if (framesPointer[i].directions[orient].frameIndex > lastFrame) {
				lastFrame = framesPointer[i].directions[orient].frameIndex;
			}
		}
	}

	RSC_FreeResource(resourcePointer);

	actor->frames = framesPointer;
	actor->framesCount = framesCount;


	debug(9, "Loading sprite resource id %d", actor->spriteListResourceId);
	if (_vm->_sprite->loadList(actor->spriteListResourceId, &actor->spriteList) != SUCCESS) {
		warning("Unable to load sprite list");
		return false;
	}

	i = _vm->_sprite->getListLen(actor->spriteList);

	if (lastFrame >= i) {
		debug(9, "Appending to sprite list %d (+ %d)", actor->spriteListResourceId, lastFrame);
		if (_vm->_sprite->appendList(actor->spriteListResourceId + 1, actor->spriteList) != SUCCESS) {
			warning("Unable append sprite list");
			return false;
		}
	}

	return true;
}

void Actor::realLocation(ActorLocation &location, uint16 objectId, uint16 walkFlags) {
	int angle;
	int distance;
	ActorData *actor;
	if (walkFlags & kWalkUseAngle) {
		// tiled stuff
		if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
			//todo: it
		} else {
			angle = location.x & 15;  
			distance = location.y;

			location.x = (angleLUT[angle][0] * distance) >> 6; //fixme - call real angle calc
			location.y = (angleLUT[angle][1] * distance) >> 6;
		}
	}

	if (objectId != ID_NOTHING) {
		if (IS_VALID_ACTOR_ID(objectId)) {
			actor = getActor(objectId);
			location.add( actor->location);
		} else {
			warning("ObjectId unsupported"); //todo: do it
		}
		
	}
}

ActorData *Actor::getActor(uint16 actorId) {
	ActorData *actor;
	
	if (!IS_VALID_ACTOR_ID(actorId))
		error("Actor::getActor Wrong actorId 0x%X", actorId);

	if (actorId == ID_PROTAG) {
		if (_protagonist == NULL) {
			error("_protagonist == NULL");
		}
		return _protagonist;
	}

	actor = &_actors[ACTOR_ID_TO_INDEX(actorId)];

	if (actor->disabled)
		error("Actor::getActor disabled actorId 0x%X", actorId);

	return actor;
}

bool Actor::validFollowerLocation(const ActorLocation &location) {
	Point point;
	location.toScreenPointXY(point);
	
	if ((point.x < 5) || (point.x >= _vm->getDisplayWidth() - 5) ||
		(point.y < 0) || (point.y >= _vm->getStatusYOffset())) {
		return false;
	}
	
	return (_vm->_scene->canWalk(point));
}

void Actor::updateActorsScene() {
	int i, j;
	int followerDirection;
	ActorData *actor;
	ActorLocation tempLocation;
	ActorLocation possibleLocation;
	Point delta;
	
	_activeSpeech.stringsCount = 0;
	_protagonist = NULL;

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];		
		if (actor->flags & (kProtagonist | kFollower)) {
			actor->sceneNumber = _vm->_scene->currentSceneNumber();
			if (actor->flags & kProtagonist) {
				actor->finalTarget = actor->location;
				_centerActor = _protagonist = actor;
			}
		}
		if (actor->sceneNumber == _vm->_scene->currentSceneNumber()) {
			actor->actionCycle = (rand() & 7) * 4;
		}
	}
	
	assert(_protagonist);

/* setup protagonist entry
	// tiled stuff
	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
	}
*/
	_protagonist->currentAction = kActionWait;

	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
		_vm->_scene->initDoorsState();
	}

	followerDirection = _protagonist->facingDirection + 3;
	calcActorScreenPosition(_protagonist);

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];		
		if (actor->flags & (kFollower)) {
			actor->facingDirection = actor->actionDirection = _protagonist->facingDirection;
			actor->currentAction = kActionWait;
			actor->walkStepsCount = actor->walkStepIndex = 0;
			actor->location.z = _protagonist->location.z;
				

			if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
				//todo: it
			} else {
				followerDirection &= 0x07;
				
				possibleLocation = _protagonist->location;


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

				actor->location = possibleLocation;
			}
			followerDirection += 2;
		}

	}

	handleActions(0, true);
}

ActorFrameRange *Actor::getActorFrameRange(uint16 actorId, int frameType) {
	ActorData *actor;
	int fourDirection;

	actor = getActor(actorId);
	if (actor->disabled)
		error("Actor::getActorFrameRange Wrong actorId 0x%X", actorId);

	if (frameType >= actor->framesCount)
		error("Actor::getActorFrameRange Wrong frameType 0x%X actorId 0x%X", frameType, actorId);
	
	if ((actor->facingDirection < kDirUp) || (actor->facingDirection > kDirUpLeft))
		error("Actor::getActorFrameRange Wrong direction 0x%X actorId 0x%X", actor->facingDirection, actorId);

	fourDirection = actorDirectectionsLUT[actor->facingDirection];
	return &actor->frames[frameType].directions[fourDirection];
}

void Actor::handleSpeech(int msec) {
	int stringLength;
	int sampleLength;
	bool removeFirst;
	int i;
	int talkspeed;
	ActorData *actor;

	if (!isSpeaking()) return;

	stringLength = strlen(_activeSpeech.strings[0]);

	if (stringLength == 0)
		error("Empty strings not allowed");

	if (_vm->_script->_skipSpeeches) {
		_activeSpeech.stringsCount = 0;
		_vm->_sound->stopVoice();
		_vm->_script->wakeUpThreads(kWaitTypeSpeech);
		return;
	}

	if (!_activeSpeech.playing) {  // just added
		talkspeed = ConfMan.getInt("talkspeed");
		if (_activeSpeech.speechFlags & kSpeakSlow) {
			if (_activeSpeech.slowModeCharIndex >= stringLength)
				error("Wrong string index");

			debug(0 , "Slow string!");
			_activeSpeech.playingTime = 10 * talkspeed;
			// 10 - fix it 

		} else {
			sampleLength = _vm->_sndRes->getVoiceLength(_activeSpeech.sampleResourceId); //fixme - too fast

			if (sampleLength < 0) {
				_activeSpeech.playingTime = stringLength * talkspeed;
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
				actor->currentAction = kActionSpeak;
				actor->actionCycle = rand() % 64; 
			}
		}
		_activeSpeech.playing = true;			
		return;
	}


	_activeSpeech.playingTime -= msec;

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
				actor->currentAction = kActionWait;
			}
		}
	}

	if (removeFirst) {
		for (i = 1; i < _activeSpeech.stringsCount; i++) {
			_activeSpeech.strings[i - 1] = _activeSpeech.strings[i];
		}
		_activeSpeech.stringsCount--;
	}

	if (!isSpeaking())
		_vm->_script->wakeUpThreadsDelayed(kWaitTypeSpeech, ticksToMSec(kScriptTimeTicksPerSecond / 3));
}

void Actor::handleActions(int msec, bool setup) {
	int i;
	ActorData *actor;
	ActorFrameRange *frameRange;
	int state;
	int speed;
	ActorLocation delta;
	ActorLocation addDelta;

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;
		if (actor->sceneNumber != _vm->_scene->currentSceneNumber()) continue;
		
		//todo: dragon stuff

		if (actor->index == 2)
			debug(9, "Action: %d Flags: %x", actor->currentAction, actor->flags);

		switch(actor->currentAction) {
			case kActionWait:
				if (!setup && (actor->flags & kFollower)) {
					followProtagonist(actor);
					if (actor->currentAction != kActionWait)
						break;
				}

				if (actor->targetObject != ID_NOTHING) {
					//todo: facetowardsobject
				}

				if (actor->flags & kCycle) {
					frameRange = getActorFrameRange(actor->actorId, kFrameStand);
					if (frameRange->frameCount > 0) {
						actor->actionCycle++;
						actor->actionCycle = (actor->actionCycle) % frameRange->frameCount;
					} else {
						actor->actionCycle = 0;
					}
					actor->frameNumber = frameRange->frameIndex + actor->actionCycle;
					break;
				}

				if ((actor->actionCycle & 3) == 0) {
					actor->cycleWrap(100);

					frameRange = getActorFrameRange(actor->actorId, kFrameWait);
					if ((frameRange->frameCount < 1 || actor->actionCycle > 33))
						frameRange = getActorFrameRange(actor->actorId, kFrameStand);

					if (frameRange->frameCount) {
						actor->frameNumber = frameRange->frameIndex + (uint16)rand() % frameRange->frameCount;
					} else {
						actor->frameNumber = frameRange->frameIndex;
					}
				}
				actor->actionCycle++;
				break;

			case kActionWalkToPoint:
			case kActionWalkToLink:
				// tiled stuff
				if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
					//todo: it
				} else {
					actor->partialTarget.delta(actor->location, delta);

					while ((delta.x == 0) && (delta.y == 0)) {

						if (actor->walkStepIndex >= actor->walkStepsCount) {
							actorEndWalk(actor->actorId, true); 
							break;
						}

						actor->partialTarget.fromScreenPoint(actor->walkStepsPoints[actor->walkStepIndex++]);
						if (actor->partialTarget.x > 224 * 2 * ACTOR_LMULT) {
							actor->partialTarget.x -= 256 * 2 * ACTOR_LMULT;
						}

						actor->partialTarget.delta(actor->location, delta);

						if (ABS(delta.y) > ABS(delta.x)) {
							actor->actionDirection = delta.y > 0 ? kDirDown : kDirUp;
						} else {
							actor->actionDirection = delta.x > 0 ? kDirRight : kDirLeft;
						}
					}

					speed = (ACTOR_LMULT * 2 * actor->screenScale + 63) / 256;
					if (speed < 1) {
						speed = 1;
					}

				if ((actor->actionDirection == kDirUp) || (actor->actionDirection == kDirDown)) {
					// move by 2's in vertical dimension
						addDelta.y = clamp(-speed, delta.y, speed);
						if (addDelta.y == delta.y) {
							addDelta.x = delta.x;
						} else {
							addDelta.x = delta.x * addDelta.y; 
							addDelta.x += (addDelta.x > 0) ? (delta.y / 2) : (-delta.y / 2);
							addDelta.x /= delta.y;
							actor->facingDirection = actor->actionDirection;
						}
					} else {						
						addDelta.x = clamp(-2 * speed, delta.x, 2 * speed);
						if (addDelta.x == delta.x) {
							addDelta.y = delta.y;
						} else {
							addDelta.y = delta.y * addDelta.x;
							addDelta.y += (addDelta.y > 0) ? (delta.x / 2) : (-delta.x / 2);
							addDelta.y /= delta.x;
							actor->facingDirection = actor->actionDirection;
						}
					}

					actor->location.add(addDelta);
				}

				if (actor->actorFlags & kActorBackwards) {
					actor->facingDirection = (actor->actionDirection + 4) & 7;
					actor->actionCycle--;
				} else {
					actor->actionCycle++;
				}

				frameRange = getActorFrameRange(actor->actorId, actor->walkFrameSequence);

				if (actor->actionCycle < 0) {
					actor->actionCycle = frameRange->frameCount - 1;
				} else {
					if (actor->actionCycle >= frameRange->frameCount) {
						actor->actionCycle = 0;
					}
				}

				actor->frameNumber = frameRange->frameIndex + actor->actionCycle;
				break;

			case kActionWalkDir:
				// tiled stuff
				if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
					//todo: it
				} else {
					actor->location.x += directionLUT[actor->actionDirection][0] * 2;
					actor->location.y += directionLUT[actor->actionDirection][1] * 2;

					frameRange = getActorFrameRange(actor->actorId, actor->walkFrameSequence);
					actor->actionCycle++;
					actor->cycleWrap(frameRange->frameCount);
					actor->frameNumber = frameRange->frameIndex + actor->actionCycle;
				}
				break;

			case kActionSpeak:
				actor->actionCycle++;
				actor->cycleWrap(64);

				frameRange = getActorFrameRange(actor->actorId, kFrameGesture);
				if (actor->actionCycle >= frameRange->frameCount) {
				if (actor->actionCycle & 1)
					break;
					frameRange = getActorFrameRange(actor->actorId, kFrameSpeak);

					state = (uint16)rand() % (frameRange->frameCount + 1);

					if (state == 0) {
						frameRange = getActorFrameRange(actor->actorId, kFrameStand);
					} else {
						state--;
					}
				} else {
					state = actor->actionCycle;
				}

				actor->frameNumber = frameRange->frameIndex + state;
				break;

			case kActionAccept:
			case kActionStoop:
				break;

			case kActionCycleFrames:
			case kActionPongFrames:
				if (actor->cycleTimeCount > 0) {
					actor->cycleTimeCount--;
					break;
				}

				actor->cycleTimeCount = actor->cycleDelay;
				actor->actionCycle++;

				frameRange = getActorFrameRange(actor->actorId, actor->cycleFrameSequence);
				
				if (actor->currentAction == kActionPongFrames) {
					if (actor->actionCycle >= frameRange->frameCount * 2 - 2) {
						if (actor->actorFlags & kActorContinuous) {
							actor->actionCycle = 0;
						} else {
							actor->currentAction = kActionFreeze;
							break;
						}
					}

					state = actor->actionCycle;
					if (state >= frameRange->frameCount) {
						state = frameRange->frameCount * 2 - 2 - state;
					}
				} else {
					if (actor->actionCycle >= frameRange->frameCount) {
						if (actor->actorFlags & kActorContinuous) {
							actor->actionCycle = 0;
						} else {
							actor->currentAction = kActionFreeze;
							break;
						}
					}
					state = actor->actionCycle;
				}

				if (frameRange->frameCount && (actor->actorFlags & kActorRandom)) {
					state = rand() % frameRange->frameCount;
				}

				if (actor->actorFlags & kActorBackwards) {
					actor->frameNumber = frameRange->frameIndex + frameRange->frameCount - 1 - state;
				} else {
					actor->frameNumber = frameRange->frameIndex + state;
				}
				break;

			case kActionFall:
				debug(9,"kActionFall not implemented");

				//todo: do it
				break;

			case kActionClimb:
				debug(9,"kActionClimb not implemented");

				//todo: do it
				break;
		}
	}

}

int Actor::direct(int msec) {

	// FIXME: HACK. This should be turned into cycle event.
	_lastTickMsec += msec;

	if (_lastTickMsec > ticksToMSec(5)) { // fixme
		_lastTickMsec = 0;
		//process actions
		handleActions(msec, false);
	}

//process speech
	handleSpeech(msec);
	return SUCCESS;
}

void Actor::calcActorScreenPosition(ActorData *actor) {
	int	beginSlope, endSlope, middle;
	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
		middle = _vm->getStatusYOffset() - actor->location.y / ACTOR_LMULT;

		_vm->_scene->getSlopes(beginSlope, endSlope);

		actor->screenDepth = (14 * middle) / endSlope + 1;

		if (middle <= beginSlope) {
			actor->screenScale = 256;
		} else {
			if (middle >= endSlope) {
				actor->screenScale = 1;
			} else {
				middle -= beginSlope;
				endSlope -= beginSlope;
				actor->screenScale = 256 - (middle * 256) / endSlope;
			}
		}

		actor->location.toScreenPointXYZ(actor->screenPosition);
	}

	if (actor->index == 2)
		debug(9, "act: %d. x: %d y: %d", actor->index, actor->screenPosition.x, actor->screenPosition.y);
}

void Actor::createDrawOrderList() {
	int i;
	ActorData *actor;

	_drawOrderList.clear();
	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;
		if (actor->sceneNumber != _vm->_scene->currentSceneNumber()) continue;

		_drawOrderList.pushBack(actor, actorCompare);

		calcActorScreenPosition(actor);
	}
}

int Actor::drawActors() {
	ActorOrderList::iterator actorDrawOrderIterator;
	ActorData *actor;
	int frameNumber;
	SPRITELIST *spriteList;

	SURFACE *back_buf;

	back_buf = _vm->_gfx->getBackBuffer();

	createDrawOrderList();

	for (actorDrawOrderIterator = _drawOrderList.begin(); actorDrawOrderIterator != _drawOrderList.end(); ++actorDrawOrderIterator) {
		actor =  actorDrawOrderIterator.operator*();

		if (_vm->_scene->currentSceneNumber() == RID_ITE_OVERMAP_SCENE) {
			if (!(actor->flags & kProtagonist)){
				warning("not protagonist");
				continue;
			}
			frameNumber = 8;			
			spriteList = _vm->_mainSprites;
		} else {
			frameNumber = actor->frameNumber;			
			spriteList = actor->spriteList;
		}
		
		if ((frameNumber < 0) || (spriteList->sprite_count <= frameNumber)) {
			warning("Actor::drawActors frameNumber invalid for actorId 0x%X", actor->actorId);
			continue;
		}
		
		if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
			//todo: it
		} else {
			_vm->_sprite->drawOccluded(back_buf, spriteList, frameNumber, actor->screenPosition, actor->screenScale, actor->screenDepth);
		}
	}

// draw speeches
	if (isSpeaking() && !_vm->_script->_skipSpeeches) {
		int i;
		int textDrawFlags, speechColor;
		Point speechCoord;
		char oneChar[2];
		oneChar[1] = 0;
		const char *outputString;

		if (_activeSpeech.speechFlags & kSpeakSlow) {
			outputString = oneChar;
			oneChar[0] = _activeSpeech.strings[0][_activeSpeech.slowModeCharIndex];
		} else {
			outputString = _activeSpeech.strings[0];
		}

		textDrawFlags = FONT_CENTERED;
		if (_activeSpeech.outlineColor != 0) {
			textDrawFlags |= FONT_OUTLINE;
		}

		if (_activeSpeech.actorIds[0] != 0) {
			
			for (i = 0; i < _activeSpeech.actorsCount; i++){
				actor = getActor(_activeSpeech.actorIds[i]);
				speechCoord.x = actor->screenPosition.x;
				speechCoord.y = actor->screenPosition.y;
				speechCoord.y -= ACTOR_DIALOGUE_HEIGHT;
				if (_activeSpeech.actorsCount > 1)
					speechColor = actor->speechColor;
				else
					speechColor = _activeSpeech.speechColor;

				_vm->textDraw(MEDIUM_FONT_ID, back_buf, outputString, speechCoord.x, speechCoord.y, speechColor, _activeSpeech.outlineColor, textDrawFlags);
			}

		} else { // non actors speech
			warning("non actors speech occures");
			//todo: write it
		}

	}

	return SUCCESS;
}

bool Actor::followProtagonist(ActorData *actor) {
	ActorLocation protagonistLocation;
	ActorLocation newLocation;
	ActorLocation delta;
	int protagonistBGMaskType;
	Point prefer1;
	Point prefer2;
	Point prefer3;
	
	assert(_protagonist);

	actor->flags &= ~(kFaster | kFastest);
	protagonistLocation = _protagonist->location;
	calcActorScreenPosition(_protagonist);

	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {		
		prefer1.x = (100 * _protagonist->screenScale) >> 8;
		prefer1.y = (50 * _protagonist->screenScale) >> 8;

		if (_protagonist->currentAction == kActionWalkDir) {
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

		actor->location.delta(protagonistLocation, delta);
		
		protagonistBGMaskType = 0;
		if (_vm->_scene->isBGMaskPresent()) {
			if (_vm->_scene->validBGMaskPoint(_protagonist->screenPosition)) {
				protagonistBGMaskType = _vm->_scene->getBGMaskType(_protagonist->screenPosition);
			}
		}

		

		if ((rand() & 0x7) == 0)
			actor->actorFlags &= ~kActorNoFollow;

		if (actor->actorFlags & kActorNoFollow) {
			return false;
		}

		if ((delta.x > prefer2.x) || (delta.x < -prefer2.x) ||
			(delta.y > prefer2.y) || (delta.y < -prefer2.y) ||
			((_protagonist->currentAction == kActionWait) &&
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
					newLocation.x += (rand() % prefer1.x) - prefer1.x / 2;
					newLocation.y += (rand() % prefer1.y) - prefer1.y / 2;
				}

				newLocation.x = clamp(-31*4, newLocation.x, (_vm->getDisplayWidth() + 31) * 4); //fixme

				return actorWalkTo(actor->actorId, newLocation);
			}
	}
	return false;
}

bool Actor::actorEndWalk(uint16 actorId, bool recurse) {
	bool walkMore = false;
	ActorData *actor;

	actor = getActor(actorId);
	actor->actorFlags &= ~kActorBackwards;

	if (actor->location.distance(actor->finalTarget) > 8) {
		if ((actor->flags & kProtagonist) && recurse && !(actor->actorFlags & kActorNoCollide)) {
			actor->actorFlags |= kActorNoCollide;
			return actorWalkTo(actorId, actor->finalTarget);
		}
	}

	actor->currentAction = kActionWait;
	if (actor->actorFlags & kActorFinalFace) {
		actor->facingDirection = actor->actionDirection = (actor->actorFlags >> 6) & 0x07; //?
	}

	actor->actorFlags &= ~(kActorNoCollide | kActorCollided | kActorFinalFace | kActorFacingMask);
	actor->flags &= ~(kFaster | kFastest);

	if (actor == _protagonist) {
		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
		//todo: it

	} else {
		if (recurse && (actor->flags & kFollower))
			walkMore = followProtagonist(actor);

		_vm->_script->wakeUpActorThread(kWaitTypeWalk, actor);
	}
	return walkMore;
}

bool Actor::actorWalkTo(uint16 actorId, const ActorLocation &toLocation) {
	ActorData *actor;
	ActorData *anotherActor;
	int	i;

	Rect testBox;
	Rect testBox2;
	Point anotherActorScreenPosition;
	Point collision;
	Point pointFrom, pointTo, pointBest, pointAdd;
	Point delta, bestDelta;
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

	if (_vm->_scene->getMode() == SCENE_MODE_ISO) {
		//todo: it
	} else {
		
		actor->location.toScreenPointXY(pointFrom);

		extraStartNode = _vm->_scene->offscreenPath(pointFrom);

		toLocation.toScreenPointXY(pointTo);

		extraEndNode = _vm->_scene->offscreenPath(pointTo);

		if (_vm->_scene->isBGMaskPresent()) {

			if ((((actor->currentAction >= kActionWalkToPoint) && 
				(actor->currentAction <= kActionWalkDir)) || (actor == _protagonist)) && 
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

			if (!(actor->actorFlags & kActorNoCollide)) {
				collision.x = ACTOR_COLLISION_WIDTH * actor->screenScale / (256 * 2);
				collision.y = ACTOR_COLLISION_HEIGHT * actor->screenScale / (256 * 2);
				
				_barrierCount = 0;

				for (i = 0; (i < ACTORCOUNT) && (_barrierCount < ACTOR_BARRIERS_MAX); i++) {
					anotherActor = &_actors[i];
					if (anotherActor->disabled) continue;
					if (anotherActor->sceneNumber != _vm->_scene->currentSceneNumber()) continue;
					if (anotherActor == actor ) continue;


					anotherActorScreenPosition = anotherActor->screenPosition;
					testBox.left = anotherActorScreenPosition.x - collision.x;
					testBox.right = anotherActorScreenPosition.x + collision.x;
					testBox.top = anotherActorScreenPosition.y - collision.y;
					testBox.bottom = anotherActorScreenPosition.y + collision.y;
					testBox2 = testBox;
					testBox2.right += 2;
					testBox2.left -= 1;
					testBox2.bottom += 1;

					if (testBox2.contains(pointFrom)) {
						if (pointFrom.x > anotherActorScreenPosition.x + 4) {
							testBox.right = pointFrom.x - 2;
						} else {
							if (pointFrom.x < anotherActorScreenPosition.x - 4) {
								testBox.left = pointFrom.x + 2;
							} else {
								if (pointFrom.y > anotherActorScreenPosition.y) {
									testBox.bottom = pointFrom.y - 1; 
								} else {
									testBox.top = pointFrom.y + 1 ;
								}
							}
						}
					}

					if ((testBox.left <= testBox.right) && (testBox.top <= testBox.bottom)) {
						_barrierList[_barrierCount++] = testBox;
					}
				}
			}


			pointBest = pointTo;
			actor->walkStepsCount = 0;
			findActorPath(actor, pointFrom, pointTo);

			if (actor->walkStepsCount == 0) {
				error("actor->walkStepsCount == 0");
			}

			if (extraStartNode) {
				actor->walkStepIndex = 0;
			} else {
				actor->walkStepIndex = 1;
			}

			if (extraEndNode) {
				Point tempPoint;
				toLocation.toScreenPointXY(tempPoint);
				actor->walkStepsCount--;
				actor->addWalkStepPoint(tempPoint);
			}


			pointBest = actor->walkStepsPoints[actor->walkStepsCount - 1];

			pointFrom.x &= ~1;
			delta.x = ABS(pointFrom.x - pointTo.x);
			delta.y = ABS(pointFrom.y - pointTo.y);

			bestDelta.x = ABS(pointBest.x - pointTo.x);
			bestDelta.y = ABS(pointBest.y - pointTo.y);

			if (delta.x + delta.y <= bestDelta.x + bestDelta.y) {
				if (actor->flags & kFollower)
					actor->actorFlags |= kActorNoFollow;
			}

			if (pointBest == pointFrom) {
				actor->walkStepsCount = 0;
			}			
		} else {
			actor->walkStepsCount = 0;
			actor->addWalkStepPoint(pointTo);
			actor->walkStepIndex = 0;
		}

		actor->partialTarget = actor->location;
		actor->finalTarget = toLocation;
		if (actor->walkStepsCount == 0) {
			actorEndWalk(actorId, false);
			return false;
		} else {
			if (actor->flags & kProtagonist) {
				_actors[1].actorFlags &= ~kActorNoFollow;
				_actors[2].actorFlags &= ~kActorNoFollow;
			}			
			actor->currentAction = (actor->walkStepsCount >= ACTOR_MAX_STEPS_COUNT) ? kActionWalkToLink : kActionWalkToPoint;
			actor->walkFrameSequence = kFrameWalk;
		}

	}
	return true;
}

void Actor::actorSpeech(uint16 actorId, const char **strings, int stringsCount, uint16 sampleResourceId, int speechFlags) {
	ActorData *actor;
	int i;

	actor = getActor(actorId);
	for (i = 0; i < stringsCount; i++) {		
		_activeSpeech.strings[i] = strings[i];
	}
	_activeSpeech.stringsCount = stringsCount;
	_activeSpeech.speechFlags = speechFlags;
	_activeSpeech.actorsCount = 1;
	_activeSpeech.actorIds[0] = actorId;
	_activeSpeech.speechColor = actor->speechColor;
	_activeSpeech.outlineColor = 15; // fixme - BLACK
	_activeSpeech.sampleResourceId = sampleResourceId;
	_activeSpeech.playing = false;
	_activeSpeech.slowModeCharIndex = 0;
}

void Actor::nonActorSpeech(const char **strings, int stringsCount, int speechFlags) {
	int i;
	
	_vm->_script->wakeUpThreads(kWaitTypeSpeech);

	for (i = 0; i < stringsCount; i++) {		
		_activeSpeech.strings[i] = strings[i];
	}
	_activeSpeech.stringsCount = stringsCount;
	_activeSpeech.speechFlags = speechFlags;
	_activeSpeech.actorsCount = 1;
	_activeSpeech.actorIds[0] = 0;
	//_activeSpeech.speechColor = ;
	//_activeSpeech.outlineColor = ; 
	_activeSpeech.sampleResourceId = -1;
	_activeSpeech.playing = false;
	_activeSpeech.slowModeCharIndex = 0;
}

void Actor::simulSpeech(const char *string, uint16 *actorIds, int actorIdsCount, int speechFlags) {
	int i;
	
	for (i = 0; i < actorIdsCount; i++) {		
		_activeSpeech.actorIds[i] = actorIds[i];
	}
	_activeSpeech.actorsCount = actorIdsCount;
	_activeSpeech.strings[0] = string;
	_activeSpeech.stringsCount = 1;
	_activeSpeech.speechFlags = speechFlags;
	//_activeSpeech.speechColor = ; // get's from every actor 
	_activeSpeech.outlineColor = 0; // disable outline 
	_activeSpeech.sampleResourceId = -1;
	_activeSpeech.playing = false;
	_activeSpeech.slowModeCharIndex = 0;
	
	// caller should call thread->wait(kWaitTypeSpeech) by itself
}

void Actor::abortAllSpeeches() {
	if (_vm->_script->_abortEnabled)
		_vm->_script->_skipSpeeches = true;

	for (int i = 0; i < 10; i++)
		_vm->_script->executeThreads(0);
}

void Actor::abortSpeech() {
	_vm->_sound->stopVoice();
	_activeSpeech.playingTime = 0;
}

void Actor::findActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point iteratorPoint;
	Point bestPoint;
	int maskType;
	int cellValue;
	int i;
	Rect intersect;
	

	actor->walkStepsCount = 0;
	if (fromPoint == toPoint) {
		actor->addWalkStepPoint(toPoint);
		return;
	}

	for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
		for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
			maskType = _vm->_scene->getBGMaskType(iteratorPoint);
			cellValue = maskType ? kPathCellBarrier : kPathCellEmpty;
			setPathCell(iteratorPoint, cellValue);
		}
	}

	for (i = 0; i < _barrierCount; i++) {
		intersect.left = MAX(_pathRect.left, _barrierList[i].left);
		intersect.top = MAX(_pathRect.top, _barrierList[i].top);
		intersect.right = MIN(_pathRect.right, _barrierList[i].right);
		intersect.bottom = MIN(_pathRect.bottom, _barrierList[i].bottom);
		

		for (iteratorPoint.y = intersect.top; iteratorPoint.y <= intersect.bottom; iteratorPoint.y++) {
			for (iteratorPoint.x = intersect.left; iteratorPoint.x <= intersect.right; iteratorPoint.x++) {
				setPathCell(iteratorPoint, kPathCellBarrier);
			}
		}
	}
	


	if (scanPathLine(fromPoint, toPoint)) {
		actor->addWalkStepPoint(fromPoint);
		actor->addWalkStepPoint(toPoint);
		return;
	}
	

	i = fillPathArray(fromPoint, toPoint, bestPoint);

#if 0
	{
		Point iteratorPoint;
		int cellValue;
		FILE	*fp =	fopen("d:\\FINDPATH.DAT", "w");
		char	c;

		fprintf(fp, "from = (%d,%d)\n", fromPoint.x, fromPoint.y);
		fprintf(fp, "to = (%d,%d)\n", toPoint.x, toPoint.y);
		fprintf(fp, "bestPoint = (%d,%d)\n", bestPoint.x, bestPoint.y);

		for (iteratorPoint.y = 0; iteratorPoint.y < _yCellCount; iteratorPoint.y++) {
			for (iteratorPoint.x = 0; iteratorPoint.x < _xCellCount; iteratorPoint.x++) {
				cellValue = getPathCell(iteratorPoint);
				c = (cellValue < 0) ? ' ' : (cellValue == kPathCellBarrier) ? kPathCellBarrier : (cellValue < 8) ? "^>v<?jLP"[cellValue] : '.';
				putc(c, fp);
			}
			putc('\n', fp);
		}
		fclose(fp);
	}
#endif

	if (fromPoint == bestPoint) {
		actor->addWalkStepPoint(bestPoint);
		return;
	}

	if (i != 0)
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
	Point bestPath;
	int pointCounter;
	int startDirection;
	PathDirectionList pathDirectionList;
	PathDirectionData *pathDirection;
	PathDirectionData *samplePathDirection;
	PathDirectionList::iterator pathDirectionIterator;
	PathDirectionList::iterator newPathDirectionIterator;
	int directionCount;

	pointCounter = 0;
	bestRating = quickDistance(fromPoint, toPoint);
	bestPath = fromPoint;
	
	for (startDirection = 0; startDirection < 4; startDirection++) {
		newPathDirectionIterator = pathDirectionList.pushBack();
		pathDirection = newPathDirectionIterator.operator->();
		pathDirection->x = fromPoint.x;
		pathDirection->y = fromPoint.y;
		pathDirection->direction = startDirection;
	}

	if (validPathCellPoint(fromPoint)) {
		setPathCell(fromPoint, 0);
	}	
	
	pathDirectionIterator = pathDirectionList.begin();

	do {
		pathDirection = pathDirectionIterator.operator->();
		for (directionCount = 0; directionCount < 3; directionCount++) {
			samplePathDirection = &pathDirectionLUT[pathDirection->direction][directionCount];
			Point nextPoint;
			nextPoint.x = samplePathDirection->x + pathDirection->x;
			nextPoint.y = samplePathDirection->y + pathDirection->y;
			if (validPathCellPoint(nextPoint) && 
				(getPathCell(nextPoint) == kPathCellEmpty)) {
				setPathCell(nextPoint, samplePathDirection->direction);

				newPathDirectionIterator = pathDirectionList.pushBack();
				pathDirection = newPathDirectionIterator.operator->();
				pathDirection->x = nextPoint.x;
				pathDirection->y = nextPoint.y;
				pathDirection->direction = samplePathDirection->direction;
				++pointCounter;
				if (nextPoint == toPoint) {
					bestPoint = toPoint;
					return pointCounter;
				}
				currentRating = quickDistance(nextPoint, toPoint);
				if (currentRating  < bestRating) {
					bestRating = currentRating;
					bestPath = nextPoint;
				}
			}
		}
		++pathDirectionIterator;
	} while (pathDirectionIterator != pathDirectionList.end());

	bestPoint = bestPath;	
	return pointCounter;
}

void Actor::setActorPath(ActorData *actor, const Point &fromPoint, const Point &toPoint) {
	Point *point;
	Point nextPoint;
	int direction;
	PathNode *node;
	int i, last;

	_pathList[0] = toPoint;
	nextPoint = toPoint;
	_pathListIndex = 0;

	point = _pathList;
	while ( !(nextPoint == fromPoint)) {
		_pathListIndex++;
		if (_pathListIndex >= PATH_LIST_MAX) {
			error("Actor::setActorPath PATH_LIST_MAX");
		}
		point++;
		direction = getPathCell(nextPoint);
		if ((direction < 0) || (direction > 8)) {
			error("Actor::setActorPath error direction 0x%X", direction);
		}
		nextPoint.x -= pathDirectionLUT2[direction][0];
		nextPoint.y -= pathDirectionLUT2[direction][1];
		point->x = nextPoint.x;
		point->y = nextPoint.y;
	}

	pathToNode();

	removeNodes();	

    nodeToPath();

	removePathPoints();

	_pathNodeIndex++;
	last = MIN(_pathNodeIndex, PATH_NODE_MAX);
	for (i = 0, node = _pathNodeList; i < last; i++, node++) {
		nextPoint.x = node->x;
		nextPoint.y = node->y;
		actor->addWalkStepPoint(nextPoint);
	}

}

void Actor::pathToNode() {
	Point point1, point2, delta;
	int direction;
	int i;
	Point *point;
	PathNode *nodeList;

	_pathNodeIndex = 0;
	point= &_pathList[_pathListIndex];	
	direction = 0;

	_pathNodeList->x = point->x;
	_pathNodeList->y = point->y;
	nodeList = _pathNodeList;

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
			++nodeList;
			++_pathNodeIndex;
			nodeList->x = point1.x;
			nodeList->y = point2.x;
			direction--;
			i++;
			point++;
		}
	}
	++nodeList;
	++_pathNodeIndex;
	nodeList->x = _pathList->x;
	nodeList->y = _pathList->y;
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

	for (i = 0, point = _pathList; i < PATH_LIST_MAX; i++, point++) {
		point->x = point->y = PATH_NODE_EMPTY;
	}

	_pathListIndex = 1;
	_pathList[0].x = _pathNodeList[0].x;
	_pathList[0].y = _pathNodeList[0].y;
	_pathNodeList[0].link = 0;
	for (i = 0, node = _pathNodeList; i < _pathNodeIndex; i++) {
		point1.x = node->x;
		point1.y = node->y;
		node++;
		point2.x = node->x;
		point2.y = node->y;
		_pathListIndex += pathLine(&_pathList[_pathListIndex], point1, point2);
		node->link = _pathListIndex - 1;
	}
	_pathListIndex--;
	_pathNodeList[_pathNodeIndex].link = _pathListIndex;

}

void Actor::removeNodes() {
	int i, j, k;
	PathNode *iNode, *jNode, *kNode, *fNode;
	Point point1, point2;
	fNode = &_pathNodeList[_pathNodeIndex];
	
	point1.x = _pathNodeList[0].x;
	point1.y = _pathNodeList[0].y;
	point2.x = fNode->x;
	point2.y = fNode->y;

	if (scanPathLine(point1, point2)) {
		_pathNodeList[1] = *fNode;
		_pathNodeIndex = 1;
	}

	if (_pathNodeIndex < 4) {
		return;
	}

	for (i = _pathNodeIndex - 1, iNode = fNode-1; i > 1 ; i--, iNode--) {
		if (iNode->x == PATH_NODE_EMPTY) {
			continue;
		}

		point1.x = _pathNodeList[0].x;
		point1.y = _pathNodeList[0].y;
		point2.x = iNode->x;
		point2.y = iNode->y;

		if (scanPathLine(point1, point2)) {
			for (j = 1, jNode = _pathNodeList + 1; j < i; j++, jNode++) {
				jNode->x = PATH_NODE_EMPTY;
			}
		}
	}

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeIndex - 1; i++, iNode++) {
		if (iNode->x == PATH_NODE_EMPTY) {
			continue;
		}
		point1.x = fNode->x;
		point1.y = fNode->y;
		point2.x = iNode->x;
		point2.y = iNode->y;

		if (scanPathLine(point1, point2)) {
			for (j = i + 1, jNode = iNode + 1; j < _pathNodeIndex; j++, jNode++) {
				jNode->x = PATH_NODE_EMPTY;
			}
		}
	}
	condenseNodeList();

	for (i = 1, iNode = _pathNodeList + 1; i < _pathNodeIndex - 1; i++, iNode++) {
		if (iNode->x == PATH_NODE_EMPTY) {
			continue;
		}
		for (j = i + 2, jNode = iNode + 2; j < _pathNodeIndex; j++, jNode++)
		{
			if (jNode->x == PATH_NODE_EMPTY) {
				continue;
			}

			point1.x = iNode->x;
			point1.y = iNode->y;
			point2.x = jNode->x;
			point2.y = jNode->y;

			if (scanPathLine(point1, point2)) {
				for (k = i + 1,kNode = iNode + 1; k < j; k++, kNode++) {
					kNode->x = PATH_NODE_EMPTY;
				}
			}
		}
	}
	condenseNodeList();
}

void Actor::condenseNodeList() {
	int i, j, count;
	PathNode *iNode, *jNode;
	
	count = _pathNodeIndex;

	for (i = 1, iNode = _pathNodeList+1; i < _pathNodeIndex; i++, iNode++) {
		if (iNode->x == PATH_NODE_EMPTY) {
			j = i + 1;
			jNode = iNode + 1;
			while ( jNode->x == PATH_NODE_EMPTY ) {
				j++;
				jNode++;
			}
			*iNode = *jNode;
			count = i;
			jNode->x = PATH_NODE_EMPTY;
			if (j == _pathNodeIndex) {
				break;
			}
		}
	}
	_pathNodeIndex = count;
}

void Actor::removePathPoints() {
	int i, j, k, l;
	PathNode *node;
	int newPathNodeIndex;
	int start;
	int end;
	Point point1, point2, point3, point4;


	if (_pathNodeIndex < 2)
		return;


	_newPathNodeList[0] = _pathNodeList[0];
	newPathNodeIndex = 0;

	for (i = 1, node = _pathNodeList + 1; i < _pathNodeIndex; i++, node++) {
		newPathNodeIndex++;
		_newPathNodeList[newPathNodeIndex] = *node;

		for (j = 5; j > 0; j--) {
			start = node->link - j;
			end = node->link + j;
		
			if (start < 0 || end > _pathListIndex) {
				continue;
			}

			point1.x = _pathList[start].x;
			if (point1.x == PATH_NODE_EMPTY) {
				continue;
			}
			point2.x = _pathList[end].x;
			if (point2.x == PATH_NODE_EMPTY) {
				continue;
			}

			point1.y = _pathList[start].y;
			point2.y = _pathList[end].y;
			
			point3.x = point1.x;
			point3.y = point1.y;
			point4.x = point2.x;
			point4.y = point2.y;
			if (scanPathLine( point3, point4)) {
				for (l = 1; l <= newPathNodeIndex; l++) {
					if (start <= _newPathNodeList[l].link) {
						newPathNodeIndex = l;
						_newPathNodeList[newPathNodeIndex].x = point1.x;
						_newPathNodeList[newPathNodeIndex].y = point1.y;
						_newPathNodeList[newPathNodeIndex].link = start;
						newPathNodeIndex++;
						break;
					}
				}
				_newPathNodeList[newPathNodeIndex].x = point2.x;
				_newPathNodeList[newPathNodeIndex].y = point2.y;
				_newPathNodeList[newPathNodeIndex].link = end;

				for (k = start + 1; k < end; k++) {
					_pathList[k].x = PATH_NODE_EMPTY;
				}
				break;
			}
		}
	}

	newPathNodeIndex++;
	_newPathNodeList[newPathNodeIndex] = _pathNodeList[_pathNodeIndex];

	for (i = 0, j = 0; i <= newPathNodeIndex; i++) {
		if (newPathNodeIndex == i || (_newPathNodeList[i].y != _newPathNodeList[i+1].y
			|| _newPathNodeList[i].x != _newPathNodeList[i+1].x) )
			_pathNodeList[j++] = _newPathNodeList[i];
	}
	_pathNodeIndex = j - 1;
}

/*
// Console wrappers - must be safe to run
// TODO - checkup ALL arguments, cause wrong arguments may fall function with "error"

void Actor::CF_actor_move(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Point movePoint;

	movePoint.x = atoi(argv[2]);
	movePoint.y = atoi(argv[3]);

	if (!IS_VALID_ACTOR_ID(actorId)) {
		_vm->_console->DebugPrintf("Actor::CF_actor_move Invalid actorId 0x%X.\n", actorId);
		return;
	}

	move(actorId, movePoint);
}

void Actor::CF_actor_moverel(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Point movePoint;

	movePoint.x = atoi(argv[2]);
	movePoint.y = atoi(argv[3]);

	if (!IS_VALID_ACTOR_ID(actorId)) {
		_vm->_console->DebugPrintf("Actor::CF_actor_moverel Invalid actorId 0x%X.\n", actorId);
		return;
	}

	moveRelative(actorId, movePoint);
}

void Actor::CF_actor_seto(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	int orient;

	orient = atoi(argv[2]);
//TODO orient check
	if (!IS_VALID_ACTOR_ID(actorId)) {
		_vm->_console->DebugPrintf("Actor::CF_actor_seto Invalid actorId 0x%X.\n",actorId);
		return;
	}

}

void Actor::CF_actor_setact(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	int action_n = 0;

	action_n = atoi(argv[2]);

	if (!IS_VALID_ACTOR_ID(actorId)) {
		_vm->_console->DebugPrintf("Actor::CF_actor_setact Invalid actorId 0x%X.\n",actorId);
		return;
	}

//TODO action_n check
	if ((action_n < 0) || (action_n >= actor->action_ct)) {
		_vm->_console->DebugPrintf("Invalid action number.\n");
		return;
	}

	_vm->_console->DebugPrintf("Action frame counts: %d %d %d %d.\n",
			actor->act_tbl[action_n].dir[0].frame_count,
			actor->act_tbl[action_n].dir[1].frame_count,
			actor->act_tbl[action_n].dir[2].frame_count,
			actor->act_tbl[action_n].dir[3].frame_count);

	setAction(actorId, action_n, ACTION_LOOP);
}
*/
} // End of namespace Saga
