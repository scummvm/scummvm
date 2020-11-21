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

#include "ac/global_listbox.h"
#include "ac/common.h"
#include "ac/listbox.h"
#include "ac/string.h"

void ListBoxClear(int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  ListBox_Clear(guisl);
}
void ListBoxAdd(int guin, int objn, const char*newitem) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  ListBox_AddItem(guisl, newitem);
}
void ListBoxRemove(int guin, int objn, int itemIndex) {
  GUIListBox*guisl = is_valid_listbox(guin,objn);
  ListBox_RemoveItem(guisl, itemIndex);
}
int ListBoxGetSelected(int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_GetSelectedIndex(guisl);
}
int ListBoxGetNumItems(int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_GetItemCount(guisl);
}
char* ListBoxGetItemText(int guin, int objn, int item, char*buffer) {
  VALIDATE_STRING(buffer);
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_GetItemText(guisl, item, buffer);
}
void ListBoxSetSelected(int guin, int objn, int newsel) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  ListBox_SetSelectedIndex(guisl, newsel);
}
void ListBoxSetTopItem (int guin, int objn, int item) {
  GUIListBox*guisl = is_valid_listbox(guin,objn);
  ListBox_SetTopItem(guisl, item);
}

int ListBoxSaveGameList (int guin, int objn) {
  GUIListBox*guisl=is_valid_listbox(guin,objn);
  return ListBox_FillSaveGameList(guisl);
}

void ListBoxDirList (int guin, int objn, const char*filemask) {
  GUIListBox *guisl = is_valid_listbox(guin,objn);
  ListBox_FillDirList(guisl, filemask);
}
