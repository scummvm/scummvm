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
 * Should really be called "moving actors.c"
 */

#include "tinsel/actors.h"
#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/config.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/inventory.h"
#include "tinsel/move.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/pcode.h"
#include "tinsel/pid.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/timers.h"
#include "tinsel/token.h"

#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

static MACTOR Movers[MAX_MOVERS];


/**
 * RebootMovers
 */
void RebootMovers(void) {
	memset(Movers, 0, sizeof(Movers));
}

/**
 * Given an actor number, return pointer to its moving actor structure,
 * if it is a moving actor.
 */
PMACTOR GetMover(int ano) {
	int i;

	// Slot 0 is reserved for lead actor
	if (ano == LeadId() || ano == LEAD_ACTOR)
		return &Movers[0];

	for (i = 1; i < MAX_MOVERS; i++)
		if (Movers[i].actorID == ano)
			return &Movers[i];

	return NULL;
}

/**
 * Register an actor as being a moving one.
 */
PMACTOR SetMover(int ano) {
	int i;

	// Slot 0 is reserved for lead actor
	if (ano == LeadId() || ano == LEAD_ACTOR) {
		Movers[0].actorToken = TOKEN_LEAD;
		Movers[0].actorID = LeadId();
		return &Movers[0];
	}

	// Check it hasn't already been declared
	for (i = 1; i < MAX_MOVERS; i++) {
		if (Movers[i].actorID == ano) {
			// Actor is already a moving actor
			return &Movers[i];
		}
	}

	// Find an empty slot
	for (i = 1; i < MAX_MOVERS; i++)
		if (!Movers[i].actorID) {
			Movers[i].actorToken = TOKEN_LEAD + i;
			Movers[i].actorID = ano;
			return &Movers[i];
		}

	error("Too many moving actors");
}

/**
 * Given an index, returns the associated moving actor.
 *
 * At the time of writing, used by the effect process.
 */
PMACTOR GetLiveMover(int index) {
	assert(index >= 0 && index < MAX_MOVERS); // out of range

	if (Movers[index].MActorState == NORM_MACTOR)
		return &Movers[index];
	else
		return NULL;
}

bool IsMAinEffectPoly(int index) {
	assert(index >= 0 && index < MAX_MOVERS); // out of range

	return Movers[index].InEffect;
}

void SetMAinEffectPoly(int index, bool tf) {
	assert(index >= 0 && index < MAX_MOVERS); // out of range

	Movers[index].InEffect = tf;
}

/**
 * Remove a moving actor from the current scene.
 */
void KillMActor(PMACTOR pActor) {
	if (pActor->MActorState == NORM_MACTOR) {
		pActor->MActorState = NO_MACTOR;
		MultiDeleteObject(GetPlayfieldList(FIELD_WORLD), pActor->actorObj);
		pActor->actorObj = NULL;
		assert(CurrentProcess() != pActor->pProc);
		ProcessKill(pActor->pProc);
	}
}

/**
 * getMActorState
 */
MAS getMActorState(PMACTOR pActor) {
	return pActor->MActorState;
}

/**
 * If the actor's object exists, move it behind the background.
 * MultiHideObject() is deliberately not used, as StepAnimScript() calls
 * cause the object to re-appear.
 */
void hideMActor(PMACTOR pActor, int sf) {
	assert(pActor); // Hiding null moving actor

	pActor->aHidden = true;
	pActor->SlowFactor = sf;

	if (pActor->actorObj)
		MultiSetZPosition(pActor->actorObj, -1);
}

/**
 * getMActorHideState
 */
bool getMActorHideState(PMACTOR pActor) {
	if (pActor)
		return pActor->aHidden;
	else
		return false;
}

/**
 * unhideMActor
 */
void unhideMActor(PMACTOR pActor) {
	assert(pActor); // unHiding null moving actor

	pActor->aHidden = false;

	// Make visible on the screen
	if (pActor->actorObj) {
		// If no path, just use first path in the scene
		if (pActor->hCpath != NOPOLY)
			MAsetZPos(pActor, pActor->objy, getPolyZfactor(pActor->hCpath));
		else
			MAsetZPos(pActor, pActor->objy, getPolyZfactor(FirstPathPoly()));
	}
}

/**
 * Get it into our heads that there's nothing doing.
 * Called at the end of a scene.
 */
void DropMActors(void) {
	for (int i = 0; i < MAX_MOVERS; i++) {
		Movers[i].MActorState = NO_MACTOR;
		Movers[i].objx = 0;
		Movers[i].objy = 0;
		Movers[i].actorObj = NULL;	// No moving actor objects

		Movers[i].hCpath = NOPOLY;	// No moving actor path
	}
}


/**
 * Reposition a moving actor.
 */
void MoveMActor(PMACTOR pActor, int x, int y) {
	int	z;
	int	node;
	HPOLYGON hPath;

	assert(pActor); // Moving null moving actor
	assert(pActor->actorObj);

	pActor->objx = x;
	pActor->objy = y;
	MultiSetAniXY(pActor->actorObj, x, y);

	hPath = InPolygon(x, y, PATH);
	if (hPath != NOPOLY) {
		pActor->hCpath = hPath;
		if (PolySubtype(hPath) == NODE) {
			node = NearestNodeWithin(hPath, x, y);
			getNpathNode(hPath, node, &pActor->objx, &pActor->objy);
			pActor->hFnpath = hPath;
			pActor->line = node;
			pActor->npstatus = GOING_UP;
		} else {
			pActor->hFnpath = NOPOLY;
			pActor->npstatus = NOT_IN;
		}

		z = GetScale(hPath, pActor->objy);
		pActor->scale = z;
		SetMActorStanding(pActor);
	} else {
		pActor->bNoPath = true;

		pActor->hFnpath = NOPOLY;	// Ain't in one
		pActor->npstatus = NOT_IN;

		// Ensure legal reel and scale
		if (pActor->dirn < 0 || pActor->dirn > 3)
			pActor->dirn = FORWARD;
		if (pActor->scale < 0 || pActor->scale > TOTAL_SCALES)
			pActor->scale = 1;
	}
}

/**
 * Get position of a moving actor.
 */
void GetMActorPosition(PMACTOR pActor, int *paniX, int *paniY) {
	assert(pActor); // Getting null moving actor's position

	if (pActor->actorObj != NULL)
		GetAniPosition(pActor->actorObj, paniX, paniY);
	else {
		*paniX = 0;
		*paniY = 0;
	}
}

/**
 * Moving actor's mid-top position.
 */
void GetMActorMidTopPosition(PMACTOR pActor, int *aniX, int *aniY) {
	assert(pActor); // Getting null moving actor's mid-top position
	assert(pActor->actorObj); // Getting null moving actor's mid-top position

	*aniX = (MultiLeftmost(pActor->actorObj) + MultiRightmost(pActor->actorObj))/2;
	*aniY = MultiHighest(pActor->actorObj);
}

/**
 * Moving actor's left-most co-ordinate.
 */
int GetMActorLeft(PMACTOR pActor) {
	assert(pActor); // Getting null moving actor's leftmost position
	assert(pActor->actorObj); // Getting null moving actor's leftmost position

	return MultiLeftmost(pActor->actorObj);
}

/**
 * Moving actor's right-most co-ordinate.
 */
int GetMActorRight(PMACTOR pActor) {
	assert(pActor); // Getting null moving actor's rightmost position
	assert(pActor->actorObj); // Getting null moving actor's rightmost position

	return MultiRightmost(pActor->actorObj);
}

/**
 * See if moving actor is stood within a polygon.
 */
bool MActorIsInPolygon(PMACTOR pActor, HPOLYGON hp) {
	assert(pActor); // Checking if null moving actor is in polygon
	assert(pActor->actorObj); // Checking if null moving actor is in polygon

	int aniX, aniY;
	GetAniPosition(pActor->actorObj, &aniX, &aniY);

	return IsInPolygon(aniX, aniY, hp);
}

/**
 * Change which reel is playing for a moving actor.
 */
void AlterMActor(PMACTOR pActor, SCNHANDLE film, AR_FUNCTION fn) {
	const FILM *pfilm;

	assert(pActor->actorObj); // Altering null moving actor's animation script

	if (fn == AR_POPREEL) {
		film = pActor->pushedfilm;	// Use the saved film
	}
	if (fn == AR_PUSHREEL) {
		// Save the one we're replacing
		pActor->pushedfilm = (pActor->TagReelRunning) ? pActor->lastfilm : 0;
	}

	if (film == 0) {
		if (pActor->TagReelRunning) {
			// Revert to 'normal' actor
			SetMActorWalkReel(pActor, pActor->dirn, pActor->scale, true);
			pActor->TagReelRunning = false;
		}
	} else {
		pActor->lastfilm = film;	// Remember this one

		pfilm = (const FILM *)LockMem(film);
		assert(pfilm != NULL);

		InitStepAnimScript(&pActor->actorAnim, pActor->actorObj, FROM_LE_32(pfilm->reels[0].script), ONE_SECOND / FROM_LE_32(pfilm->frate));
		pActor->scount = 0;

		// If no path, just use first path in the scene
		if (pActor->hCpath != NOPOLY)
			MAsetZPos(pActor, pActor->objy, getPolyZfactor(pActor->hCpath));
		else
			MAsetZPos(pActor, pActor->objy, getPolyZfactor(FirstPathPoly()));

		if (fn == AR_WALKREEL) {
			pActor->TagReelRunning = false;
			pActor->walkReel = true;
		} else {
			pActor->TagReelRunning = true;
			pActor->walkReel = false;

#ifdef DEBUG
			assert(StepAnimScript(&pActor->actorAnim) != ScriptFinished); // Actor reel has finished!
#else
			StepAnimScript(&pActor->actorAnim);	// 04/01/95
#endif
		}
			
		// Hang on, we may not want him yet! 04/01/95
		if (pActor->aHidden)
			MultiSetZPosition(pActor->actorObj, -1);
	}
}

/**
 * Return the actor's direction.
 */
DIRREEL GetMActorDirection(PMACTOR pActor) {
	return pActor->dirn;
}

/**
 * Return the actor's scale.
 */
int GetMActorScale(PMACTOR pActor) {
	return pActor->scale;
}

/**
 * Point actor in specified derection
 */
void SetMActorDirection(PMACTOR pActor, DIRREEL dirn) {
	pActor->dirn = dirn;
}

/**
 * MAmoving
 */
bool MAmoving(PMACTOR pActor) {
	return pActor->bMoving;
}

/**
 * Return an actor's walk ticket.
 */
int GetActorTicket(PMACTOR pActor) {
	return pActor->ticket;
}

/**
 * Get actor to adopt its appropriate standing reel.
 */
void SetMActorStanding(PMACTOR pActor) {
	assert(pActor->actorObj);
	AlterMActor(pActor, pActor->StandReels[pActor->scale-1][pActor->dirn], AR_NORMAL);
}

/**
 * Get actor to adopt its appropriate walking reel.
 */
void SetMActorWalkReel(PMACTOR pActor, DIRREEL reel, int scale, bool force) {
	SCNHANDLE	whichReel;
	const FILM *pfilm;

	// Kill off any play that may be going on for this actor
	// and restore the real actor
	storeActorReel(pActor->actorID, NULL, 0, NULL, 0, 0, 0);
	unhideMActor(pActor);

	// Don't do it if using a special walk reel
	if (pActor->walkReel)
		return;

	if (force || pActor->scale != scale || pActor->dirn != reel) {
		assert(reel >= 0 && reel <= 3 && scale > 0 && scale <= TOTAL_SCALES); // out of range scale or reel

		// If scale change and both are regular scales
		// and there's a scaling reel in the right direction
		if (pActor->scale != scale
				&& scale <= NUM_MAINSCALES && pActor->scale <= NUM_MAINSCALES
				&& (whichReel = ScalingReel(pActor->actorID, pActor->scale, scale, reel)) != 0) {
//			error("Cripes!");
			;	// Use what is now in 'whichReel'
		} else {
			whichReel = pActor->WalkReels[scale-1][reel];
			assert(whichReel); // no reel
		}

		pfilm = (const FILM *)LockMem(whichReel);
		assert(pfilm != NULL); // no film

		InitStepAnimScript(&pActor->actorAnim, pActor->actorObj, FROM_LE_32(pfilm->reels[0].script), 1);

		// Synchronised walking reels
		SkipFrames(&pActor->actorAnim, pActor->scount);

		pActor->scale = scale;
		pActor->dirn = reel;
	}
}

/**
 * Sort some stuff out at actor start-up time.
 */
static void InitialPathChecks(PMACTOR pActor, int xpos, int ypos) {
	HPOLYGON hPath;
	int	node;
	int	z;

	pActor->objx = xpos;
	pActor->objy = ypos;

	/*--------------------------------------
	| If Actor is in a follow nodes path,	|
	| position it at the nearest node.	|
	 --------------------------------------*/
	hPath = InPolygon(xpos, ypos, PATH);

	if (hPath != NOPOLY) {
		pActor->hCpath = hPath;
		if (PolySubtype(hPath) == NODE) {
			node = NearestNodeWithin(hPath, xpos, ypos);
			getNpathNode(hPath, node, &pActor->objx, &pActor->objy);
			pActor->hFnpath = hPath;
			pActor->line = node;
			pActor->npstatus = GOING_UP;
		}

		z = GetScale(hPath, pActor->objy);
	} else {
		pActor->bNoPath = true;

		z = GetScale(FirstPathPoly(), pActor->objy);
	}
	SetMActorWalkReel(pActor, FORWARD, z, false);	
}

/**
 * Clear everything out at actor start-up time.
 */
static void InitMActor(PMACTOR pActor) {
	
	pActor->objx = pActor->objy = 0;
	pActor->targetX = pActor->targetY = -1;
	pActor->ItargetX = pActor->ItargetY = -1;
	pActor->hIpath = NOPOLY;
	pActor->UtargetX = pActor->UtargetY = -1;
	pActor->hUpath = NOPOLY;
	pActor->hCpath = NOPOLY;

	pActor->over = false;
	pActor->InDifficulty = NO_PROB;

	pActor->hFnpath = NOPOLY;
	pActor->npstatus = NOT_IN;
	pActor->line = 0;

	pActor->Tline = 0;

	pActor->TagReelRunning = false;

	if (pActor->dirn != FORWARD || pActor->dirn != AWAY
			|| pActor->dirn != LEFTREEL || pActor->dirn != RIGHTREEL)
		pActor->dirn = FORWARD;

	if (pActor->scale < 0 || pActor->scale > TOTAL_SCALES)
		pActor->scale = 1;

	pActor->scount = 0;

	pActor->fromx = pActor->fromy = 0;

	pActor->bMoving = false;
	pActor->bNoPath = false;
	pActor->bIgPath = false;
	pActor->walkReel = false;

	pActor->actorObj = NULL;

	pActor->lastfilm = 0;
	pActor->pushedfilm = 0;

	pActor->InEffect = false;
	pActor->aHidden = false;	// 20/2/95
}

static void MActorProcessHelper(int X, int Y, int id, PMACTOR pActor) {
	const FILM *pfilm;
	const MULTI_INIT *pmi;
	const FRAME *pFrame;
	PIMAGE		pim;


	assert(BackPal()); // Can't start actor without a background palette
	assert(pActor->WalkReels[0][FORWARD]); // Starting actor process without walk reels

	InitMActor(pActor);
	InitialPathChecks(pActor, X, Y);

	pfilm = (const FILM *)LockMem(pActor->WalkReels[0][FORWARD]);
	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pfilm->reels[0].mobj));

//---
	pFrame = (const FRAME *)LockMem(FROM_LE_32(pmi->hMulFrame));

	// get pointer to image
	pim = (PIMAGE)LockMem(READ_LE_UINT32(pFrame));	// handle to image
	pim->hImgPal = TO_LE_32(BackPal());
//---
	pActor->actorObj = MultiInitObject(pmi);

/**/	assert(pActor->actorID == id);
	pActor->actorID = id;

	// add it to display list
	MultiInsertObject(GetPlayfieldList(FIELD_WORLD), pActor->actorObj);
	storeActorReel(id, NULL, 0, pActor->actorObj, 0, 0, 0);

	InitStepAnimScript(&pActor->actorAnim, pActor->actorObj, FROM_LE_32(pfilm->reels[0].script), ONE_SECOND / FROM_LE_32(pfilm->frate));
	pActor->scount = 0;

	MultiSetAniXY(pActor->actorObj, pActor->objx, pActor->objy);

	// If no path, just use first path in the scene
	if (pActor->hCpath != NOPOLY)
		MAsetZPos(pActor, pActor->objy, getPolyZfactor(pActor->hCpath));
	else
		MAsetZPos(pActor, pActor->objy, getPolyZfactor(FirstPathPoly()));

	// Make him the right size
	SetMActorStanding(pActor);

//**** if added 18/11/94, am
	if (X != MAGICX && Y != MAGICY) {
		hideMActor(pActor, 0);		// Allows a play to come in before this appears
		pActor->aHidden = false;	// ...but don't stay hidden
	}

	pActor->MActorState = NORM_MACTOR;
}

/**
 * Moving actor process - 1 per moving actor in current scene.
 */
void MActorProcess(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	PMACTOR pActor = *(PMACTOR *)ProcessGetParamsSelf();

	CORO_BEGIN_CODE(_ctx);
	
	while (1) {
		if (pActor->TagReelRunning) {
 			if (!pActor->aHidden)
#ifdef DEBUG
			assert(StepAnimScript(&pActor->actorAnim) != ScriptFinished); // Actor reel has finished!
#else
			StepAnimScript(&pActor->actorAnim);
#endif
		} else
			DoMoveActor(pActor);

		CORO_SLEEP(1);		// allow rescheduling

	}

	CORO_END_CODE;
}

void MActorProcessCreate(int X, int Y, int id, PMACTOR pActor) {
	MActorProcessHelper(X, Y, id, pActor);
	pActor->pProc = ProcessCreate(PID_MACTOR, MActorProcess, &pActor, sizeof(PMACTOR));
}


/**
 * Check for moving actor collision.
 */
PMACTOR InMActorBlock(PMACTOR pActor, int x, int y) {
	int	caX;		// Calling actor's pos'n
	int	caL, caR;	// Calling actor's left and right
	int	taX, taY;	// Test actor's pos'n
	int	taL, taR;	// Test actor's left and right

	caX = pActor->objx;
	if (pActor->hFnpath != NOPOLY || bNoBlocking)
		return NULL;

	caL = GetMActorLeft(pActor) + x - caX;
	caR = GetMActorRight(pActor) + x - caX;

	for (int i = 0; i < MAX_MOVERS; i++) {
		if (pActor == &Movers[i] || Movers[i].MActorState == NO_MACTOR)
			continue;

		// At around the same height?
		GetMActorPosition(&Movers[i], &taX, &taY);
		if (Movers[i].hFnpath != NOPOLY)
			continue;

		if (ABS(y - taY) > 2)	// 2 was 8
			continue;

		// To the left?
		taL = GetMActorLeft(&Movers[i]);
		if (caR <= taL)
			continue;

		// To the right?
		taR = GetMActorRight(&Movers[i]);
		if (caL >= taR)
			continue;

		return &Movers[i];
	}
	return NULL;
}

/**
 * Copies key information for savescn.c to store away.
 */
void SaveMovers(SAVED_MOVER *sMoverInfo) {
	for (int i = 0; i < MAX_MOVERS; i++) {
		sMoverInfo[i].MActorState= Movers[i].MActorState;
		sMoverInfo[i].actorID	= Movers[i].actorID;
		sMoverInfo[i].objx	= Movers[i].objx;
		sMoverInfo[i].objy	= Movers[i].objy;
		sMoverInfo[i].lastfilm	= Movers[i].lastfilm;

		memcpy(sMoverInfo[i].WalkReels, Movers[i].WalkReels, TOTAL_SCALES*4*sizeof(SCNHANDLE));
		memcpy(sMoverInfo[i].StandReels, Movers[i].StandReels, TOTAL_SCALES*4*sizeof(SCNHANDLE));
		memcpy(sMoverInfo[i].TalkReels, Movers[i].TalkReels, TOTAL_SCALES*4*sizeof(SCNHANDLE));
	}
}

void RestoreAuxScales(SAVED_MOVER *sMoverInfo) {
	for (int i = 0; i < MAX_MOVERS; i++) {
		memcpy(Movers[i].WalkReels, sMoverInfo[i].WalkReels, TOTAL_SCALES*4*sizeof(SCNHANDLE));
		memcpy(Movers[i].StandReels, sMoverInfo[i].StandReels, TOTAL_SCALES*4*sizeof(SCNHANDLE));
		memcpy(Movers[i].TalkReels, sMoverInfo[i].TalkReels, TOTAL_SCALES*4*sizeof(SCNHANDLE));
	}
}

} // end of namespace Tinsel
