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
 * Clipping rectangle defines
 */

#ifndef TINSEL_CURSOR_H // prevent multiple includes
#define TINSEL_CURSOR_H

#include "common/frac.h"
#include "tinsel/anim.h"
#include "tinsel/dw.h" // for SCNHANDLE

namespace Tinsel {

struct IMAGE;
struct FREEL;
struct MULTI_INIT;
struct FILM;
struct OBJECT;

#define MAX_TRAILERS 10

class Cursor {
public:
	Cursor();
	virtual ~Cursor() {}
	void AdjustCursorXY(int deltaX, int deltaY);
	void SetCursorXY(int x, int y);
	void SetCursorScreenXY(int newx, int newy);
	void GetCursorXY(int *x, int *y, bool absolute);
	bool GetCursorXYNoWait(int *x, int *y, bool absolute);
	bool isCursorShown();

	void RestoreMainCursor();
	void SetTempCursor(SCNHANDLE pScript);
	void DwHideCursor();
	void UnHideCursor();
	void HideCursorProcess();
	void AnimateProcess();
	void FreezeCursor();
	void DoFreezeCursor(bool bFreeze);
	void HideCursorTrails();
	void UnHideCursorTrails();
	void DelAuxCursor();
	void SetAuxCursor(SCNHANDLE hFilm);
	void DwInitCursor(SCNHANDLE bfilm);
	void DropCursor();
	void RestartCursor();
	void RebootCursor();
	void StartCursorFollowed();
	void EndCursorFollowed();
	void InitCurObj();
	void InitCurPos();
	void DoCursorMove();
	IMAGE *GetImageFromReel(const FREEL *pfr, const MULTI_INIT **ppmi = nullptr);
	IMAGE *GetImageFromFilm(SCNHANDLE hFilm, int reel, const FREEL **ppfr = nullptr, const MULTI_INIT **ppmi = nullptr, const FILM **ppfilm = nullptr);

	bool CursorIsFrozen() { return _frozenCursor; }
	int NumTrails() { return _numTrails; }
	bool IsHidden() { return _hiddenCursor; }
	bool ShouldBeHidden() { return _hiddenCursor || _tempHiddenCursor; }
	bool HasReelData() { return _cursorFilm != 0; }

	bool _cursorProcessesStopped; // Set by DropCursor() at the end of a scene
	    // - causes cursor processes to do nothing
	    // Reset when main cursor has re-initialized

	bool _cursorProcessesRestarted; // When main cursor has been bWhoa-ed, it waits
	                // for this to be true.
	                // Main cursor sets this to true after a re-start

private:
	void InitCurTrailObj(int i, int x, int y);
	bool GetDriverPosition(int *x, int *y);

	OBJECT *_mainCursor; // Main cursor object
	OBJECT *_auxCursor; // Auxiliary cursor object

	ANIM _mainCursorAnim; // Main cursor animation structure
	ANIM _auxCursorAnim; // Auxiliary cursor animation structure

	bool _hiddenCursor;   // Set when cursor is hidden
	bool _hiddenTrails; // Set when cursor trails are hidden
	bool _tempHiddenCursor;       // Set when cursor is hidden

	bool _frozenCursor; // Set when cursor position is frozen

	frac_t _iterationSize;

	SCNHANDLE _cursorFilm; // Handle to cursor reel data

	int _numTrails;
	int _nextTrail;

    // Auxillary cursor image's animation offsets
	short _auxCursorOffsetX;
    short _auxCursorOffsetY;

	struct {
		ANIM trailAnim;   // Animation structure
		OBJECT *trailObj; // This trailer's object
	} _trailData[MAX_TRAILERS];

	int _lastCursorX, _lastCursorY;
};

} // End of namespace Tinsel

#endif // TINSEL_CURSOR_H
