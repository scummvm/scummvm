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
int ActorDirectectionsLUT[8] = {
	ACTOR_DIRECTION_BACK,	// kDirUp
	ACTOR_DIRECTION_RIGHT,	// kkDirUpRight
	ACTOR_DIRECTION_RIGHT,	// kDirRight
	ACTOR_DIRECTION_RIGHT,	// kDirDownRight
	ACTOR_DIRECTION_FORWARD,// kDirDown
	ACTOR_DIRECTION_LEFT,	// kDirDownLeft
	ACTOR_DIRECTION_LEFT,	// kDirLeft
	ACTOR_DIRECTION_LEFT,	// kDirUpLeft
};

Actor::Actor(SagaEngine *vm) : _vm(vm) {
	int i;
	ActorData *actor;
	debug(9, "Actor::Actor()");

	_centerActor = _protagonist = NULL;
	_lastTickMsec = 0;
	// Get actor resource file context
	_actorContext = _vm->getFileContext(GAME_RESOURCEFILE, 0);
	if (_actorContext == NULL) {
		error("Actor::Actor(): Couldn't load actor module resource context.");
	}

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		actor->actorId = ACTOR_INDEX_TO_ID(i);
		actor->index = i;
		debug(9, "init actorId=0x%X index=0x%X", actor->actorId, actor->index);
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

		actor->location.x = ActorTable[i].x;
		actor->location.y = ActorTable[i].y;
		actor->location.z = ActorTable[i].z;

		actor->disabled = !loadActorResources(actor);
		if (actor->disabled) {
			warning("Disabling actorId=0x%X index=0x%X", actor->actorId, actor->index);
		} 
	}
}

Actor::~Actor() {
	int i;
	ActorData *actor;

	debug(9, "Actor::~Actor()");
	//release resources
	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		free(actor->frames);
		_vm->_sprite->freeSprite(actor->spriteList);
	}
}

bool Actor::loadActorResources(ActorData * actor) {
	byte *resourcePointer;
	size_t resourceLength;
	int framesCount;
	ActorFrameSequence *framesPointer;
	int lastFrame;
	int i, orient;
	int result;

	debug(9, "Loading frame resource id 0x%X", actor->frameListResourceId);
	result = RSC_LoadResource(_actorContext, actor->frameListResourceId, &resourcePointer, &resourceLength);
	if (result != SUCCESS) {
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


	debug(9, "Loading sprite resource id 0x%X", actor->spriteListResourceId);
	if (_vm->_sprite->loadList(actor->spriteListResourceId, &actor->spriteList) != SUCCESS) {
		warning("Unable to load sprite list");
		return false;
	}

	i = _vm->_sprite->getListLen(actor->spriteList);

	if ( (lastFrame >= i)) {
		debug(9, "Appending to sprite list 0x%X", actor->spriteListResourceId);
		if (_vm->_sprite->appendList(actor->spriteListResourceId + 1, actor->spriteList) != SUCCESS) {
			warning("Unable append sprite list");
			return false;
		}
	}

	return true;
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

void Actor::updateActorsScene() {
	int i;
	ActorData *actor;
	
	_activeSpeech.stringsCount = 0;

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];		
		if (actor->flags & (kProtagonist | kFollower)) {
			actor->sceneNumber = _vm->_scene->currentSceneNumber();
			if (actor->flags & kProtagonist) {
//				actor->finalTarget = a->obj.loc;
				_centerActor = _protagonist = actor;
			}

		}
		if (actor->sceneNumber == _vm->_scene->currentSceneNumber())
			actor->actionCycle = (rand() & 7) * 4;
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

	fourDirection = ActorDirectectionsLUT[actor->facingDirection];
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
	
	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;
		if (actor->sceneNumber != _vm->_scene->currentSceneNumber()) continue;
		
		//todo: dragon stuff

		switch(actor->currentAction) {
			case kActionWait: {
				if (!setup && (actor->flags & kFollower)) {
					//todo: followProtagonist
					if (actor->currentAction != kActionWait)
						break;
				}

				if (actor->targetObject != ID_NOTHING) {
					//todo: facetowardsobject
				}

				if (actor->flags & kCycle) {
					frameRange = getActorFrameRange( actor->actorId, kFrameStand);
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

					frameRange = getActorFrameRange( actor->actorId, kFrameWait);
					if ((frameRange->frameCount < 1 || actor->actionCycle > 33))
						frameRange = getActorFrameRange( actor->actorId, kFrameStand);

					if (frameRange->frameCount) {
						actor->frameNumber = frameRange->frameIndex + (uint16)rand() % frameRange->frameCount;
					} else {
						actor->frameNumber = frameRange->frameIndex;
					}
				}
				actor->actionCycle++;
			} break;
			case kActionWalkToPoint:
			case kActionWalkToLink: {
					//todo: do it
				debug(9,"kActionWalk* not implemented");
			} break;
			case kActionWalkDir: {
				debug(9,"kActionWalkDir not implemented");
				//todo: do it
			} break;
			case kActionSpeak: {
				actor->actionCycle++;
				actor->cycleWrap(64);

				frameRange = getActorFrameRange( actor->actorId, kFrameGesture);
				if (actor->actionCycle >= frameRange->frameCount) {
					if (actor->actionCycle & 1) break;
					frameRange = getActorFrameRange( actor->actorId, kFrameSpeak);

					state = (uint16)rand() % (frameRange->frameCount + 1);

					if (state == 0) {
						frameRange = getActorFrameRange( actor->actorId, kFrameStand);
					} else {
						state--;
					}
				} else {
					state = actor->actionCycle;
				}

				actor->frameNumber = frameRange->frameIndex + state;
			} break;

			case kActionAccept:
			case kActionStoop:
				break;

			case kActionCycleFrames:
			case kActionPongFrames: {
				if (actor->cycleTimeCount > 0) {
					actor->cycleTimeCount--;
					break;
				}

				actor->cycleTimeCount = actor->cycleDelay;
				actor->actionCycle++;

				frameRange = getActorFrameRange( actor->actorId, actor->cycleFrameNumber);
				
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
			} break;
			case kActionFall: {
				debug(9,"kActionFall not implemented");

				//todo: do it
			} break;
			case kActionClimb: {
				debug(9,"kActionClimb not implemented");

				//todo: do it
			} break;
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

void Actor::createDrawOrderList() {
	int i;
	int	beginSlope, endSlope, middle;
	ActorData *actor;

	_drawOrderList.clear();
	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;
		if (actor->sceneNumber != _vm->_scene->currentSceneNumber()) continue;

		_drawOrderList.pushBack(actor, actorCompare);

		// tiled stuff
		{
		}
		{
			middle = ITE_STATUS_Y - actor->location.y / ACTOR_LMULT,

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

			actor->screenPosition.x = (actor->location.x / ACTOR_LMULT);
			actor->screenPosition.y = (actor->location.y / ACTOR_LMULT) - actor->location.z;
		}
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
		
		// tiled stuff
		{
		}
		_vm->_sprite->drawOccluded(back_buf, spriteList, frameNumber, actor->screenPosition, actor->screenScale, actor->screenDepth);
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

void Actor::StoA(Point &actorPoint, const Point &screenPoint) {
	actorPoint.x = (screenPoint.x * ACTOR_LMULT);
	actorPoint.y = (screenPoint.y * ACTOR_LMULT);
}

bool Actor::actorWalkTo(uint16 actorId, const ActorLocation &actorLocation) {
	BOOL				result = TRUE;
	ActorData *actor;

	actor = getActor(actorId);

/*	if (a == protag)
	{
		sceneDoors[ 2 ] = 0xff;				// closed
		sceneDoors[ 3 ] = 0;				// open
	}
	else
	{
		sceneDoors[ 2 ] = 0;				// open
		sceneDoors[ 3 ] = 0xff;				// closed
	}*/

	// tiled stuff
	{
	}
	{
	}
	return false;

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
