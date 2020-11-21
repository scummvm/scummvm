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
#ifndef __AGS_EE_AC__INVWINDOW_H
#define __AGS_EE_AC__INVWINDOW_H

#include "ac/characterinfo.h"
#include "ac/dynobj/scriptinvitem.h"
#include "gui/guiinv.h"

using AGS::Common::GUIInvWindow;

void            InvWindow_SetCharacterToUse(GUIInvWindow *guii, CharacterInfo *chaa);
CharacterInfo*  InvWindow_GetCharacterToUse(GUIInvWindow *guii);
void            InvWindow_SetItemWidth(GUIInvWindow *guii, int newwidth);
int             InvWindow_GetItemWidth(GUIInvWindow *guii);
void            InvWindow_SetItemHeight(GUIInvWindow *guii, int newhit);
int             InvWindow_GetItemHeight(GUIInvWindow *guii);
void            InvWindow_SetTopItem(GUIInvWindow *guii, int topitem);
int             InvWindow_GetTopItem(GUIInvWindow *guii);
int             InvWindow_GetItemsPerRow(GUIInvWindow *guii);
int             InvWindow_GetItemCount(GUIInvWindow *guii);
int             InvWindow_GetRowCount(GUIInvWindow *guii);
void            InvWindow_ScrollDown(GUIInvWindow *guii);
void            InvWindow_ScrollUp(GUIInvWindow *guii);
ScriptInvItem*  InvWindow_GetItemAtIndex(GUIInvWindow *guii, int index);

//=============================================================================

int				offset_over_inv(GUIInvWindow *inv);
// NOTE: This function is valid for AGS 2.72 and lower
int             invscreen();

#endif // __AGS_EE_AC__INVWINDOW_H
