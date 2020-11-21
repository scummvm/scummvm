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

#ifndef __AC_MYLABEL_H
#define __AC_MYLABEL_H

#include "gui/newcontrol.h"

struct MyLabel:public NewControl
{
  char text[150];
  MyLabel(int xx, int yy, int wii, const char *tee);

  void draw(Common::Bitmap *ds) override;

  int pressedon(int mousex, int mousey) override;

  int processmessage(int mcode, int wParam, long lParam) override;
};

#endif // __AC_MYLABEL_H