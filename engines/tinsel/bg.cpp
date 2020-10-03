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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Plays the background film of a scene.
 */

#include "tinsel/anim.h"
#include "tinsel/background.h"
#include "tinsel/cursor.h"
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

#include "common/textconsole.h"
#include "common/util.h"

namespace Tinsel {

/**
 * Run main animation that comprises the scene background.
 */
void BGmainProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pFilm;
	const FREEL *pReel;
	const MULTI_INIT *pmi;

	// get the stuff copied to process when it was created
	if (_vm->_bg->_pBG[0] == NULL) {
		/*** At start of scene ***/

		if (!TinselV2) {
			pReel = (const FREEL *)param;

			// Get the MULTI_INIT structure
			pmi = (const MULTI_INIT *)_vm->_handle->LockMem(FROM_32(pReel->mobj));

			// Initialize and insert the object, and initialize its script.
			_vm->_bg->_pBG[0] = MultiInitObject(pmi);
			MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_WORLD), _vm->_bg->_pBG[0]);
			InitStepAnimScript(&_vm->_bg->_thisAnim[0], _vm->_bg->_pBG[0], FROM_32(pReel->script), _vm->_bg->getBgSpeed());
			_vm->_bg->_bgReels = 1;
		} else {
			/*** At start of scene ***/
			pFilm = (const FILM *)_vm->_handle->LockMem(_vm->_bg->GetBgroundHandle());
			_vm->_bg->_bgReels = FROM_32(pFilm->numreels);

			int i;
			for (i = 0; i < _vm->_bg->_bgReels; i++) {
				// Get the MULTI_INIT structure
				pmi = (PMULTI_INIT)_vm->_handle->LockMem(FROM_32(pFilm->reels[i].mobj));

				// Initialize and insert the object, and initialize its script.
				_vm->_bg->_pBG[i] = MultiInitObject(pmi);
				MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_WORLD), _vm->_bg->_pBG[i]);
				MultiSetZPosition(_vm->_bg->_pBG[i], 0);
				InitStepAnimScript(&_vm->_bg->_thisAnim[i], _vm->_bg->_pBG[i], FROM_32(pFilm->reels[i].script), _vm->_bg->getBgSpeed());

				if (i > 0)
					_vm->_bg->_pBG[i-1]->pSlave = _vm->_bg->_pBG[i];
			}
		}

		if (_vm->_bg->GetDoFadeIn()) {
			FadeInFast();
			_vm->_bg->SetDoFadeIn(false);
		} else if (TinselV2)
			PokeInTagColor();

		for (;;) {
			for (int i = 0; i < _vm->_bg->_bgReels; i++) {
				if (StepAnimScript(&_vm->_bg->_thisAnim[i]) == ScriptFinished)
					error("Background animation has finished");
			}

			CORO_SLEEP(1);
		}
	} else {
		// New background during scene
		if (!TinselV2) {
			pReel = (const FREEL *)param;
			InitStepAnimScript(&_vm->_bg->_thisAnim[0], _vm->_bg->_pBG[0], FROM_32(pReel->script), _vm->_bg->getBgSpeed());
			StepAnimScript(&_vm->_bg->_thisAnim[0]);
		} else {
			pFilm = (const FILM *)_vm->_handle->LockMem(_vm->_bg->GetBgroundHandle());
			assert(_vm->_bg->_bgReels == (int32)FROM_32(pFilm->numreels));

			// Just re-initialize the scripts.
			for (int i = 0; i < _vm->_bg->_bgReels; i++) {
				InitStepAnimScript(&_vm->_bg->_thisAnim[i], _vm->_bg->_pBG[i], pFilm->reels[i].script, _vm->_bg->getBgSpeed());
				StepAnimScript(&_vm->_bg->_thisAnim[i]);
			}
		}
	}

	CORO_END_CODE;
}

/**
 * Runs secondary reels for a scene background
 */
void BGotherProcess(CORO_PARAM, const void *param) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
		OBJECT *pObj;
		ANIM anim;
	CORO_END_CONTEXT(_ctx);

	const FREEL *pReel = (const FREEL *)param;
	const MULTI_INIT *pmi = (const MULTI_INIT *)_vm->_handle->LockMem(FROM_32(pReel->mobj));

	CORO_BEGIN_CODE(_ctx);

	// Initialize and insert the object, and initialize its script.
	_ctx->pObj = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_WORLD), _ctx->pObj);

	InitStepAnimScript(&_ctx->anim, _vm->_bg->_pBG[0], FROM_32(pReel->script), _vm->_bg->getBgSpeed());

	while (StepAnimScript(&_ctx->anim) != ScriptFinished)
		CORO_SLEEP(1);

	CORO_END_CODE;
}

/**
 * Given the scene background film, extracts the palette handle for
 * everything else's use, then starts a display process for each reel
 * in the film.
 * @param hFilm			Scene background film
 */
void Background::StartupBackground(CORO_PARAM, SCNHANDLE hFilm) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	const FILM *pfilm;
	IMAGE *pim;

	_hBackground = hFilm;		// Save handle in case of Save_Scene()

	pim = _vm->_cursor->GetImageFromFilm(hFilm, 0, NULL, NULL, &pfilm);

	SetBackPal(FROM_32(pim->hImgPal));

	// Extract the film speed
	_BGspeed = ONE_SECOND / FROM_32(pfilm->frate);

	// Start display process for each reel in the film
	CoroScheduler.createProcess(PID_REEL, BGmainProcess, &pfilm->reels[0], sizeof(FREEL));

	if (TinselV0) {
		for (uint i = 1; i < FROM_32(pfilm->numreels); ++i)
			CoroScheduler.createProcess(PID_REEL, BGotherProcess, &pfilm->reels[i], sizeof(FREEL));
	}

	if (_pBG[0] == NULL)
		ControlStartOff();

	if (TinselV2 && (coroParam != Common::nullContext))
		CORO_GIVE_WAY;

	CORO_END_CODE;
}

} // End of namespace Tinsel
