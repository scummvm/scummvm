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
/*
 Description:	
 
	Actor management module header file

 Notes: 
 
	Hardcoded actor table present in r_actordata.c
*/

#ifndef SAGA_ACTOR_H__
#define SAGA_ACTOR_H__

namespace Saga {

#define ACTOR_BASE_SPEED 0.25
#define ACTOR_BASE_ZMOD 0.5

#define ACTOR_DEFAULT_ORIENT 2
#define ACTOR_ORIENTMAX 7

#define ACTOR_ACTIONTIME 80

#define ACTOR_DIALOGUE_LETTERTIME 50
#define ACTOR_DIALOGUE_HEIGHT 100

#define R_ACTOR_LMULT 4

enum R_ACTOR_INTENTS {

	INTENT_NONE = 0,
	INTENT_PATH = 1,
	INTENT_SPEAK = 2
};

struct R_ACTORACTIONITEM {

	int frame_index;
	int frame_count;

};

struct R_ACTORACTION {

	R_ACTORACTIONITEM dir[4];

};

struct R_WALKINTENT {

	int wi_active;
	uint wi_flags;
	int wi_init;

	int time;
	float slope;
	int x_dir;
	R_POINT org;
	R_POINT cur;

	R_POINT dst_pt;
	YS_DL_LIST *nodelist;

	int sem_held;
	R_SEMAPHORE *sem;
	
	R_WALKINTENT() { memset(this, 0, sizeof(*this)); }

};

struct R_WALKNODE {

	int calc_flag;
	R_POINT node_pt;

};

struct R_SPEAKINTENT {

	int si_init;
	uint si_flags;
	int si_last_action;

	YS_DL_LIST *si_diaglist;	/* Actor dialogue list */

};

struct R_ACTORINTENT {

	int a_itype;
	uint a_iflags;
	int a_idone;

	void *a_data;
	
	R_ACTORINTENT() { memset(this, 0, sizeof(*this)); }

};

struct R_ACTOR {

	int id;			/* Actor id */
	int name_i;		/* Actor's index in actor name string list */
	uint flags;

	R_POINT a_pt;		/* Actor's logical coordinates */
	R_POINT s_pt;		/* Actor's screen coordinates */

	int sl_rn;		/* Actor's sprite list res # */
	int si_rn;		/* Actor's sprite index res # */
	R_SPRITELIST *sl_p;	/* Actor's sprite list data */

	int idle_time;
	int orient;
	int speaking;

	int a_dcolor;		/* Actor dialogue color */

	/* The actor intent list describes what the actor intends to do;
	 * multiple intents can be queued. The actor must complete an 
	 * intent before moving on to the next; thus actor movements, esp
	 * as described from scripts, can be serialized */

	YS_DL_LIST *a_intentlist;
/*	
    R_WALKPATH path;
*/

	int def_action;
	uint def_action_flags;

	int action;
	uint action_flags;

	int action_frame;
	int action_time;

	R_ACTORACTION *act_tbl;	/* Action lookup table */
	int action_ct;		/* Number of actions in the action LUT */

	YS_DL_NODE *node;	/* Actor's node in the actor list */
	
	R_ACTOR() { memset(this, 0, sizeof(*this)); }

};

struct R_ACTORDIALOGUE {

	int d_playing;
	const char *d_string;
	uint d_voice_rn;

	long d_time;
	int d_sem_held;
	R_SEMAPHORE *d_sem;
	
	R_ACTORDIALOGUE() { memset(this, 0, sizeof(*this)); }

};

struct R_ACTIONTIMES {

	int action;
	int time;

};

struct R_ACTOR_MODULE {

	int init;

	R_RSCFILE_CONTEXT *actor_ctxt;

	uint count;

	int *alias_tbl;
	YS_DL_NODE **tbl;
	YS_DL_LIST *list;

	int err_n;
	const char *err_str;

};

R_ACTOR *LookupActor(int index);

int AddActor(R_ACTOR * actor);

int Z_Compare(const void *elem1, const void *elem2);

int
HandleWalkIntent(R_ACTOR * actor,
    R_WALKINTENT * a_walk_int, int *complete_p, int msec);

int
HandleSpeakIntent(R_ACTOR * actor,
    R_SPEAKINTENT * a_speakint, int *complete_p, int msec);

int
ACTOR_SetPathNode(R_WALKINTENT * walk_int,
    R_POINT * src_pt, R_POINT * dst_pt, R_SEMAPHORE * sem);

int LoadActorSpriteIndex(R_ACTOR * actor, int si_rn, int *last_frame_p);

static void CF_actor_add(int argc, char *argv[]);

static void CF_actor_del(int argc, char *argv[]);

static void CF_actor_move(int argc, char *argv[]);

static void CF_actor_moverel(int argc, char *argv[]);

static void CF_actor_seto(int argc, char *argv[]);

static void CF_actor_setact(int argc, char *argv[]);

} // End of namespace Saga

#endif				/* R_ACTOR_H__ */
/* end "r_actor.h" */
