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

#include "ags/engine/ac/global_debug.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/move_list.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/translation.h"
#include "ags/engine/ac/walkable_area.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/engine/gui/gui_dialog.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/script/cc_common.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/main/graphics_mode.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

String GetRuntimeInfo() {
	DisplayMode mode = _G(gfxDriver)->GetDisplayMode();
	Rect render_frame = _G(gfxDriver)->GetRenderDestination();
	PGfxFilter filter = _G(gfxDriver)->GetGraphicsFilter();
	const size_t total_spr =  _GP(spriteset).GetCacheSize();
	const size_t total_lockspr =  _GP(spriteset).GetLockedSize();
	const size_t total_normspr = total_spr - total_lockspr;
	const size_t max_normspr =  _GP(spriteset).GetMaxCacheSize() - total_lockspr;
	const unsigned norm_spr_filled = (uint64_t)total_normspr * 100 / max_normspr;
	String runtimeInfo = String::FromFormat(
		"%s[Engine version %s"
		"[Game resolution %d x %d (%d-bit)"
		"[Running %d x %d at %d-bit%s[GFX: %s; %s[Draw frame %d x %d["
		"Sprite cache KB: %zu, norm: %zu / %zu (%u%%), locked: %zu",
		get_engine_name(),
		get_engine_version_and_build().GetCStr(),
		_GP(game).GetGameRes().Width, _GP(game).GetGameRes().Height, _GP(game).GetColorDepth(),
		mode.Width, mode.Height, mode.ColorDepth,
		mode.IsWindowed() ? " W" : "",
		_G(gfxDriver)->GetDriverName(), filter->GetInfo().Name.GetCStr(),
		render_frame.GetWidth(), render_frame.GetHeight(),
		total_spr / 1024, total_normspr / 1024, max_normspr / 1024, norm_spr_filled, total_lockspr / 1024);
	if (_GP(play).separate_music_lib)
		runtimeInfo.Append("[AUDIO.VOX enabled");
	if (_GP(play).voice_avail)
		runtimeInfo.Append("[SPEECH.VOX enabled");
	if (get_translation_tree().size() > 0) {
		runtimeInfo.Append("[Using translation ");
		runtimeInfo.Append(get_translation_name());
	}

	return runtimeInfo;
}

void script_debug(int cmdd, int dataa) {
	if (_GP(play).debug_mode == 0) return;
	int rr;
	if (cmdd == 0) {
		for (rr = 1; rr < _GP(game).numinvitems; rr++)
			_G(playerchar)->inv[rr] = 1;
		update_invorder();
		//    Display("invorder decided there are %d items[display %d",_GP(play).inv_numorder,_GP(play).inv_numdisp);
	} else if (cmdd == 1) {
		String toDisplay = GetRuntimeInfo();
		DisplayMB(toDisplay.GetCStr());
		//    Display("shftR: %d  shftG: %d  shftB: %d", _G(_rgb_r_shift_16), _G(_rgb_g_shift_16), _G(_rgb_b_shift_16));
		//    Display("Remaining memory: %d kb",_go32_dpmi_remaining_virtual_memory()/1024);
		//Display("Play char bcd: %d",->GetColorDepth(_GP(spriteset)[_GP(views)[_G(playerchar)->view].frames[_G(playerchar)->loop][_G(playerchar)->frame].pic]));
	} else if (cmdd == 2) {  // show walkable areas from here
		// TODO: support multiple viewports?!
		const int viewport_index = 0;
		const int camera_index = 0;
		Bitmap *tempw = BitmapHelper::CreateBitmap(_GP(thisroom).WalkAreaMask->GetWidth(), _GP(thisroom).WalkAreaMask->GetHeight());
		tempw->Blit(prepare_walkable_areas(-1), 0, 0, 0, 0, tempw->GetWidth(), tempw->GetHeight());
		const Rect &viewport = _GP(play).GetRoomViewport(viewport_index)->GetRect();
		const Rect &camera = _GP(play).GetRoomCamera(camera_index)->GetRect();
		Bitmap *view_bmp = BitmapHelper::CreateBitmap(viewport.GetWidth(), viewport.GetHeight());
		Rect mask_src = Rect(camera.Left / _GP(thisroom).MaskResolution, camera.Top / _GP(thisroom).MaskResolution, camera.Right / _GP(thisroom).MaskResolution, camera.Bottom / _GP(thisroom).MaskResolution);
		view_bmp->StretchBlt(tempw, mask_src, RectWH(0, 0, viewport.GetWidth(), viewport.GetHeight()), Shared::kBitmap_Transparency);

		IDriverDependantBitmap *ddb = _G(gfxDriver)->CreateDDBFromBitmap(view_bmp, false, true);
		render_graphics(ddb, viewport.Left, viewport.Top);

		delete tempw;
		delete view_bmp;
		_G(gfxDriver)->DestroyDDB(ddb);
		ags_wait_until_keypress();
		invalidate_screen();
	} else if (cmdd == 3) {
		int goToRoom = -1;
		if (_GP(game).roomCount == 0) {
			char inroomtex[80];
			snprintf(inroomtex, sizeof(inroomtex), "!Enter new room: (in room %d)", _G(displayed_room));
			setup_for_dialog();
			goToRoom = enternumberwindow(inroomtex);
			restore_after_dialog();
		} else {
			setup_for_dialog();
			goToRoom = roomSelectorWindow(_G(displayed_room), _GP(game).roomCount, _GP(game).roomNumbers, _GP(game).roomNames);
			restore_after_dialog();
		}
		if (goToRoom >= 0)
			NewRoom(goToRoom);
	} else if (cmdd == 4) {
		if (_G(display_fps) != kFPS_Forced)
			_G(display_fps) = (FPSDisplayMode)dataa;
	} else if (cmdd == 5) {
		if (dataa == 0) dataa = _GP(game).playercharacter;
		if (_GP(game).chars[dataa].walking < 1) {
			Display("Not currently moving.");
			return;
		}
		Bitmap *tempw = BitmapHelper::CreateTransparentBitmap(_GP(thisroom).WalkAreaMask->GetWidth(), _GP(thisroom).WalkAreaMask->GetHeight());
		int mlsnum = _GP(game).chars[dataa].walking;
		if (_GP(game).chars[dataa].walking >= TURNING_AROUND)
			mlsnum %= TURNING_AROUND;
		MoveList *cmls = &_GP(mls)[mlsnum];
		for (int i = 0; i < cmls->numstage - 1; i++) {
			short srcx = cmls->pos[i].X;
			short srcy = cmls->pos[i].Y;
			short targetx = cmls->pos[i + 1].X;
			short targety = cmls->pos[i + 1].Y;
			tempw->DrawLine(Line(srcx, srcy, targetx, targety), MakeColor(i + 1));
		}

		// TODO: support multiple viewports?!
		const int viewport_index = 0;
		const int camera_index = 0;
		const Rect &viewport = _GP(play).GetRoomViewport(viewport_index)->GetRect();
		const Rect &camera = _GP(play).GetRoomCamera(camera_index)->GetRect();
		Bitmap *view_bmp = BitmapHelper::CreateBitmap(viewport.GetWidth(), viewport.GetHeight());
		Rect mask_src = Rect(camera.Left / _GP(thisroom).MaskResolution, camera.Top / _GP(thisroom).MaskResolution, camera.Right / _GP(thisroom).MaskResolution, camera.Bottom / _GP(thisroom).MaskResolution);
		view_bmp->StretchBlt(tempw, mask_src, RectWH(0, 0, viewport.GetWidth(), viewport.GetHeight()), Shared::kBitmap_Transparency);

		IDriverDependantBitmap *ddb = _G(gfxDriver)->CreateDDBFromBitmap(view_bmp, false, true);
		render_graphics(ddb, viewport.Left, viewport.Top);

		delete tempw;
		delete view_bmp;
		_G(gfxDriver)->DestroyDDB(ddb);
		ags_wait_until_keypress();
	} else if (cmdd == 99)
		ccSetOption(SCOPT_DEBUGRUN, dataa);
	else quit("!Debug: unknown command code");
}

} // namespace AGS3
