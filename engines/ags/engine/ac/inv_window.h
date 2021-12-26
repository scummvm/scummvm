/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AGS_ENGINE_AC_INVWINDOW_H
#define AGS_ENGINE_AC_INVWINDOW_H

#include "ags/shared/ac/character_info.h"
#include "ags/engine/ac/dynobj/script_inv_item.h"
#include "ags/shared/gui/gui_inv.h"

namespace AGS3 {

using AGS::Shared::GUIInvWindow;

void            InvWindow_SetCharacterToUse(GUIInvWindow *guii, CharacterInfo *chaa);
CharacterInfo *InvWindow_GetCharacterToUse(GUIInvWindow *guii);
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
ScriptInvItem *InvWindow_GetItemAtIndex(GUIInvWindow *guii, int index);

//=============================================================================

int             offset_over_inv(GUIInvWindow *inv);
// NOTE: This function is valid for AGS 2.72 and lower
int             invscreen();

} // namespace AGS3

#endif
