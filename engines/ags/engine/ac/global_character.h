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

#ifndef AGS_ENGINE_AC_GLOBAL_CHARACTER_H
#define AGS_ENGINE_AC_GLOBAL_CHARACTER_H

#include "ags/shared/ac/character_info.h"

namespace AGS3 {

void StopMoving(int chaa);
void ReleaseCharacterView(int chat);
void MoveToWalkableArea(int charid);
void FaceLocation(int cha, int xx, int yy);
void FaceCharacter(int cha, int toface);
void SetCharacterIdle(int who, int iview, int itime);
int  GetCharacterWidth(int ww);
int  GetCharacterHeight(int charid);
void SetCharacterBaseline(int obn, int basel);
// pass trans=0 for fully solid, trans=100 for fully transparent
void SetCharacterTransparency(int obn, int trans);
void AnimateCharacter4(int chh, int loopn, int sppd, int rept);
void AnimateCharacter6(int chh, int loopn, int sppd, int rept, int direction, int blocking);
void SetPlayerCharacter(int newchar);
void FollowCharacterEx(int who, int tofollow, int distaway, int eagerness);
void FollowCharacter(int who, int tofollow);
void SetCharacterIgnoreLight(int who, int yesorno);
void MoveCharacter(int cc, int xx, int yy);
void MoveCharacterDirect(int cc, int xx, int yy);
void MoveCharacterStraight(int cc, int xx, int yy);
// Append to character path
void MoveCharacterPath(int chac, int tox, int toy);

void SetCharacterSpeedEx(int chaa, int xspeed, int yspeed);
void SetCharacterSpeed(int chaa, int nspeed);
void SetTalkingColor(int chaa, int ncol);
void SetCharacterSpeechView(int chaa, int vii);
void SetCharacterBlinkView(int chaa, int vii, int intrv);
void SetCharacterView(int chaa, int vii);
void SetCharacterFrame(int chaa, int view, int loop, int frame);
// similar to SetCharView, but aligns the frame to make it line up
void SetCharacterViewEx(int chaa, int vii, int loop, int align);
void SetCharacterViewOffset(int chaa, int vii, int xoffs, int yoffs);
void ChangeCharacterView(int chaa, int vii);
void SetCharacterClickable(int cha, int clik);
void SetCharacterIgnoreWalkbehinds(int cha, int clik);
void MoveCharacterToObject(int chaa, int obbj);
void MoveCharacterToHotspot(int chaa, int hotsp);
int MoveCharacterBlocking(int chaa, int xx, int yy, int direct);

void RunCharacterInteraction(int cc, int mood);
int  AreCharObjColliding(int charid, int objid);
int  AreCharactersColliding(int cchar1, int cchar2);

int  GetCharacterProperty(int cha, const char *property);
void SetCharacterProperty(int who, int flag, int yesorno);
int  GetPlayerCharacter();
void GetCharacterPropertyText(int item, const char *property, char *bufer);

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
void DisplaySpeechAt(int xx, int yy, int wii, int aschar, const char *spch);
int DisplaySpeechBackground(int charid, const char *speel);

} // namespace AGS3

#endif
