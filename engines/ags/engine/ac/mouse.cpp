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

#include "ags/engine/ac/mouse.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynobj/scriptmouse.h"
#include "ags/engine/ac/dynobj/scriptsystem.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_mouse.h"
#include "ags/engine/ac/global_plugin.h"
#include "ags/engine/ac/global_screen.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/viewframe.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/gui/guibutton.h"
#include "ags/shared/gui/guimain.h"
#include "ags/engine/device/mousew32.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/global_game.h"
#include "ags/engine/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameState play;
extern ScriptSystem scsystem;
extern SpriteCache spriteset;
extern CharacterInfo *playerchar;
extern IGraphicsDriver *gfxDriver;

extern void ags_domouse(int str);
extern int misbuttondown(int buno);

ScriptMouse scmouse;
int cur_mode, cur_cursor;
int mouse_frame = 0, mouse_delay = 0;
int lastmx = -1, lastmy = -1;
char alpha_blend_cursor = 0;
Bitmap *dotted_mouse_cursor = nullptr;
IDriverDependantBitmap *mouseCursor = nullptr;
Bitmap *blank_mouse_cursor = nullptr;

// The Mouse:: functions are static so the script doesn't pass
// in an object parameter
void Mouse_SetVisible(int isOn) {
	if (isOn)
		ShowMouseCursor();
	else
		HideMouseCursor();
}

int Mouse_GetVisible() {
	if (play.mouse_cursor_hidden)
		return 0;
	return 1;
}

void SetMouseBounds(int x1, int y1, int x2, int y2) {
	int xmax = game_to_data_coord(play.GetMainViewport().GetWidth()) - 1;
	int ymax = game_to_data_coord(play.GetMainViewport().GetHeight()) - 1;
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

	play.mboundx1 = x1;
	play.mboundx2 = x2;
	play.mboundy1 = y1;
	play.mboundy2 = y2;
	Mouse::SetMoveLimit(Rect(x1, y1, x2, y2));
}

// mouse cursor functions:
// set_mouse_cursor: changes visual appearance to specified cursor
void set_mouse_cursor(int newcurs) {
	const int hotspotx = game.mcurs[newcurs].hotx, hotspoty = game.mcurs[newcurs].hoty;
	msethotspot(hotspotx, hotspoty);

	// if it's same cursor and there's animation in progress, then don't assign a new pic just yet
	if (newcurs == cur_cursor && game.mcurs[newcurs].view >= 0 &&
		(mouse_frame > 0 || mouse_delay > 0)) {
		return;
	}

	// reset animation timing only if it's another cursor
	if (newcurs != cur_cursor) {
		cur_cursor = newcurs;
		mouse_frame = 0;
		mouse_delay = 0;
	}

	// Assign new pic
	set_new_cursor_graphic(game.mcurs[newcurs].pic);
	delete dotted_mouse_cursor;
	dotted_mouse_cursor = nullptr;

	// If it's inventory cursor, draw hotspot crosshair sprite upon it
	if ((newcurs == MODE_USE) && (game.mcurs[newcurs].pic > 0) &&
		((game.hotdot > 0) || (game.invhotdotsprite > 0))) {
		// If necessary, create a copy of the cursor and put the hotspot
		// dot onto it
		dotted_mouse_cursor = BitmapHelper::CreateBitmapCopy(_G(mousecurs)[0]);

		if (game.invhotdotsprite > 0) {
			draw_sprite_slot_support_alpha(dotted_mouse_cursor,
				(game.SpriteInfos[game.mcurs[newcurs].pic].Flags & SPF_ALPHACHANNEL) != 0,
				hotspotx - game.SpriteInfos[game.invhotdotsprite].Width / 2,
				hotspoty - game.SpriteInfos[game.invhotdotsprite].Height / 2,
				game.invhotdotsprite);
		} else {
			putpixel_compensate(dotted_mouse_cursor, hotspotx, hotspoty, MakeColor(game.hotdot));

			if (game.hotdotouter > 0) {
				int outercol = MakeColor(game.hotdotouter);

				putpixel_compensate(dotted_mouse_cursor, hotspotx + get_fixed_pixel_size(1), hotspoty, outercol);
				putpixel_compensate(dotted_mouse_cursor, hotspotx, hotspoty + get_fixed_pixel_size(1), outercol);
				putpixel_compensate(dotted_mouse_cursor, hotspotx - get_fixed_pixel_size(1), hotspoty, outercol);
				putpixel_compensate(dotted_mouse_cursor, hotspotx, hotspoty - get_fixed_pixel_size(1), outercol);
			}
		}
		_G(mousecurs)[0] = dotted_mouse_cursor;
		update_cached_mouse_cursor();
	}
}

// set_default_cursor: resets visual appearance to current mode (walk, look, etc)
void set_default_cursor() {
	set_mouse_cursor(cur_mode);
}

// permanently change cursor graphic
void ChangeCursorGraphic(int curs, int newslot) {
	if ((curs < 0) || (curs >= game.numcursors))
		quit("!ChangeCursorGraphic: invalid mouse cursor");

	if ((curs == MODE_USE) && (game.options[OPT_FIXEDINVCURSOR] == 0))
		debug_script_warn("Mouse.ChangeModeGraphic should not be used on the Inventory cursor when the cursor is linked to the active inventory item");

	game.mcurs[curs].pic = newslot;
	spriteset.Precache(newslot);
	if (curs == cur_mode)
		set_mouse_cursor(curs);
}

int Mouse_GetModeGraphic(int curs) {
	if ((curs < 0) || (curs >= game.numcursors))
		quit("!Mouse.GetModeGraphic: invalid mouse cursor");

	return game.mcurs[curs].pic;
}

void ChangeCursorHotspot(int curs, int x, int y) {
	if ((curs < 0) || (curs >= game.numcursors))
		quit("!ChangeCursorHotspot: invalid mouse cursor");
	game.mcurs[curs].hotx = data_to_game_coord(x);
	game.mcurs[curs].hoty = data_to_game_coord(y);
	if (curs == cur_cursor)
		set_mouse_cursor(cur_cursor);
}

void Mouse_ChangeModeView(int curs, int newview) {
	if ((curs < 0) || (curs >= game.numcursors))
		quit("!Mouse.ChangeModeView: invalid mouse cursor");

	newview--;

	game.mcurs[curs].view = newview;

	if (newview >= 0) {
		precache_view(newview);
	}

	if (curs == cur_cursor)
		mouse_delay = 0;  // force update
}

void SetNextCursor() {
	set_cursor_mode(find_next_enabled_cursor(cur_mode + 1));
}

void SetPreviousCursor() {
	set_cursor_mode(find_previous_enabled_cursor(cur_mode - 1));
}

// set_cursor_mode: changes mode and appearance
void set_cursor_mode(int newmode) {
	if ((newmode < 0) || (newmode >= game.numcursors))
		quit("!SetCursorMode: invalid cursor mode specified");

	guis_need_update = 1;
	if (game.mcurs[newmode].flags & MCF_DISABLED) {
		find_next_enabled_cursor(newmode);
		return;
	}
	if (newmode == MODE_USE) {
		if (playerchar->activeinv == -1) {
			find_next_enabled_cursor(0);
			return;
		}
		update_inv_cursor(playerchar->activeinv);
	}
	cur_mode = newmode;
	set_default_cursor();

	debug_script_log("Cursor mode set to %d", newmode);
}

void enable_cursor_mode(int modd) {
	game.mcurs[modd].flags &= ~MCF_DISABLED;
	// now search the interfaces for related buttons to re-enable
	int uu, ww;

	for (uu = 0; uu < game.numgui; uu++) {
		for (ww = 0; ww < guis[uu].GetControlCount(); ww++) {
			if (guis[uu].GetControlType(ww) != kGUIButton) continue;
			GUIButton *gbpt = (GUIButton *)guis[uu].GetControl(ww);
			if (gbpt->ClickAction[kMouseLeft] != kGUIAction_SetMode) continue;
			if (gbpt->ClickData[kMouseLeft] != modd) continue;
			gbpt->SetEnabled(true);
		}
	}
	guis_need_update = 1;
}

void disable_cursor_mode(int modd) {
	game.mcurs[modd].flags |= MCF_DISABLED;
	// now search the interfaces for related buttons to kill
	int uu, ww;

	for (uu = 0; uu < game.numgui; uu++) {
		for (ww = 0; ww < guis[uu].GetControlCount(); ww++) {
			if (guis[uu].GetControlType(ww) != kGUIButton) continue;
			GUIButton *gbpt = (GUIButton *)guis[uu].GetControl(ww);
			if (gbpt->ClickAction[kMouseLeft] != kGUIAction_SetMode) continue;
			if (gbpt->ClickData[kMouseLeft] != modd) continue;
			gbpt->SetEnabled(false);
		}
	}
	if (cur_mode == modd) find_next_enabled_cursor(modd);
	guis_need_update = 1;
}

void RefreshMouse() {
	ags_domouse(DOMOUSE_NOCURSOR);
	scmouse.x = game_to_data_coord(_G(mousex));
	scmouse.y = game_to_data_coord(_G(mousey));
}

void SetMousePosition(int newx, int newy) {
	const Rect &viewport = play.GetMainViewport();

	if (newx < 0)
		newx = 0;
	if (newy < 0)
		newy = 0;
	if (newx >= viewport.GetWidth())
		newx = viewport.GetWidth() - 1;
	if (newy >= viewport.GetHeight())
		newy = viewport.GetHeight() - 1;

	data_to_game_coords(&newx, &newy);
	Mouse::SetPosition(Point(newx, newy));
	RefreshMouse();
}

int GetCursorMode() {
	return cur_mode;
}

int IsButtonDown(int which) {
	if ((which < 1) || (which > 3))
		quit("!IsButtonDown: only works with eMouseLeft, eMouseRight, eMouseMiddle");
	if (misbuttondown(which - 1))
		return 1;
	return 0;
}

int IsModeEnabled(int which) {
	return (which < 0) || (which >= game.numcursors) ? 0 :
		which == MODE_USE ? playerchar->activeinv > 0 :
	(game.mcurs[which].flags & MCF_DISABLED) == 0;
}

void Mouse_EnableControl(bool on) {
	usetup.mouse_ctrl_enabled = on; // remember setting in config

	bool is_windowed = scsystem.windowed != 0;
	// Whether mouse movement should be controlled by the engine - this is
	// determined based on related config option.
	bool should_control_mouse = usetup.mouse_ctrl_when == kMouseCtrl_Always ||
		(usetup.mouse_ctrl_when == kMouseCtrl_Fullscreen && !is_windowed);
	// Whether mouse movement control is supported by the engine - this is
	// determined on per platform basis. Some builds may not have such
	// capability, e.g. because of how backend library implements mouse utils.
	bool can_control_mouse = platform->IsMouseControlSupported(is_windowed);
	// The resulting choice is made based on two aforementioned factors.
	on &= should_control_mouse && can_control_mouse;
	if (on)
		Mouse::EnableControl(!is_windowed);
	else
		Mouse::DisableControl();
}

//=============================================================================

int GetMouseCursor() {
	return cur_cursor;
}

void update_script_mouse_coords() {
	scmouse.x = game_to_data_coord(_G(mousex));
	scmouse.y = game_to_data_coord(_G(mousey));
}

void update_inv_cursor(int invnum) {

	if ((game.options[OPT_FIXEDINVCURSOR] == 0) && (invnum > 0)) {
		int cursorSprite = game.invinfo[invnum].cursorPic;

		// Fall back to the inventory pic if no cursor pic is defined.
		if (cursorSprite == 0)
			cursorSprite = game.invinfo[invnum].pic;

		game.mcurs[MODE_USE].pic = cursorSprite;
		// all cursor images must be pre-cached
		spriteset.Precache(cursorSprite);

		if ((game.invinfo[invnum].hotx > 0) || (game.invinfo[invnum].hoty > 0)) {
			// if the hotspot was set (unfortunately 0,0 isn't a valid co-ord)
			game.mcurs[MODE_USE].hotx = game.invinfo[invnum].hotx;
			game.mcurs[MODE_USE].hoty = game.invinfo[invnum].hoty;
		} else {
			game.mcurs[MODE_USE].hotx = game.SpriteInfos[cursorSprite].Width / 2;
			game.mcurs[MODE_USE].hoty = game.SpriteInfos[cursorSprite].Height / 2;
		}
	}
}

void update_cached_mouse_cursor() {
	if (mouseCursor != nullptr)
		gfxDriver->DestroyDDB(mouseCursor);
	mouseCursor = gfxDriver->CreateDDBFromBitmap(_G(mousecurs)[0], alpha_blend_cursor != 0);
}

void set_new_cursor_graphic(int spriteslot) {
	_G(mousecurs)[0] = spriteset[spriteslot];

	// It looks like spriteslot 0 can be used in games with version 2.72 and lower.
	// The NULL check should ensure that the sprite is valid anyway.
	if (((spriteslot < 1) && (loaded_game_file_version > kGameVersion_272)) || (_G(mousecurs)[0] == nullptr)) {
		if (blank_mouse_cursor == nullptr) {
			blank_mouse_cursor = BitmapHelper::CreateTransparentBitmap(1, 1, game.GetColorDepth());
		}
		_G(mousecurs)[0] = blank_mouse_cursor;
	}

	if (game.SpriteInfos[spriteslot].Flags & SPF_ALPHACHANNEL)
		alpha_blend_cursor = 1;
	else
		alpha_blend_cursor = 0;

	update_cached_mouse_cursor();
}

bool is_standard_cursor_enabled(int curs) {
	if ((game.mcurs[curs].flags & MCF_DISABLED) == 0) {
		// inventory cursor, and they have an active item
		if (curs == MODE_USE) {
			if (playerchar->activeinv > 0)
				return true;
		}
		// standard cursor that's not disabled, go with it
		else if (game.mcurs[curs].flags & MCF_STANDARD)
			return true;
	}
	return false;
}

int find_next_enabled_cursor(int startwith) {
	if (startwith >= game.numcursors)
		startwith = 0;
	int testing = startwith;
	do {
		if (is_standard_cursor_enabled(testing)) break;
		testing++;
		if (testing >= game.numcursors) testing = 0;
	} while (testing != startwith);

	if (testing != startwith)
		set_cursor_mode(testing);

	return testing;
}

int find_previous_enabled_cursor(int startwith) {
	if (startwith < 0)
		startwith = game.numcursors - 1;
	int testing = startwith;
	do {
		if (is_standard_cursor_enabled(testing)) break;
		testing--;
		if (testing < 0) testing = game.numcursors - 1;
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
RuntimeScriptValue Sc_Mouse_ChangeModeView(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT2(Mouse_ChangeModeView);
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
	API_SCALL_VOID_PINT(PluginSimulateMouseClick);
}

RuntimeScriptValue Sc_Mouse_GetControlEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL(Mouse::IsControlEnabled);
}

RuntimeScriptValue Sc_Mouse_SetControlEnabled(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PBOOL(Mouse_EnableControl);
}


RuntimeScriptValue Sc_Mouse_GetSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_FLOAT(Mouse::GetSpeed);
}

RuntimeScriptValue Sc_Mouse_SetSpeed(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_VARIABLE_VALUE("Mouse::Speed");
	Mouse::SetSpeed(params[0].FValue);
	return RuntimeScriptValue();
}

void RegisterMouseAPI() {
	ccAddExternalStaticFunction("Mouse::ChangeModeGraphic^2", Sc_ChangeCursorGraphic);
	ccAddExternalStaticFunction("Mouse::ChangeModeHotspot^3", Sc_ChangeCursorHotspot);
	ccAddExternalStaticFunction("Mouse::ChangeModeView^2", Sc_Mouse_ChangeModeView);
	ccAddExternalStaticFunction("Mouse::Click^1", Sc_Mouse_Click);
	ccAddExternalStaticFunction("Mouse::DisableMode^1", Sc_disable_cursor_mode);
	ccAddExternalStaticFunction("Mouse::EnableMode^1", Sc_enable_cursor_mode);
	ccAddExternalStaticFunction("Mouse::GetModeGraphic^1", Sc_Mouse_GetModeGraphic);
	ccAddExternalStaticFunction("Mouse::IsButtonDown^1", Sc_IsButtonDown);
	ccAddExternalStaticFunction("Mouse::IsModeEnabled^1", Sc_IsModeEnabled);
	ccAddExternalStaticFunction("Mouse::SaveCursorUntilItLeaves^0", Sc_SaveCursorForLocationChange);
	ccAddExternalStaticFunction("Mouse::SelectNextMode^0", Sc_SetNextCursor);
	ccAddExternalStaticFunction("Mouse::SelectPreviousMode^0", Sc_SetPreviousCursor);
	ccAddExternalStaticFunction("Mouse::SetBounds^4", Sc_SetMouseBounds);
	ccAddExternalStaticFunction("Mouse::SetPosition^2", Sc_SetMousePosition);
	ccAddExternalStaticFunction("Mouse::Update^0", Sc_RefreshMouse);
	ccAddExternalStaticFunction("Mouse::UseDefaultGraphic^0", Sc_set_default_cursor);
	ccAddExternalStaticFunction("Mouse::UseModeGraphic^1", Sc_set_mouse_cursor);
	ccAddExternalStaticFunction("Mouse::get_ControlEnabled", Sc_Mouse_GetControlEnabled);
	ccAddExternalStaticFunction("Mouse::set_ControlEnabled", Sc_Mouse_SetControlEnabled);
	ccAddExternalStaticFunction("Mouse::get_Mode", Sc_GetCursorMode);
	ccAddExternalStaticFunction("Mouse::set_Mode", Sc_set_cursor_mode);
	ccAddExternalStaticFunction("Mouse::get_Speed", Sc_Mouse_GetSpeed);
	ccAddExternalStaticFunction("Mouse::set_Speed", Sc_Mouse_SetSpeed);
	ccAddExternalStaticFunction("Mouse::get_Visible", Sc_Mouse_GetVisible);
	ccAddExternalStaticFunction("Mouse::set_Visible", Sc_Mouse_SetVisible);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("Mouse::ChangeModeGraphic^2", (void *)ChangeCursorGraphic);
	ccAddExternalFunctionForPlugin("Mouse::ChangeModeHotspot^3", (void *)ChangeCursorHotspot);
	ccAddExternalFunctionForPlugin("Mouse::ChangeModeView^2", (void *)Mouse_ChangeModeView);
	ccAddExternalFunctionForPlugin("Mouse::DisableMode^1", (void *)disable_cursor_mode);
	ccAddExternalFunctionForPlugin("Mouse::EnableMode^1", (void *)enable_cursor_mode);
	ccAddExternalFunctionForPlugin("Mouse::GetModeGraphic^1", (void *)Mouse_GetModeGraphic);
	ccAddExternalFunctionForPlugin("Mouse::IsButtonDown^1", (void *)IsButtonDown);
	ccAddExternalFunctionForPlugin("Mouse::IsModeEnabled^1", (void *)IsModeEnabled);
	ccAddExternalFunctionForPlugin("Mouse::SaveCursorUntilItLeaves^0", (void *)SaveCursorForLocationChange);
	ccAddExternalFunctionForPlugin("Mouse::SelectNextMode^0", (void *)SetNextCursor);
	ccAddExternalFunctionForPlugin("Mouse::SelectPreviousMode^0", (void *)SetPreviousCursor);
	ccAddExternalFunctionForPlugin("Mouse::SetBounds^4", (void *)SetMouseBounds);
	ccAddExternalFunctionForPlugin("Mouse::SetPosition^2", (void *)SetMousePosition);
	ccAddExternalFunctionForPlugin("Mouse::Update^0", (void *)RefreshMouse);
	ccAddExternalFunctionForPlugin("Mouse::UseDefaultGraphic^0", (void *)set_default_cursor);
	ccAddExternalFunctionForPlugin("Mouse::UseModeGraphic^1", (void *)set_mouse_cursor);
	ccAddExternalFunctionForPlugin("Mouse::get_Mode", (void *)GetCursorMode);
	ccAddExternalFunctionForPlugin("Mouse::set_Mode", (void *)set_cursor_mode);
	ccAddExternalFunctionForPlugin("Mouse::get_Visible", (void *)Mouse_GetVisible);
	ccAddExternalFunctionForPlugin("Mouse::set_Visible", (void *)Mouse_SetVisible);
}

} // namespace AGS3
