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

#ifndef AGS_ENGINE_AC_LISTBOX_H
#define AGS_ENGINE_AC_LISTBOX_H

#include "ags/shared/gui/gui_listbox.h"

namespace AGS3 {

using AGS::Shared::GUIListBox;

int         ListBox_AddItem(GUIListBox *lbb, const char *text);
int         ListBox_InsertItemAt(GUIListBox *lbb, int index, const char *text);
void        ListBox_Clear(GUIListBox *listbox);
void        ListBox_FillDirList(GUIListBox *listbox, const char *filemask);
int         ListBox_GetSaveGameSlots(GUIListBox *listbox, int index);
int         ListBox_FillSaveGameList(GUIListBox *listbox);
int         ListBox_GetItemAtLocation(GUIListBox *listbox, int x, int y);
char *ListBox_GetItemText(GUIListBox *listbox, int index, char *buffer);
const char *ListBox_GetItems(GUIListBox *listbox, int index);
void        ListBox_SetItemText(GUIListBox *listbox, int index, const char *newtext);
void        ListBox_RemoveItem(GUIListBox *listbox, int itemIndex);
int         ListBox_GetItemCount(GUIListBox *listbox);
int         ListBox_GetFont(GUIListBox *listbox);
void        ListBox_SetFont(GUIListBox *listbox, int newfont);
int         ListBox_GetHideBorder(GUIListBox *listbox);
void        ListBox_SetHideBorder(GUIListBox *listbox, int newValue);
int         ListBox_GetHideScrollArrows(GUIListBox *listbox);
void        ListBox_SetHideScrollArrows(GUIListBox *listbox, int newValue);
int         ListBox_GetSelectedIndex(GUIListBox *listbox);
void        ListBox_SetSelectedIndex(GUIListBox *guisl, int newsel);
int         ListBox_GetTopItem(GUIListBox *listbox);
void        ListBox_SetTopItem(GUIListBox *guisl, int item);
int         ListBox_GetRowCount(GUIListBox *listbox);
void        ListBox_ScrollDown(GUIListBox *listbox);
void        ListBox_ScrollUp(GUIListBox *listbox);

GUIListBox *is_valid_listbox(int guin, int objn);

} // namespace AGS3

#endif
