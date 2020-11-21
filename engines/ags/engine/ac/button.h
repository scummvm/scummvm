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
//
//
//=============================================================================
#ifndef __AGS_EE_AC__BUTTON_H
#define __AGS_EE_AC__BUTTON_H

#include "gui/guibutton.h"

using AGS::Common::GUIButton;

void		Button_Animate(GUIButton *butt, int view, int loop, int speed, int repeat);
const char* Button_GetText_New(GUIButton *butt);
void		Button_GetText(GUIButton *butt, char *buffer);
void		Button_SetText(GUIButton *butt, const char *newtx);
void		Button_SetFont(GUIButton *butt, int newFont);
int			Button_GetFont(GUIButton *butt);
int			Button_GetClipImage(GUIButton *butt);
void		Button_SetClipImage(GUIButton *butt, int newval);
int			Button_GetGraphic(GUIButton *butt);
int			Button_GetMouseOverGraphic(GUIButton *butt);
void		Button_SetMouseOverGraphic(GUIButton *guil, int slotn);
int			Button_GetNormalGraphic(GUIButton *butt);
void		Button_SetNormalGraphic(GUIButton *guil, int slotn);
int			Button_GetPushedGraphic(GUIButton *butt);
void		Button_SetPushedGraphic(GUIButton *guil, int slotn);
int			Button_GetTextColor(GUIButton *butt);
void		Button_SetTextColor(GUIButton *butt, int newcol);

int			UpdateAnimatingButton(int bu);
void		StopButtonAnimation(int idxn);
void		FindAndRemoveButtonAnimation(int guin, int objn);

#endif // __AGS_EE_AC__BUTTON_H
