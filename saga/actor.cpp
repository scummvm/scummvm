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

#include "saga/game_mod.h"
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

namespace Saga {

static int actorCompare(const ActorDataPointer& actor1, const ActorDataPointer& actor2) {
	if (actor1->a_pt.y == actor2->a_pt.y) {
		return 0;
	} else if (actor1->a_pt.y < actor2->a_pt.y) {
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

	// Get actor resource file context
	_actorContext = GAME_GetFileContext(GAME_RESOURCEFILE, 0);
	if (_actorContext == NULL) {
		error("Actor::Actor(): Couldn't load actor module resource context.");
	}

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		actor->actorId = ACTOR_INDEX_TO_ID(i);
		actor->index = i;
		debug(0, "init actorId=0x%X index=0x%X", actor->actorId, actor->index);
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

	debug(0, "Actor::~Actor()");
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

	debug(0, "Loading frame resource id 0x%X", actor->frameListResourceId);
	result = RSC_LoadResource(_actorContext, actor->frameListResourceId, &resourcePointer, &resourceLength);
	if (result != SUCCESS) {
		warning("Couldn't load sprite action index resource");
		return false;
	}

	framesCount = resourceLength / 16;
	debug(0, "Frame resource contains %d frames", framesCount);
	
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


	debug(0, "Loading sprite resource id 0x%X", actor->spriteListResourceId);
	if (_vm->_sprite->loadList(actor->spriteListResourceId, &actor->spriteList) != SUCCESS) {
		warning("Unable to load sprite list");
		return false;
	}

	i = _vm->_sprite->getListLen(actor->spriteList);

	if ( (lastFrame >= i)) {
		debug(0, "Appending to sprite list 0x%X", actor->spriteListResourceId);
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
			case INTENT_SPEAK:
				// Actor wants to blab
				{
					handleSpeakIntent(actor, a_intent, &a_intent->a_idone, msec);
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

	return SUCCESS;
}

void Actor::createDrawOrderList() {
	int i;
	ActorData *actor;

	_drawOrderList.clear();
	for (i = 0;i < ACTORCOUNT;i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;

		if (actor->sceneNumber == _vm->_scene->currentSceneNumber())
			_drawOrderList.pushBack(actor, actorCompare);
	}
}

int Actor::drawActors() {
	ActorOrderList::iterator actorDrawOrderIterator;
	ActorData *actor;

	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent;

	ActorDialogList::iterator actorDialogIterator;
	ACTORDIALOGUE *a_dialogue;

	int o_idx; //Orientation index
	int sprite_num;

	int diag_x, diag_y; // dialog coordinates

	SURFACE *back_buf;

	back_buf = _vm->_gfx->getBackBuffer();

	createDrawOrderList();

	for (actorDrawOrderIterator = _drawOrderList.begin(); actorDrawOrderIterator != _drawOrderList.end(); ++actorDrawOrderIterator) {
		actor =  actorDrawOrderIterator.operator*();
		if (actor->framesCount == 0) continue;

		o_idx = ActorOrientationLUT[actor->orient];
		sprite_num = actor->frames[actor->action].dir[o_idx].frameIndex;
		sprite_num += actor->action_frame;
		if (actor->spriteList->sprite_count <= sprite_num) continue;
		_vm->_sprite->drawOccluded(back_buf, actor->spriteList, sprite_num, actor->s_pt.x, actor->s_pt.y);

		// If actor's current intent is to speak, oblige him by 
		// displaying his dialogue 
		actorIntentIterator = actor->a_intentlist.begin();
		if (actorIntentIterator != actor->a_intentlist.end()) {
			a_intent = actorIntentIterator.operator->();
			if (a_intent->a_itype == INTENT_SPEAK) {
				actorDialogIterator = a_intent->si_diaglist.begin();
				if (actorDialogIterator != a_intent->si_diaglist.end()) {
					a_dialogue = actorDialogIterator.operator->();
					diag_x = actor->s_pt.x;
					diag_y = actor->s_pt.y;
					diag_y -= ACTOR_DIALOGUE_HEIGHT;
					_vm->textDraw(MEDIUM_FONT_ID, back_buf, a_dialogue->d_string, diag_x, diag_y, actor->speechColor, 0,
								FONT_OUTLINE | FONT_CENTERED);
				}
			}
		}
	}

	return SUCCESS;
}

// Called if the user wishes to skip a line of dialogue (spacebar in the 
// original game). Will find all actors currently talking and remove one
// dialogue entry if there is a current speak intent present.

int Actor::skipDialogue() {
	int i;
	ActorData *actor;

	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent;

	ActorDialogList::iterator actorDialogIterator;
	ACTORDIALOGUE *a_dialogue;

	for (i = 0; i < ACTORCOUNT; i++) {
		actor = &_actors[i];
		if (actor->disabled) continue;
		// Check the actor's current intent for a speak intent
		actorIntentIterator = actor->a_intentlist.begin();
		if (actorIntentIterator != actor->a_intentlist.end()) {
			a_intent = actorIntentIterator.operator->();
			if (a_intent->a_itype == INTENT_SPEAK) {
				// Okay, found a speak intent. Remove one dialogue entry 
				// from it, releasing any semaphore */
				actorDialogIterator = a_intent->si_diaglist.begin();
				if (actorDialogIterator != a_intent->si_diaglist.end()) {
					a_dialogue = actorDialogIterator.operator->();
					if (a_dialogue->d_sem != NULL) {
						_vm->_script->SThreadReleaseSem(a_dialogue->d_sem);
					}
					a_intent->si_diaglist.erase(actorDialogIterator);
					// And stop any currently playing voices
					_vm->_sound->stopVoice();
				}
			}
		}
	}

	return SUCCESS;
}

void Actor::speak(uint16 actorId, const char *d_string, uint16 d_voice_rn, SEMAPHORE *sem) {
	ActorData *actor;
	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent_p = NULL;
	ACTORINTENT a_intent;
	int use_existing_ai = 0;
	ACTORDIALOGUE a_dialogue;

	a_dialogue.d_string = d_string;
	a_dialogue.d_voice_rn = d_voice_rn;
	a_dialogue.d_time = getSpeechTime(d_string, d_voice_rn);
	a_dialogue.d_sem_held = 1;
	a_dialogue.d_sem = sem;

	actor = getActor(actorId);

	// If actor's last registered intent is to speak, we can queue the
	// requested dialogue on that intent context; so examine the last
	// intent

	actorIntentIterator = actor->a_intentlist.end();
	--actorIntentIterator;
	if (actorIntentIterator != actor->a_intentlist.end()) {
		a_intent_p = actorIntentIterator.operator->();
		if (a_intent_p->a_itype == INTENT_SPEAK) {
			use_existing_ai = 1;
		}
	}

	if (use_existing_ai) {
		// Store the current dialogue off the existing actor intent
		a_intent_p->si_diaglist.push_back(a_dialogue);
	} else {
		// Create a new actor intent
		a_intent.a_itype = INTENT_SPEAK;
		a_intent.a_idone = 0;
		a_intent.a_iflags = 0;

		a_intent.si_last_action = actor->action;
		a_intent.si_diaglist.push_back(a_dialogue);

		actor->a_intentlist.push_back(a_intent);
	}

	if (sem != NULL) {
		_vm->_script->SThreadHoldSem(sem);
	}
}

int Actor::handleSpeakIntent(ActorData *actor, ACTORINTENT *a_aintent, int *complete_p, int msec) {
	ActorDialogList::iterator actorDialogIterator;
	ActorDialogList::iterator nextActorDialogIterator;
	ACTORDIALOGUE *a_dialogue;
	ACTORDIALOGUE *a_dialogue2;
	long carry_time;
	int intent_complete = 0;

	if (!a_aintent->si_init) {
		// Initialize speak intent by setting up action
		actor->action = ACTION_SPEAK;
		actor->action_frame = 0;
		actor->action_time = 0;
		actor->action_flags = ACTION_LOOP;
		a_aintent->si_init = 1;
	}

	// Process actor dialogue list
	actorDialogIterator = a_aintent->si_diaglist.begin();
	if (actorDialogIterator != a_aintent->si_diaglist.end()) {
		a_dialogue = actorDialogIterator.operator->();
		if (!a_dialogue->d_playing) {
			// Dialogue voice hasn't played yet - play it now
			_vm->_sndRes->playVoice(a_dialogue->d_voice_rn);
			a_dialogue->d_playing = 1;
		}

		a_dialogue->d_time -= msec;
		if (a_dialogue->d_time <= 0) {
			// Dialogue time has expired; carry negative time to next
			// dialogue entry if present, release any semaphores and
			// delete the expired entry

			//actor->action = ACTION_IDLE;

			if (a_dialogue->d_sem != NULL) {
				_vm->_script->SThreadReleaseSem(a_dialogue->d_sem);
			}

			carry_time = a_dialogue->d_time;

			nextActorDialogIterator = actorDialogIterator;
			++nextActorDialogIterator;
			if (nextActorDialogIterator != a_aintent->si_diaglist.end()) {
				a_dialogue2 = nextActorDialogIterator.operator->();
				a_dialogue2->d_time -= carry_time;
			}

			// Check if there are any dialogue nodes left. If not, 
			// flag this speech intent as complete

			actorDialogIterator = a_aintent->si_diaglist.erase(actorDialogIterator);
			if (actorDialogIterator == a_aintent->si_diaglist.end()) {
				intent_complete = 1;
			}
		}
	} else {
		intent_complete = 1;
	}

	if (intent_complete) {
		*complete_p = 1;
	}

	return SUCCESS;
}

int Actor::getSpeechTime(const char *d_string, uint16 d_voice_rn) {
	int voice_len;

	voice_len = _vm->_sndRes->getVoiceLength(d_voice_rn);

	if (voice_len < 0) {
		voice_len = strlen(d_string) * ACTOR_DIALOGUE_LETTERTIME;
	}

	return voice_len;
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
	debug(0, "actor->a_intentlist.size() %i", is);

	if (sem != NULL) {
		_vm->_script->SThreadHoldSem(sem);
	}
}

int Actor::setPathNode(WALKINTENT *walk_int, Point *src_pt, Point *dst_pt, SEMAPHORE *sem) {
	WALKNODE new_node;

	walk_int->wi_active = 1;
	walk_int->org = *src_pt;

	assert((walk_int != NULL) && (src_pt != NULL) && (dst_pt != NULL));

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
		setPathNode(a_walkint, &actor->a_pt, &a_walkint->dst_pt, a_walkint->sem);
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

	actor->a_pt.x = (int)new_a_x;
	actor->a_pt.y = (int)new_a_y;

	actor->s_pt.x = actor->a_pt.x >> 2;
	actor->s_pt.y = actor->a_pt.y >> 2;

	return SUCCESS;
}

void Actor::move(uint16 actorId, const Point &movePoint) {
	ActorData *actor;

	int moveUp = 0;

	actor = getActor(actorId);

	if (movePoint.y < actor->a_pt.y) {
		moveUp = 1;
	}

	actor->a_pt = movePoint;

	AtoS(actor->s_pt, actor->a_pt);
}

void Actor::moveRelative(uint16 actorId, const Point &movePoint) {
	ActorData *actor;

	actor = getActor(actorId);

	actor->a_pt.x += movePoint.x; //TODO user rect.h
	actor->a_pt.y += movePoint.y;

	AtoS(actor->s_pt, actor->a_pt);
}

void Actor::AtoS(Point &screenPoint, const Point &actorPoint) {
	screenPoint.x = (actorPoint.x / ACTOR_LMULT);
	screenPoint.y = (actorPoint.y / ACTOR_LMULT);
}

void Actor::StoA(Point &actorPoint, const Point &screenPoint) {
	actorPoint.x = (screenPoint.x * ACTOR_LMULT);
	actorPoint.y = (screenPoint.y * ACTOR_LMULT);
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
