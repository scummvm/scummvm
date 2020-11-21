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
#ifndef __AGS_EE_AC__GLOBALCHARACTER_H
#define __AGS_EE_AC__GLOBALCHARACTER_H

#include "ac/characterinfo.h"

void StopMoving(int chaa);
void ReleaseCharacterView(int chat);
void MoveToWalkableArea(int charid);
void FaceLocation(int cha, int xx, int yy);
void FaceCharacter(int cha,int toface);
void SetCharacterIdle(int who, int iview, int itime);
int  GetCharacterWidth(int ww);
int  GetCharacterHeight(int charid);
void SetCharacterBaseline (int obn, int basel);
// pass trans=0 for fully solid, trans=100 for fully transparent
void SetCharacterTransparency(int obn,int trans);
void scAnimateCharacter (int chh, int loopn, int sppd, int rept);
void AnimateCharacterEx(int chh, int loopn, int sppd, int rept, int direction, int blocking);
void SetPlayerCharacter(int newchar);
void FollowCharacterEx(int who, int tofollow, int distaway, int eagerness);
void FollowCharacter(int who, int tofollow);
void SetCharacterIgnoreLight (int who, int yesorno);
void MoveCharacter(int cc,int xx,int yy);
void MoveCharacterDirect(int cc,int xx, int yy);
void MoveCharacterStraight(int cc,int xx, int yy);
// Append to character path
void MoveCharacterPath (int chac, int tox, int toy);

void SetCharacterSpeedEx(int chaa, int xspeed, int yspeed);
void SetCharacterSpeed(int chaa,int nspeed);
void SetTalkingColor(int chaa,int ncol);
void SetCharacterSpeechView (int chaa, int vii);
void SetCharacterBlinkView (int chaa, int vii, int intrv);
void SetCharacterView(int chaa,int vii);
void SetCharacterFrame(int chaa, int view, int loop, int frame);
// similar to SetCharView, but aligns the frame to make it line up
void SetCharacterViewEx (int chaa, int vii, int loop, int align);
void SetCharacterViewOffset (int chaa, int vii, int xoffs, int yoffs);
void ChangeCharacterView(int chaa,int vii);
void SetCharacterClickable (int cha, int clik);
void SetCharacterIgnoreWalkbehinds (int cha, int clik);
void MoveCharacterToObject(int chaa,int obbj);
void MoveCharacterToHotspot(int chaa,int hotsp);
void MoveCharacterBlocking(int chaa,int xx,int yy,int direct);

void RunCharacterInteraction (int cc, int mood);
int  AreCharObjColliding(int charid,int objid);
int  AreCharactersColliding(int cchar1,int cchar2);

int  GetCharacterProperty (int cha, const char *property);
void SetCharacterProperty (int who, int flag, int yesorno);
int  GetPlayerCharacter();
void GetCharacterPropertyText (int item, const char *property, char *bufer);

int GetCharacterSpeechAnimationDelay(CharacterInfo *cha);
int GetCharIDAtScreen(int xx, int yy);

void SetActiveInventory(int iit);
void AddInventoryToCharacter(int charid, int inum);
void LoseInventoryFromCharacter(int charid, int inum);
void update_invorder();
void add_inventory(int inum);
void lose_inventory(int inum);

void DisplayThought(int chid, const char *text);
void __sc_displayspeech(int chid, const char *text);
void DisplaySpeechAt (int xx, int yy, int wii, int aschar, const char*spch);
int DisplaySpeechBackground(int charid, const char*speel);

#endif // __AGS_EE_AC__CHARACTEREXTRAS_H
