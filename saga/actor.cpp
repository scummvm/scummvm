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
	if (actor1->actorY == actor2->actorY) {
		return 0;
	} else if (actor1->actorY < actor2->actorY) {
		return -1;
	} else {
		return 1;
	}
}

ACTIONTIMES ActionTDeltas[] = {
	{ ACTION_IDLE, 80 },
	{ ACTION_WALK, 80 },
	{ ACTION_SPEAK, 200 }
};

Actor::Actor(SagaEngine *vm) : _vm(vm) {
	int i;
	ActorData *actor;
	debug(9, "Actor::Actor()");

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
		actor->orient = ACTOR_DEFAULT_ORIENT;
		actor->def_action = 0;
		actor->def_action_flags = 0;
		actor->action = 0;
		actor->action_flags = 0;
		actor->action_time = 0;
		actor->action_frame = 0;

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
	ActorFrame *framesPointer;
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
	
	framesPointer = (ActorFrame *)malloc(sizeof(ActorFrame) * framesCount);
	if (framesPointer == NULL) {
		error("Couldn't allocate memory for sprite frames");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);

	lastFrame = 0;

	for (i = 0; i < framesCount; i++) {
		for (orient = 0; orient < ACTOR_ORIENTATION_COUNT; orient++) {
			// Load all four orientations
			framesPointer[i].dir[orient].frameIndex = readS.readUint16();
			framesPointer[i].dir[orient].frameCount = readS.readUint16();
			if (framesPointer[i].dir[orient].frameIndex > lastFrame) {
				lastFrame = framesPointer[i].dir[orient].frameIndex;
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

	actor = &_actors[ACTOR_ID_TO_INDEX(actorId)];

	if (actor->disabled)
		error("Actor::getActor disabled actorId 0x%X", actorId);

	return actor;
}

void Actor::updateActorsScene() {
	int i;
	ActorData *actor;

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->flags & (kProtagonist | kFollower)) {
			actor->sceneNumber = _vm->_scene->currentSceneNumber();
		}
	}
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
				//a->actionCycle = rand() % 64; todo
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

int Actor::direct(int msec) {
	int i;
	ActorData *actor;

	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent;

	int o_idx;
	int action_tdelta;

	// Walk down the actor list and direct each actor
	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;

		// Process the actor intent list
		actorIntentIterator = actor->a_intentlist.begin();
		if (actorIntentIterator != actor->a_intentlist.end()) {
			a_intent = actorIntentIterator.operator->();
			switch (a_intent->a_itype) {
			case INTENT_NONE:
				// Actor doesn't really feel like doing anything at all
				break;
			case INTENT_PATH:
				// Actor intends to go somewhere. Well good for him
				{
					handleWalkIntent(actor, &a_intent->walkIntent, &a_intent->a_idone, msec);
				}
				break;
			default:
				break;
			}

			// If this actor intent was flagged as completed, remove it.
			if (a_intent->a_idone) {
				actor->a_intentlist.erase(actorIntentIterator);
				actor->action = actor->def_action;
				actor->action_flags = actor->def_action_flags;
				actor->action_frame = 0;
				actor->action_time = 0;
			}
		} else {
			// Actor has no intent, idle?
		}

		// Process actor actions
		actor->action_time += msec;

		if (actor->action >= ACTION_COUNT) {
			action_tdelta = ACTOR_ACTIONTIME;
		} else {
			action_tdelta = ActionTDeltas[actor->action].time;
		}

		if (actor->action_time >= action_tdelta) {
			actor->action_time -= action_tdelta;
			actor->action_frame++;

			o_idx = ActorOrientationLUT[actor->orient];
			if (actor->frames[actor->action].dir[o_idx].frameCount <= actor->action_frame) {
				if (actor->action_flags & ACTION_LOOP) {
					actor->action_frame = 0;
				} else {
					actor->action_frame--;
				}
			}
		}
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

		middle = ITE_STATUS_Y - actor->actorY / ACTOR_LMULT,

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

		actor->screenPosition.x = (actor->actorX / ACTOR_LMULT);
		actor->screenPosition.y = (actor->actorY / ACTOR_LMULT) - actor->actorZ;
	}
}

int Actor::drawActors() {
	ActorOrderList::iterator actorDrawOrderIterator;
	ActorData *actor;


	int o_idx; //Orientation index
	int sprite_num;


	SURFACE *back_buf;

	back_buf = _vm->_gfx->getBackBuffer();

	createDrawOrderList();

	for (actorDrawOrderIterator = _drawOrderList.begin(); actorDrawOrderIterator != _drawOrderList.end(); ++actorDrawOrderIterator) {
		actor =  actorDrawOrderIterator.operator*();
		if (actor->framesCount == 0) {
			warning("actor->framesCount == 0 actorId 0x%X", actor->actorId);
			continue;
		}

		o_idx = ActorOrientationLUT[actor->orient];
		sprite_num = actor->frames[actor->action].dir[o_idx].frameIndex;
		sprite_num += actor->action_frame;
		if (actor->spriteList->sprite_count <= sprite_num) {
			warning("(actor->spriteList->sprite_count <= sprite_num) actorId 0x%X", actor->actorId);
			continue;
		}
		_vm->_sprite->drawOccluded(back_buf, actor->spriteList, sprite_num, actor->screenPosition, actor->screenScale, actor->screenDepth);
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

void Actor::setOrientation(uint16 actorId, int orient) {
	ActorData *actor;
	
	actor = getActor(actorId);

	if ((orient < 0) || (orient > 7)) {
		error("Actor::setOrientation wrong orientation 0x%X", orient);
	}

	actor->orient = orient;
}

void Actor::setAction(uint16 actorId, int action_n, uint16 action_flags) {
	ActorData *actor;

	actor = getActor(actorId);

	if ((action_n < 0) || (action_n >= actor->framesCount)) {
		error("Actor::setAction wrong action_n 0x%X", action_n);
	}

	actor->action = action_n;
	actor->action_flags = action_flags;
	actor->action_frame = 0;
	actor->action_time = 0;

}

void Actor::setDefaultAction(uint16 actorId, int action_n, uint16 action_flags) {
	ActorData *actor;

	actor = getActor(actorId);

	if ((action_n < 0) || (action_n >= actor->framesCount)) {
		error("Actor::setDefaultAction wrong action_n 0x%X", action_n);
	}

	actor->def_action = action_n;
	actor->def_action_flags = action_flags;
}

void Actor::walkTo(uint16 actorId, const Point *walk_pt, uint16 flags, SEMAPHORE *sem) {
	ACTORINTENT actor_intent;
	ActorData *actor;

	assert(walk_pt != NULL);
	
	actor = getActor(actorId);
	
	actor_intent.a_itype = INTENT_PATH;
	actor_intent.a_iflags = 0;
		

	actor_intent.walkIntent.wi_flags = flags;
	actor_intent.walkIntent.sem_held = 1;
	actor_intent.walkIntent.sem = sem;

	// handleWalkIntent() will create path on initialization
	actor_intent.walkIntent.wi_init = 0;
	actor_intent.walkIntent.dst_pt = *walk_pt;

	actor->a_intentlist.push_back(actor_intent);
	int is = actor->a_intentlist.size();
	debug(9, "actor->a_intentlist.size() %i", is);

	if (sem != NULL) {
		_vm->_script->SThreadHoldSem(sem);
	}
}

int Actor::setPathNode(WALKINTENT *walk_int, const Point &src_pt, Point *dst_pt, SEMAPHORE *sem) {
	WALKNODE new_node;

	walk_int->wi_active = 1;
	walk_int->org = src_pt;

	assert((walk_int != NULL) && (dst_pt != NULL));

	new_node.node_pt = *dst_pt;
	new_node.calc_flag = 0;
	
	walk_int->nodelist.push_back(new_node);
	
	return SUCCESS;
}

int Actor::handleWalkIntent(ActorData *actor, WALKINTENT *a_walkint, int *complete_p, int delta_time) {
	WalkNodeList::iterator walkNodeIterator;
	WalkNodeList::iterator nextWalkNodeIterator;

	WALKNODE *node_p;
	int dx;
	int dy;

	double path_a;
	double path_b;
	double path_slope;

	double path_x;
	double path_y;
	int path_time;

	double new_a_x;
	double new_a_y;

	int actor_x;
	int actor_y;

	char buf[100];

	// Initialize walk intent 
	if (!a_walkint->wi_init) {
		setPathNode(a_walkint, Point(actor->actorX,actor->actorY), &a_walkint->dst_pt, a_walkint->sem);
		setDefaultAction(actor->actorId, ACTION_IDLE, ACTION_NONE);
		a_walkint->wi_init = 1;
	}

	assert(a_walkint->wi_active);

	walkNodeIterator = a_walkint->nodelist.begin();
	nextWalkNodeIterator = walkNodeIterator;

	node_p = walkNodeIterator.operator->();

	if (node_p->calc_flag == 0) {

		debug(2, "Calculating new path vector to point (%d, %d)", node_p->node_pt.x, node_p->node_pt.y);

		dx = a_walkint->org.x - node_p->node_pt.x;
		dy = a_walkint->org.y - node_p->node_pt.y;

		if (dx == 0) {

			debug(0, "Vertical paths not implemented.");

			a_walkint->nodelist.erase(walkNodeIterator);
			a_walkint->wi_active = 0;

			// Release path semaphore
			if ((a_walkint->sem != NULL) && a_walkint->sem_held) {
				_vm->_script->SThreadReleaseSem(a_walkint->sem);
			}

			*complete_p = 1;
			return FAILURE;
		}

		a_walkint->slope = (float)dy / dx;

		if (dx > 0) {
			a_walkint->x_dir = -1;
			if (!(a_walkint->wi_flags & WALK_NOREORIENT)) {
				if (a_walkint->slope > 1.0) {
					actor->orient = ORIENT_N;
				} else if (a_walkint->slope < -1.0) {
					actor->orient = ORIENT_S;
				} else {
					actor->orient = ORIENT_W;
				}
			}
		} else {
			a_walkint->x_dir = 1;
			if (!(a_walkint->wi_flags & WALK_NOREORIENT)) {
				if (a_walkint->slope > 1.0) {
					actor->orient = ORIENT_S;
				} else if (a_walkint->slope < -1.0) {
					actor->orient = ORIENT_N;
				} else {
					actor->orient = ORIENT_E;
				}
			}
		}

		sprintf(buf, "%f", a_walkint->slope);

		debug(2, "Path slope: %s.", buf);

		actor->action = ACTION_WALK;
		actor->action_flags = ACTION_LOOP;
		a_walkint->time = 0;
		node_p->calc_flag = 1;
	}

	a_walkint->time += delta_time;
	path_time = a_walkint->time;

	path_a = ACTOR_BASE_SPEED * path_time;
	path_b = ACTOR_BASE_SPEED * path_time * ACTOR_BASE_ZMOD;
	path_slope = a_walkint->slope * a_walkint->x_dir;

	path_x = (path_a * path_b) / sqrt((path_a * path_a) * (path_slope * path_slope) + (path_b * path_b));

	path_y = path_slope * path_x;
	path_x = path_x * a_walkint->x_dir;

	new_a_x = path_x + a_walkint->org.x;
	new_a_y = path_y + a_walkint->org.y;

	if (((a_walkint->x_dir == 1) && new_a_x >= node_p->node_pt.x) ||
		((a_walkint->x_dir != 1) && (new_a_x <= node_p->node_pt.x))) {
		Point endpoint;
		int exitNum;

		debug(2, "Path complete.");
		a_walkint->nodelist.erase(walkNodeIterator);
		a_walkint->wi_active = 0;

		// Release path semaphore
		if (a_walkint->sem != NULL) {
			_vm->_script->SThreadReleaseSem(a_walkint->sem);
		}

		actor->action_frame = 0;
		actor->action = ACTION_IDLE;

		endpoint.x = (int)new_a_x / ACTOR_LMULT;
		endpoint.y = (int)new_a_y / ACTOR_LMULT;
		if ((exitNum = _vm->_scene->_actionMap->hitTest(endpoint)) != -1) {
			if (actor->flags & kProtagonist)
				_vm->_scene->changeScene(_vm->_scene->_actionMap->getExitScene(exitNum));
		}
		*complete_p = 1;
		return FAILURE;
	}

	actor_x = (int)new_a_x;
	actor_y = (int)new_a_y;

	actor->actorX = (int)new_a_x;
	actor->actorY = (int)new_a_y;

	return SUCCESS;
}

void Actor::move(uint16 actorId, const Point &movePoint) {
	ActorData *actor;

	actor = getActor(actorId);

	actor->actorX = movePoint.x;
	actor->actorY = movePoint.y;
}

void Actor::moveRelative(uint16 actorId, const Point &movePoint) {
	ActorData *actor;

	actor = getActor(actorId);

	actor->actorX += movePoint.x; 
	actor->actorY += movePoint.y;
}

void Actor::StoA(Point &actorPoint, const Point &screenPoint) {
	actorPoint.x = (screenPoint.x * ACTOR_LMULT);
	actorPoint.y = (screenPoint.y * ACTOR_LMULT);
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

	setOrientation(actorId, orient);
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
/*	if ((action_n < 0) || (action_n >= actor->action_ct)) {
		_vm->_console->DebugPrintf("Invalid action number.\n");
		return;
	}

	_vm->_console->DebugPrintf("Action frame counts: %d %d %d %d.\n",
			actor->act_tbl[action_n].dir[0].frame_count,
			actor->act_tbl[action_n].dir[1].frame_count,
			actor->act_tbl[action_n].dir[2].frame_count,
			actor->act_tbl[action_n].dir[3].frame_count);
*/
	setAction(actorId, action_n, ACTION_LOOP);
}

} // End of namespace Saga
