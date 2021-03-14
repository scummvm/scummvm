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
 */

//=============================================================================
//
// MOUSELIBW32.CPP
//
// Library of mouse functions for graphics and text mode
//
// (c) 1994 Chris Jones
// Win32 (allegro) update (c) 1999 Chris Jones
//
//=============================================================================

#include "ags/shared/core/platform.h"

#define AGS_SIMULATE_RIGHT_CLICK (AGS_PLATFORM_OS_MACOS)

#if AGS_PLATFORM_OS_WINDOWS
//include <dos.h>
//include <conio.h>
//include <process.h>
#endif

#include "ags/shared/util/wgt2allg.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include "ags/engine/ac/gamestate.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/device/mousew32.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/gfx_util.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/shared/util/math.h"
#include "ags/globals.h"
#if AGS_SIMULATE_RIGHT_CLICK
#include "ags/shared/ac/sys_events.h" // j for ags_iskeypressed
#endif

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

enum {
	NONE = -1, LEFT = 0, RIGHT = 1, MIDDLE = 2
};

static const int MB_ARRAY[3] = { 1, 2, 4 };

void mgraphconfine(int x1, int y1, int x2, int y2) {
	_GP(mouse).ControlRect = Rect(x1, y1, x2, y2);
	set_mouse_range(_GP(mouse).ControlRect.Left, _GP(mouse).ControlRect.Top, _GP(mouse).ControlRect.Right, _GP(mouse).ControlRect.Bottom);
	Debug::Printf("Mouse confined: (%d,%d)-(%d,%d) (%dx%d)",
		_GP(mouse).ControlRect.Left, _GP(mouse).ControlRect.Top, _GP(mouse).ControlRect.Right, _GP(mouse).ControlRect.Bottom,
		_GP(mouse).ControlRect.GetWidth(), _GP(mouse).ControlRect.GetHeight());
}

void mgetgraphpos() {
	poll_mouse();
	if (_G(disable_mgetgraphpos)) {
		// The cursor coordinates are provided from alternate source;
		// in this case we completely ignore actual cursor movement.
		if (!_G(ignore_bounds) &&
			(_G(mousex) < _G(boundx1) || _G(mousey) < _G(boundy1) || _G(mousex) > _G(boundx2) || _G(mousey) > _G(boundy2))) {
			_G(mousex) = Math::Clamp(_G(mousex), _G(boundx1), _G(boundx2));
			_G(mousey) = Math::Clamp(_G(mousey), _G(boundy1), _G(boundy2));
			msetgraphpos(_G(mousex), _G(mousey));
		}
		return;
	}

	if (!_G(switched_away) && _GP(mouse).ControlEnabled) {
		// Control mouse movement by querying mouse mickeys (movement deltas)
		// and applying them to saved mouse coordinates.
		int mickey_x, mickey_y;
		get_mouse_mickeys(&mickey_x, &mickey_y);

		// Apply mouse speed
		int dx = _GP(mouse).Speed * mickey_x;
		int dy = _GP(mouse).Speed * mickey_y;

		//
		// Perform actual cursor update
		//---------------------------------------------------------------------
		// If the real cursor is inside the control rectangle (read - game window),
		// then apply sensitivity factors and adjust real cursor position
		if (_GP(mouse).ControlRect.IsInside(_G(real_mouse_x) + dx, _G(real_mouse_y) + dy)) {
			_G(real_mouse_x) += dx;
			_G(real_mouse_y) += dy;
			position_mouse(_G(real_mouse_x), _G(real_mouse_y));
		}
		// Otherwise, if real cursor was moved outside the control rect, yet we
		// are required to confine cursor inside one, then adjust cursor position
		// to stay inside the rect's bounds.
		else if (_GP(mouse).ConfineInCtrlRect) {
			_G(real_mouse_x) = Math::Clamp(_G(real_mouse_x) + dx, _GP(mouse).ControlRect.Left, _GP(mouse).ControlRect.Right);
			_G(real_mouse_y) = Math::Clamp(_G(real_mouse_y) + dy, _GP(mouse).ControlRect.Top, _GP(mouse).ControlRect.Bottom);
			position_mouse(_G(real_mouse_x), _G(real_mouse_y));
		}
		// Lastly, if the real cursor is out of the control rect, simply add
		// actual movement to keep up with the system cursor coordinates.
		else {
			_G(real_mouse_x) += mickey_x;
			_G(real_mouse_y) += mickey_y;
		}

		// Do not update the game cursor if the real cursor is beyond the control rect
		if (!_GP(mouse).ControlRect.IsInside(_G(real_mouse_x), _G(real_mouse_y)))
			return;
	} else {
		// Save real cursor coordinates provided by system
		_G(real_mouse_x) = _G(mouse_x);
		_G(real_mouse_y) = _G(mouse_y);
	}

	// Set new in-game cursor position
	_G(mousex) = _G(real_mouse_x);
	_G(mousey) = _G(real_mouse_y);

	if (!_G(ignore_bounds) &&
		(_G(mousex) < _G(boundx1) || _G(mousey) < _G(boundy1) || _G(mousex) > _G(boundx2) || _G(mousey) > _G(boundy2))) {
		_G(mousex) = Math::Clamp(_G(mousex), _G(boundx1), _G(boundx2));
		_G(mousey) = Math::Clamp(_G(mousey), _G(boundy1), _G(boundy2));
		msetgraphpos(_G(mousex), _G(mousey));
	}

	// Convert to virtual coordinates
	_GP(mouse).AdjustPosition(_G(mousex), _G(mousey));
}

void msetcursorlimit(int x1, int y1, int x2, int y2) {
	_G(boundx1) = x1;
	_G(boundy1) = y1;
	_G(boundx2) = x2;
	_G(boundy2) = y2;
}

void domouse(int str) {
	/*
	   TO USE THIS ROUTINE YOU MUST LOAD A MOUSE CURSOR USING mloadcursor.
	   YOU MUST ALSO REMEMBER TO CALL mfreemem AT THE END OF THE PROGRAM.
	*/
	int poow = _G(mousecurs)[(int)_G(currentcursor)]->GetWidth();
	int pooh = _G(mousecurs)[(int)_G(currentcursor)]->GetHeight();
	//int smx = _G(mousex) - _G(hotxwas), smy = _G(mousey) - _G(hotywas);
	const Rect &viewport = _GP(play).GetMainViewport();

	mgetgraphpos();
	_G(mousex) -= _G(hotx);
	_G(mousey) -= _G(hoty);

	if (_G(mousex) + poow >= viewport.GetWidth())
		poow = viewport.GetWidth() - _G(mousex);

	if (_G(mousey) + pooh >= viewport.GetHeight())
		pooh = viewport.GetHeight() - _G(mousey);

	_G(mousex) += _G(hotx);
	_G(mousey) += _G(hoty);
	_G(hotxwas) = _G(hotx);
	_G(hotywas) = _G(hoty);
}

int ismouseinbox(int lf, int tp, int rt, int bt) {
	if ((_G(mousex) >= lf) & (_G(mousex) <= rt) & (_G(mousey) >= tp) & (_G(mousey) <= bt))
		return TRUE;
	else
		return FALSE;
}

void mfreemem() {
	for (int re = 0; re < _G(numcurso); re++) {
		delete _G(mousecurs)[re];
	}
}

void mloadwcursor(char *namm) {
	color dummypal[256];
	if (wloadsprites(&dummypal[0], namm, _G(mousecurs), 0, MAXCURSORS)) {
		error("mloadwcursor: Error reading mouse cursor file");
	}
}

int mgetbutton() {
	int toret = NONE;
	poll_mouse();
	int butis = _G(mouse_b);

	if ((butis > 0) &(_G(butwas) > 0))
		return NONE;  // don't allow holding button down

	if (butis & 1) {
		toret = LEFT;
#if AGS_SIMULATE_RIGHT_CLICK
		// j Ctrl-left click should be right-click
		if (ags_iskeypressed(__allegro_KEY_LCONTROL) || ags_iskeypressed(__allegro_KEY_RCONTROL)) {
			toret = RIGHT;
		}
#endif
	} else if (butis & 2)
		toret = RIGHT;
	else if (butis & 4)
		toret = MIDDLE;

	_G(butwas) = butis;
	return toret;
}

int misbuttondown(int buno) {
	poll_mouse();
	if (_G(mouse_b) & MB_ARRAY[buno])
		return TRUE;
	return FALSE;
}

void msetgraphpos(int xa, int ya) {
	_G(real_mouse_x) = xa;
	_G(real_mouse_y) = ya;
	position_mouse(_G(real_mouse_x), _G(real_mouse_y));
}

void msethotspot(int xx, int yy) {
	_G(hotx) = xx;  // _G(mousex) -= _G(hotx); _G(mousey) -= _G(hoty);
	_G(hoty) = yy;  // _G(mousex) += _G(hotx); _G(mousey) += _G(hoty);
}

int minstalled() {
	return install_mouse();
}

void Mouse::AdjustPosition(int &x, int &y) {
	x = _GP(GameScaling).X.UnScalePt(x) - _GP(play).GetMainViewport().Left;
	y = _GP(GameScaling).Y.UnScalePt(y) - _GP(play).GetMainViewport().Top;
}

void Mouse::SetGraphicArea() {
	Rect dst_r = _GP(GameScaling).ScaleRange(_GP(play).GetMainViewport());
	mgraphconfine(dst_r.Left, dst_r.Top, dst_r.Right, dst_r.Bottom);
}

void Mouse::SetMoveLimit(const Rect &r) {
	Rect src_r = OffsetRect(r, _GP(play).GetMainViewport().GetLT());
	Rect dst_r = _GP(GameScaling).ScaleRange(src_r);
	msetcursorlimit(dst_r.Left, dst_r.Top, dst_r.Right, dst_r.Bottom);
}

void Mouse::SetPosition(const Point p) {
	msetgraphpos(_GP(GameScaling).X.ScalePt(p.X + _GP(play).GetMainViewport().Left), _GP(GameScaling).Y.ScalePt(p.Y + _GP(play).GetMainViewport().Top));
}

bool Mouse::IsLockedToWindow() {
	return LockedToWindow;
}

bool Mouse::TryLockToWindow() {
	if (!LockedToWindow)
		LockedToWindow = _G(platform)->LockMouseToWindow();
	return LockedToWindow;
}

void Mouse::UnlockFromWindow() {
	_G(platform)->UnlockMouse();
	LockedToWindow = false;
}

void Mouse::EnableControl(bool confine) {
	ControlEnabled = true;
	ConfineInCtrlRect = confine;
}

void Mouse::DisableControl() {
	ControlEnabled = false;
	ConfineInCtrlRect = false;
}

bool Mouse::IsControlEnabled() {
	return ControlEnabled;
}

void Mouse::SetSpeedUnit(float f) {
	SpeedUnit = f;
	Speed = SpeedVal / SpeedUnit;
}

float Mouse::GetSpeedUnit() {
	return SpeedUnit;
}

void Mouse::SetSpeed(float speed) {
	SpeedVal = Math::Max(0.f, speed);
	Speed = SpeedUnit * SpeedVal;
}

float Mouse::GetSpeed() {
	return SpeedVal;
}

} // namespace AGS3
