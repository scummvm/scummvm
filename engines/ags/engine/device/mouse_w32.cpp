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

void Mouse::Poll() {
	// TODO:
	// disable or update mouse speed control to sdl
	// (does sdl support mouse cursor speed? is it even necessary anymore?);

	// TODO: [sonneveld] find out where mgetgraphpos is needed, are events polled before that?
	sys_evt_process_pending();

	if (_G(switched_away))
		return;

	// Save absolute cursor coordinates provided by system
	// NOTE: relative motion and the speed factor should already be applied by SDL2 or our custom devices.
	_G(real_mouse_x) = CLIP((int)_G(sys_mouse_x), _GP(mouse).ControlRect.Left, _GP(mouse).ControlRect.Right);
	_G(real_mouse_y) = CLIP((int)_G(sys_mouse_y), _GP(mouse).ControlRect.Top, _GP(mouse).ControlRect.Bottom);

	// Set new in-game cursor position, convert to the in-game logic coordinates
	_G(mousex) = _G(real_mouse_x);
	_G(mousey) = _G(real_mouse_y);
	if (!_G(ignore_bounds) &&
		// When applying script bounds we only do so while cursor is inside game viewport
		_GP(mouse).ControlRect.IsInside(_G(mousex), _G(mousey)) &&
		(_G(mousex) < _G(boundx1) || _G(mousey) < _G(boundy1) || _G(mousex) > _G(boundx2) || _G(mousey) > _G(boundy2))) {
		_G(mousex) = Math::Clamp(_G(mousex), _G(boundx1), _G(boundx2));
		_G(mousey) = Math::Clamp(_G(mousey), _G(boundy1), _G(boundy2));
		_GP(mouse).SetSysPosition(_G(mousex), _G(mousey));
	}
	// Convert to virtual coordinates
	_GP(mouse).WindowToGame(_G(mousex), _G(mousey));
}

void Mouse::SetSysPosition(int x, int y) {
	_G(sys_mouse_x) = x;
	_G(sys_mouse_y) = y;
	_G(real_mouse_x) = x;
	_G(real_mouse_y) = y;
	sys_window_set_mouse(_G(real_mouse_x), _G(real_mouse_y));
}

void Mouse::SetHotspot(int x, int y) {
	_G(hotx) = x;
	_G(hoty) = y;
}

int Mouse::GetButtonCount() {
	// TODO: can SDL tell number of available/supported buttons at all, or whether mouse is present?
	// this is not that critical, but maybe some game devs would like to detect if player has or not a mouse.
	return 3; // SDL *theoretically* support 3 mouse buttons, but that does not mean they are physically present...
}

void Mouse::WindowToGame(int &x, int &y) {
	x = _GP(GameScaling).X.UnScalePt(x) - _GP(play).GetMainViewport().Left;
	y = _GP(GameScaling).Y.UnScalePt(y) - _GP(play).GetMainViewport().Top;
}

void Mouse::SetMoveLimit(const Rect &r) {
	Rect src_r = OffsetRect(r, _GP(play).GetMainViewport().GetLT());
	Rect dst_r = _GP(GameScaling).ScaleRange(src_r);
	_G(boundx1) = dst_r.Left;
	_G(boundy1) = dst_r.Top;
	_G(boundx2) = dst_r.Right;
	_G(boundy2) = dst_r.Bottom;
}

void Mouse::SetPosition(const Point &p) {
	_GP(mouse).SetSysPosition(_GP(GameScaling).X.ScalePt(p.X + _GP(play).GetMainViewport().Left),
							  _GP(GameScaling).Y.ScalePt(p.Y + _GP(play).GetMainViewport().Top));
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
	SpeedVal = MAX(0.f, speed);
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
