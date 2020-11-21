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
#ifndef __AGS_EE_AC__GLOBALINVENTORYITEM_H
#define __AGS_EE_AC__GLOBALINVENTORYITEM_H

void set_inv_item_pic(int invi, int piccy);
void SetInvItemName(int invi, const char *newName);
int  GetInvAt (int xxx, int yyy);
void GetInvName(int indx,char*buff);
int  GetInvGraphic(int indx);
void RunInventoryInteraction (int iit, int modd);
int  IsInventoryInteractionAvailable (int item, int mood);
int  GetInvProperty (int item, const char *property);
void GetInvPropertyText (int item, const char *property, char *bufer);

#endif // __AGS_EE_AC__GLOBALINVENTORYITEM_H
