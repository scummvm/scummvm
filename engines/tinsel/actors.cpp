/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Handles things to do with actors, delegates much moving actor stuff.
 */

#include "tinsel/actors.h"
#include "tinsel/events.h"
#include "tinsel/film.h"	// for FREEL
#include "tinsel/handle.h"
#include "tinsel/inventory.h"	// INV_NOICON
#include "tinsel/move.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"	// for POBJECT
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/serializer.h"
#include "tinsel/token.h"

#include "common/util.h"

namespace Tinsel {


//----------------- LOCAL DEFINES --------------------


#include "common/pack-start.h"	// START STRUCT PACKING

/** actor struct - one per actor */
struct ACTOR_STRUC {
	int32 masking;			//!< type of actor masking
	SCNHANDLE hActorId;		//!< handle actor ID string index
	SCNHANDLE hActorCode;	//!< handle to actor script
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING



//----------------- LOCAL GLOBAL DATA --------------------

static int LeadActorId = 0;		// The lead actor

static int NumActors = 0;	// The total number of actors in the game

struct ACTORINFO {
	bool		alive;		// TRUE == alive
	bool		hidden;		// TRUE == hidden
	bool		completed;	// TRUE == script played out

	int			x, y, z;

	int32		mtype;		// DEFAULT(b'ground), MASK, ALWAYS
	SCNHANDLE	actorCode;	// The actor's script

	const FREEL	*presReel;	// the present reel
	int			presRnum;	// the present reel number
	SCNHANDLE 	presFilm;	// the film that reel belongs to
	OBJECT		*presObj;	// reference for position information
	int			presX, presY;

	bool		tagged;		// actor tagged?
	SCNHANDLE	hTag;		// handle to tag text
	int			tType;		// e.g. TAG_Q1TO3

	bool		escOn;
	int			escEv;

	COLORREF	tColour;	// Text colour

	SCNHANDLE 	playFilm;	// revert to this after talks
	SCNHANDLE 	talkFilm;	// this be deleted in the future!
	SCNHANDLE 	latestFilm;	// the last film ordered
	bool		talking;

	int			steps;

};

static ACTORINFO *actorInfo = 0;

static COLORREF defaultColour = 0;		// Text colour

static bool bActorsOn = false;

static int ti = 0;

/**
 * Called once at start-up time, and again at restart time.
 * Registers the total number of actors in the game.
 * @param num			Chunk Id
 */
void RegisterActors(int num) {
	if (actorInfo == NULL) 	{
		// Store the total number of actors in the game
		NumActors = num;

		// Check we can save so many
		assert(NumActors <= MAX_SAVED_ALIVES);

		// Allocate RAM for actorInfo
		// FIXME: For now, we always allocate MAX_SAVED_ALIVES blocks,
		//   as this makes the save/load code simpler
		actorInfo = (ACTORINFO *)calloc(MAX_SAVED_ALIVES, sizeof(ACTORINFO));

		// make sure memory allocated
		if (actorInfo == NULL) {
			error("Cannot allocate memory for actors");
		}
	} else {
		// Check the total number of actors is still the same
		assert(num == NumActors);

		memset(actorInfo, 0, MAX_SAVED_ALIVES * sizeof(ACTORINFO));
	}

	// All actors start off alive.
	while (num--)
		actorInfo[num].alive = true;
}

void FreeActors() {
	if (actorInfo) {
		free(actorInfo);
		actorInfo = NULL;
	}
}

/**
 * Called from dec_lead(), i.e. normally once at start of master script.
 * @param leadID			Lead Id
 */
void setleadid(int leadID) {
	LeadActorId = leadID;
	actorInfo[leadID-1].mtype = ACT_MASK;
}

/**
 * No comment.
 */
int LeadId(void) {
	return LeadActorId;
}

struct ATP_INIT {
	int		id;		// Actor number
	USER_EVENT	event;		// Event
	BUTEVENT	bev;		// Causal mouse event
};

/**
 * Runs actor's glitter code.
 */
static void ActorTinselProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	ATP_INIT *atp = (ATP_INIT *)param;

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_1(AllowDclick, atp->bev);		// May kill us if single click

	// Run the Glitter code
	assert(actorInfo[atp->id - 1].actorCode); // no code to run

	_ctx->pic = InitInterpretContext(GS_ACTOR, actorInfo[atp->id - 1].actorCode, atp->event, NOPOLY, atp->id, NULL);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	// If it gets here, actor's code has run to completion
	actorInfo[atp->id - 1].completed = true;

	CORO_END_CODE;
}


//---------------------------------------------------------------------------

struct RATP_INIT {
	INT_CONTEXT *pic;
	int		id;		// Actor number
};

static void ActorRestoredProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		INT_CONTEXT *pic;
	CORO_END_CONTEXT(_ctx);

	// get the stuff copied to process when it was created
	RATP_INIT *r = (RATP_INIT *)param;

	CORO_BEGIN_CODE(_ctx);

	_ctx->pic = RestoreInterpretContext(r->pic);
	CORO_INVOKE_1(Interpret, _ctx->pic);

	// If it gets here, actor's code has run to completion
	actorInfo[r->id - 1].completed = true;

	CORO_END_CODE;
}

void RestoreActorProcess(int id, INT_CONTEXT *pic) {
	RATP_INIT r = { pic, id };

	g_scheduler->createProcess(PID_TCODE, ActorRestoredProcess, &r, sizeof(r));
}

/**
 * Starts up process to runs actor's glitter code.
 * @param ano			Actor Id
 * @param event			Event structure
 * @param be			ButEvent
 */
void actorEvent(int ano, USER_EVENT event, BUTEVENT be) {
	ATP_INIT atp;

	// Only if there is Glitter code associated with this actor.
	if (actorInfo[ano - 1].actorCode) {
		atp.id = ano;
		atp.event = event;
		atp.bev = be;
		g_scheduler->createProcess(PID_TCODE, ActorTinselProcess, &atp, sizeof(atp));
	}
}

/**
 * Called at the start of each scene for each actor with a code block.
 * @param as			Actor structure
 * @param bRunScript	Flag for whether to run actor's script for the scene
 */
void StartActor(const ACTOR_STRUC *as, bool bRunScript) {
	SCNHANDLE hActorId = FROM_LE_32(as->hActorId);

	// Zero-out many things
	actorInfo[hActorId - 1].hidden = false;
	actorInfo[hActorId - 1].completed = false;
	actorInfo[hActorId - 1].x = 0;
	actorInfo[hActorId - 1].y = 0;
	actorInfo[hActorId - 1].presReel = NULL;
	actorInfo[hActorId - 1].presFilm = 0;
	actorInfo[hActorId - 1].presObj = NULL;

	// Store current scene's parameters for this actor
	actorInfo[hActorId - 1].mtype = FROM_LE_32(as->masking);
	actorInfo[hActorId - 1].actorCode = FROM_LE_32(as->hActorCode);

	// Run actor's script for this scene
	if (bRunScript) {
		if (bActorsOn)
			actorInfo[hActorId - 1].alive = true;

		if (actorInfo[hActorId - 1].alive && FROM_LE_32(as->hActorCode))
			actorEvent(hActorId, STARTUP, BE_NONE);
	}
}

/**
 * Called at the start of each scene. Start each actor with a code block.
 * @param ah			Scene handle
 * @param numActors		Number of actors
 * @param bRunScript	Flag for whether to run actor scene scripts
 */
void StartActors(SCNHANDLE ah, int numActors, bool bRunScript) {
	int	i;

	// Only actors with code blocks got (x, y) re-initialised, so...
	for (i = 0; i < NumActors; i++) {
		actorInfo[i].x = actorInfo[i].y = 0;
		actorInfo[i].mtype = 0;
	}

	const ACTOR_STRUC *as = (const ACTOR_STRUC *)LockMem(ah);
	for (i = 0; i < numActors; i++, as++) {
		StartActor(as, bRunScript);
	}
}

/**
 * Called between scenes, zeroises all actors.
 */
void DropActors(void) {
	for (int i = 0; i < NumActors; i++) {
		actorInfo[i].actorCode = 0;	// No script
		actorInfo[i].presReel = NULL;	// No reel running
		actorInfo[i].presFilm = 0;	//   ditto
		actorInfo[i].presObj = NULL;	// No object
		actorInfo[i].x = 0;		// No position
		actorInfo[i].y = 0;		//   ditto

		actorInfo[i].talkFilm = 0;
		actorInfo[i].latestFilm = 0;
		actorInfo[i].playFilm = 0;
		actorInfo[i].talking = false;
	}
}

/**
 * Kill actors.
 * @param ano			Actor Id
 */
void DisableActor(int ano) {
	PMACTOR	pActor;

	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].alive = false;	// Record as dead
	actorInfo[ano - 1].x = actorInfo[ano - 1].y = 0;

	// Kill off moving actor properly
	pActor = GetMover(ano);
	if (pActor)
		KillMActor(pActor);
}

/**
 * Enable actors.
 * @param ano			Actor Id
 */
void EnableActor(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	// Re-incarnate only if it's dead, or it's script ran to completion
	if (!actorInfo[ano - 1].alive || actorInfo[ano - 1].completed) {
		actorInfo[ano - 1].alive = true;
		actorInfo[ano - 1].hidden = false;
		actorInfo[ano - 1].completed = false;

		// Re-run actor's script for this scene
		if (actorInfo[ano-1].actorCode)
			actorEvent(ano, STARTUP, BE_NONE);
	}
}

/**
 * Returns the aliveness (to coin a word) of the actor.
 * @param ano			Actor Id
 */
bool actorAlive(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].alive;
}

/**
 * Define an actor as being tagged.
 * @param ano			Actor Id
 * @param tagtext		Scene handle
 * @param tp			tType
 */
void Tag_Actor(int ano, SCNHANDLE tagtext, int tp) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano-1].tagged = true;
	actorInfo[ano-1].hTag = tagtext;
	actorInfo[ano-1].tType = tp;
}

/**
 * Undefine  an actor as being tagged.
 * @param ano			Actor Id
 * @param tagtext		Scene handle
 * @param tp			tType
 */
void UnTagActor(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano-1].tagged = false;
}

/**
 * Redefine an actor as being tagged.
 * @param ano			Actor Id
 * @param tagtext		Scene handle
 * @param tp			tType
 */
void ReTagActor(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (actorInfo[ano-1].hTag)
		actorInfo[ano-1].tagged = true;
}

/**
 * Returns a tagged actor's tag type. e.g. TAG_Q1TO3
 * @param ano			Actor Id
 */
int TagType(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano-1].tType;
}

/**
 * Returns handle to tagged actor's tag text
 * @param ano			Actor Id
 */
SCNHANDLE GetActorTag(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].hTag;
}

/**
 * Called from TagProcess, FirstTaggedActor() resets the index, then
 * NextTagged Actor is repeatedly called until the caller gets fed up
 * or there are no more tagged actors to look at.
 */
void FirstTaggedActor(void) {
	ti = 0;
}

/**
 * Called from TagProcess, FirstTaggedActor() resets the index, then
 * NextTagged Actor is repeatedly called until the caller gets fed up
 * or there are no more tagged actors to look at.
 */
int NextTaggedActor(void) {
	PMACTOR	pActor;
	bool	hid;

	do {
		if (actorInfo[ti].tagged) {
			pActor = GetMover(ti+1);
			if (pActor)
				hid = getMActorHideState(pActor);
			else
				hid = actorInfo[ti].hidden;

			if (!hid) {
				return ++ti;
			}
		}
	} while (++ti < NumActors);

	return 0;
}

/**
 * Returns the masking type of the actor.
 * @param ano			Actor Id
 */
int32 actorMaskType(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].mtype;
}

/**
 * Store/Return the currently stored co-ordinates of the actor.
 * Delegate the task for moving actors.
 * @param ano			Actor Id
 * @param x				X position
 * @param y				Y position
 */
void storeActorPos(int ano, int x, int y) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].x = x;
	actorInfo[ano - 1].y = y;
}

void storeActorSteps(int ano, int steps) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].steps = steps;
}

int getActorSteps(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].steps;
}

void storeActorZpos(int ano, int z) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].z = z;
}


void GetActorPos(int ano, int *x, int *y) {
	PMACTOR pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // unknown actor

	pActor = GetMover(ano);

	if (pActor)
		GetMActorPosition(pActor, x, y);
	else {
		*x = actorInfo[ano - 1].x;
		*y = actorInfo[ano - 1].y;
	}
}

/**
 * Returns the position of the mid-top of the actor.
 * Delegate the task for moving actors.
 * @param ano			Actor Id
 * @param x				Output x
 * @param y				Output y
 */
void GetActorMidTop(int ano, int *x, int *y) {
	// Not used in JAPAN version
	PMACTOR pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // unknown actor

	pActor = GetMover(ano);

	if (pActor)
		GetMActorMidTopPosition(pActor, x, y);
	else if (actorInfo[ano - 1].presObj) {
		*x = (MultiLeftmost(actorInfo[ano - 1].presObj)
		      + MultiRightmost(actorInfo[ano - 1].presObj)) / 2;
		*y = MultiHighest(actorInfo[ano - 1].presObj);
	} else
		GetActorPos(ano, x, y);		// The best we can do!
}

/**
 * Return the appropriate co-ordinate of the actor.
 * @param ano			Actor Id
 */
int GetActorLeft(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!actorInfo[ano - 1].presObj)
		return 0;

	return MultiLeftmost(actorInfo[ano - 1].presObj);
}

/**
 * Return the appropriate co-ordinate of the actor.
 * @param ano			Actor Id
 */
int GetActorRight(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!actorInfo[ano - 1].presObj)
		return 0;

	return MultiRightmost(actorInfo[ano - 1].presObj);
}

/**
 * Return the appropriate co-ordinate of the actor.
 * @param ano			Actor Id
 */
int GetActorTop(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!actorInfo[ano - 1].presObj)
		return 0;

	return MultiHighest(actorInfo[ano - 1].presObj);
}

/**
 * Return the appropriate co-ordinate of the actor.
 */
int GetActorBottom(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (!actorInfo[ano - 1].presObj)
		return 0;

	return MultiLowest(actorInfo[ano - 1].presObj);
}

/**
 * Set actor hidden status to true.
 * For a moving actor, actually hide it.
 * @param ano			Actor Id
 */
void HideActor(int ano) {
	PMACTOR pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // illegal actor

	// Get moving actor involved
	pActor = GetMover(ano);

	if (pActor)
		hideMActor(pActor, 0);
	else
		actorInfo[ano - 1].hidden = true;
}

/**
 * Hide an actor if it's a moving actor.
 * @param ano			Actor Id
 * @param sf			sf
 */
bool HideMovingActor(int ano, int sf) {
	PMACTOR pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // illegal actor

	// Get moving actor involved
	pActor = GetMover(ano);

	if (pActor) {
		hideMActor(pActor, sf);
		return true;
	} else {
		if (actorInfo[ano - 1].presObj != NULL)
			MultiHideObject(actorInfo[ano - 1].presObj);	// Hidee object
		return false;
	}
}

/**
 * Unhide an actor if it's a moving actor.
 * @param ano			Actor Id
 */
void unHideMovingActor(int ano) {
	PMACTOR pActor;

	assert((ano > 0 && ano <= NumActors) || ano == LEAD_ACTOR); // illegal actor

	// Get moving actor involved
	pActor = GetMover(ano);

	assert(pActor); // not a moving actor

	unhideMActor(pActor);
}

/**
 * Called after a moving actor had been replaced by an splay().
 * Moves the actor to where the splay() left it, and continues the
 * actor's walk (if any) from the new co-ordinates.
 */
void restoreMovement(int ano) {
	PMACTOR pActor;

	assert(ano > 0 && ano <= NumActors); // illegal actor number

	// Get moving actor involved
	pActor = GetMover(ano);

	assert(pActor); // not a moving actor

	if (pActor->objx == actorInfo[ano - 1].x && pActor->objy == actorInfo[ano - 1].y)
		return;

	pActor->objx = actorInfo[ano - 1].x;
	pActor->objy = actorInfo[ano - 1].y;

	if (pActor->actorObj)
		SSetActorDest(pActor);
}

/**
 * More properly should be called:
 * 'store_actor_reel_and/or_film_and/or_object()'
 */
void storeActorReel(int ano, const FREEL *reel, SCNHANDLE film, OBJECT *pobj, int reelnum, int x, int y) {
	PMACTOR pActor;

	assert(ano > 0 && ano <= NumActors); // illegal actor number

	pActor = GetMover(ano);

	// Only store the reel and film for a moving actor if NOT called from MActorProcess()
	// (MActorProcess() calls with reel=film=NULL, pobj not NULL)
	if (!pActor
	|| !(reel == NULL && film == 0 && pobj != NULL)) {
		actorInfo[ano - 1].presReel = reel;	// Store reel
		actorInfo[ano - 1].presRnum = reelnum;	// Store reel number
		actorInfo[ano - 1].presFilm = film;	// Store film
		actorInfo[ano - 1].presX = x;
		actorInfo[ano - 1].presY = y;
	}

	// Only store the object for a moving actor if called from MActorProcess()
	if (!pActor) {
		actorInfo[ano - 1].presObj = pobj;	// Store object
	} else if (reel == NULL && film == 0 && pobj != NULL) {
		actorInfo[ano - 1].presObj = pobj;	// Store object
	}
}

/**
 * Return the present reel/film of the actor.
 */
const FREEL *actorReel(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].presReel;	// the present reel
}

/***************************************************************************/

void setActorPlayFilm(int ano, SCNHANDLE film) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].playFilm = film;
}

SCNHANDLE getActorPlayFilm(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].playFilm;
}

void setActorTalkFilm(int ano, SCNHANDLE film) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].talkFilm = film;
}

SCNHANDLE getActorTalkFilm(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].talkFilm;
}

void setActorTalking(int ano, bool tf) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].talking = tf;;
}

bool isActorTalking(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].talking;
}

void setActorLatestFilm(int ano, SCNHANDLE film) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].latestFilm = film;
	actorInfo[ano - 1].steps = 0;
}

SCNHANDLE getActorLatestFilm(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].latestFilm;
}

/***************************************************************************/

void updateActorEsc(int ano, bool escOn, int escEvent) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	actorInfo[ano - 1].escOn = escOn;
	actorInfo[ano - 1].escEv = escEvent;
}

bool actorEsc(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].escOn;
}

int actorEev(int ano) {
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	return actorInfo[ano - 1].escEv;
}

/**
 * Guess what these do.
 */
int AsetZPos(OBJECT *pObj, int y, int32 z) {
	int zPos;

	z += z ? -1 : 0;

	zPos = y + (z << 10);
	MultiSetZPosition(pObj, zPos);
	return zPos;
}

/**
 * Guess what these do.
 */
void MAsetZPos(PMACTOR pActor, int y, int32 zFactor) {
	if (!pActor->aHidden)
		AsetZPos(pActor->actorObj, y, zFactor);
}

/**
 * Stores actor's attributes.
 * Currently only the speech colours.
 */
void storeActorAttr(int ano, int r1, int g1, int b1) {
	assert((ano > 0 && ano <= NumActors) || ano == -1); // illegal actor number

	if (r1 > MAX_INTENSITY)	r1 = MAX_INTENSITY;	// } Ensure
	if (g1 > MAX_INTENSITY)	g1 = MAX_INTENSITY;	// } within limits
	if (b1 > MAX_INTENSITY)	b1 = MAX_INTENSITY;	// }

	if (ano == -1)
		defaultColour = RGB(r1, g1, b1);
	else
		actorInfo[ano - 1].tColour = RGB(r1, g1, b1);
}

/**
 * Get the actor's stored speech colour.
 * @param ano			Actor Id
 */
COLORREF getActorTcol(int ano) {
	// Not used in JAPAN version
	assert(ano > 0 && ano <= NumActors); // illegal actor number

	if (actorInfo[ano - 1].tColour)
		return actorInfo[ano - 1].tColour;
	else
		return defaultColour;
}

/**
 * Store relevant information pertaining to currently existing actors.
 */
int SaveActors(SAVED_ACTOR *sActorInfo) {
	int	i, j;

	for (i = 0, j = 0; i < NumActors; i++) {
		if (actorInfo[i].presObj != NULL) {
			assert(j < MAX_SAVED_ACTORS); // Saving too many actors

//			sActorInfo[j].hidden	= actorInfo[i].hidden;
			sActorInfo[j].bAlive	= actorInfo[i].alive;
//			sActorInfo[j].x		= (short)actorInfo[i].x;
//			sActorInfo[j].y		= (short)actorInfo[i].y;
			sActorInfo[j].z		= (short)actorInfo[i].z;
//			sActorInfo[j].presReel	= actorInfo[i].presReel;
			sActorInfo[j].presRnum	= (short)actorInfo[i].presRnum;
			sActorInfo[j].presFilm	= actorInfo[i].presFilm;
			sActorInfo[j].presX	= (short)actorInfo[i].presX;
			sActorInfo[j].presY	= (short)actorInfo[i].presY;
			sActorInfo[j].actorID	= (short)(i+1);
			j++;
		}
	}

	return j;
}

void setactorson(void) {
	bActorsOn = true;
}

void ActorsLife(int ano, bool bAlive) {
	assert((ano > 0 && ano <= NumActors) || ano == -1); // illegal actor number

	actorInfo[ano-1].alive = bAlive;
}


void syncAllActorsAlive(Serializer &s) {
	for (int i = 0; i < MAX_SAVED_ALIVES; i++) {
		s.syncAsByte(actorInfo[i].alive);
		s.syncAsByte(actorInfo[i].tagged);
		s.syncAsByte(actorInfo[i].tType);
		s.syncAsUint32LE(actorInfo[i].hTag);
	}
}


} // end of namespace Tinsel
