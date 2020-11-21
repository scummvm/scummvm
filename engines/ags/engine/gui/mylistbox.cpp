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
#include "ac/gamesetup.h"
#include "font/fonts.h"
#include "gfx/bitmap.h"
#include "gui/guidialog.h"
#include "gui/guidialoginternaldefs.h"
#include "gui/mylistbox.h"

using AGS::Common::Bitmap;

extern GameSetup usetup;
extern int numcurso, hotx, hoty;

extern int windowbackgroundcolor;
extern int cbuttfont;
extern int smcode;

  MyListBox::MyListBox(int xx, int yy, int wii, int hii)
  {
    x = xx;
    y = yy;
    wid = wii;
    hit = hii;
    hit -= (hit - 4) % TEXT_HT; // resize to multiple of text height
    numonscreen = (hit - 4) / TEXT_HT;
    items = 0;
    topitem = 0;
    selected = -1;
    memset(itemnames, 0, sizeof(itemnames));
  }

  void MyListBox::clearlist()
  {
    for (int kk = 0; kk < items; kk++)
      free(itemnames[kk]);

    items = 0;
  }

  MyListBox::~MyListBox() {
    clearlist();
  }

  void MyListBox::draw(Bitmap *ds)
  {
    color_t draw_color = ds->GetCompatibleColor(windowbackgroundcolor);
    ds->FillRect(Rect(x, y, x + wid, y + hit), draw_color);
    draw_color = ds->GetCompatibleColor(0);
    ds->DrawRect(Rect(x, y, x + wid, y + hit), draw_color);
  
    int widwas = wid;
    wid -= ARROWWIDTH;
    ds->DrawLine(Line(x + wid, y, x + wid, y + hit), draw_color);        // draw the up/down arrows
    ds->DrawLine(Line(x + wid, y + hit / 2, x + widwas, y + hit / 2), draw_color);

    int xmidd = x + wid + (widwas - wid) / 2;
    if (topitem < 1)
      draw_color = ds->GetCompatibleColor(7);

    ds->DrawLine(Line(xmidd, y + 2, xmidd, y + 10), draw_color); // up arrow
    ds->DrawLine(Line(xmidd - 1, y + 3, xmidd + 1, y + 3), draw_color);
    ds->DrawLine(Line(xmidd - 2, y + 4, xmidd + 2, y + 4), draw_color);
    draw_color = ds->GetCompatibleColor(0);
    if (topitem + numonscreen >= items)
      draw_color = ds->GetCompatibleColor(7);

    ds->DrawLine(Line(xmidd, y + hit - 10, xmidd, y + hit - 3), draw_color);     // down arrow
    ds->DrawLine(Line(xmidd - 1, y + hit - 4, xmidd + 1, y + hit - 4), draw_color);
    ds->DrawLine(Line(xmidd - 2, y + hit - 5, xmidd + 2, y + hit - 5), draw_color);
    draw_color = ds->GetCompatibleColor(0);

    for (int tt = 0; tt < numonscreen; tt++) {
      int inum = tt + topitem;
      if (inum >= items)
        break;

      int thisypos = y + 2 + tt * TEXT_HT;
      color_t text_color;
      if (inum == selected) {
        draw_color = ds->GetCompatibleColor(0);
        ds->FillRect(Rect(x, thisypos, x + wid, thisypos + TEXT_HT - 1), draw_color);
        text_color = ds->GetCompatibleColor(7);
      }
      else text_color = ds->GetCompatibleColor(0);

      wouttextxy(ds, x + 2, thisypos, cbuttfont, text_color, itemnames[inum]);
    }
    wid = widwas;
  }

  int MyListBox::pressedon(int mousex, int mousey)
  {
    if (mousex > x + wid - ARROWWIDTH) {
      if ((mousey - y < hit / 2) & (topitem > 0))
        topitem--;
      else if ((mousey - y > hit / 2) & (topitem + numonscreen < items))
        topitem++;

    } else {
      selected = ((mousey - y) - 2) / TEXT_HT + topitem;
      if (selected >= items)
        selected = items - 1;

    }

//    ags_domouse(DOMOUSE_DISABLE);
    draw(get_gui_screen());
  //  ags_domouse(DOMOUSE_ENABLE);
    smcode = CM_SELCHANGE;
    return 0;
  }

  void MyListBox::additem(char *texx)
  {
    if (items >= MAXLISTITEM)
      quit("!CSCIUSER16: Too many items added to listbox");
    itemnames[items] = (char *)malloc(strlen(texx) + 1);
    strcpy(itemnames[items], texx);
    items++;
    needredraw = 1;
  }

  int MyListBox::processmessage(int mcode, int wParam, long lParam)
  {
    if (mcode == CLB_ADDITEM) {
      additem((char *)lParam);
    } else if (mcode == CLB_CLEAR)
      clearlist();
    else if (mcode == CLB_GETCURSEL)
      return selected;
    else if (mcode == CLB_SETCURSEL)
    {
      selected = wParam;

      if ((selected < topitem) && (selected >= 0))
        topitem = selected;

      if (topitem + numonscreen <= selected)
        topitem = (selected + 1) - numonscreen;
    }
    else if (mcode == CLB_GETTEXT)
      strcpy((char *)lParam, itemnames[wParam]);
    else if (mcode == CLB_SETTEXT) {
      if (wParam < items)
        free(itemnames[wParam]);

      char *newstri = (char *)lParam;
      itemnames[wParam] = (char *)malloc(strlen(newstri) + 2);
      strcpy(itemnames[wParam], newstri);

    } else if (mcode == CTB_KEYPRESS) {
      if ((wParam == 380) && (selected < items - 1))
        selected++;

      if ((wParam == 372) && (selected > 0))
        selected--;

      if (wParam == 373)
        selected -= (numonscreen - 1);

      if (wParam == 381)
        selected += (numonscreen - 1);

      if ((selected < 0) && (items > 0))
        selected = 0;

      if (selected >= items)
        selected = items - 1;

      if ((selected < topitem) & (selected >= 0))
        topitem = selected;

      if (topitem + numonscreen <= selected)
        topitem = (selected + 1) - numonscreen;

      drawandmouse();
      smcode = CM_SELCHANGE;
    } else
      return -1;

    return 0;
  }
