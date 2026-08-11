/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

Cursor::Cursor() {
	_mainCursor = nullptr;
	_auxCursor = nullptr;

	_mainCursorAnim = {0, 0, 0, 0, 0};
	_auxCursorAnim = {0, 0, 0, 0, 0};

	_hiddenCursor = false;
	_hiddenTrails = false;
	_tempHiddenCursor = false;
	_frozenCursor = false;

	_iterationSize = 0;

	_cursorFilm = 0;

	_numTrails = 0;
	_nextTrail = 0;

	_cursorProcessesStopped = false;
	_cursorProcessesRestarted = false;

	_auxCursorOffsetX = 0;
	_auxCursorOffsetY = 0;

	_lastCursorX = 0;
	_lastCursorY = 0;
}

/**
 * Initialize and insert a cursor trail object, set its Z-pos, and hide
 * it. Also initialize its animation script.
 */
void Cursor::InitCurTrailObj(int i, int x, int y) {
	if (!_numTrails)
		return;

	const FILM *pFilm = (const FILM *)_vm->_handle->LockMem(_cursorFilm);
	const FREEL *pfr = (const FREEL *)&pFilm->reels[i + 1];
	const MULTI_INIT *pmi = pfr->GetMultiInit();

	PokeInPalette(pmi);

	// Get rid of old object
	MultiDeleteObjectIfExists(FIELD_STATUS, &_trailData[i].trailObj);

	// Initialize and insert the object, set its Z-pos, and hide it
	_trailData[i].trailObj = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _trailData[i].trailObj);
	MultiSetAniXYZ(_trailData[i].trailObj, x, y, Z_CURSORTRAIL);

	// Initialize the animation script
	InitStepAnimScript(&_trailData[i].trailAnim, _trailData[i].trailObj, FROM_32(pfr->script), ONE_SECOND / FROM_32(pFilm->frate));
	StepAnimScript(&_trailData[i].trailAnim);
}

/**
 * Get the cursor position from the mouse driver.
 */
bool Cursor::GetDriverPosition(int *x, int *y) {
	Common::Point ptMouse = _vm->getMousePosition();
	*x = ptMouse.x;
	*y = ptMouse.y;

	return(*x >= 0 && *x <= SCREEN_WIDTH - 1 &&
		*y >= 0 && *y <= SCREEN_HEIGHT - 1);
}

/**
 * Move the cursor relative to current position.
 */
void Cursor::AdjustCursorXY(int deltaX, int deltaY) {
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
void Cursor::SetCursorXY(int newx, int newy) {
	int	x, y;
	int	Loffset, Toffset;	// Screen offset

	_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
	newx -= Loffset;
	newy -= Toffset;

	if (GetDriverPosition(&x, &y))
		_vm->setMousePosition(Common::Point(newx, newy));
	DoCursorMove();
}

/**
 * Move the cursor to a screen position.
 */
void Cursor::SetCursorScreenXY(int newx, int newy) {
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
bool Cursor::GetCursorXYNoWait(int *x, int *y, bool absolute) {
	if (_mainCursor == NULL) {
		*x = *y = 0;
		return false;
	}

	GetAniPosition(_mainCursor, x, y);

	if (absolute) {
		int	Loffset, Toffset;	// Screen offset
		_vm->_bg->PlayfieldGetPos(FIELD_WORLD, &Loffset, &Toffset);
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
void Cursor::GetCursorXY(int *x, int *y, bool absolute) {
	//while (McurObj == NULL)
	//	ProcessSleepSelf();
	assert(_mainCursor);
	GetCursorXYNoWait(x, y, absolute);
}

/**
 * Re-initialize the main cursor to use the main cursor reel.
 * Called from TINLIB.C to restore cursor after hiding it.
 * Called from INVENTRY.C to restore cursor after customising it.
 */
void Cursor::RestoreMainCursor() {
	const FILM *pfilm;

	if (_mainCursor != NULL) {
		pfilm = (const FILM *)_vm->_handle->LockMem(_cursorFilm);

		InitStepAnimScript(&_mainCursorAnim, _mainCursor, FROM_32(pfilm->reels->script), ONE_SECOND / FROM_32(pfilm->frate));
		StepAnimScript(&_mainCursorAnim);
	}
	_hiddenCursor = false;
	_frozenCursor = false;
}

/**
 * Called from INVENTRY.C to customise the main cursor.
 */
void Cursor::SetTempCursor(SCNHANDLE pScript) {
	if (_mainCursor != NULL)
		InitStepAnimScript(&_mainCursorAnim, _mainCursor, pScript, 2);
}

/**
 * Hide the cursor.
 */
void Cursor::DwHideCursor() {
	_hiddenCursor = true;

	if (_mainCursor)
		MultiHideObject(_mainCursor);
	if (_auxCursor)
		MultiHideObject(_auxCursor);

	for (int i = 0; i < _numTrails; i++) {
		MultiDeleteObjectIfExists(FIELD_STATUS, &_trailData[i].trailObj);
	}
}

void Cursor::HideCursorProcess() {
	if (_mainCursor)
		MultiHideObject(_mainCursor);
	if (_auxCursor)
		MultiHideObject(_auxCursor);

	for (int i = 0; i < _numTrails; i++) {
		if (_trailData[i].trailObj != NULL)
			MultiHideObject(_trailData[i].trailObj);
	}
}

/**
 * Unhide the cursor.
 */
void Cursor::UnHideCursor() {
	_hiddenCursor = false;
}

/**
 * Freeze the cursor.
 */
void Cursor::FreezeCursor() {
	_frozenCursor = true;
}

/**
 * Freeze the cursor, or not.
 */
void Cursor::DoFreezeCursor(bool bFreeze) {
	_frozenCursor = bFreeze;
}

/**
 * HideCursorTrails
 */
void Cursor::HideCursorTrails() {
	int i;

	_hiddenTrails = true;

	for (i = 0; i < _numTrails; i++)	{
		MultiDeleteObjectIfExists(FIELD_STATUS, &_trailData[i].trailObj);
	}
}

/**
 * UnHideCursorTrails
 */
void Cursor::UnHideCursorTrails() {
	_hiddenTrails = false;
}

/**
 * Delete auxiliary cursor. Restore animation offsets in the image.
 */
void Cursor::DelAuxCursor() {
	MultiDeleteObjectIfExists(FIELD_STATUS, &_auxCursor);
}

/**
 * Set auxiliary cursor.
 * Save animation offsets from the image if required.
 */
void Cursor::SetAuxCursor(SCNHANDLE hFilm) {
	const FILM *pfilm = (const FILM *)_vm->_handle->LockMem(hFilm);
	const FREEL *pfr = &pfilm->reels[0];
	const MULTI_INIT *pmi = pfr->GetMultiInit();
	const FRAME *pFrame = pmi->GetFrame();
	const IMAGE *pim;
	int	x, y;		// Cursor position

	DelAuxCursor();		// Get rid of previous

	// Noir does not use palettes
	if (TinselVersion < 3) {
		// WORKAROUND: There's no palette when loading a DW1 savegame with a held item, so exit if so
		if (!_vm->_bg->BgPal())
			return;

		assert(_vm->_bg->BgPal()); // no background palette
		PokeInPalette(pmi);
	}

	GetCursorXY(&x, &y, false);	// Note: also waits for cursor to appear

	pim = _vm->_handle->GetImage(READ_32(pFrame)); // Get pointer to auxiliary cursor's image

	_auxCursorOffsetX = (short)(pim->imgWidth / 2 - ((int16) pim->anioffX));
	_auxCursorOffsetY = (short)((pim->imgHeight & ~C16_FLAG_MASK) / 2 -
		((int16) pim->anioffY));
	delete pim;

	// Initialize and insert the auxiliary cursor object
	_auxCursor = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _auxCursor);

	// Initialize the animation and set its position
	InitStepAnimScript(&_auxCursorAnim, _auxCursor, FROM_32(pfr->script), ONE_SECOND / FROM_32(pfilm->frate));
	MultiSetAniXYZ(_auxCursor, x - _auxCursorOffsetX, y - _auxCursorOffsetY, Z_ACURSOR);

	if (_hiddenCursor)
		MultiHideObject(_auxCursor);
}

/**
 * MoveCursor
 */
void Cursor::DoCursorMove() {
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
			newX -= _iterationSize;

		if (dir & MSK_RIGHT)
			newX += _iterationSize;

		if (dir & MSK_UP)
			newY -= _iterationSize;

		if (dir & MSK_DOWN)
			newY += _iterationSize;

		_iterationSize += ITER_ACCELERATION;

		// set new mouse driver position
		_vm->setMousePosition(Common::Point(fracToInt(newX), fracToInt(newY)));
	} else

		_iterationSize = ITERATION_BASE;

	// get new mouse driver position - could have been modified
	ptMouse = _vm->getMousePosition();

	if (_lastCursorX != ptMouse.x || _lastCursorY != ptMouse.y) {
		resetUserEventTime();

		if (!_hiddenTrails && !_hiddenCursor) {
			InitCurTrailObj(_nextTrail++, _lastCursorX, _lastCursorY);
			if (_nextTrail == _numTrails)
				_nextTrail = 0;
		}
	}

	// adjust cursor to new mouse position
	if (_mainCursor)
		MultiSetAniXY(_mainCursor, ptMouse.x, ptMouse.y);
	if (_auxCursor != NULL)
		MultiSetAniXY(_auxCursor, ptMouse.x - _auxCursorOffsetX, ptMouse.y - _auxCursorOffsetY);

	if (_vm->_dialogs->inventoryActive() && _mainCursor) {
		// Notify the inventory
		_vm->_dialogs->xMovement(ptMouse.x - startX);
		_vm->_dialogs->yMovement(ptMouse.y - startY);
	}

	_lastCursorX = ptMouse.x;
	_lastCursorY = ptMouse.y;
}

/**
 * Initialize cursor object.
 */
void Cursor::InitCurObj() {
	const FILM *pFilm = (const FILM *)_vm->_handle->LockMem(_cursorFilm);
	const FREEL *pfr = (const FREEL *)&pFilm->reels[0];
	const MULTI_INIT *pmi = pfr->GetMultiInit();

	if (TinselVersion != 3) {
		PokeInPalette(pmi);
	}

	if (TinselVersion <= 1)
		_auxCursor = nullptr; // No auxiliary cursor

	_mainCursor = MultiInitObject(pmi);
	MultiInsertObject(_vm->_bg->GetPlayfieldList(FIELD_STATUS), _mainCursor);

	InitStepAnimScript(&_mainCursorAnim, _mainCursor, FROM_32(pfr->script), ONE_SECOND / FROM_32(pFilm->frate));
}

/**
 * Initialize the cursor position.
 */
void Cursor::InitCurPos() {
	Common::Point ptMouse = _vm->getMousePosition();
	_lastCursorX = ptMouse.x;
	_lastCursorY = ptMouse.y;

	MultiSetZPosition(_mainCursor, Z_CURSOR);
	DoCursorMove();
	MultiHideObject(_mainCursor);

	_iterationSize = ITERATION_BASE;
}

/**
 * Called from dec_cursor() Glitter function.
 * Register the handle to cursor reel data.
 */
void Cursor::DwInitCursor(SCNHANDLE bfilm) {
	const FILM *pfilm;

	_cursorFilm = bfilm;

	pfilm = (const FILM *)_vm->_handle->LockMem(_cursorFilm);
	_numTrails = FROM_32(pfilm->numreels) - 1;

	assert(_numTrails <= MAX_TRAILERS);
}

/**
 * DropCursor is called when a scene is closing down.
 */
void Cursor::DropCursor() {
	if (TinselVersion >= 2) {
		if (_auxCursor)
			MultiDeleteObjectIfExists(FIELD_STATUS, &_auxCursor);
		if (_mainCursor)
			MultiDeleteObjectIfExists(FIELD_STATUS, &_mainCursor);

		_cursorProcessesRestarted = false;
	}

	_auxCursor = nullptr;		// No auxiliary cursor
	_mainCursor = nullptr;		// No cursor object (imminently deleted elsewhere)
	_hiddenCursor = false;	// Not hidden in next scene
	_hiddenTrails = false;	// Trailers not hidden in next scene
	_cursorProcessesStopped = true;		// Suspend cursor processes

	for (int i = 0; i < _numTrails; i++) {
		MultiDeleteObjectIfExists(FIELD_STATUS, &_trailData[i].trailObj);
	}
}

/**
 * RestartCursor is called when a new scene is starting up.
 */
void Cursor::RestartCursor() {
	_cursorProcessesRestarted = true;	// Get the main cursor to re-initialize
}

/**
 * Called when restarting the game, ensures correct re-start with NULL
 * pointers etc.
 */
void Cursor::RebootCursor() {
	_mainCursor = _auxCursor = nullptr;
	for (int i = 0; i < MAX_TRAILERS; i++)
		_trailData[i].trailObj = nullptr;

	_hiddenCursor = _hiddenTrails = _frozenCursor = false;

	_cursorFilm = 0;

	_cursorProcessesStopped = false;
	_cursorProcessesRestarted = false;
}

void Cursor::StartCursorFollowed() {
	DelAuxCursor();

	if (!SysVar(SV_ENABLEPRINTCURSOR))
		_tempHiddenCursor = true;
}

void Cursor::EndCursorFollowed() {
	_vm->_dialogs->inventoryIconCursor(false); // May be holding something
	_tempHiddenCursor = false;
}

bool Cursor::isCursorShown() {
	return !(_tempHiddenCursor || _hiddenCursor);
}

void Cursor::AnimateProcess() {
	// Step the animation script(s)
	StepAnimScript(&_mainCursorAnim);
	if (_auxCursor != NULL)
		StepAnimScript(&_auxCursorAnim);
	for (int i = 0; i < _vm->_cursor->NumTrails(); i++) {
		if (_trailData[i].trailObj != NULL) {
			if (StepAnimScript(&_trailData[i].trailAnim) == ScriptFinished) {
				MultiDeleteObjectIfExists(FIELD_STATUS, &_trailData[i].trailObj);
			}
		}
	}

	// Move the cursor as appropriate
	if (!_vm->_cursor->CursorIsFrozen())
		_vm->_cursor->DoCursorMove();
}

/**
 * CursorStoppedCheck
 */
void CursorStoppedCheck(CORO_PARAM) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If scene is closing down
	if (_vm->_cursor->_cursorProcessesStopped) {
		// ...wait for next scene start-up
		while (!_vm->_cursor->_cursorProcessesRestarted)
			CORO_SLEEP(1);

		// Re-initialize
		_vm->_cursor->InitCurObj();
		_vm->_cursor->InitCurPos();
		_vm->_dialogs->inventoryIconCursor(false); // May be holding something

		// Re-start the cursor trails
		_vm->_cursor->_cursorProcessesRestarted = true;
		_vm->_cursor->_cursorProcessesStopped = false;
	}
	CORO_END_CODE;
}

bool CanInitializeCursor() {
	if (!_vm->_cursor->HasReelData()) {
		return false;
	} else if (TinselVersion != 3) {
		return (_vm->_bg->BgPal() != 0);
	}
	return true;
}

/**
 * The main cursor process.
 */
void CursorProcess(CORO_PARAM, const void *) {
	// COROUTINE
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	while (!CanInitializeCursor())
		CORO_SLEEP(1);

	_vm->_cursor->InitCurObj();
	_vm->_cursor->InitCurPos();
	_vm->_dialogs->inventoryIconCursor(false); // May be holding something

	_vm->_cursor->_cursorProcessesStopped = false;
	_vm->_cursor->_cursorProcessesRestarted = false;

	while (1) {
		// allow rescheduling
		CORO_SLEEP(1);

		// Stop/start between scenes
		CORO_INVOKE_0(CursorStoppedCheck);

		_vm->_cursor->AnimateProcess();

		// If the cursor should be hidden...
		if (_vm->_cursor->ShouldBeHidden()) {
			_vm->_cursor->HideCursorProcess();

			// Wait 'til cursor is again required.
			while (_vm->_cursor->IsHidden()) {
				CORO_SLEEP(1);

				// Stop/start between scenes
				CORO_INVOKE_0(CursorStoppedCheck);
			}
		}
	}
	CORO_END_CODE;
}

} // End of namespace Tinsel
