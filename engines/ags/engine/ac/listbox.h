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
#ifndef __AGS_EE_AC__LISTBOX_H
#define __AGS_EE_AC__LISTBOX_H

#include "gui/guilistbox.h"

using AGS::Common::GUIListBox;

int			ListBox_AddItem(GUIListBox *lbb, const char *text);
int			ListBox_InsertItemAt(GUIListBox *lbb, int index, const char *text);
void		ListBox_Clear(GUIListBox *listbox);
void		ListBox_FillDirList(GUIListBox *listbox, const char *filemask);
int			ListBox_GetSaveGameSlots(GUIListBox *listbox, int index);
int			ListBox_FillSaveGameList(GUIListBox *listbox);
int			ListBox_GetItemAtLocation(GUIListBox *listbox, int x, int y);
char		*ListBox_GetItemText(GUIListBox *listbox, int index, char *buffer);
const char* ListBox_GetItems(GUIListBox *listbox, int index);
void		ListBox_SetItemText(GUIListBox *listbox, int index, const char *newtext);
void		ListBox_RemoveItem(GUIListBox *listbox, int itemIndex);
int			ListBox_GetItemCount(GUIListBox *listbox);
int			ListBox_GetFont(GUIListBox *listbox);
void		ListBox_SetFont(GUIListBox *listbox, int newfont);
int			ListBox_GetHideBorder(GUIListBox *listbox);
void		ListBox_SetHideBorder(GUIListBox *listbox, int newValue);
int			ListBox_GetHideScrollArrows(GUIListBox *listbox);
void		ListBox_SetHideScrollArrows(GUIListBox *listbox, int newValue);
int			ListBox_GetSelectedIndex(GUIListBox *listbox);
void		ListBox_SetSelectedIndex(GUIListBox *guisl, int newsel);
int			ListBox_GetTopItem(GUIListBox *listbox);
void		ListBox_SetTopItem(GUIListBox *guisl, int item);
int			ListBox_GetRowCount(GUIListBox *listbox);
void		ListBox_ScrollDown(GUIListBox *listbox);
void		ListBox_ScrollUp(GUIListBox *listbox);

GUIListBox* is_valid_listbox (int guin, int objn);

#endif // __AGS_EE_AC__LISTBOX_H
