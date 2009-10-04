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
 * Plays the background film of a scene.
 */

#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/dw.h"
#include "tinsel/faders.h"
#include "tinsel/film.h"
#include "tinsel/font.h"
#include "tinsel/handle.h"
#include "tinsel/multiobj.h"
#include "tinsel/object.h"
#include "tinsel/pcode.h"		// CONTROL_STARTOFF
#include "tinsel/pid.h"
#include "tinsel/sched.h"
#include "tinsel/timers.h"		// For ONE_SECOND constant
#include "tinsel/tinlib.h"		// For Control()
#include "tinsel/tinsel.h"

#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

#define MAX_BG	10

static SCNHANDLE hBgPal = 0;	// Background's palette
static POBJECT pBG[MAX_BG];
static ANIM	thisAnim[MAX_BG];	// used by BGmainProcess()
static int BGspeed = 0;
static SCNHANDLE hBackground = 0;	// Current scene handle - stored in case of Save_Scene()
static bool bDoFadeIn = false;
static int bgReels;

/**
 * GetBgObject
 */
OBJECT *GetBgObject() {
	return pBG[0];
}

/**
 * BackPal
 */
SCNHANDLE BgPal(void) {
	return hBgPal;
}

/**
 * SetDoFadeIn
*/
void SetDoFadeIn(bool tf) {
	bDoFadeIn = tf;
}

/**
 * Called before scene change.
 */
void DropBackground(void) {
	pBG[0] = NULL;	// No background

	if (!TinselV2)
		hBgPal = 0;	// No background palette
}

/**
 * Return the width of the current background.
 */
int BgWidth(void) {
	assert(pBG[0]);
	return MultiRightmost(pBG[0]) + 1;
}

/**
 * Return the height of the current background.
 */
int BgHeight(void) {
	assert(pBG[0]);
	return MultiLowest(pBG[0]) + 1;
}

/**
 * Run main animation that comprises the scene background.
 */
static void BGmainProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pFilm;
	const FREEL *pReel;
	const MULTI_INIT *pmi;

	// get the stuff copied to process when it was created
	if (pBG[0] == NULL) {
		/*** At start of scene ***/

		if (!TinselV2) {
			pReel = (const FREEL *)param;

			// Get the MULTI_INIT structure
			pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pReel->mobj));

			// Initialise and insert the object, and initialise its script.
			pBG[0] = MultiInitObject(pmi);
			MultiInsertObject(GetPlayfieldList(FIELD_WORLD), pBG[0]);
			InitStepAnimScript(&thisAnim[0], pBG[0], FROM_LE_32(pReel->script), BGspeed);
			bgReels = 1;
		} else {
			/*** At start of scene ***/
			pFilm = (const FILM *)LockMem(hBackground);
			bgReels = FROM_LE_32(pFilm->numreels);

			int i;
			for (i = 0; i < bgReels; i++) {
				// Get the MULTI_INIT structure
				pmi = (PMULTI_INIT) LockMem(FROM_LE_32(pFilm->reels[i].mobj));

				// Initialise and insert the object, and initialise its script.
				pBG[i] = MultiInitObject(pmi);
				MultiInsertObject(GetPlayfieldList(FIELD_WORLD), pBG[i]);
				MultiSetZPosition(pBG[i], 0);
				InitStepAnimScript(&thisAnim[i], pBG[i], FROM_LE_32(pFilm->reels[i].script), BGspeed);

				if (i > 0)
					pBG[i-1]->pSlave = pBG[i];
			}
		}

		if (bDoFadeIn) {
			FadeInFast(NULL);
			bDoFadeIn = false;
		} else if (TinselV2)
			PokeInTagColour();

		for (;;) {
			for (int i = 0; i < bgReels; i++) {
				if (StepAnimScript(&thisAnim[i]) == ScriptFinished)
					error("Background animation has finished");
			}

			CORO_SLEEP(1);
		}
	} else {
		// New background during scene
		if (!TinselV2) {
			pReel = (const FREEL *)param;
			InitStepAnimScript(&thisAnim[0], pBG[0], FROM_LE_32(pReel->script), BGspeed);
			StepAnimScript(&thisAnim[0]);
		} else {
			pFilm = (const FILM *)LockMem(hBackground);
			assert(bgReels == (int32)FROM_LE_32(pFilm->numreels));

			// Just re-initialise the scripts.
			for (int i = 0; i < bgReels; i++) {
				InitStepAnimScript(&thisAnim[i], pBG[i], pFilm->reels[i].script, BGspeed);
				StepAnimScript(&thisAnim[i]);
			}
		}
	}

	CORO_END_CODE;
}

/**
 * Runs secondary reels for a scene background
 */
static void BGotherProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		OBJECT *pObj;
		ANIM anim;
	CORO_END_CONTEXT(_ctx);

	const FREEL *pReel = (const FREEL *)param;
	const MULTI_INIT *pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pReel->mobj));

	CORO_BEGIN_CODE(_ctx);

	// Initialise and insert the object, and initialise its script.
	_ctx->pObj = MultiInitObject(pmi);
	MultiInsertObject(GetPlayfieldList(FIELD_WORLD), _ctx->pObj);

	InitStepAnimScript(&_ctx->anim, pBG[0], FROM_LE_32(pReel->script), BGspeed);

	while (StepAnimScript(&_ctx->anim) != ScriptFinished)
		CORO_SLEEP(1);

	CORO_END_CODE;
}

/**
 * AetBgPal()
 */
void SetBackPal(SCNHANDLE hPal) {
	hBgPal = hPal;

	FettleFontPal(hBgPal);
	CreateTranslucentPalette(hBgPal);
}

void ChangePalette(SCNHANDLE hPal) {
	SwapPalette(FindPalette(hBgPal), hPal);

	SetBackPal(hPal);
}

/**
 * Given the scene background film, extracts the palette handle for
 * everything else's use, then starts a display process for each reel
 * in the film.
 * @param hFilm			Scene background film
 */
void StartupBackground(CORO_PARAM, SCNHANDLE hFilm) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;
	IMAGE *pim;

	hBackground = hFilm;		// Save handle in case of Save_Scene()

	pim = GetImageFromFilm(hFilm, 0, NULL, NULL, &pfilm);

	SetBackPal(FROM_LE_32(pim->hImgPal));

	// Extract the film speed
	BGspeed = ONE_SECOND / FROM_LE_32(pfilm->frate);

	// Start display process for each reel in the film
	g_scheduler->createProcess(PID_REEL, BGmainProcess, &pfilm->reels[0], sizeof(FREEL));

	if (TinselV0) {
		for (uint i = 1; i < FROM_LE_32(pfilm->numreels); ++i)
			g_scheduler->createProcess(PID_REEL, BGotherProcess, &pfilm->reels[i], sizeof(FREEL));
	}

	if (pBG[0] == NULL)
		ControlStartOff();

	if (TinselV2 && (coroParam != nullContext))
		CORO_GIVE_WAY;

	CORO_END_CODE;
}

/**
 * Return the current scene handle.
 */
SCNHANDLE GetBgroundHandle(void) {
	return hBackground;
}

} // End of namespace Tinsel
