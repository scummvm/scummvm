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

#include "ac/global_overlay.h"
#include "ac/common.h"
#include "ac/display.h"
#include "ac/draw.h"
#include "ac/gamesetupstruct.h"
#include "ac/gamestate.h"
#include "ac/global_translation.h"
#include "ac/overlay.h"
#include "ac/runtime_defines.h"
#include "ac/screenoverlay.h"
#include "ac/string.h"
#include "ac/spritecache.h"
#include "ac/system.h"
#include "gfx/bitmap.h"

using namespace Common;
using namespace Engine;

extern SpriteCache spriteset;
extern GameSetupStruct game;



void RemoveOverlay(int ovrid) {
    if (find_overlay_of_type(ovrid) < 0) quit("!RemoveOverlay: invalid overlay id passed");
    remove_screen_overlay(ovrid);
}

int CreateGraphicOverlay(int xx,int yy,int slott,int trans) {
    data_to_game_coords(&xx, &yy);

    Bitmap *screeno=BitmapHelper::CreateTransparentBitmap(game.SpriteInfos[slott].Width, game.SpriteInfos[slott].Height, game.GetColorDepth());
    wputblock(screeno, 0,0,spriteset[slott],trans);
    bool hasAlpha = (game.SpriteInfos[slott].Flags & SPF_ALPHACHANNEL) != 0;
    int nse = add_screen_overlay(xx, yy, OVER_CUSTOM, screeno, hasAlpha);
    return screenover[nse].type;
}

int CreateTextOverlayCore(int xx, int yy, int wii, int fontid, int text_color, const char *text, int disp_type, int allowShrink) {
    if (wii<8) wii=play.GetUIViewport().GetWidth()/2;
    if (xx<0) xx=play.GetUIViewport().GetWidth()/2-wii/2;
    if (text_color ==0) text_color =16;
    return _display_main(xx,yy,wii, text, disp_type, fontid, -text_color, 0, allowShrink, false);
}

int CreateTextOverlay(int xx, int yy, int wii, int fontid, int text_color, const char* text, int disp_type) {
    int allowShrink = 0;

    if (xx != OVR_AUTOPLACE) {
        data_to_game_coords(&xx,&yy);
        wii = data_to_game_coord(wii);
    }
    else  // allow DisplaySpeechBackground to be shrunk
        allowShrink = 1;

    return CreateTextOverlayCore(xx, yy, wii, fontid, text_color, text, disp_type, allowShrink);
}

void SetTextOverlay(int ovrid, int xx, int yy, int wii, int fontid, int text_color, const char *text) {
    RemoveOverlay(ovrid);
    const int disp_type = ovrid;
    if (CreateTextOverlay(xx, yy, wii, fontid, text_color, text, disp_type) !=ovrid)
        quit("SetTextOverlay internal error: inconsistent type ids");
}

void MoveOverlay(int ovrid, int newx,int newy) {
    data_to_game_coords(&newx, &newy);

    int ovri=find_overlay_of_type(ovrid);
    if (ovri<0) quit("!MoveOverlay: invalid overlay ID specified");
    screenover[ovri].x=newx;
    screenover[ovri].y=newy;
}

int IsOverlayValid(int ovrid) {
    if (find_overlay_of_type(ovrid) < 0)
        return 0;

    return 1;
}
