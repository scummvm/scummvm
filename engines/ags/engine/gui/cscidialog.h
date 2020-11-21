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
//
// Legacy built-in GUI dialogs and controls.
//
//=============================================================================
#ifndef __AGS_EE_GUI__CSCIDIALOG_H
#define __AGS_EE_GUI__CSCIDIALOG_H

#include "gui/guidialoginternaldefs.h"

int  CSCIGetVersion();
int  CSCIDrawWindow(int xx, int yy, int wid, int hit);
void CSCIEraseWindow(int handl);
int  CSCIWaitMessage(CSCIMessage * cscim);
int  CSCICreateControl(int typeandflags, int xx, int yy, int wii, int hii, const char *title);
void CSCIDeleteControl(int haa);
int  CSCISendControlMessage(int haa, int mess, int wPar, long lPar);
void multiply_up_to_game_res(int *x, int *y);
void multiply_up(int *x1, int *y1, int *x2, int *y2);
int  checkcontrols();
int  finddefaultcontrol(int flagmask);
int  GetBaseWidth ();

#endif // __AGS_EE_GUI__CSCIDIALOG_H
