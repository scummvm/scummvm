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
#ifndef __AGS_EE_AC__GLOBALROOM_H
#define __AGS_EE_AC__GLOBALROOM_H

void SetAmbientTint (int red, int green, int blue, int opacity, int luminance);
void SetAmbientLightLevel(int light_level);
void NewRoom(int nrnum);
void NewRoomEx(int nrnum,int newx,int newy);
void NewRoomNPC(int charid, int nrnum, int newx, int newy);
void ResetRoom(int nrnum);
int  HasPlayerBeenInRoom(int roomnum);
void CallRoomScript (int value);
int  HasBeenToRoom (int roomnum);
void GetRoomPropertyText (const char *property, char *bufer);

void SetBackgroundFrame(int frnum);
int GetBackgroundFrame() ;

#endif // __AGS_EE_AC__GLOBALROOM_H
