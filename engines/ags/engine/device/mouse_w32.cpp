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

#include "common/util.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/platform/base/sys_main.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

enum {
	NONE = -1, LEFT = 0, RIGHT = 1, MIDDLE = 2
};

// static const int MB_ARRAY[3] = { 1, 2, 4 };

void mgetgraphpos() {
	// TODO: review and possibly rewrite whole thing;
	// research what disable_mgetgraphpos does, and is this still necessary?
	// disable or update mouse speed control to sdl
	// (does sdl support mouse cursor speed? is it even necessary anymore?);

	// TODO: [sonneveld] find out where mgetgraphpos is needed, are events polled before that?
	sys_evt_process_pending();

	if (_G(disable_mgetgraphpos)) {
		// The cursor coordinates are provided from alternate source;
		// in this case we completely ignore actual cursor movement.
		if (!_G(ignore_bounds) &&
			// When applying script bounds we only do so while cursor is inside game viewport
			_GP(mouse).ControlRect.IsInside(_G(mousex), _G(mousey)) &&
			(_G(mousex) < _G(boundx1) || _G(mousey) < _G(boundy1) || _G(mousex) > _G(boundx2) || _G(mousey) > _G(boundy2))) {
			_G(mousex) = CLIP(_G(mousex), _G(boundx1), _G(boundx2));
			_G(mousey) = CLIP(_G(mousey), _G(boundy1), _G(boundy2));
			msetgraphpos(_G(mousex), _G(mousey));
		}
		return;
	}

	if (!_G(switched_away) && _GP(mouse).ControlEnabled) {
		// Use relative mouse movement; speed factor should already be applied by SDL in this mode
		int rel_x, rel_y;
		ags_mouse_get_relxy(rel_x, rel_y);
		_G(real_mouse_x) = CLIP(_G(real_mouse_x) + rel_x, _GP(mouse).ControlRect.Left, _GP(mouse).ControlRect.Right);
		_G(real_mouse_y) = CLIP(_G(real_mouse_y) + rel_y, _GP(mouse).ControlRect.Top, _GP(mouse).ControlRect.Bottom);
	} else {
		// Save real cursor coordinates provided by system
		_G(real_mouse_x) = _G(sys_mouse_x);
		_G(real_mouse_y) = _G(sys_mouse_y);
	}

	// Set new in-game cursor position
	_G(mousex) = _G(real_mouse_x);
	_G(mousey) = _G(real_mouse_y);

	if (!_G(ignore_bounds) &&
		// When applying script bounds we only do so while cursor is inside game viewport
		_GP(mouse).ControlRect.IsInside(_G(mousex), _G(mousey)) &&
		(_G(mousex) < _G(boundx1) || _G(mousey) < _G(boundy1) || _G(mousex) > _G(boundx2) || _G(mousey) > _G(boundy2))) {
		_G(mousex) = Math::Clamp(_G(mousex), _G(boundx1), _G(boundx2));
		_G(mousey) = Math::Clamp(_G(mousey), _G(boundy1), _G(boundy2));
		msetgraphpos(_G(mousex), _G(mousey));
	}

	// Convert to virtual coordinates
	_GP(mouse).WindowToGame(_G(mousex), _G(mousey));
}

void msetcursorlimit(int x1, int y1, int x2, int y2) {
	_G(boundx1) = x1;
	_G(boundy1) = y1;
	_G(boundx2) = x2;
	_G(boundy2) = y2;
}

void domouse(int str) {
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

void msetgraphpos(int xa, int ya) {
	_G(real_mouse_x) = xa;
	_G(real_mouse_y) = ya;
	sys_window_set_mouse(_G(real_mouse_x), _G(real_mouse_y));
}

void msethotspot(int xx, int yy) {
	_G(hotx) = xx;  // _G(mousex) -= _G(hotx); _G(mousey) -= _G(hoty);
	_G(hoty) = yy;  // _G(mousex) += _G(hotx); _G(mousey) += _G(hoty);
}

int minstalled() {
	// Number of buttons supported
	return 3;
}

void Mouse::WindowToGame(int &x, int &y) {
	x = _GP(GameScaling).X.UnScalePt(x) - _GP(play).GetMainViewport().Left;
	y = _GP(GameScaling).Y.UnScalePt(y) - _GP(play).GetMainViewport().Top;
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

void Mouse::SetSpeedUnit(float f) {
	SpeedUnit = f;
	Speed = SpeedVal / SpeedUnit;
}

bool Mouse::IsControlEnabled() const {
	return ControlEnabled;
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

void Mouse::UpdateGraphicArea() {
	Mouse::ControlRect = _GP(GameScaling).ScaleRange(_GP(play).GetMainViewport());
	Debug::Printf("Mouse cursor graphic area: (%d,%d)-(%d,%d) (%dx%d)",
		Mouse::ControlRect.Left, Mouse::ControlRect.Top, Mouse::ControlRect.Right, Mouse::ControlRect.Bottom,
		Mouse::ControlRect.GetWidth(), Mouse::ControlRect.GetHeight());
}

void Mouse::SetMovementControl(bool flag) {
	ControlEnabled = false;
	warning("movement control not supported, mouse control can't be enabled");
	ags_clear_mouse_movement();
}

} // namespace AGS3
