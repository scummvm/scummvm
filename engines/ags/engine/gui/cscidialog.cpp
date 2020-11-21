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

#include <cctype>
#include "util/wgt2allg.h"
#include "ac/common.h"
#include "ac/draw.h"
#include "ac/gamesetup.h"
#include "ac/gamestate.h"
#include "ac/gui.h"
#include "ac/keycode.h"
#include "ac/mouse.h"
#include "ac/sys_events.h"
#include "ac/runtime_defines.h"
#include "font/fonts.h"
#include "gui/cscidialog.h"
#include "gui/guidialog.h"
#include "gui/guimain.h"
#include "gui/mycontrols.h"
#include "main/game_run.h"
#include "gfx/graphicsdriver.h"
#include "gfx/bitmap.h"
#include "media/audio/audio_system.h"
#include "platform/base/agsplatformdriver.h"
#include "ac/timer.h"

using AGS::Common::Bitmap;
namespace BitmapHelper = AGS::Common::BitmapHelper;

extern char ignore_bounds; // from mousew32
extern IGraphicsDriver *gfxDriver;
extern GameSetup usetup;


//-----------------------------------------------------------------------------
// DIALOG SYSTEM STUFF below

int windowbackgroundcolor = COL254, pushbuttondarkcolor = COL255;
int pushbuttonlightcolor = COL253;
int topwindowhandle = -1;
int cbuttfont;

int acdialog_font;

int smcode = 0;

#define MAXCONTROLS 20
#define MAXSCREENWINDOWS 5
NewControl *vobjs[MAXCONTROLS];
OnScreenWindow oswi[MAXSCREENWINDOWS];

int controlid = 0;


//-----------------------------------------------------------------------------

void __my_wbutt(Bitmap *ds, int x1, int y1, int x2, int y2)
{
    color_t draw_color = ds->GetCompatibleColor(COL254);            //wsetcolor(15);
    ds->FillRect(Rect(x1, y1, x2, y2), draw_color);
    draw_color = ds->GetCompatibleColor(0);
    ds->DrawRect(Rect(x1, y1, x2, y2), draw_color);
}

//-----------------------------------------------------------------------------

OnScreenWindow::OnScreenWindow()
{
    x = y = 0;
    handle = -1;
    oldtop = -1;
}

int CSCIGetVersion()
{
    return 0x0100;
}

int windowcount = 0, curswas = 0;
int win_x = 0, win_y = 0, win_width = 0, win_height = 0;
int CSCIDrawWindow(int xx, int yy, int wid, int hit)
{
    ignore_bounds++;
    multiply_up(&xx, &yy, &wid, &hit);
    int drawit = -1;
    for (int aa = 0; aa < MAXSCREENWINDOWS; aa++) {
        if (oswi[aa].handle < 0) {
            drawit = aa;
            break;
        }
    }

    if (drawit < 0)
        quit("Too many windows created.");

    windowcount++;
    //  ags_domouse(DOMOUSE_DISABLE);
    xx -= 2;
    yy -= 2;
    wid += 4;
    hit += 4;
    Bitmap *ds = prepare_gui_screen(xx, yy, wid, hit, true);
    oswi[drawit].x = xx;
    oswi[drawit].y = yy;
    __my_wbutt(ds, 0, 0, wid - 1, hit - 1);    // wbutt goes outside its area
    //  ags_domouse(DOMOUSE_ENABLE);
    oswi[drawit].oldtop = topwindowhandle;
    topwindowhandle = drawit;
    oswi[drawit].handle = topwindowhandle;
    win_x = xx;
    win_y = yy;
    win_width = wid;
    win_height = hit;
    return drawit;
}

void CSCIEraseWindow(int handl)
{
    //  ags_domouse(DOMOUSE_DISABLE);
    ignore_bounds--;
    topwindowhandle = oswi[handl].oldtop;
    oswi[handl].handle = -1;
    //  ags_domouse(DOMOUSE_ENABLE);
    windowcount--;
    clear_gui_screen();
}

int CSCIWaitMessage(CSCIMessage * cscim)
{
    for (int uu = 0; uu < MAXCONTROLS; uu++) {
        if (vobjs[uu] != nullptr) {
            //      ags_domouse(DOMOUSE_DISABLE);
            vobjs[uu]->drawifneeded();
            //      ags_domouse(DOMOUSE_ENABLE);
        }
    }

    prepare_gui_screen(win_x, win_y, win_width, win_height, true);

    while (1) {
        update_audio_system_on_game_loop();
        refresh_gui_screen();

        cscim->id = -1;
        cscim->code = 0;
        smcode = 0;
        int keywas;
        if (run_service_key_controls(keywas) && !play.IsIgnoringInput()) {
            if (keywas == 13) {
                cscim->id = finddefaultcontrol(CNF_DEFAULT);
                cscim->code = CM_COMMAND;
            } else if (keywas == 27) {
                cscim->id = finddefaultcontrol(CNF_CANCEL);
                cscim->code = CM_COMMAND;
            } else if ((keywas < 32) && (keywas != 8)) ;
            else if ((keywas >= 372) & (keywas <= 381) & (finddefaultcontrol(CNT_LISTBOX) >= 0))
                vobjs[finddefaultcontrol(CNT_LISTBOX)]->processmessage(CTB_KEYPRESS, keywas, 0);
            else if (finddefaultcontrol(CNT_TEXTBOX) >= 0)
                vobjs[finddefaultcontrol(CNT_TEXTBOX)]->processmessage(CTB_KEYPRESS, keywas, 0);

            if (cscim->id < 0) {
                cscim->code = CM_KEYPRESS;
                cscim->wParam = keywas;
            }
        }

        int mbut, mwheelz;
        if (run_service_mb_controls(mbut, mwheelz) && mbut >= 0 && !play.IsIgnoringInput()) {
            if (checkcontrols()) {
                cscim->id = controlid;
                cscim->code = CM_COMMAND;
            }
        }

        if (smcode) {
            cscim->code = smcode;
            cscim->id = controlid;
        }

        if (cscim->code > 0)
            break;

        WaitForNextFrame();
    }

    return 0;
}

int CSCICreateControl(int typeandflags, int xx, int yy, int wii, int hii, const char *title)
{
    multiply_up(&xx, &yy, &wii, &hii);
    int usec = -1;
    for (int hh = 1; hh < MAXCONTROLS; hh++) {
        if (vobjs[hh] == nullptr) {
            usec = hh;
            break;
        }
    }

    if (usec < 0)
        quit("Too many controls created");

    int type = typeandflags & 0x00ff;     // 256 control types
    if (type == CNT_PUSHBUTTON) {
        if (wii == -1)
            wii = wgettextwidth(title, cbuttfont) + 20;

        vobjs[usec] = new MyPushButton(xx, yy, wii, hii, title);

    } else if (type == CNT_LISTBOX) {
        vobjs[usec] = new MyListBox(xx, yy, wii, hii);
    } else if (type == CNT_LABEL) {
        vobjs[usec] = new MyLabel(xx, yy, wii, title);
    } else if (type == CNT_TEXTBOX) {
        vobjs[usec] = new MyTextBox(xx, yy, wii, title);
    } else
        quit("Unknown control type requested");

    vobjs[usec]->typeandflags = typeandflags;
    vobjs[usec]->wlevel = topwindowhandle;
    //  ags_domouse(DOMOUSE_DISABLE);
    vobjs[usec]->draw( get_gui_screen() );
    //  ags_domouse(DOMOUSE_ENABLE);
    return usec;
}

void CSCIDeleteControl(int haa)
{
    delete vobjs[haa];
    vobjs[haa] = nullptr;
}

int CSCISendControlMessage(int haa, int mess, int wPar, long lPar)
{
    if (vobjs[haa] == nullptr)
        return -1;
    return vobjs[haa]->processmessage(mess, wPar, lPar);
}

void multiply_up_to_game_res(int *x, int *y)
{
    x[0] = get_fixed_pixel_size(x[0]);
    y[0] = get_fixed_pixel_size(y[0]);
}

// TODO: this is silly, make a uniform formula
void multiply_up(int *x1, int *y1, int *x2, int *y2)
{
    multiply_up_to_game_res(x1, y1);
    multiply_up_to_game_res(x2, y2);

    // adjust for 800x600
    if ((GetBaseWidth() == 400) || (GetBaseWidth() == 800)) {
        x1[0] = (x1[0] * 5) / 4;
        x2[0] = (x2[0] * 5) / 4;
        y1[0] = (y1[0] * 3) / 2;
        y2[0] = (y2[0] * 3) / 2;
    }
    else if (GetBaseWidth() == 1024)
    {
        x1[0] = (x1[0] * 16) / 10;
        x2[0] = (x2[0] * 16) / 10;
        y1[0] = (y1[0] * 384) / 200;
        y2[0] = (y2[0] * 384) / 200;
    }
}

int checkcontrols()
{
    // NOTE: this is because old code was working with full game screen
    const int mousex = ::mousex - win_x;
    const int mousey = ::mousey - win_y;

    smcode = 0;
    for (int kk = 0; kk < MAXCONTROLS; kk++) {
        if (vobjs[kk] != nullptr) {
            if (vobjs[kk]->mouseisinarea(mousex, mousey)) {
                controlid = kk;
                return vobjs[kk]->pressedon(mousex, mousey);
            }
        }
    }
    return 0;
}

int finddefaultcontrol(int flagmask)
{
    for (int ff = 0; ff < MAXCONTROLS; ff++) {
        if (vobjs[ff] == nullptr)
            continue;

        if (vobjs[ff]->wlevel != topwindowhandle)
            continue;

        if (vobjs[ff]->typeandflags & flagmask)
            return ff;
    }

    return -1;
}

int GetBaseWidth () {
    return play.GetUIViewport().GetWidth();
}
