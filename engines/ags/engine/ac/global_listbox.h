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
#ifndef __AGS_EE_AC__GLOBALLISTBOX_H
#define __AGS_EE_AC__GLOBALLISTBOX_H

void		ListBoxClear(int guin, int objn);
void		ListBoxAdd(int guin, int objn, const char*newitem);
void		ListBoxRemove(int guin, int objn, int itemIndex);
int			ListBoxGetSelected(int guin, int objn);
int			ListBoxGetNumItems(int guin, int objn);
char*		ListBoxGetItemText(int guin, int objn, int item, char*buffer);
void		ListBoxSetSelected(int guin, int objn, int newsel);
void		ListBoxSetTopItem (int guin, int objn, int item);
int			ListBoxSaveGameList (int guin, int objn);
void		ListBoxDirList (int guin, int objn, const char*filemask);

#endif // __AGS_EE_AC__GLOBALLISTBOX_H
