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

#include <string.h>
#include "util/wgt2allg.h"
#include "ac/common.h"
#include "ac/mouse.h"
#include "font/fonts.h"
#include "gui/mypushbutton.h"
#include "gui/guidialog.h"
#include "gui/guidialoginternaldefs.h"
#include "main/game_run.h"
#include "gfx/bitmap.h"
#include "platform/base/agsplatformdriver.h"
#include "ac/timer.h"

using AGS::Common::Bitmap;

extern int windowbackgroundcolor, pushbuttondarkcolor;
extern int pushbuttonlightcolor;
extern int cbuttfont;

MyPushButton::MyPushButton(int xx, int yy, int wi, int hi, const char *tex)
{                             //wlevel=2;
    x = xx;
    y = yy;
    wid = wi;
    hit = hi + 1;               //hit=hi;
    state = 0;
    strncpy(text, tex, 50);
    text[49] = 0;
};

void MyPushButton::draw(Bitmap *ds)
{
    color_t text_color = ds->GetCompatibleColor(0);
    color_t draw_color = ds->GetCompatibleColor(COL254);
    ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
    if (state == 0)
        draw_color = ds->GetCompatibleColor(pushbuttondarkcolor);
    else
        draw_color = ds->GetCompatibleColor(pushbuttonlightcolor);

    ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
    if (state == 0)
        draw_color = ds->GetCompatibleColor(pushbuttonlightcolor);
    else
        draw_color = ds->GetCompatibleColor(pushbuttondarkcolor);

    ds->DrawLine(Line(x, y, x + wid - 1, y), draw_color);
    ds->DrawLine(Line(x, y, x, y + hit - 1), draw_color);
    wouttextxy(ds, x + (wid / 2 - wgettextwidth(text, cbuttfont) / 2), y + 2, cbuttfont, text_color, text);
    if (typeandflags & CNF_DEFAULT)
        draw_color = ds->GetCompatibleColor(0);
    else
        draw_color = ds->GetCompatibleColor(windowbackgroundcolor);

    ds->DrawRect(Rect(x - 1, y - 1, x + wid + 1, y + hit + 1), draw_color);
}

//extern const int LEFT;  // in mousew32

int MyPushButton::pressedon(int mousex, int mousey)
{
    int wasstat;
    while (mbutrelease(LEFT) == 0) {

        wasstat = state;
        state = mouseisinarea(mousex, mousey);
        // stop mp3 skipping if button held down
        update_polled_stuff_if_runtime();
        if (wasstat != state) {
            //        ags_domouse(DOMOUSE_DISABLE);
            draw(get_gui_screen());
            //ags_domouse(DOMOUSE_ENABLE);
        }

        //      ags_domouse(DOMOUSE_UPDATE);

        refresh_gui_screen();

        WaitForNextFrame();
    }
    wasstat = state;
    state = 0;
    //    ags_domouse(DOMOUSE_DISABLE);
    draw(get_gui_screen());
    //  ags_domouse(DOMOUSE_ENABLE);
    return wasstat;
}

int MyPushButton::processmessage(int mcode, int wParam, long lParam)
{
    return -1;                  // doesn't support messages
}