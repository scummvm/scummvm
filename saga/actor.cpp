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

static int actorCompare(const ACTOR& actor1, const ACTOR& actor2) {
	if (actor1.a_pt.y == actor2.a_pt.y) {
		return 0;
	} else if (actor1.a_pt.y < actor2.a_pt.y) {
		return -1;
	} else {
		return 1;
	}
}
static ActorList::iterator zeroActorIterator;

ACTIONTIMES ActionTDeltas[] = {
	{ ACTION_IDLE, 80 },
	{ ACTION_WALK, 80 },
	{ ACTION_SPEAK, 200 }
};

Actor::Actor(SagaEngine *vm) : _vm(vm) {
	int i;

	// Get actor resource file context
	_actorContext = GAME_GetFileContext(GAME_RESOURCEFILE, 0);
	if (_actorContext == NULL) {
		error("Actor::Actor(): Couldn't load actor module resource context.");
	}


	// Initialize alias table so each index contains itself
	for (i = 0; i < ACTORCOUNT; i++) {
		_aliasTbl[i] = i;
	}

	_count = 0;
}

Actor::~Actor() {
	debug(0, "Actor::~Actor()");
}

bool Actor::isValidActor(int index) {
	
	if (!IS_VALID_ACTOR_INDEX(index))
		return false;

	return (_tbl[index] != zeroActorIterator);
}

ActorList::iterator Actor::getActorIterator(int index) {

	if(!isValidActor(index))
		error("Actor::getActorIterator wrong actor 0x%x", index);

	return _tbl[index];
}

void Actor::reorderActorUp(int index) {
	ActorList::iterator actorIterator;

	actorIterator = getActorIterator(index);
	actorIterator = _list.reorderUp(actorIterator, actorCompare);
	_tbl[index] = actorIterator;
}

void Actor::reorderActorDown(int index) {
	ActorList::iterator actorIterator;

	actorIterator = getActorIterator(index);
	actorIterator = _list.reorderDown(actorIterator, actorCompare);
	_tbl[index] = actorIterator;
}

int Actor::direct(int msec) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent;

	int o_idx;
	int action_tdelta;

	// Walk down the actor list and direct each actor
	for (actorIterator = _list.begin(); actorIterator != _list.end(); ++actorIterator) {
		actor =  actorIterator.operator->();

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
					uint16 actorId = actor->actorId; //backup
					handleWalkIntent(actor, &a_intent->walkIntent, &a_intent->a_idone, msec);

					actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
					actor = actorIterator.operator->();
					actorIntentIterator = actor->a_intentlist.begin();
					a_intent = actorIntentIterator.operator->();
				}
				break;
			case INTENT_SPEAK:
				// Actor wants to blab
				{
					handleSpeakIntent(actor, &a_intent->speakIntent, &a_intent->a_idone, msec);
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
			if (actor->act_tbl[actor->action].dir[o_idx].frame_count <= actor->action_frame) {
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

int Actor::drawList() {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent;

	ActorDialogList::iterator actorDialogIterator;
	ACTORDIALOGUE *a_dialogue;

	int o_idx; //Orientation index
	int sprite_num;

	int diag_x, diag_y; // dialog coordinates

	SURFACE *back_buf;

	back_buf = _vm->_gfx->getBackBuffer();

	for (actorIterator = _list.begin(); actorIterator != _list.end(); ++actorIterator) {
		actor =  actorIterator.operator->();
		o_idx = ActorOrientationLUT[actor->orient];
		sprite_num = actor->act_tbl[actor->action].dir[o_idx].frame_index;
		sprite_num += actor->action_frame;
		_vm->_sprite->drawOccluded(back_buf, actor->sl_p, sprite_num, actor->s_pt.x, actor->s_pt.y);

		// If actor's current intent is to speak, oblige him by 
		// displaying his dialogue 
		actorIntentIterator = actor->a_intentlist.begin();
		if (actorIntentIterator != actor->a_intentlist.end()) {
			a_intent = actorIntentIterator.operator->();
			if (a_intent->a_itype == INTENT_SPEAK) {
				actorDialogIterator = a_intent->speakIntent.si_diaglist.begin();
				if (actorDialogIterator != a_intent->speakIntent.si_diaglist.end()) {
					a_dialogue = actorDialogIterator.operator->();
					diag_x = actor->s_pt.x;
					diag_y = actor->s_pt.y;
					diag_y -= ACTOR_DIALOGUE_HEIGHT;
					_vm->textDraw(MEDIUM_FONT_ID, back_buf, a_dialogue->d_string, diag_x, diag_y, actor->a_dcolor, 0,
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
	ActorList::iterator actorIterator;
	ACTOR *actor;

	ActorIntentList::iterator actorIntentIterator;
	ACTORINTENT *a_intent;

	ActorDialogList::iterator actorDialogIterator;
	ACTORDIALOGUE *a_dialogue;

	for (actorIterator = _list.begin(); actorIterator != _list.end(); ++actorIterator) {
		actor =  actorIterator.operator->();
		// Check the actor's current intent for a speak intent
		actorIntentIterator = actor->a_intentlist.begin();
		if (actorIntentIterator != actor->a_intentlist.end()) {
			a_intent = actorIntentIterator.operator->();
			if (a_intent->a_itype == INTENT_SPEAK) {
				// Okay, found a speak intent. Remove one dialogue entry 
				// from it, releasing any semaphore */
				actorDialogIterator = a_intent->speakIntent.si_diaglist.begin();
				if (actorDialogIterator != a_intent->speakIntent.si_diaglist.end()) {
					a_dialogue = actorDialogIterator.operator->();
					if (a_dialogue->d_sem != NULL) {
						_vm->_script->SThreadReleaseSem(a_dialogue->d_sem);
					}
					a_intent->speakIntent.si_diaglist.erase(actorDialogIterator);
					// And stop any currently playing voices
					_vm->_sound->stopVoice();
				}
			}
		}
	}

	return SUCCESS;
}

void Actor::create(uint16 actorId, int x, int y) {
	ACTOR sampleActor;
	sampleActor.actorId = actorId;
	sampleActor.a_pt.x = x;
	sampleActor.a_pt.y = y;

	addActor(&sampleActor);
}

void Actor::addActor(ACTOR * actor) {
	ActorList::iterator actorIterator;
	int last_frame;

	actor->index = ACTOR_ID_TO_INDEX(actor->actorId);

	debug(0, "Actor::addActor actorId=0x%X index=0x%X", actor->actorId, actor->index);

	if (!IS_VALID_ACTOR_INDEX(actor->index)) {
		error("Wrong Actor actorId=0x%X index=0x%X", actor->actorId, actor->index);
	}
	
	if (_tbl[actor->index] != zeroActorIterator) {
		error("Actor::addActor actor already exist actorId=0x%X index=0x%X", actor->actorId, actor->index);
	}

	AtoS(&actor->s_pt, &actor->a_pt);


	actor->sl_rn = ActorTable[actor->index].spritelist_rn;
	actor->si_rn = ActorTable[actor->index].spriteindex_rn;

	loadActorSpriteIndex(actor, actor->si_rn, &last_frame);

	if (_vm->_sprite->loadList(actor->sl_rn, &actor->sl_p) != SUCCESS) {
		error("Actor::addActor unable to load sprite list actorId=0x%X index=0x%X", actor->actorId, actor->index);
	}

	if (last_frame >= _vm->_sprite->getListLen(actor->sl_p)) {
		debug(0, "Appending to sprite list %d.", actor->sl_rn);
		if (_vm->_sprite->appendList(actor->sl_rn + 1, actor->sl_p) != SUCCESS) {
			error("Actor::addActor unable append sprite list actorId=0x%X index=0x%X", actor->actorId, actor->index);
		}
	}

	actor->flags = ActorTable[actor->index].flags;
	actor->a_dcolor = ActorTable[actor->index].color;
	actor->orient = ACTOR_DEFAULT_ORIENT;
	actor->def_action = 0;
	actor->def_action_flags = 0;
	actor->action = 0;
	actor->action_flags = 0;
	actor->action_time = 0;
	actor->action_frame = 0;

	actorIterator = _list.pushBack(*actor, actorCompare);

	actor = actorIterator.operator->();

    _tbl[actor->index] = actorIterator;
	_count++;
}

int Actor::getActorIndex(uint16 actorId) {
	int actorIdx = ACTOR_ID_TO_INDEX(actorId);

	if (!IS_VALID_ACTOR_INDEX(actorIdx)) {
		error("Wrong Actor actorId=0x%X actorIdx=0x%X", actorId, actorIdx);
	}

	if (_tbl[actorIdx] == zeroActorIterator) {
		_vm->_console->DebugPrintf(S_WARN_PREFIX "Actor::getActorIndex Actor id 0x%X not found.\n", actorId);
		warning("Actor::getActorIndex Actor not found actorId=0x%X actorIdx=0x%X", actorId, actorIdx);
		return -1;
	}

	return actorIdx;
}

bool Actor::actorExists(uint16 actorId) {
	int actorIdx = ACTOR_ID_TO_INDEX(actorId);
	
	if (!IS_VALID_ACTOR_INDEX(actorIdx)) {
		error("Wrong Actor actorId=0x%X actorIdx=0x%X", actorId, actorIdx);
	}

	if (_tbl[actorIdx] == zeroActorIterator) {
		return false;
	}

	return true;
}

void Actor::speak(uint16 actorId, const char *d_string, uint16 d_voice_rn, SEMAPHORE *sem) {
	ActorList::iterator actorIterator;
	ACTOR *actor;
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

	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

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
		a_intent_p->speakIntent.si_diaglist.push_back(a_dialogue);
	} else {
		// Create a new actor intent
		a_intent.a_itype = INTENT_SPEAK;
		a_intent.a_idone = 0;
		a_intent.a_iflags = 0;

		a_intent.speakIntent.si_last_action = actor->action;
		a_intent.speakIntent.si_diaglist.push_back(a_dialogue);

		actor->a_intentlist.push_back(a_intent);
	}

	if (sem != NULL) {
		_vm->_script->SThreadHoldSem(sem);
	}
}

int Actor::handleSpeakIntent(ACTOR *actor, SPEAKINTENT *a_speakint, int *complete_p, int msec) {
	ActorDialogList::iterator actorDialogIterator;
	ActorDialogList::iterator nextActorDialogIterator;
	ACTORDIALOGUE *a_dialogue;
	ACTORDIALOGUE *a_dialogue2;
	long carry_time;
	int intent_complete = 0;

	if (!a_speakint->si_init) {
		// Initialize speak intent by setting up action
		actor->action = ACTION_SPEAK;
		actor->action_frame = 0;
		actor->action_time = 0;
		actor->action_flags = ACTION_LOOP;
		a_speakint->si_init = 1;
	}

	// Process actor dialogue list
	actorDialogIterator = a_speakint->si_diaglist.begin();
	if (actorDialogIterator != a_speakint->si_diaglist.end()) {
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
			if (nextActorDialogIterator != a_speakint->si_diaglist.end()) {
				a_dialogue2 = nextActorDialogIterator.operator->();
				a_dialogue2->d_time -= carry_time;
			}

			// Check if there are any dialogue nodes left. If not, 
			// flag this speech intent as complete

			actorDialogIterator = a_speakint->si_diaglist.erase(actorDialogIterator);
			if (actorDialogIterator != a_speakint->si_diaglist.end()) {
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
	ActorList::iterator actorIterator;
	ACTOR *actor;
	
	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

	if ((orient < 0) || (orient > 7)) {
		error("Actor::setOrientation wrong orientation 0x%X", orient);
	}

	actor->orient = orient;
}

void Actor::setAction(uint16 actorId, int action_n, uint16 action_flags) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

	if ((action_n < 0) || (action_n >= actor->action_ct)) {
		error("Actor::setAction wrong action_n 0x%X", action_n);
	}

	actor->action = action_n;
	actor->action_flags = action_flags;
	actor->action_frame = 0;
	actor->action_time = 0;

}

void Actor::setDefaultAction(uint16 actorId, int action_n, uint16 action_flags) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

	if ((action_n < 0) || (action_n >= actor->action_ct)) {
		error("Actor::setDefaultAction wrong action_n 0x%X", action_n);
	}

	actor->def_action = action_n;
	actor->def_action_flags = action_flags;
}
/*
ACTOR *Actor::lookupActor(int index) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	actorIterator = getActorIterator(index);
	actor = actorIterator.operator->();

	return actor;
}*/

int Actor::loadActorSpriteIndex(ACTOR * actor, int si_rn, int *last_frame_p) {
	byte *res_p;
	size_t res_len;
	int s_action_ct;
	ACTORACTION *action_p;
	int last_frame;
	int i, orient;
	int result;

	result = RSC_LoadResource(_actorContext, si_rn, &res_p, &res_len);
	if (result != SUCCESS) {
		warning("Couldn't load sprite action index resource");
		return FAILURE;
	}

	s_action_ct = res_len / 16;
	debug(0, "Sprite resource contains %d sprite actions.", s_action_ct);
	action_p = (ACTORACTION *)malloc(sizeof(ACTORACTION) * s_action_ct);

	MemoryReadStreamEndian readS(res_p, res_len, IS_BIG_ENDIAN);

	if (action_p == NULL) {
		warning("Couldn't allocate memory for sprite actions");
		RSC_FreeResource(res_p);
		return MEM;
	}

	last_frame = 0;

	for (i = 0; i < s_action_ct; i++) {
		for (orient = 0; orient < 4; orient++) {
			// Load all four orientations
			action_p[i].dir[orient].frame_index = readS.readUint16();
			action_p[i].dir[orient].frame_count = readS.readUint16();
			if (action_p[i].dir[orient].frame_index > last_frame) {
				last_frame = action_p[i].dir[orient].frame_index;
			}
		}
	}

	actor->act_tbl = action_p;
	actor->action_ct = s_action_ct;

	RSC_FreeResource(res_p);

	if (last_frame_p != NULL) {
		*last_frame_p = last_frame;
	}

	return SUCCESS;
}

void Actor::deleteActor(uint16 actorId) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	debug(0, "Actor::deleteActor actorId=0x%X", actorId);
	
	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

	_vm->_sprite->freeSprite(actor->sl_p);

	_list.erase(actorIterator);

	_tbl[ACTOR_ID_TO_INDEX(actorId)] = zeroActorIterator;
}

void Actor::walkTo(uint16 actorId, const Point *walk_pt, uint16 flags, SEMAPHORE *sem) {
	ACTORINTENT actor_intent;
	ActorList::iterator actorIterator;
	ACTOR *actor;

	assert(walk_pt != NULL);
	
	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();
	
	actor_intent.a_itype = INTENT_PATH;
	actor_intent.a_iflags = 0;
		

	actor_intent.walkIntent.wi_flags = flags;
	actor_intent.walkIntent.sem_held = 1;
	actor_intent.walkIntent.sem = sem;

	// handleWalkIntent() will create path on initialization
	actor_intent.walkIntent.wi_init = 0;
	actor_intent.walkIntent.dst_pt = *walk_pt;

	actor->a_intentlist.push_back(actor_intent);

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

int Actor::handleWalkIntent(ACTOR *actor, WALKINTENT *a_walkint, int *complete_p, int delta_time) {
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

	if (path_slope < 0) {
		reorderActorUp(actor->index);
	} else {
		reorderActorDown(actor->index);
	}
	// here "actor" pointer may be invalid 
	return SUCCESS;
}

void Actor::move(uint16 actorId, const Point *move_pt) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	int move_up = 0;

	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

	if (move_pt->y < actor->a_pt.y) {
		move_up = 1;
	}

	actor->a_pt.x = move_pt->x;
	actor->a_pt.y = move_pt->y;

	AtoS(&actor->s_pt, &actor->a_pt);

 	if (move_up) {
		reorderActorUp(actor->index);
	} else {
		reorderActorDown(actor->index);
	}
	// here "actor" pointer may be invalid 

}

void Actor::moveRelative(uint16 actorId, const Point *move_pt) {
	ActorList::iterator actorIterator;
	ACTOR *actor;

	actorIterator = getActorIterator(ACTOR_ID_TO_INDEX(actorId));
	actor = actorIterator.operator->();

	actor->a_pt.x += move_pt->x;
	actor->a_pt.y += move_pt->y;

	AtoS(&actor->s_pt, &actor->a_pt);

	if (actor->a_pt.y < 0) {
		reorderActorUp(actor->index);
	} else {
		reorderActorDown(actor->index);
	}
	// here "actor" pointer may be invalid 
}


int Actor::AtoS(Point *screen, const Point *actor) {
	screen->x = (actor->x / ACTOR_LMULT);
	screen->y = (actor->y / ACTOR_LMULT);

	return SUCCESS;
}

int Actor::StoA(Point *actor, const Point screen) {
	actor->x = (screen.x * ACTOR_LMULT);
	actor->y = (screen.y * ACTOR_LMULT);

	return SUCCESS;
}

// Console wrappers - must be safe to run
// TODO - checkup ALL arguments, cause wrong arguments may fall function with "error"

void Actor::CF_actor_add(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	int x = atoi(argv[2]);
	int y = atoi(argv[3]);
	int actorIdx = ACTOR_ID_TO_INDEX(actorId);
	
	if (!IS_VALID_ACTOR_INDEX(actorIdx)) {
		_vm->_console->DebugPrintf("Actor::CF_actor_add Invalid actorId 0x%X.\n",actorId);
		return;
	}
	
	if (actorExists(actorId)) {
		_vm->_console->DebugPrintf("Actor::CF_actor_add Actor already exist actorId 0x%X.\n",actorId);
		return;
	}

	create(actorId, x, y);
}

void Actor::CF_actor_del(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	
	if (!isValidActor(ACTOR_ID_TO_INDEX(actorId))) {
		_vm->_console->DebugPrintf("Actor::CF_actor_del Invalid actorId 0x%X.\n",actorId);
		return;
	}
	deleteActor(actorId);
}

void Actor::CF_actor_move(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Point move_pt;

	move_pt.x = atoi(argv[2]);
	move_pt.y = atoi(argv[3]);

	if (!isValidActor(ACTOR_ID_TO_INDEX(actorId))) {
		_vm->_console->DebugPrintf("Actor::CF_actor_move Invalid actorId 0x%X.\n",actorId);
		return;
	}

	move(actorId, &move_pt);
}

void Actor::CF_actor_moverel(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	Point move_pt;

	move_pt.x = atoi(argv[2]);
	move_pt.y = atoi(argv[3]);

	if (!isValidActor(ACTOR_ID_TO_INDEX(actorId))) {
		_vm->_console->DebugPrintf("Actor::CF_actor_moverel Invalid actorId 0x%X.\n",actorId);
		return;
	}

	moveRelative(actorId, &move_pt);
}

void Actor::CF_actor_seto(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	int orient;

	orient = atoi(argv[2]);
//TODO orient check
	if (!isValidActor(ACTOR_ID_TO_INDEX(actorId))) {
		_vm->_console->DebugPrintf("Actor::CF_actor_seto Invalid actorId 0x%X.\n",actorId);
		return;
	}

	setOrientation(actorId, orient);
}

void Actor::CF_actor_setact(int argc, const char **argv) {
	uint16 actorId = (uint16) atoi(argv[1]);
	int action_n = 0;

	action_n = atoi(argv[2]);

	if (!isValidActor(ACTOR_ID_TO_INDEX(actorId))) {
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
