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

#ifndef AGS_ENGINE_AC_GLOBAL_LISTBOX_H
#define AGS_ENGINE_AC_GLOBAL_LISTBOX_H

namespace AGS3 {

void        ListBoxClear(int guin, int objn);
void        ListBoxAdd(int guin, int objn, const char *newitem);
void        ListBoxRemove(int guin, int objn, int itemIndex);
int         ListBoxGetSelected(int guin, int objn);
int         ListBoxGetNumItems(int guin, int objn);
char *ListBoxGetItemText(int guin, int objn, int item, char *buffer);
void        ListBoxSetSelected(int guin, int objn, int newsel);
void        ListBoxSetTopItem(int guin, int objn, int item);
int         ListBoxSaveGameList(int guin, int objn);
void        ListBoxDirList(int guin, int objn, const char *filemask);

} // namespace AGS3

#endif
