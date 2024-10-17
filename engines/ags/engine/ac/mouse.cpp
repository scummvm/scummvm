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

#include "ags/engine/ac/mouse.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynobj/script_mouse.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_mouse.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/view_frame.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/gui/gui_button.h"
#include "ags/shared/gui/gui_main.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/global_game.h"
#include "ags/plugins/ags_plugin.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

// The mouse functions are static so the script doesn't pass
// in an object parameter
void Mouse_SetVisible(int isOn) {
	if (isOn)
		ShowMouseCursor();
	else
		HideMouseCursor();
}

int Mouse_GetVisible() {
	if (_GP(play).mouse_cursor_hidden)
		return 0;
	return 1;
}

void SetMouseBounds(int x1, int y1, int x2, int y2) {
	int xmax = game_to_data_coord(_GP(play).GetMainViewport().GetWidth()) - 1;
	int ymax = game_to_data_coord(_GP(play).GetMainViewport().GetHeight()) - 1;
	if ((x1 == 0) && (y1 == 0) && (x2 == 0) && (y2 == 0)) {
		x2 = xmax;
		y2 = ymax;
	} else {
		if (x1 < 0 || x1 > xmax || x2 < 0 || x2 > xmax || x1 > x2 || y1 < 0 || y1 > ymax || y2 < 0 || y2 > ymax || y1 > y2)
			debug_script_warn("SetMouseBounds: arguments are out of range and will be corrected: (%d,%d)-(%d,%d), range is (%d,%d)-(%d,%d)",
			                  x1, y1, x2, y2, 0, 0, xmax, ymax);
		x1 = Math::Clamp(x1, 0, xmax);
		x2 = Math::Clamp(x2, x1, xmax);
		y1 = Math::Clamp(y1, 0, ymax);
		y2 = Math::Clamp(y2, y1, ymax);
	}

	debug_script_log("Mouse bounds constrained to (%d,%d)-(%d,%d)", x1, y1, x2, y2);
	data_to_game_coords(&x1, &y1);
	data_to_game_round_up(&x2, &y2);

	_GP(play).mboundx1 = x1;
	_GP(play).mboundx2 = x2;
	_GP(play).mboundy1 = y1;
	_GP(play).mboundy2 = y2;
	_GP(mouse).SetMoveLimit(Rect(x1, y1, x2, y2));
}

// mouse cursor functions:
// set_mouse_cursor: changes visual appearance to specified cursor
void set_mouse_cursor(int newcurs, bool force_update) {
	const int hotspotx = _GP(game).mcurs[newcurs].hotx, hotspoty = _GP(game).mcurs[newcurs].hoty;
	_GP(mouse).SetHotspot(hotspotx, hotspoty);

	// if it's same cursor and there's animation in progress, then don't assign a new pic just yet
	if (!force_update && newcurs == _G(cur_cursor) && _GP(game).mcurs[newcurs].view >= 0 && (_G(mouse_frame) > 0 || _G(mouse_delay) > 0)) {
		return;
	}

	// reset animation timing only if it's another cursor
	if (newcurs != _G(cur_cursor)) {
		_G(cur_cursor) = newcurs;
		_G(mouse_frame) = 0;
		_G(mouse_delay) = 0;
	}

	// Assign new pic
	set_new_cursor_graphic(_GP(game).mcurs[newcurs].pic);
	delete _G(dotted_mouse_cursor);
	_G(dotted_mouse_cursor) = nullptr;

	// If it's inventory cursor, draw hotspot crosshair sprite upon it
	if ((newcurs == MODE_USE) && (_GP(game).mcurs[newcurs].pic > 0) &&
	        ((_GP(game).hotdot > 0) || (_GP(game).invhotdotsprite > 0))) {
		// If necessary, create a copy of the cursor and put the hotspot
		// dot onto it
		_G(dotted_mouse_cursor) = BitmapHelper::CreateBitmapCopy(_G(mousecurs)[0]);

		if (_GP(game).invhotdotsprite > 0) {
			draw_sprite_slot_support_alpha(_G(dotted_mouse_cursor),
			                               (_GP(game).SpriteInfos[_GP(game).mcurs[newcurs].pic].Flags & SPF_ALPHACHANNEL) != 0,
			                               hotspotx - _GP(game).SpriteInfos[_GP(game).invhotdotsprite].Width / 2,
			                               hotspoty - _GP(game).SpriteInfos[_GP(game).invhotdotsprite].Height / 2,
			                               _GP(game).invhotdotsprite);
		} else {
			putpixel_compensate(_G(dotted_mouse_cursor), hotspotx, hotspoty, MakeColor(_GP(game).hotdot));

			if (_GP(game).hotdotouter > 0) {
				int outercol = MakeColor(_GP(game).hotdotouter);

				putpixel_compensate(_G(dotted_mouse_cursor), hotspotx + get_fixed_pixel_size(1), hotspoty, outercol);
				putpixel_compensate(_G(dotted_mouse_cursor), hotspotx, hotspoty + get_fixed_pixel_size(1), outercol);
				putpixel_compensate(_G(dotted_mouse_cursor), hotspotx - get_fixed_pixel_size(1), hotspoty, outercol);
				putpixel_compensate(_G(dotted_mouse_cursor), hotspotx, hotspoty - get_fixed_pixel_size(1), outercol);
			}
		}
		_G(mousecurs)[0] = _G(dotted_mouse_cursor);
		update_cached_mouse_cursor();
	}
}

// set_default_cursor: resets visual appearance to current mode (walk, look, etc)
void set_default_cursor() {
	set_mouse_cursor(_G(cur_mode));
}

// permanently change cursor graphic
void ChangeCursorGraphic(int curs, int newslot) {
	if ((curs < 0) || (curs >= _GP(game).numcursors))
		quit("!ChangeCursorGraphic: invalid mouse cursor");

	if ((curs == MODE_USE) && (_GP(game).options[OPT_FIXEDINVCURSOR] == 0))
		debug_script_warn("Mouse.ChangeModeGraphic should not be used on the Inventory cursor when the cursor is linked to the active inventory item");

	_GP(game).mcurs[curs].pic = newslot;
	_GP(spriteset).PrecacheSprite(newslot);
	if (curs == _G(cur_mode))
		set_mouse_cursor(curs);
}

int Mouse_GetModeGraphic(int curs) {
	if ((curs < 0) || (curs >= _GP(game).numcursors))
		quit("!Mouse.GetModeGraphic: invalid mouse cursor");

	return _GP(game).mcurs[curs].pic;
}

void ChangeCursorHotspot(int curs, int x, int y) {
	if ((curs < 0) || (curs >= _GP(game).numcursors))
		quit("!ChangeCursorHotspot: invalid mouse cursor");
	_GP(game).mcurs[curs].hotx = data_to_game_coord(x);
	_GP(game).mcurs[curs].hoty = data_to_game_coord(y);
	if (curs == _G(cur_cursor))
		set_mouse_cursor(_G(cur_cursor));
}

void Mouse_ChangeModeView(int curs, int newview, int delay) {
	if ((curs < 0) || (curs >= _GP(game).numcursors))
		quit("!Mouse.ChangeModeView: invalid mouse cursor");

	newview--;

	_GP(game).mcurs[curs].view = newview;
	if (delay != SCR_NO_VALUE)
		_GP(game).mcurs[curs].animdelay = delay;

	if (newview >= 0) {
		precache_view(newview);
	}

	if (curs == _G(cur_cursor))
		_G(mouse_delay) = 0;  // force update
}

void Mouse_ChangeModeView2(int curs, int newview) {
	Mouse_ChangeModeView(curs, newview, SCR_NO_VALUE);
}

void SetNextCursor() {
	set_cursor_mode(find_next_enabled_cursor(_G(cur_mode) + 1));
}

void SetPreviousCursor() {
	set_cursor_mode(find_previous_enabled_cursor(_G(cur_mode) - 1));
}

// set_cursor_mode: changes mode and appearance
void set_cursor_mode(int newmode) {
	if ((newmode < 0) || (newmode >= _GP(game).numcursors))
		quit("!SetCursorMode: invalid cursor mode specified");

	if (_GP(game).mcurs[newmode].flags & MCF_DISABLED) {
		find_next_enabled_cursor(newmode);
		return;
	}
	if (newmode == MODE_USE) {
		if (_G(playerchar)->activeinv == -1) {
			find_next_enabled_cursor(0);
			return;
		}
		update_inv_cursor(_G(playerchar)->activeinv);
	}
	_G(cur_mode) = newmode;
	set_default_cursor();

	debug_script_log("Cursor mode set to %d", newmode);
}

void enable_cursor_mode(int modd) {
	if (modd < 0 || modd >= (int)_GP(game).mcurs.size()) {
		warning("Attempt to enable invalid cursor (%d), ignoring", modd);
		return;
	}
	_GP(game).mcurs[modd].flags &= ~MCF_DISABLED;
	// now search the interfaces for related buttons to re-enable
	int uu, ww;

	for (uu = 0; uu < _GP(game).numgui; uu++) {
		for (ww = 0; ww < _GP(guis)[uu].GetControlCount(); ww++) {
			if (_GP(guis)[uu].GetControlType(ww) != kGUIButton) continue;
			GUIButton *gbpt = (GUIButton *)_GP(guis)[uu].GetControl(ww);
			if (gbpt->ClickAction[kGUIClickLeft] != kGUIAction_SetMode) continue;
			if (gbpt->ClickData[kGUIClickLeft] != modd) continue;
			gbpt->SetEnabled(true);
		}
	}
}

void disable_cursor_mode(int modd) {
	_GP(game).mcurs[modd].flags |= MCF_DISABLED;
	// now search the interfaces for related buttons to kill
	int uu, ww;

	for (uu = 0; uu < _GP(game).numgui; uu++) {
		for (ww = 0; ww < _GP(guis)[uu].GetControlCount(); ww++) {
			if (_GP(guis)[uu].GetControlType(ww) != kGUIButton) continue;
			GUIButton *gbpt = (GUIButton *)_GP(guis)[uu].GetControl(ww);
			if (gbpt->ClickAction[kGUIClickLeft] != kGUIAction_SetMode) continue;
			if (gbpt->ClickData[kGUIClickLeft] != modd) continue;
			gbpt->SetEnabled(false);
		}
	}
	if (_G(cur_mode) == modd) find_next_enabled_cursor(modd);
}

void RefreshMouse() {
	ags_domouse();
	_GP(scmouse).x = game_to_data_coord(_G(mousex));
	_GP(scmouse).y = game_to_data_coord(_G(mousey));
}

void SetMousePosition(int newx, int newy) {
	const Rect &viewport = _GP(play).GetMainViewport();

	if (newx < 0)
		newx = 0;
	if (newy < 0)
		newy = 0;
	if (newx >= viewport.GetWidth())
		newx = viewport.GetWidth() - 1;
	if (newy >= viewport.GetHeight())
		newy = viewport.GetHeight() - 1;

	data_to_game_coords(&newx, &newy);
	_GP(mouse).SetPosition(Point(newx, newy));
	RefreshMouse();
}

int GetCursorMode() {
	return _G(cur_mode);
}

int IsButtonDown(int which) {
	if ((which < kMouseLeft) || (which > kMouseMiddle))
		quit("!IsButtonDown: only works with eMouseLeft, eMouseRight, eMouseMiddle");
	return ags_misbuttondown(static_cast<eAGSMouseButton>(which)) ? 1 : 0;
}

int IsModeEnabled(int which) {
	return (which < 0) || (which >= _GP(game).numcursors) ? 0 :
	       which == MODE_USE ? _G(playerchar)->activeinv > 0 :
	       (_GP(game).mcurs[which].flags & MCF_DISABLED) == 0;
}

void SimulateMouseClick(int button_id) {
	_G(simulatedClick) = static_cast<eAGSMouseButton>(button_id);
}

void Mouse_EnableControl(bool on) {
	bool should_control_mouse =
	    _GP(usetup).mouse_ctrl_when == kMouseCtrl_Always ||
	    (_GP(usetup).mouse_ctrl_when == kMouseCtrl_Fullscreen && (_GP(scsystem).windowed == 0));
	_GP(mouse).SetMovementControl(should_control_mouse & on);
	_GP(usetup).mouse_ctrl_enabled = on; // remember setting in config
}

bool Mouse_GetAutoLock() {
	return _GP(usetup).mouse_auto_lock;
}

void Mouse_SetAutoLock(bool on) {
	_GP(usetup).mouse_auto_lock = on;
	if (_GP(scsystem).windowed) {
		if (_GP(usetup).mouse_auto_lock)
			_GP(mouse).TryLockToWindow();
		else
			_GP(mouse).UnlockFromWindow();
	}
}

//=============================================================================

int GetMouseCursor() {
	return _G(cur_cursor);
}

void update_script_mouse_coords() {
	_GP(scmouse).x = game_to_data_coord(_G(mousex));
	_GP(scmouse).y = game_to_data_coord(_G(mousey));
}

void update_inv_cursor(int invnum) {

	if ((_GP(game).options[OPT_FIXEDINVCURSOR] == 0) && (invnum > 0)) {
		int cursorSprite = _GP(game).invinfo[invnum].cursorPic;

		// Fall back to the inventory pic if no cursor pic is defined.
		if (cursorSprite == 0)
			cursorSprite = _GP(game).invinfo[invnum].pic;

		_GP(game).mcurs[MODE_USE].pic = cursorSprite;
		// all cursor images must be pre-cached
		_GP(spriteset).PrecacheSprite(cursorSprite);

		if ((_GP(game).invinfo[invnum].hotx > 0) || (_GP(game).invinfo[invnum].hoty > 0)) {
			// if the hotspot was set (unfortunately 0,0 isn't a valid co-ord)
			_GP(game).mcurs[MODE_USE].hotx = _GP(game).invinfo[invnum].hotx;
			_GP(game).mcurs[MODE_USE].hoty = _GP(game).invinfo[invnum].hoty;
		} else {
			_GP(game).mcurs[MODE_USE].hotx = _GP(game).SpriteInfos[cursorSprite].Width / 2;
			_GP(game).mcurs[MODE_USE].hoty = _GP(game).SpriteInfos[cursorSprite].Height / 2;
		}
	}
}

void update_cached_mouse_cursor() {
	if (_G(mouseCursor) != nullptr)
		_G(gfxDriver)->DestroyDDB(_G(mouseCursor));
	_G(mouseCursor) = _G(gfxDriver)->CreateDDBFromBitmap(_G(mousecurs)[0], _G(alpha_blend_cursor) != 0);
}

void set_new_cursor_graphic(int spriteslot) {
	_G(mousecurs)[0] = _GP(spriteset)[spriteslot];

	// It looks like spriteslot 0 can be used in games with version 2.72 and lower.
	// The NULL check should ensure that the sprite is valid anyway.
	if (((spriteslot < 1) && (_G(loaded_game_file_version) > kGameVersion_272)) || (_G(mousecurs)[0] == nullptr)) {
		if (_G(blank_mouse_cursor) == nullptr) {
			_G(blank_mouse_cursor) = BitmapHelper::CreateTransparentBitmap(1, 1, _GP(game).GetColorDepth());
		}
		_G(mousecurs)[0] = _G(blank_mouse_cursor);
	}

	if (_GP(game).SpriteInfos[spriteslot].Flags & SPF_ALPHACHANNEL)
		_G(alpha_blend_cursor) = 1;
	else
		_G(alpha_blend_cursor) = 0;

	update_cached_mouse_cursor();
}

bool is_standard_cursor_enabled(int curs) {
	if ((_GP(game).mcurs[curs].flags & MCF_DISABLED) == 0) {
		// inventory cursor, and they have an active item
		if (curs == MODE_USE) {
			if (_G(playerchar)->activeinv > 0)
				return true;
		}
		// standard cursor that's not disabled, go with it
		else if (_GP(game).mcurs[curs].flags & MCF_STANDARD)
			return true;
	}
	return false;
}

int find_next_enabled_cursor(int startwith) {
	if (startwith >= _GP(game).numcursors)
		startwith = 0;
	int testing = startwith;
	do {
		if (is_standard_cursor_enabled(testing)) break;
		testing++;
		if (testing >= _GP(game).numcursors) testing = 0;
	} while (testing != startwith);

	if (testing != startwith)
		set_cursor_mode(testing);

	return testing;
}

int find_previous_enabled_cursor(int startwith) {
	if (startwith < 0)
		startwith = _GP(game).numcursors - 1;
	int testing = startwith;
	do {
		if (is_standard_cursor_enabled(testing)) break;
		testing--;
		if (testing < 0) testing = _GP(game).numcursors - 1;
	} while (testing != startwith);

	if (testing != startwith)
		set_cursor_mode(testing);

	return testing;
}


//=============================================================================
//
// Script API Functions
//
//=============================================================================

// void  (int curs, int newslot)
RuntimeScriptValue Sc_ChangeCursorGraphic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(ChangeCursorGraphic);
}

// void  (int curs, int x, int y)
RuntimeScriptValue Sc_ChangeCursorHotspot(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(ChangeCursorHotspot);
}

// void (int curs, int newview)
RuntimeScriptValue Sc_Mouse_ChangeModeView2(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(Mouse_ChangeModeView2);
}

RuntimeScriptValue Sc_Mouse_ChangeModeView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT3(Mouse_ChangeModeView);
}

// void (int modd)
RuntimeScriptValue Sc_disable_cursor_mode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(disable_cursor_mode);
}

// void (int modd)
RuntimeScriptValue Sc_enable_cursor_mode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(enable_cursor_mode);
}

// int (int curs)
RuntimeScriptValue Sc_Mouse_GetModeGraphic(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(Mouse_GetModeGraphic);
}

// int (int which)
RuntimeScriptValue Sc_IsButtonDown(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsButtonDown);
}

// int (int which)
RuntimeScriptValue Sc_IsModeEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT_PINT(IsModeEnabled);
}

// void ();
RuntimeScriptValue Sc_SaveCursorForLocationChange(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(SaveCursorForLocationChange);
}

// void  ()
RuntimeScriptValue Sc_SetNextCursor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(SetNextCursor);
}

// void  ()
RuntimeScriptValue Sc_SetPreviousCursor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(SetPreviousCursor);
}

// void  (int x1, int y1, int x2, int y2)
RuntimeScriptValue Sc_SetMouseBounds(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT4(SetMouseBounds);
}

// void  (int newx, int newy)
RuntimeScriptValue Sc_SetMousePosition(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(SetMousePosition);
}

// void ()
RuntimeScriptValue Sc_RefreshMouse(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(RefreshMouse);
}

// void ()
RuntimeScriptValue Sc_set_default_cursor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(set_default_cursor);
}

// void (int newcurs)
RuntimeScriptValue Sc_set_mouse_cursor(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(set_mouse_cursor);
}

// int ()
RuntimeScriptValue Sc_GetCursorMode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetCursorMode);
}

// void (int newmode)
RuntimeScriptValue Sc_set_cursor_mode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(set_cursor_mode);
}

// int ()
RuntimeScriptValue Sc_Mouse_GetVisible(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Mouse_GetVisible);
}

// void (int isOn)
RuntimeScriptValue Sc_Mouse_SetVisible(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Mouse_SetVisible);
}

RuntimeScriptValue Sc_Mouse_Click(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(SimulateMouseClick);
}

RuntimeScriptValue Sc_Mouse_GetControlEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL(_GP(mouse).IsControlEnabled);
}

RuntimeScriptValue Sc_Mouse_SetControlEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PBOOL(Mouse_EnableControl);
}

RuntimeScriptValue Sc_Mouse_GetAutoLock(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL(Mouse_GetAutoLock);
}

RuntimeScriptValue Sc_Mouse_SetAutoLock(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PBOOL(Mouse_SetAutoLock);
}

RuntimeScriptValue Sc_Mouse_GetSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_FLOAT(_GP(mouse).GetSpeed);
}

RuntimeScriptValue Sc_Mouse_SetSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_PARAM_COUNT("Mouse::Speed", 1);
	_GP(mouse).SetSpeed(params[0].FValue);
	return RuntimeScriptValue();
}

void RegisterMouseAPI() {
	ScFnRegister mouse_api[] = {
		{"Mouse::ChangeModeGraphic^2", API_FN_PAIR(ChangeCursorGraphic)},
		{"Mouse::ChangeModeHotspot^3", API_FN_PAIR(ChangeCursorHotspot)},
		{"Mouse::ChangeModeView^2", API_FN_PAIR(Mouse_ChangeModeView2)},
		{"Mouse::ChangeModeView^3", API_FN_PAIR(Mouse_ChangeModeView)},
		{"Mouse::Click^1", Sc_Mouse_Click},
		{"Mouse::DisableMode^1", API_FN_PAIR(disable_cursor_mode)},
		{"Mouse::EnableMode^1", API_FN_PAIR(enable_cursor_mode)},
		{"Mouse::GetModeGraphic^1", API_FN_PAIR(Mouse_GetModeGraphic)},
		{"Mouse::IsButtonDown^1", API_FN_PAIR(IsButtonDown)},
		{"Mouse::IsModeEnabled^1", API_FN_PAIR(IsModeEnabled)},
		{"Mouse::SaveCursorUntilItLeaves^0", API_FN_PAIR(SaveCursorForLocationChange)},
		{"Mouse::SelectNextMode^0", API_FN_PAIR(SetNextCursor)},
		{"Mouse::SelectPreviousMode^0", API_FN_PAIR(SetPreviousCursor)},
		{"Mouse::SetBounds^4", API_FN_PAIR(SetMouseBounds)},
		{"Mouse::SetPosition^2", API_FN_PAIR(SetMousePosition)},
		{"Mouse::Update^0", API_FN_PAIR(RefreshMouse)},
		{"Mouse::UseDefaultGraphic^0", API_FN_PAIR(set_default_cursor)},
		{"Mouse::UseModeGraphic^1", API_FN_PAIR(set_mouse_cursor)},
		{"Mouse::get_AutoLock", API_FN_PAIR(Mouse_GetAutoLock)},
		{"Mouse::set_AutoLock", API_FN_PAIR(Mouse_SetAutoLock)},
		{"Mouse::get_ControlEnabled", Sc_Mouse_GetControlEnabled},
		{"Mouse::set_ControlEnabled", Sc_Mouse_SetControlEnabled},
		{"Mouse::get_Mode", API_FN_PAIR(GetCursorMode)},
		{"Mouse::set_Mode", API_FN_PAIR(set_cursor_mode)},
		{"Mouse::get_Speed", Sc_Mouse_GetSpeed},
		{"Mouse::set_Speed", Sc_Mouse_SetSpeed},
		{"Mouse::get_Visible", API_FN_PAIR(Mouse_GetVisible)},
		{"Mouse::set_Visible", API_FN_PAIR(Mouse_SetVisible)},
	};

	ccAddExternalFunctions361(mouse_api);
}

} // namespace AGS3
