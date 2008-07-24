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
 * Plays films within a scene, takes into account the actor in each 'column'.								|
 */

#include "tinsel/actors.h"
#include "tinsel/background.h"
#include "tinsel/dw.h"
#include "tinsel/film.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"
#include "tinsel/pid.h"
#include "tinsel/polygons.h"
#include "tinsel/rince.h"
#include "tinsel/sched.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"	// stand()

namespace Tinsel {

/**
 * Poke the background palette into an image.
 */
static void PokeInPalette(SCNHANDLE hMulFrame) {
	const FRAME *pFrame;		// Pointer to frame
	PIMAGE	pim;		// Pointer to image

	// Could be an empty column
	if (hMulFrame) {
		pFrame = (const FRAME *)LockMem(hMulFrame);

		// get pointer to image
		pim = (PIMAGE)LockMem(READ_LE_UINT32(pFrame));	// handle to image

		pim->hImgPal = TO_LE_32(BackPal());
	}
}


int32 NoNameFunc(int actorID, bool bNewMover) {
	PMACTOR	pActor;
	int32	retval;

	pActor = GetMover(actorID);

	if (pActor != NULL && !bNewMover) {
		// If no path, just use first path in the scene
		if (pActor->hCpath == NOPOLY)
			retval = getPolyZfactor(FirstPathPoly());
		else
			retval = getPolyZfactor(pActor->hCpath);
	} else {
		switch (actorMaskType(actorID)) {
		case ACT_DEFAULT:
			retval = 0;
			break;
		case ACT_MASK:
			retval = 0;
			break;
		case ACT_ALWAYS:
			retval = 10;
			break;
		default:
			retval = actorMaskType(actorID);
			break;
		}
	}

	return retval;
}

struct PPINIT {
	SCNHANDLE hFilm;	// The 'film'
	short	x;			// } Co-ordinates from the play()
	short	y;			// } - set to (-1, -1) if none.
	short	z;			// normally 0, set if from restore
	short	speed;		// Film speed
	short 	actorid;	// Set if called from an actor code block
	bool	splay;		// Set if called from splay()
	bool	bTop;		// Set if called from topplay()
	short	sf;			// SlowFactor - only used for moving actors
	short	column;		// Column number, first column = 0

	bool	escOn;
	int	myescEvent;
};


/**
 * - Don't bother if this reel is already playing for this actor.
 * - If explicit co-ordinates, use these, If embedded co-ordinates,
 * leave alone, otherwise use actor's current position.
 * - Moving actors get hidden during this play, other actors get
 * _ctx->replaced by this play.
 * - Column 0 of a film gets its appropriate Z-position, slave columns
 * get slightly bigger Z-positions, in column order.
 * - Play proceeds until the script finishes, another reel starts up for
 * this actor, or the actor gets killed.
 * - If called from an splay(), moving actor's co-ordinates are updated
 * after the play, any walk still in progress will go on from there.
 */
void PlayReel(CORO_PARAM, const PPINIT *ppi) {
	CORO_BEGIN_CONTEXT;
		OBJECT	*pPlayObj;	// Object
		ANIM	thisAnim;	// Animation structure
	
		bool	mActor;		// Gets set if this is a moving actor
		bool	lifeNoMatter;
		bool	replaced;
	
		const FREEL *pfreel;	// The 'column' to play
		int		stepCount;
		int		frameCount;
		int		reelActor;
	CORO_END_CONTEXT(_ctx);

	static int	firstColZ = 0;	// Z-position of column zero
	static int32	fColZfactor = 0;	// Z-factor of column zero's actor

	CORO_BEGIN_CODE(_ctx);

	const MULTI_INIT *pmi;		// MULTI_INIT structure
	PMACTOR	pActor;
	bool	bNewMover;	// Gets set if a moving actor that isn't in scene yet

	const FILM *pfilm;

	_ctx->lifeNoMatter = false;
	_ctx->replaced = false;
	pActor = NULL;
	bNewMover = false;

	pfilm = (const FILM *)LockMem(ppi->hFilm);
	_ctx->pfreel = &pfilm->reels[ppi->column];

	// Get the MULTI_INIT structure
	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(_ctx->pfreel->mobj));

	// Save actor's ID
	_ctx->reelActor = (int32)FROM_LE_32(pmi->mulID);

	/**** New (experimental? bit 5/1/95 ****/
	if (!actorAlive(_ctx->reelActor))
		return;
	/**** Delete a bit down there if this stays ****/

	updateActorEsc(_ctx->reelActor, ppi->escOn, ppi->myescEvent);

	// To handle the play()-talk(), talk()-play(), talk()-talk() and play()-play() scenarios
	if (ppi->hFilm != getActorLatestFilm(_ctx->reelActor)) {
		// This in not the last film scheduled for this actor

		// It may be the last non-talk film though
		if (isActorTalking(_ctx->reelActor))
			setActorPlayFilm(_ctx->reelActor, ppi->hFilm);	// Revert to this film after talk

		return;
	}
	if (isActorTalking(_ctx->reelActor)) {
		// Note: will delete this and there'll be no need to store the talk film!
		if (ppi->hFilm != getActorTalkFilm(_ctx->reelActor)) {
			setActorPlayFilm(_ctx->reelActor, ppi->hFilm);	// Revert to this film after talk
			return;
		}
	} else {
		setActorPlayFilm(_ctx->reelActor, ppi->hFilm);
	}

	// If this reel is already playing for this actor, just forget it.
	if (actorReel(_ctx->reelActor) == _ctx->pfreel)
		return;

	// Poke in the background palette
	PokeInPalette(FROM_LE_32(pmi->hMulFrame));

	// Set up and insert the multi-object
	_ctx->pPlayObj = MultiInitObject(pmi);
	if (!ppi->bTop)
		MultiInsertObject(GetPlayfieldList(FIELD_WORLD), _ctx->pPlayObj);
	else
		MultiInsertObject(GetPlayfieldList(FIELD_STATUS), _ctx->pPlayObj);

	// If co-ordinates are specified, use specified.
	// Otherwise, use actor's position if there are not embedded co-ords.
	// Add this first test for nth columns with offsets
	// in plays with (x,y)
	int tmpX, tmpY;
	tmpX = ppi->x;
	tmpY = ppi->y;
	if (ppi->column != 0 && (pmi->mulX || pmi->mulY)) {
	} else if (tmpX != -1 || tmpY != -1) {
		MultiSetAniXY(_ctx->pPlayObj, tmpX, tmpY);
	} else if (!pmi->mulX && !pmi->mulY) {
		GetActorPos(_ctx->reelActor, &tmpX, &tmpY);
		MultiSetAniXY(_ctx->pPlayObj, tmpX, tmpY);
	}

	// If it's a moving actor, this hides the moving actor
	// used to do this only if (actorid == 0) - I don't know why
	_ctx->mActor = HideMovingActor(_ctx->reelActor, ppi->sf);

	// If it's a moving actor, get its MACTOR structure.
	// If it isn't in the scene yet, get its task running - using
	// stand() - to prevent a glitch at the end of the play.
	if (_ctx->mActor) {
		pActor = GetMover(_ctx->reelActor);
		if (getMActorState(pActor) == NO_MACTOR) {
			stand(_ctx->reelActor, MAGICX, MAGICY, 0);
			bNewMover = true;
		}
	}

	// Register the fact that we're playing this for this actor
	storeActorReel(_ctx->reelActor, _ctx->pfreel, ppi->hFilm, _ctx->pPlayObj, ppi->column, tmpX, tmpY);

	/**** Will get rid of this if the above is kept ****/
	// We may be temporarily resuscitating a dead actor
	if (ppi->actorid == 0 && !actorAlive(_ctx->reelActor))
		_ctx->lifeNoMatter = true;

	InitStepAnimScript(&_ctx->thisAnim, _ctx->pPlayObj,  FROM_LE_32(_ctx->pfreel->script), ppi->speed);

	// If first column, set Z position as per
	// Otherwise, column 0's + column number
	// N.B. It HAS been ensured that the first column gets here first
	if (ppi->z != 0) {
		MultiSetZPosition(_ctx->pPlayObj, ppi->z);
		storeActorZpos(_ctx->reelActor, ppi->z);
	} else if (ppi->bTop) {
		if (ppi->column == 0) {
			firstColZ = Z_TOPPLAY + actorMaskType(_ctx->reelActor);
			MultiSetZPosition(_ctx->pPlayObj, firstColZ);
			storeActorZpos(_ctx->reelActor, firstColZ);
		} else {
			MultiSetZPosition(_ctx->pPlayObj, firstColZ + ppi->column);
			storeActorZpos(_ctx->reelActor, firstColZ + ppi->column);
		}
	} else if (ppi->column == 0) {
		if (_ctx->mActor && !bNewMover) {
			// If no path, just use first path in the scene
			if (pActor->hCpath == NOPOLY)
				fColZfactor = getPolyZfactor(FirstPathPoly());
			else
				fColZfactor = getPolyZfactor(pActor->hCpath);
			firstColZ = AsetZPos(_ctx->pPlayObj, MultiLowest(_ctx->pPlayObj), fColZfactor);
		} else {
			switch (actorMaskType(_ctx->reelActor)) {
			case ACT_DEFAULT:
				fColZfactor = 0;
				firstColZ = 2;
				MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				break;
			case ACT_MASK:
				fColZfactor = 0;
				firstColZ = MultiLowest(_ctx->pPlayObj);
				MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				break;
			case ACT_ALWAYS:
				fColZfactor = 10;
				firstColZ = 10000;
				MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				break;
			default:
				fColZfactor = actorMaskType(_ctx->reelActor);
				firstColZ = AsetZPos(_ctx->pPlayObj, MultiLowest(_ctx->pPlayObj), fColZfactor);
				if (firstColZ < 2) {
					// This is an experiment!
					firstColZ = 2;
					MultiSetZPosition(_ctx->pPlayObj, firstColZ);
				}
				break;
			}
		}
		storeActorZpos(_ctx->reelActor, firstColZ);
	} else {
		if (NoNameFunc(_ctx->reelActor, bNewMover) > fColZfactor) {
			fColZfactor = NoNameFunc(_ctx->reelActor, bNewMover);
			firstColZ = fColZfactor << 10;
		}
		MultiSetZPosition(_ctx->pPlayObj, firstColZ + ppi->column);
		storeActorZpos(_ctx->reelActor, firstColZ + ppi->column);
	}

	/*
	 * Play until the script finishes,
	 * another reel starts up for this actor,
	 * or the actor gets killed.
	 */
	_ctx->stepCount = 0;
	_ctx->frameCount = 0;
	do {
		if (_ctx->stepCount++ == 0) {
			_ctx->frameCount++;
			storeActorSteps(_ctx->reelActor, _ctx->frameCount);
		}
		if (_ctx->stepCount == ppi->speed)
			_ctx->stepCount = 0;

		if (StepAnimScript(&_ctx->thisAnim) == ScriptFinished)
			break;

		int x, y;
		GetAniPosition(_ctx->pPlayObj, &x, &y);
		storeActorPos(_ctx->reelActor, x, y);

		CORO_SLEEP(1);

		if (actorReel(_ctx->reelActor) != _ctx->pfreel) {
			_ctx->replaced = true;
			break;
		}

		if (actorEsc(_ctx->reelActor) && actorEev(_ctx->reelActor) != GetEscEvents())
			break;

	} while (_ctx->lifeNoMatter || actorAlive(_ctx->reelActor));

	// Register the fact that we're NOT playing this for this actor
	if (actorReel(_ctx->reelActor) == _ctx->pfreel)
		storeActorReel(_ctx->reelActor, NULL, 0, NULL, 0, 0, 0);

	// Ditch the object
	if (!ppi->bTop)
		MultiDeleteObject(GetPlayfieldList(FIELD_WORLD), _ctx->pPlayObj);
	else
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), _ctx->pPlayObj);

	if (_ctx->mActor) {
		if (!_ctx->replaced)
			unHideMovingActor(_ctx->reelActor);	// Restore moving actor

		// Update it's co-ordinates if this is an splay()
		if (ppi->splay)
			restoreMovement(_ctx->reelActor);
	}
	CORO_END_CODE;
}

/**
 * Run all animations that comprise the play film.
 */
static void playProcess(CORO_PARAM, const void *param) {
	// get the stuff copied to process when it was created
	PPINIT *ppi = (PPINIT *)param;

	PlayReel(coroParam, ppi);
}

// *******************************************************


// To handle the play()-talk(), talk()-play(), talk()-talk() and play()-play() scenarios
void newestFilm(SCNHANDLE film, const FREEL *reel) {
	const MULTI_INIT *pmi;		// MULTI_INIT structure

	// Get the MULTI_INIT structure
	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(reel->mobj));

	setActorLatestFilm((int32)FROM_LE_32(pmi->mulID), film);
}

// *******************************************************

/**
 * Start up a play process for each column in a film.
 *
 * NOTE: The processes are started in reverse order so that the first
 *   column's process kicks in first.
 */
void playFilm(SCNHANDLE film, int x, int y, int actorid, bool splay, int sfact, bool escOn, 
			  int myescEvent, bool bTop) {
	const FILM *pfilm = (const FILM *)LockMem(film);
	PPINIT ppi;

	assert(film != 0); // Trying to play NULL film

	// Now allowed empty films!
	if (pfilm->numreels == 0)
		return;                 // Nothing to do!

	ppi.hFilm = film;
	ppi.x = x;
	ppi.y = y;
	ppi.z = 0;
	ppi.speed = (ONE_SECOND / FROM_LE_32(pfilm->frate));
	ppi.actorid = actorid;
	ppi.splay = splay;
	ppi.bTop = bTop;
	ppi.sf = sfact;
	ppi.escOn = escOn;
	ppi.myescEvent = myescEvent;

	// Start display process for each reel in the film
	for (int i = FROM_LE_32(pfilm->numreels) - 1; i >= 0; i--) {
		newestFilm(film, &pfilm->reels[i]);

		ppi.column = i;
		g_scheduler->createProcess(PID_REEL, playProcess, &ppi, sizeof(ppi));
	}
}

/**
 * Start up a play process for each slave column in a film.
 * Play the first column directly from the parent process.
 */
void playFilmc(CORO_PARAM, SCNHANDLE film, int x, int y, int actorid, bool splay, int sfact, 
			   bool escOn, int myescEvent, bool bTop) {
	CORO_BEGIN_CONTEXT;
		PPINIT ppi;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	assert(film != 0); // Trying to play NULL film
	const FILM *pfilm;
	
	pfilm = (const FILM *)LockMem(film);

	// Now allowed empty films!
	if (pfilm->numreels == 0)
		return;                 //  Already played to completion!

	_ctx->ppi.hFilm = film;
	_ctx->ppi.x = x;
	_ctx->ppi.y = y;
	_ctx->ppi.z = 0;
	_ctx->ppi.speed = (ONE_SECOND / FROM_LE_32(pfilm->frate));
	_ctx->ppi.actorid = actorid;
	_ctx->ppi.splay = splay;
	_ctx->ppi.bTop = bTop;
	_ctx->ppi.sf = sfact;
	_ctx->ppi.escOn = escOn;
	_ctx->ppi.myescEvent = myescEvent;

	// Start display process for each secondary reel in the film
	for (int i = FROM_LE_32(pfilm->numreels) - 1; i > 0; i--) {
		newestFilm(film, &pfilm->reels[i]);

		_ctx->ppi.column = i;
		g_scheduler->createProcess(PID_REEL, playProcess, &_ctx->ppi, sizeof(PPINIT));
	}

	newestFilm(film, &pfilm->reels[0]);

	_ctx->ppi.column = 0;
	CORO_INVOKE_1(PlayReel, &_ctx->ppi);

	CORO_END_CODE;
}

/**
 * Start up a play process for a particular column in a film.
 *
 * NOTE: This is specifically for actors during a restore scene.
 */
void playThisReel(SCNHANDLE film, short reelnum, short z, int x, int y) {
	const FILM *pfilm = (const FILM *)LockMem(film);
	PPINIT ppi;

	ppi.hFilm = film;
	ppi.x = x;
	ppi.y = y;
	ppi.z = z;
	ppi.speed = (ONE_SECOND / FROM_LE_32(pfilm->frate));
	ppi.actorid = 0;
	ppi.splay = false;
	ppi.bTop = false;
	ppi.sf = 0;
	ppi.column = reelnum;

	// FIXME: The PlayReel play loop was previously breaking out, and then deleting objects, when 
	// returning to a scene because escOn and myescEvent were undefined. Need to make sure whether 
	// restored objects should have any particular combination of these two values
	ppi.escOn = false;
	ppi.myescEvent = GetEscEvents();

	assert(pfilm->numreels);

	newestFilm(film, &pfilm->reels[reelnum]);

	// Start display process for the reel
	g_scheduler->createProcess(PID_REEL, playProcess, &ppi, sizeof(ppi));
}

} // end of namespace Tinsel
