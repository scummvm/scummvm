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

#include "ags/engine/ac/global_debug.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/characterinfo.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game.h"
#include "ags/engine/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_character.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/global_room.h"
#include "ags/engine/ac/movelist.h"
#include "ags/engine/ac/properties.h"
#include "ags/engine/ac/sys_events.h"
#include "ags/engine/ac/tree_map.h"
#include "ags/engine/ac/walkablearea.h"
#include "ags/engine/gfx/gfxfilter.h"
#include "ags/engine/gui/guidialog.h"
#include "ags/shared/script/cc_options.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/engine/debugging/debugger.h"
#include "ags/engine/main/main.h"
#include "ags/shared/ac/spritecache.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/engine/gfx/graphicsdriver.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/globals.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameSetup usetup;
extern GameState play;
extern RoomStruct thisroom;
extern CharacterInfo *playerchar;

extern int convert_16bit_bgr;
extern IGraphicsDriver *gfxDriver;
extern SpriteCache spriteset;
extern TreeMap *transtree;
extern int displayed_room, starting_room;
extern MoveList *mls;
extern char transFileName[MAX_PATH];

String GetRuntimeInfo() {
	DisplayMode mode = gfxDriver->GetDisplayMode();
	Rect render_frame = gfxDriver->GetRenderDestination();
	PGfxFilter filter = gfxDriver->GetGraphicsFilter();
	String runtimeInfo = String::FromFormat(
		"Adventure Game Studio run-time engine[ACI version %s"
		"[Game resolution %d x %d (%d-bit)"
		"[Running %d x %d at %d-bit%s%s[GFX: %s; %s[Draw frame %d x %d["
		"Sprite cache size: %d KB (limit %d KB; %d locked)",
		_G(EngineVersion).LongString.GetCStr(), game.GetGameRes().Width, game.GetGameRes().Height, game.GetColorDepth(),
		mode.Width, mode.Height, mode.ColorDepth, (convert_16bit_bgr) ? " BGR" : "",
		mode.Windowed ? " W" : "",
		gfxDriver->GetDriverName(), filter->GetInfo().Name.GetCStr(),
		render_frame.GetWidth(), render_frame.GetHeight(),
		spriteset.GetCacheSize() / 1024, spriteset.GetMaxCacheSize() / 1024, spriteset.GetLockedSize() / 1024);
	if (play.separate_music_lib)
		runtimeInfo.Append("[AUDIO.VOX enabled");
	if (play.want_speech >= 1)
		runtimeInfo.Append("[SPEECH.VOX enabled");
	if (transtree != nullptr) {
		runtimeInfo.Append("[Using translation ");
		runtimeInfo.Append(transFileName);
	}

	return runtimeInfo;
}

void script_debug(int cmdd, int dataa) {
	if (play.debug_mode == 0) return;
	int rr;
	if (cmdd == 0) {
		for (rr = 1; rr < game.numinvitems; rr++)
			playerchar->inv[rr] = 1;
		update_invorder();
		//    Display("invorder decided there are %d items[display %d",play.inv_numorder,play.inv_numdisp);
	} else if (cmdd == 1) {
		String toDisplay = GetRuntimeInfo();
		Display(toDisplay.GetCStr());
		//    Display("shftR: %d  shftG: %d  shftB: %d", _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16);
		//    Display("Remaining memory: %d kb",_go32_dpmi_remaining_virtual_memory()/1024);
		//Display("Play char bcd: %d",->GetColorDepth(spriteset[views[playerchar->view].frames[playerchar->loop][playerchar->frame].pic]));
	} else if (cmdd == 2) {
		// show walkable areas from here
		// TODO: support multiple viewports?!
		const int viewport_index = 0;
		const int camera_index = 0;
		Bitmap *tempw = BitmapHelper::CreateBitmap(thisroom.WalkAreaMask->GetWidth(), thisroom.WalkAreaMask->GetHeight());
		tempw->Blit(prepare_walkable_areas(-1), 0, 0, 0, 0, tempw->GetWidth(), tempw->GetHeight());
		const Rect &viewport = play.GetRoomViewport(viewport_index)->GetRect();
		const Rect &camera = play.GetRoomCamera(camera_index)->GetRect();
		Bitmap *view_bmp = BitmapHelper::CreateBitmap(viewport.GetWidth(), viewport.GetHeight());
		Rect mask_src = Rect(camera.Left / thisroom.MaskResolution, camera.Top / thisroom.MaskResolution, camera.Right / thisroom.MaskResolution, camera.Bottom / thisroom.MaskResolution);
		view_bmp->StretchBlt(tempw, mask_src, RectWH(0, 0, viewport.GetWidth(), viewport.GetHeight()), Shared::kBitmap_Transparency);

		IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(view_bmp, false, true);
		render_graphics(ddb, viewport.Left, viewport.Top);

		delete tempw;
		delete view_bmp;
		gfxDriver->DestroyDDB(ddb);
		ags_wait_until_keypress();
		invalidate_screen();
	} else if (cmdd == 3) {
		int goToRoom = -1;
		if (game.roomCount == 0) {
			char inroomtex[80];
			sprintf(inroomtex, "!Enter new room: (in room %d)", displayed_room);
			setup_for_dialog();
			goToRoom = enternumberwindow(inroomtex);
			restore_after_dialog();
		} else {
			setup_for_dialog();
			goToRoom = roomSelectorWindow(displayed_room, game.roomCount, game.roomNumbers, game.roomNames);
			restore_after_dialog();
		}
		if (goToRoom >= 0)
			NewRoom(goToRoom);
	} else if (cmdd == 4) {
		if (display_fps != kFPS_Forced)
			display_fps = (FPSDisplayMode)dataa;
	} else if (cmdd == 5) {
		if (dataa == 0) dataa = game.playercharacter;
		if (game.chars[dataa].walking < 1) {
			Display("Not currently moving.");
			return;
		}
		Bitmap *tempw = BitmapHelper::CreateTransparentBitmap(thisroom.WalkAreaMask->GetWidth(), thisroom.WalkAreaMask->GetHeight());
		int mlsnum = game.chars[dataa].walking;
		if (game.chars[dataa].walking >= TURNING_AROUND)
			mlsnum %= TURNING_AROUND;
		MoveList *cmls = &mls[mlsnum];
		for (int i = 0; i < cmls->numstage - 1; i++) {
			short srcx = short((cmls->pos[i] >> 16) & 0x00ffff);
			short srcy = short(cmls->pos[i] & 0x00ffff);
			short targetx = short((cmls->pos[i + 1] >> 16) & 0x00ffff);
			short targety = short(cmls->pos[i + 1] & 0x00ffff);
			tempw->DrawLine(Line(srcx, srcy, targetx, targety), MakeColor(i + 1));
		}

		// TODO: support multiple viewports?!
		const int viewport_index = 0;
		const int camera_index = 0;
		const Rect &viewport = play.GetRoomViewport(viewport_index)->GetRect();
		const Rect &camera = play.GetRoomCamera(camera_index)->GetRect();
		Bitmap *view_bmp = BitmapHelper::CreateBitmap(viewport.GetWidth(), viewport.GetHeight());
		Rect mask_src = Rect(camera.Left / thisroom.MaskResolution, camera.Top / thisroom.MaskResolution, camera.Right / thisroom.MaskResolution, camera.Bottom / thisroom.MaskResolution);
		view_bmp->StretchBlt(tempw, mask_src, RectWH(0, 0, viewport.GetWidth(), viewport.GetHeight()), Shared::kBitmap_Transparency);

		IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(view_bmp, false, true);
		render_graphics(ddb, viewport.Left, viewport.Top);

		delete tempw;
		delete view_bmp;
		gfxDriver->DestroyDDB(ddb);
		ags_wait_until_keypress();
	} else if (cmdd == 99)
		ccSetOption(SCOPT_DEBUGRUN, dataa);
	else quit("!Debug: unknown command code");
}

} // namespace AGS3
