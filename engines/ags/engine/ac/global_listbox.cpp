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

#include "ags/engine/ac/global_listbox.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/listbox.h"
#include "ags/engine/ac/string.h"

namespace AGS3 {

void ListBoxClear(int guin, int objn) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	ListBox_Clear(guisl);
}
void ListBoxAdd(int guin, int objn, const char *newitem) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	ListBox_AddItem(guisl, newitem);
}
void ListBoxRemove(int guin, int objn, int itemIndex) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	ListBox_RemoveItem(guisl, itemIndex);
}
int ListBoxGetSelected(int guin, int objn) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	return ListBox_GetSelectedIndex(guisl);
}
int ListBoxGetNumItems(int guin, int objn) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	return ListBox_GetItemCount(guisl);
}
char *ListBoxGetItemText(int guin, int objn, int item, char *buffer) {
	VALIDATE_STRING(buffer);
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	return ListBox_GetItemText(guisl, item, buffer);
}
void ListBoxSetSelected(int guin, int objn, int newsel) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	ListBox_SetSelectedIndex(guisl, newsel);
}
void ListBoxSetTopItem(int guin, int objn, int item) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	ListBox_SetTopItem(guisl, item);
}

int ListBoxSaveGameList(int guin, int objn) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	return ListBox_FillSaveGameList(guisl);
}

void ListBoxDirList(int guin, int objn, const char *filemask) {
	GUIListBox *guisl = is_valid_listbox(guin, objn);
	ListBox_FillDirList(guisl, filemask);
}

} // namespace AGS3
