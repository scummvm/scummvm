/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TRECISION_PROTO_H
#define TRECISION_PROTO_H

#include "common/str.h"

void warning(const char *format, ...);

namespace Common {
class SeekableReadStream;
}

namespace Trecision {

struct ATFHandle;
struct SDObj;
struct STexture;
struct SPan;

uint32 DecCR(Common::String fileName, uint8 *DestArea, uint8 *DecArea);
void performLoad(int slot, bool skipLoad);
bool pointInside(int pan, double x, double z);
void doAction();
void doMouse();
void StartCharacterAction(uint16 Act, uint16 NewRoom, uint8 NewPos, uint16 sent);
void doCharacter();
void doSystem();
void doIdle();
void DialogPrint(int x, int y, int c, const char *txt);
void ShowChoices(uint16 i);
void UpdateChoices(int16 dmx, int16 dmy);
void SelectChoice(int16 dmx, int16 dmy);
void PlayDialog(uint16 i);
void afterChoice(int numframe);
void DialogHandler(int numframe);
void PlayChoice(uint16 i);
void doDialog();
void doRoomIn(uint16 curObj);
void doRoomOut(uint16 curObj);
void doMouseExamine(uint16 curObj);
void doMouseOperate(uint16 curObj);
void doMouseTake(uint16 curObj);
void doMouseTalk(uint16 curObj);
void doUseWith();
void doScreenUseWithScreen();
void doInvExamine();
void doInvOperate();
void doDoing();
void doScript();
bool AtMouseClick(uint16 curObj);
void AtEndChangeRoom();
void ProcessATF(ATFHandle *h, int type, int atf);
void InitAtFrameHandler(uint16 an, uint16 obj);
void AtFrameNext();
void AtFrameEnd(int type);
void AtFrameHandler(int type);
void ProcessTime();
void ProcessMouse();
void PaintScreen(uint8 flag);
void PaintObjAnm(uint16 CurBox);
int IntersecateRect(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
void doEvent(uint8 cls, uint8 event, uint8 priority, uint16 u16Param1, uint16 u16Param2, uint8 u8Param, uint32 u32Param);
void Scheduler();
void ProcessTheMessage();
void PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool Homo);
void ShowObjName(uint16 obj, bool showhide);
void FormattingSuperString();
void FormattingOneString();
void CharacterSay(uint16 i);
void CharacterTalk(const char *s, bool FromCharacterSay);
void CharacterTalkInAction(uint16 ss);
void CharacterContinueTalk();
void CharacterMute();
void SomeOneTalk(uint16 s, uint16 Person, uint16 NewAnim, bool FromSomeOneSay);
void SomeoneContinueTalk();
void SomeOneMute();
void doString();
char GetKey();
void soundtimefunct();

#endif
} // End of namespace Trecision
