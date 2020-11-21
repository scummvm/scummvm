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
#include "font/fonts.h"
#include "gui/mytextbox.h"
#include "gui/guidialoginternaldefs.h"
#include "gfx/bitmap.h"

using AGS::Common::Bitmap;

extern GameSetup usetup;

extern int windowbackgroundcolor;
extern int cbuttfont;

MyTextBox::MyTextBox(int xx, int yy, int wii, const char *tee)
{
    x = xx;
    y = yy;
    wid = wii;
    if (tee != nullptr)
        strcpy(text, tee);
    else
        text[0] = 0;

    hit = TEXT_HT + 1;
}

void MyTextBox::draw(Bitmap *ds)
{
    color_t draw_color = ds->GetCompatibleColor(windowbackgroundcolor);
    ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
    draw_color = ds->GetCompatibleColor(0);
    ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
    color_t text_color = ds->GetCompatibleColor(0);
    wouttextxy(ds, x + 2, y + 1, cbuttfont, text_color, text);

    char tbu[2] = "_";
    wouttextxy(ds, x + 2 + wgettextwidth(text, cbuttfont), y + 1, cbuttfont, text_color, tbu);
}

int MyTextBox::pressedon(int mousex, int mousey)
{
    return 0;
}

int MyTextBox::processmessage(int mcode, int wParam, long lParam)
{
    if (mcode == CTB_SETTEXT) {
        strcpy(text, (char *)lParam);
        needredraw = 1;
    } else if (mcode == CTB_GETTEXT)
        strcpy((char *)lParam, text);
    else if (mcode == CTB_KEYPRESS) {
        if (wParam == 8) {
            if (text[0] != 0)
                text[strlen(text) - 1] = 0;

            drawandmouse();
        } else if (strlen(text) >= TEXTBOX_MAXLEN - 1)
            ;
        else if (wgettextwidth(text, cbuttfont) >= wid - 5)
            ;
        else if (wParam > 127)
            ;  // font only has 128 chars
        else {
            text[strlen(text) + 1] = 0;
            text[strlen(text)] = wParam;
            drawandmouse();
        }
    } else
        return -1;

    return 0;
}
