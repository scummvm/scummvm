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
 * Cursor and cursor trails.
 */

#include "tinsel/cursor.h"

#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
#include "tinsel/dw.h"
#include "tinsel/events.h"		// For EventsManager class
#include "tinsel/film.h"
#include "tinsel/graphics.h"
#include "tinsel/handle.h"
#include "tinsel/dialogs.h"
#include "tinsel/multiobj.h"	// multi-part object defintions etc.
#include "tinsel/object.h"
#include "tinsel/pid.h"
#include "tinsel/play.h"
#include "tinsel/sched.h"
#include "tinsel/sysvar.h"
#include "tinsel/text.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinlib.h"		// resetidletime()
#include "tinsel/tinsel.h"		// For engine access


namespace Tinsel {

//----------------- LOCAL DEFINES --------------------

#define ITERATION_BASE		FRAC_ONE
#define ITER_ACCELERATION	(10L << (FRAC_BITS - 4))


//----------------- LOCAL GLOBAL DATA --------------------

// FIXME: Avoid non-const global vars

static OBJECT *McurObj = NULL;		// Main cursor object
static OBJECT *AcurObj = NULL;		// Auxiliary cursor object

static ANIM McurAnim = {0,0,0,0,0};		// Main cursor animation structure
static ANIM AcurAnim = {0,0,0,0,0};		// Auxiliary cursor animation structure

static bool bHiddenCursor = false;		// Set when cursor is hidden
static bool bTempNoTrailers = false;	// Set when cursor trails are hidden
static bool bTempHide = false;			// Set when cursor is hidden

static bool bFrozenCursor = false;	// Set when cursor position is frozen

static frac_t IterationSize = 0;

static SCNHANDLE hCursorFilm = 0;	// Handle to cursor reel data

static int numTrails = 0;
static int nextTrail = 0;

static bool bWhoa = false;		// Set by DropCursor() at the end of a scene
				// - causes cursor processes to do nothing
				// Reset when main cursor has re-initialized

static uint16 restart = 0;	// When main cursor has been bWhoa-ed, it waits
							// for this to be set to 0x8000.
							// Main cursor sets all the bits after a re-start
							// - each cursor trail examines it's own bit
							// to trigger a trail restart.

static short ACoX = 0, ACoY = 0;	// Auxillary cursor image's animation offsets



#define MAX_TRAILERS	10

static struct {

	ANIM	trailAnim;	// Animation structure
	OBJECT *trailObj;	// This trailer's object

} ntrailData [MAX_TRAILERS];

static int lastCursorX = 0, lastCursorY = 0;


//----------------- FORWARD REFERENCES --------------------

static void DoCursorMove();

/**
 * Initialize and insert a cursor trail object, set its Z-pos, and hide
 * it. Also initialize its animation script.
 */
static void InitCurTrailObj(int i, int x, int y) {
	const FREEL *pfr;		// pointer to reel
	IMAGE *pim;		// pointer to image
	const MULTI_INIT *pmi;		// MULTI_INIT structure

	const FILM *pfilm;

	if (!numTrails)
		return;

	// Get rid of old object
	if (ntrailData[i].trailObj != NULL)
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), ntrailData[i].trailObj);

	pim = GetImageFromFilm(hCursorFilm, i+1, &pfr, &pmi, &pfilm);// Get pointer to image
	assert(BgPal()); // No background palette
	pim->hImgPal = TO_LE_32(BgPal());

	// Initialize and insert the object, set its Z-pos, and hide it
	ntrailData[i].trailObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), ntrailData[i].trailObj);
	MultiSetZPosition(ntrailData[i].trailObj, Z_CURSORTRAIL);
	MultiSetAniXY(ntrailData[i].trailObj, x, y);

	// Initialize the animation script
	InitStepAnimScript(&ntrailData[i].trailAnim, ntrailData[i].trailObj, FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pfilm->frate));
	StepAnimScript(&ntrailData[i].trailAnim);
}

/**
 * Get the cursor position from the mouse driver.
 */
static bool GetDriverPosition(int *x, int *y) {
	Common::Point ptMouse = _vm->getMousePosition();
	*x = ptMouse.x;
	*y = ptMouse.y;

	return(*x >= 0 && *x <= SCREEN_WIDTH - 1 &&
		*y >= 0 && *y <= SCREEN_HEIGHT - 1);
}

/**
 * Move the cursor relative to current position.
 */
void AdjustCursorXY(int deltaX, int deltaY) {
	int x, y;

	if (deltaX || deltaY) {
		if (GetDriverPosition(&x, &y))
			_vm->setMousePosition(Common::Point(x + deltaX, y + deltaY));
	}
	DoCursorMove();
}

/**
 * Move the cursor to an absolute position.
 */
void SetCursorXY(int newx, int newy) {
	int	x, y;
	int	Loffset, Toffset;	// Screen offset

	PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	newx -= Loffset;
	newy -= Toffset;

	if (GetDriverPosition(&x, &y))
		_vm->setMousePosition(Common::Point(newx, newy));
	DoCursorMove();
}

/**
 * Move the cursor to a screen position.
 */
void SetCursorScreenXY(int newx, int newy) {
	int	x, y;

	if (GetDriverPosition(&x, &y))
		_vm->setMousePosition(Common::Point(newx, newy));
	DoCursorMove();
}

/**
 * Called by the world and his brother.
 * Returns the cursor's animation position in (x,y).
 * Returns false if there is no cursor object.
 */
bool GetCursorXYNoWait(int *x, int *y, bool absolute) {
	if (McurObj == NULL) {
		*x = *y = 0;
		return false;
	}

	GetAniPosition(McurObj, x, y);

	if (absolute) {
		int	Loffset, Toffset;	// Screen offset
		PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
		*x += Loffset;
		*y += Toffset;
	}

	return true;
}

/**
 * Called by the world and his brother.
 * Returns the cursor's animation position.
 * If called while there is no cursor object, the calling process ends
 * up waiting until there is.
 */
void GetCursorXY(int *x, int *y, bool absolute) {
	//while (McurObj == NULL)
	//	ProcessSleepSelf();
	assert(McurObj);
	GetCursorXYNoWait(x, y, absolute);
}

/**
 * Re-initialize the main cursor to use the main cursor reel.
 * Called from TINLIB.C to restore cursor after hiding it.
 * Called from INVENTRY.C to restore cursor after customising it.
 */
void RestoreMainCursor() {
	const FILM *pfilm;

	if (McurObj != NULL) {
		pfilm = (const FILM *)LockMem(hCursorFilm);

		InitStepAnimScript(&McurAnim, McurObj, FROM_LE_32(pfilm->reels->script), ONE_SECOND / FROM_LE_32(pfilm->frate));
		StepAnimScript(&McurAnim);
	}
	bHiddenCursor = false;
	bFrozenCursor = false;
}

/**
 * Called from INVENTRY.C to customise the main cursor.
 */
void SetTempCursor(SCNHANDLE pScript) {
	if (McurObj != NULL)
		InitStepAnimScript(&McurAnim, McurObj, pScript, 2);
}

/**
 * Hide the cursor.
 */
void DwHideCursor() {
	int i;

	bHiddenCursor = true;

	if (McurObj)
		MultiHideObject(McurObj);
	if (AcurObj)
		MultiHideObject(AcurObj);

	for (i = 0; i < numTrails; i++) {
		if (ntrailData[i].trailObj != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), ntrailData[i].trailObj);
			ntrailData[i].trailObj = NULL;
		}
	}
}

/**
 * Unhide the cursor.
 */
void UnHideCursor() {
	bHiddenCursor = false;
}

/**
 * Freeze the cursor.
 */
void FreezeCursor() {
	bFrozenCursor = true;
}

/**
 * Freeze the cursor, or not.
 */
void DoFreezeCursor(bool bFreeze) {
	bFrozenCursor = bFreeze;
}

/**
 * HideCursorTrails
 */
void HideCursorTrails() {
	int i;

	bTempNoTrailers = true;

	for (i = 0; i < numTrails; i++)	{
		if (ntrailData[i].trailObj != NULL) {
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), ntrailData[i].trailObj);
			ntrailData[i].trailObj = NULL;
		}
	}
}

/**
 * UnHideCursorTrails
 */
void UnHideCursorTrails() {
	bTempNoTrailers = false;
}

/**
 * Get pointer to image from a film reel. And the rest.
 */
IMAGE *GetImageFromReel(const FREEL *pfr, const MULTI_INIT **ppmi) {
	const MULTI_INIT *pmi;
	const FRAME *pFrame;

	pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pfr->mobj));
	if (ppmi)
		*ppmi = pmi;

	pFrame = (const FRAME *)LockMem(FROM_LE_32(pmi->hMulFrame));

	// get pointer to image
	return (IMAGE *)LockMem(READ_LE_UINT32(pFrame));
}

/**
 * Get pointer to image from a film. And the rest.
 */
IMAGE *GetImageFromFilm(SCNHANDLE hFilm, int reel, const FREEL **ppfr, const MULTI_INIT **ppmi, const FILM **ppfilm) {
	const FILM *pfilm;
	const FREEL *pfr;

	pfilm = (const FILM *)LockMem(hFilm);
	if (ppfilm)
		*ppfilm = pfilm;

	pfr = &pfilm->reels[reel];
	if (ppfr)
		*ppfr = pfr;

	return GetImageFromReel(pfr, ppmi);
}

/**
 * Delete auxillary cursor. Restore animation offsets in the image.
 */
void DelAuxCursor() {
	if (AcurObj != NULL) {
		MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), AcurObj);
		AcurObj = NULL;
	}
}

/**
 * Set auxillary cursor.
 * Save animation offsets from the image if required.
 */
void SetAuxCursor(SCNHANDLE hFilm) {
	IMAGE *pim;		// Pointer to auxillary cursor's image
	const FREEL *pfr;
	const MULTI_INIT *pmi;
	const FILM *pfilm;
	int	x, y;		// Cursor position

	DelAuxCursor();		// Get rid of previous

	GetCursorXY(&x, &y, false);	// Note: also waits for cursor to appear

	pim = GetImageFromFilm(hFilm, 0, &pfr, &pmi, &pfilm);// Get pointer to image
	assert(BgPal()); // no background palette
	pim->hImgPal = TO_LE_32(BgPal());			// Poke in the background palette

	ACoX = (short)(FROM_LE_16(pim->imgWidth)/2 - ((int16) FROM_LE_16(pim->anioffX)));
	ACoY = (short)((FROM_LE_16(pim->imgHeight) & ~C16_FLAG_MASK)/2 -
		((int16) FROM_LE_16(pim->anioffY)));

	// Initialize and insert the auxillary cursor object
	AcurObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), AcurObj);

	// Initialize the animation and set its position
	InitStepAnimScript(&AcurAnim, AcurObj, FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pfilm->frate));
	MultiSetAniXY(AcurObj, x - ACoX, y - ACoY);
	MultiSetZPosition(AcurObj, Z_ACURSOR);

	if (bHiddenCursor)
		MultiHideObject(AcurObj);
}

/**
 * MoveCursor
 */
static void DoCursorMove() {
	int	startX, startY;
	Common::Point ptMouse;
	frac_t newX, newY;
	unsigned dir;

	// get cursors start animation position
	GetCursorXYNoWait(&startX, &startY, false);

	// get mouse drivers current position
	ptMouse = _vm->getMousePosition();

	// convert to fixed point
	newX = intToFrac(ptMouse.x);
	newY = intToFrac(ptMouse.y);

	// modify mouse driver position depending on cursor keys
	dir = _vm->getKeyDirection();
	if (dir != 0) {
		if (dir & MSK_LEFT)
			newX -= IterationSize;

		if (dir & MSK_RIGHT)
			newX += IterationSize;

		if (dir & MSK_UP)
			newY -= IterationSize;

		if (dir & MSK_DOWN)
			newY += IterationSize;

		IterationSize += ITER_ACCELERATION;

		// set new mouse driver position
		_vm->setMousePosition(Common::Point(fracToInt(newX), fracToInt(newY)));
	} else

		IterationSize = ITERATION_BASE;

	// get new mouse driver position - could have been modified
	ptMouse = _vm->getMousePosition();

	if (lastCursorX != ptMouse.x || lastCursorY != ptMouse.y) {
		resetUserEventTime();

		if (!bTempNoTrailers && !bHiddenCursor) {
			InitCurTrailObj(nextTrail++, lastCursorX, lastCursorY);
			if (nextTrail == numTrails)
				nextTrail = 0;
		}
	}

	// adjust cursor to new mouse position
	if (McurObj)
		MultiSetAniXY(McurObj, ptMouse.x, ptMouse.y);
	if (AcurObj != NULL)
		MultiSetAniXY(AcurObj, ptMouse.x - ACoX, ptMouse.y - ACoY);

	if (InventoryActive() && McurObj) {
		// Notify the inventory
		Xmovement(ptMouse.x - startX);
		Ymovement(ptMouse.y - startY);
	}

	lastCursorX = ptMouse.x;
	lastCursorY = ptMouse.y;
}

/**
 * Initialize cursor object.
 */
static void InitCurObj() {
	const FILM *pFilm;
	const FREEL *pfr;
	const MULTI_INIT *pmi;
	IMAGE *pim;

	if (TinselV2) {
		pFilm = (const FILM *)LockMem(hCursorFilm);
		pfr = (const FREEL *)&pFilm->reels[0];
		pmi = (MULTI_INIT *)LockMem(FROM_LE_32(pfr->mobj));

		PokeInPalette(pmi);
	} else {
		assert(BgPal()); // no background palette

		pim = GetImageFromFilm(hCursorFilm, 0, &pfr, &pmi, &pFilm);// Get pointer to image
		pim->hImgPal = TO_LE_32(BgPal());

		AcurObj = NULL;		// No auxillary cursor
	}

	McurObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_STATUS), McurObj);

	InitStepAnimScript(&McurAnim, McurObj, FROM_LE_32(pfr->script), ONE_SECOND / FROM_LE_32(pFilm->frate));
}

/**
 * Initialize the cursor position.
 */
static void InitCurPos() {
	Common::Point ptMouse = _vm->getMousePosition();
	lastCursorX = ptMouse.x;
	lastCursorY = ptMouse.y;

	MultiSetZPosition(McurObj, Z_CURSOR);
	DoCursorMove();
	MultiHideObject(McurObj);

	IterationSize = ITERATION_BASE;
}

/**
 * CursorStoppedCheck
 */
static void CursorStoppedCheck(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If scene is closing down
	if (bWhoa) {
		// ...wait for next scene start-up
		while (restart != 0x8000)
			CORO_SLEEP(1);

		// Re-initialize
		InitCurObj();
		InitCurPos();
		InventoryIconCursor(false);	// May be holding something

		// Re-start the cursor trails
		restart = (uint16)-1;		// set all bits
		bWhoa = false;
	}
	CORO_END_CODE;
}

/**
 * The main cursor process.
 */
void CursorProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (!hCursorFilm || !BgPal())
		CORO_SLEEP(1);

	InitCurObj();
	InitCurPos();
	InventoryIconCursor(false);		// May be holding something

	bWhoa = false;
	restart = 0;

	while (1) {
		// allow rescheduling
		CORO_SLEEP(1);

		// Stop/start between scenes
		CORO_INVOKE_0(CursorStoppedCheck);

		// Step the animation script(s)
		StepAnimScript(&McurAnim);
		if (AcurObj != NULL)
			StepAnimScript(&AcurAnim);
		for (int i = 0; i < numTrails; i++) {
			if (ntrailData[i].trailObj != NULL) {
				if (StepAnimScript(&ntrailData[i].trailAnim) == ScriptFinished) {
					MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), ntrailData[i].trailObj);
					ntrailData[i].trailObj = NULL;
				}
			}
		}

		// Move the cursor as appropriate
		if (!bFrozenCursor)
			DoCursorMove();

		// If the cursor should be hidden...
		if (bHiddenCursor || bTempHide) {
			// ...hide the cursor object(s)
			MultiHideObject(McurObj);
			if (AcurObj)
				MultiHideObject(AcurObj);

			for (int i = 0; i < numTrails; i++) {
				if (ntrailData[i].trailObj != NULL)
					MultiHideObject(ntrailData[i].trailObj);
			}

			// Wait 'til cursor is again required.
			while (bHiddenCursor) {
				CORO_SLEEP(1);

				// Stop/start between scenes
				CORO_INVOKE_0(CursorStoppedCheck);
			}
		}
	}
	CORO_END_CODE;
}

/**
 * Called from dec_cursor() Glitter function.
 * Register the handle to cursor reel data.
 */
void DwInitCursor(SCNHANDLE bfilm) {
	const FILM *pfilm;

	hCursorFilm = bfilm;

	pfilm = (const FILM *)LockMem(hCursorFilm);
	numTrails = FROM_LE_32(pfilm->numreels) - 1;

	assert(numTrails <= MAX_TRAILERS);
}

/**
 * DropCursor is called when a scene is closing down.
 */
void DropCursor() {
	if (TinselV2) {
		if (AcurObj)
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), AcurObj);
		if (McurObj)
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), McurObj);

		restart = 0;
	}

	AcurObj = NULL;		// No auxillary cursor
	McurObj = NULL;		// No cursor object (imminently deleted elsewhere)
	bHiddenCursor = false;	// Not hidden in next scene
	bTempNoTrailers = false;	// Trailers not hidden in next scene
	bWhoa = true;		// Suspend cursor processes

	for (int i = 0; i < numTrails; i++) {
		if (ntrailData[i].trailObj != NULL)		{
			MultiDeleteObject(GetPlayfieldList(FIELD_STATUS), ntrailData[i].trailObj);
			ntrailData[i].trailObj = NULL;
		}
	}
}

/**
 * RestartCursor is called when a new scene is starting up.
 */
void RestartCursor() {
	restart = 0x8000;	// Get the main cursor to re-initialize
}

/**
 * Called when restarting the game, ensures correct re-start with NULL
 * pointers etc.
 */
void RebootCursor() {
	McurObj = AcurObj = NULL;
	for (int i = 0; i < MAX_TRAILERS; i++)
		ntrailData[i].trailObj = NULL;

	bHiddenCursor = bTempNoTrailers = bFrozenCursor = false;

	hCursorFilm = 0;

	bWhoa = false;
	restart = 0;
}

void StartCursorFollowed() {
	DelAuxCursor();

	if (!SysVar(SV_ENABLEPRINTCURSOR))
		bTempHide = true;
}

void EndCursorFollowed() {
	InventoryIconCursor(false);	// May be holding something
	bTempHide = false;
}

bool isCursorShown() {
	return !(bTempHide || bHiddenCursor);
}

} // End of namespace Tinsel
