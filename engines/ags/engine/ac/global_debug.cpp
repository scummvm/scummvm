//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "ac/global_debug.h"
#include "ac/common.h"
#include "ac/characterinfo.h"
#include "ac/draw.h"
#include "ac/game.h"
#include "ac/gamesetup.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_character.h"
#include "ac/global_display.h"
#include "ac/global_room.h"
#include "ac/movelist.h"
#include "ac/properties.h"
#include "ac/sys_events.h"
#include "ac/tree_map.h"
#include "ac/walkablearea.h"
#include "gfx/gfxfilter.h"
#include "gui/guidialog.h"
#include "script/cc_options.h"
#include "debug/debug_log.h"
#include "debug/debugger.h"
#include "main/main.h"
#include "ac/spritecache.h"
#include "gfx/bitmap.h"
#include "gfx/graphicsdriver.h"
#include "main/graphics_mode.h"

using namespace AGS::Common;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameSetup usetup;
extern GameState play;
extern RoomStruct thisroom;
extern CharacterInfo*playerchar;

extern int convert_16bit_bgr;
extern IGraphicsDriver *gfxDriver;
extern SpriteCache spriteset;
extern TreeMap *transtree;
extern int displayed_room, starting_room;
extern MoveList *mls;
extern char transFileName[MAX_PATH];

String GetRuntimeInfo()
{
    DisplayMode mode = gfxDriver->GetDisplayMode();
    Rect render_frame = gfxDriver->GetRenderDestination();
    PGfxFilter filter = gfxDriver->GetGraphicsFilter();
    String runtimeInfo = String::FromFormat(
        "Adventure Game Studio run-time engine[ACI version %s"
        "[Game resolution %d x %d (%d-bit)"
        "[Running %d x %d at %d-bit%s%s[GFX: %s; %s[Draw frame %d x %d["
        "Sprite cache size: %d KB (limit %d KB; %d locked)",
        EngineVersion.LongString.GetCStr(), game.GetGameRes().Width, game.GetGameRes().Height, game.GetColorDepth(),
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
    if (usetup.mod_player == 0)
        runtimeInfo.Append("[(mod/xm player discarded)");

    return runtimeInfo;
}

void script_debug(int cmdd,int dataa) {
    if (play.debug_mode==0) return;
    int rr;
    if (cmdd==0) {
        for (rr=1;rr<game.numinvitems;rr++)
            playerchar->inv[rr]=1;
        update_invorder();
        //    Display("invorder decided there are %d items[display %d",play.inv_numorder,play.inv_numdisp);
    }
    else if (cmdd==1) {
        String toDisplay = GetRuntimeInfo();
        Display(toDisplay.GetCStr());
        //    Display("shftR: %d  shftG: %d  shftB: %d", _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16);
        //    Display("Remaining memory: %d kb",_go32_dpmi_remaining_virtual_memory()/1024);
        //Display("Play char bcd: %d",->GetColorDepth(spriteset[views[playerchar->view].frames[playerchar->loop][playerchar->frame].pic]));
    }
    else if (cmdd==2) 
    {  // show walkable areas from here
        // TODO: support multiple viewports?!
        const int viewport_index = 0;
        const int camera_index = 0;
        Bitmap *tempw=BitmapHelper::CreateBitmap(thisroom.WalkAreaMask->GetWidth(),thisroom.WalkAreaMask->GetHeight());
        tempw->Blit(prepare_walkable_areas(-1),0,0,0,0,tempw->GetWidth(),tempw->GetHeight());
        const Rect &viewport = play.GetRoomViewport(viewport_index)->GetRect();
        const Rect &camera = play.GetRoomCamera(camera_index)->GetRect();
        Bitmap *view_bmp = BitmapHelper::CreateBitmap(viewport.GetWidth(), viewport.GetHeight());
        Rect mask_src = Rect(camera.Left / thisroom.MaskResolution, camera.Top / thisroom.MaskResolution, camera.Right / thisroom.MaskResolution, camera.Bottom / thisroom.MaskResolution);
        view_bmp->StretchBlt(tempw, mask_src, RectWH(0, 0, viewport.GetWidth(), viewport.GetHeight()), Common::kBitmap_Transparency);

        IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(view_bmp, false, true);
        render_graphics(ddb, viewport.Left, viewport.Top);

        delete tempw;
        delete view_bmp;
        gfxDriver->DestroyDDB(ddb);
        ags_wait_until_keypress();
        invalidate_screen();
    }
    else if (cmdd==3) 
    {
        int goToRoom = -1;
        if (game.roomCount == 0)
        {
            char inroomtex[80];
            sprintf(inroomtex, "!Enter new room: (in room %d)", displayed_room);
            setup_for_dialog();
            goToRoom = enternumberwindow(inroomtex);
            restore_after_dialog();
        }
        else
        {
            setup_for_dialog();
            goToRoom = roomSelectorWindow(displayed_room, game.roomCount, game.roomNumbers, game.roomNames);
            restore_after_dialog();
        }
        if (goToRoom >= 0) 
            NewRoom(goToRoom);
    }
    else if (cmdd == 4) {
        if (display_fps != kFPS_Forced)
            display_fps = (FPSDisplayMode)dataa;
    }
    else if (cmdd == 5) {
        if (dataa == 0) dataa = game.playercharacter;
        if (game.chars[dataa].walking < 1) {
            Display("Not currently moving.");
            return;
        }
        Bitmap *tempw=BitmapHelper::CreateTransparentBitmap(thisroom.WalkAreaMask->GetWidth(),thisroom.WalkAreaMask->GetHeight());
        int mlsnum = game.chars[dataa].walking;
        if (game.chars[dataa].walking >= TURNING_AROUND)
            mlsnum %= TURNING_AROUND;
        MoveList*cmls = &mls[mlsnum];
        for (int i = 0; i < cmls->numstage-1; i++) {
            short srcx=short((cmls->pos[i] >> 16) & 0x00ffff);
            short srcy=short(cmls->pos[i] & 0x00ffff);
            short targetx=short((cmls->pos[i+1] >> 16) & 0x00ffff);
            short targety=short(cmls->pos[i+1] & 0x00ffff);
            tempw->DrawLine(Line(srcx, srcy, targetx, targety), MakeColor(i+1));
        }

        // TODO: support multiple viewports?!
        const int viewport_index = 0;
        const int camera_index = 0;
        const Rect &viewport = play.GetRoomViewport(viewport_index)->GetRect();
        const Rect &camera = play.GetRoomCamera(camera_index)->GetRect();
        Bitmap *view_bmp = BitmapHelper::CreateBitmap(viewport.GetWidth(), viewport.GetHeight());
        Rect mask_src = Rect(camera.Left / thisroom.MaskResolution, camera.Top / thisroom.MaskResolution, camera.Right / thisroom.MaskResolution, camera.Bottom / thisroom.MaskResolution);
        view_bmp->StretchBlt(tempw, mask_src, RectWH(0, 0, viewport.GetWidth(), viewport.GetHeight()), Common::kBitmap_Transparency);

        IDriverDependantBitmap *ddb = gfxDriver->CreateDDBFromBitmap(view_bmp, false, true);
        render_graphics(ddb, viewport.Left, viewport.Top);

        delete tempw;
        delete view_bmp;
        gfxDriver->DestroyDDB(ddb);
        ags_wait_until_keypress();
    }
    else if (cmdd == 99)
        ccSetOption(SCOPT_DEBUGRUN, dataa);
    else quit("!Debug: unknown command code");
}
