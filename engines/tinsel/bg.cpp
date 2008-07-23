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
#include "tinsel/tinlib.h"		// For control()

#include "common/util.h"

namespace Tinsel {

//----------------- LOCAL GLOBAL DATA --------------------

static SCNHANDLE BackPalette = 0;	// Background's palette
static OBJECT *pBG = 0;		// The main picture's object.
static int BGspeed = 0;
static SCNHANDLE BgroundHandle = 0;	// Current scene handle - stored in case of Save_Scene()
static bool DoFadeIn = false;
static ANIM	thisAnim;	// used by BGmainProcess()

/**
 * BackPal
 */
SCNHANDLE BackPal(void) {
	return BackPalette;
}

/**
 * SetDoFadeIn
*/
void SetDoFadeIn(bool tf) {
	DoFadeIn = tf;
}

/**
 * Called before scene change.
 */
void DropBackground(void) {
	pBG = NULL;		// No background
	BackPalette = 0;	// No background palette
}

/**
 * Return the width of the current background.
 */
int BackgroundWidth(void) {
	assert(pBG);
	return MultiRightmost(pBG) + 1;
}

/**
 * Return the height of the current background.
 */
int BackgroundHeight(void) {
	assert(pBG);
	return MultiLowest(pBG) + 1;
}

/**
 * Run main animation that comprises the scene background.
 */
static void BGmainProcess(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FREEL *pfr;
	const MULTI_INIT *pmi;

	// get the stuff copied to process when it was created
	pfr = (const FREEL *)ProcessGetParamsSelf();

	if (pBG == NULL) {
		/*** At start of scene ***/

		// Get the MULTI_INIT structure
		pmi = (const MULTI_INIT *)LockMem(FROM_LE_32(pfr->mobj));

		// Initialise and insert the object, and initialise its script.
		pBG = MultiInitObject(pmi);
		MultiInsertObject(GetPlayfieldList(FIELD_WORLD), pBG);
		InitStepAnimScript(&thisAnim, pBG, FROM_LE_32(pfr->script), BGspeed);

		if (DoFadeIn) {
			FadeInFast(NULL);
			DoFadeIn = false;
		}

		while (StepAnimScript(&thisAnim) != ScriptFinished)
			CORO_SLEEP(1);

		error("Background animation has finished!");
	} else {
		// New background during scene

		// Just re-initialise the script.
		InitStepAnimScript(&thisAnim, pBG, FROM_LE_32(pfr->script), BGspeed);
		StepAnimScript(&thisAnim);
	}

	CORO_END_CODE;
}

/**
 * setBackPal()
 */
void setBackPal(SCNHANDLE hPal) {
	BackPalette = hPal;

	fettleFontPal(BackPalette);
	CreateTranslucentPalette(BackPalette);
}

void ChangePalette(SCNHANDLE hPal) {
	SwapPalette(FindPalette(BackPalette), hPal);

	setBackPal(hPal);
}

/**
 * Given the scene background film, extracts the palette handle for
 * everything else's use, then starts a display process for each reel
 * in the film.
 * @param bfilm			Scene background film
 */
void startupBackground(SCNHANDLE bfilm) {
	const FILM *pfilm;
	PIMAGE	pim;

	BgroundHandle = bfilm;		// Save handle in case of Save_Scene()

	pim = GetImageFromFilm(bfilm, 0, NULL, NULL, &pfilm);
	setBackPal(FROM_LE_32(pim->hImgPal));

	// Extract the film speed
	BGspeed = ONE_SECOND / FROM_LE_32(pfilm->frate);

	if (pBG == NULL)
		control(CONTROL_STARTOFF);	// New feature - start scene with control off

	// Start display process for each reel in the film
	assert(FROM_LE_32(pfilm->numreels) == 1); // Multi-reeled backgrounds withdrawn
	ProcessCreate(PID_REEL, BGmainProcess, &pfilm->reels[0], sizeof(FREEL));
}

/**
 * Return the current scene handle.
 */
SCNHANDLE GetBgroundHandle(void) {
	return BgroundHandle;
}

} // end of namespace Tinsel
