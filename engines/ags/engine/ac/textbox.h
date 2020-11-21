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
#ifndef __AGS_EE_AC__TEXTBOX_H
#define __AGS_EE_AC__TEXTBOX_H

#include "gui/guitextbox.h"

using AGS::Common::GUITextBox;

const char*	TextBox_GetText_New(GUITextBox *texbox);
void		TextBox_GetText(GUITextBox *texbox, char *buffer);
void		TextBox_SetText(GUITextBox *texbox, const char *newtex);
int			TextBox_GetTextColor(GUITextBox *guit);
void		TextBox_SetTextColor(GUITextBox *guit, int colr);
int			TextBox_GetFont(GUITextBox *guit);
void		TextBox_SetFont(GUITextBox *guit, int fontnum);

#endif // __AGS_EE_AC__TEXTBOX_H
