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

// Actor management module header file

#ifndef SAGA_ACTOR_H__
#define SAGA_ACTOR_H__

#include "saga/sprite.h"
#include "saga/actordata.h"
#include "saga/list.h"

namespace Saga {

#define ACTOR_BASE_SPEED 0.25
#define ACTOR_BASE_ZMOD 0.5

#define ACTOR_DEFAULT_ORIENT 2
#define ACTOR_ORIENTMAX 7

#define ACTOR_ACTIONTIME 80

#define ACTOR_DIALOGUE_LETTERTIME 50
#define ACTOR_DIALOGUE_HEIGHT 100

#define ACTOR_LMULT 4

enum ACTOR_INTENTS {
	INTENT_NONE = 0,
	INTENT_PATH = 1,
	INTENT_SPEAK = 2
};

enum ACTOR_WALKFLAGS {
	WALK_NONE = 0x00,
	WALK_NOREORIENT = 0x01
};

enum ACTOR_ORIENTATIONS {
	ORIENT_N = 0,
	ORIENT_NE = 1,
	ORIENT_E = 2,
	ORIENT_SE = 3,
	ORIENT_S = 4,
	ORIENT_SW = 5,
	ORIENT_W = 6,
	ORIENT_NW = 7
};

enum ACTOR_ACTIONS {
	ACTION_IDLE = 0,
	ACTION_WALK = 1,
	ACTION_SPEAK = 2,
	ACTION_COUNT
};

enum ACTOR_ACTIONFLAGS {
	ACTION_NONE = 0x00,
	ACTION_LOOP = 0x01
};

struct ACTORACTIONITEM {
	int frame_index;
	int frame_count;
};

struct ACTORACTION {
	ACTORACTIONITEM dir[4];
};

struct WALKNODE {
	int calc_flag;
	Point node_pt;
	WALKNODE() {
		calc_flag = 0;
	}
};

typedef Common::List<WALKNODE> WalkNodeList;

struct WALKINTENT {
	int wi_active;
	uint16 wi_flags;
	int wi_init;

	int time;
	float slope;
	int x_dir;
	Point org;
	Point cur;

	Point dst_pt;
	WalkNodeList nodelist;

	int sem_held;
	SEMAPHORE *sem;

	WALKINTENT() { 
		wi_active = 0;
		wi_flags = 0;
		wi_init = 0;

		time = 0;
		slope = 0;
		x_dir = 0;

		sem_held = 0;
		sem = NULL;
	}
};


struct ACTORDIALOGUE {
	int d_playing;
	const char *d_string;
	uint16 d_voice_rn;
	long d_time;
	int d_sem_held;
	SEMAPHORE *d_sem;
	ACTORDIALOGUE() { memset(this, 0, sizeof(*this)); }
};

typedef Common::List<ACTORDIALOGUE> ActorDialogList;

struct SPEAKINTENT {
	int si_init;
	uint16 si_flags;
	int si_last_action;
	ActorDialogList si_diaglist;	/* Actor dialogue list */
	SPEAKINTENT() {
		si_init = 0;
		si_flags = 0;
		si_last_action = 0;
	}
};

struct ACTORINTENT {
	int a_itype;
	uint16 a_iflags;
	int a_idone;
	void *a_data;

	void createData() {
		assert(a_data == NULL);
		
		if(INTENT_SPEAK == a_itype) {
			a_data = new SPEAKINTENT;
		}
		else
			if(INTENT_PATH == a_itype) {
				a_data = new WALKINTENT;
			}
	}
	void deleteData() {

		if(INTENT_SPEAK == a_itype) {
			SPEAKINTENT *a_speakint;

			assert(a_data);
			a_speakint = (SPEAKINTENT *)a_data;
			delete a_speakint;
		}
		else
			if(INTENT_PATH == a_itype) {
				WALKINTENT *a_walkint;

				assert(a_data);
				a_walkint = (WALKINTENT *)a_data;
				delete a_walkint;
			}
		a_data = NULL;
	}

	ACTORINTENT() { memset(this, 0, sizeof(*this)); }
};

typedef Common::List<ACTORINTENT> ActorIntentList;

struct ACTOR {
	int id;			// Actor id
	int name_i;		// Actor's index in actor name string list
	uint16 flags;

	Point a_pt;		// Actor's logical coordinates
	Point s_pt;		// Actor's screen coordinates

	int sl_rn;		// Actor's sprite list res #
	int si_rn;		// Actor's sprite index res #
	SPRITELIST *sl_p;	// Actor's sprite list data

	int idle_time;
	int orient;
	int speaking;

	int a_dcolor;		// Actor dialogue color

	// The actor intent list describes what the actor intends to do;
	// multiple intents can be queued. The actor must complete an 
	// intent before moving on to the next; thus actor movements, esp
	// as described from scripts, can be serialized

	ActorIntentList a_intentlist;

	// WALKPATH path;

	int def_action;
	uint16 def_action_flags;

	int action;
	uint16 action_flags;
	int action_frame;
	int action_time;

	ACTORACTION *act_tbl;	// Action lookup table
	int action_ct;		// Number of actions in the action LUT
	ACTOR() {
		id = 0;
		name_i = 0;
		flags = 0;
		sl_rn = 0;
		si_rn = 0;
		sl_p = 0;
		idle_time = 0;
		orient = 0;
		speaking = 0;
		a_dcolor = 0;
		def_action = 0;
		def_action_flags = 0;
		action = 0;
		action_flags = 0;
		action_frame = 0;
		action_time = 0;
		act_tbl = NULL;
		action_ct = 0;
	}
};

typedef SortedList<ACTOR> ActorList;


struct ACTIONTIMES {
	int action;
	int time;
};

class Actor {
public:
	Actor(SagaEngine *vm);
	~Actor();

	void CF_actor_add(int argc, const char **argv);
	void CF_actor_del(int argc, const char **argv);
	void CF_actor_move(int argc, const char **argv);
	void CF_actor_moverel(int argc, const char **argv);
	void CF_actor_seto(int argc, const char **argv);
	void CF_actor_setact(int argc, const char **argv);

	int direct(int msec);

	int create(int actor_id, int x, int y);
	int actorExists(uint16 actor_id);

	int drawList();
	int AtoS(Point *logical, const Point *actor);
	int StoA(Point *actor, const Point screen);

	int move(int index, const Point *move_pt);
	int moveRelative(int index, const Point *move_pt);

	int walkTo(int index, const Point *walk_pt, uint16 flags, SEMAPHORE *sem);
	
	int getActorIndex(uint16 actor_id);
	
	int speak(int index, const char *d_string, uint16 d_voice_rn, SEMAPHORE *sem);
	
	int skipDialogue();
	
	int getSpeechTime(const char *d_string, uint16 d_voice_rn);
	int setOrientation(int index, int orient);
	int setAction(int index, int action_n, uint16 action_flags);
	int setDefaultAction(int index, int action_n, uint16 action_flags);

	int addActor(ACTOR * actor);
	int deleteActor(int index);
	ACTOR *lookupActor(int index);

private:
	int handleWalkIntent(ACTOR *actor, WALKINTENT *a_walk_int, int *complete_p, int msec);
	int handleSpeakIntent(ACTOR *actor, SPEAKINTENT *a_speakint, int *complete_p, int msec);
	int setPathNode(WALKINTENT *walk_int, Point *src_pt, Point *dst_pt, SEMAPHORE *sem);
	int loadActorSpriteIndex(ACTOR *actor, int si_rn, int *last_frame_p);

	SagaEngine *_vm;
	bool _initialized;
	RSCFILE_CONTEXT *_actorContext;
	uint16 _count;
	int _aliasTbl[ACTORCOUNT];
	ActorList::iterator _tbl[ACTORCOUNT];
	ActorList _list;
};

} // End of namespace Saga

#endif
