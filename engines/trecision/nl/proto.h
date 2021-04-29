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

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Trecision {

struct ATFHandle;
struct SDObj;
struct STexture;
struct SPan;

void performLoad(int slot, bool skipLoad);
bool pointInside(int pan, double x, double z);
void ProcessAtFrame(ATFHandle *h, int type, int atf);
void InitAtFrameHandler(uint16 an, uint16 obj);
void AtFrameNext();
void AtFrameEnd(int type);
void AtFrameHandler(int type);
void PaintScreen(bool flag);
void PaintObjAnm(uint16 CurBox);
void doEvent(uint8 cls, uint8 event, uint8 priority, uint16 u16Param1, uint16 u16Param2, uint8 u8Param, uint32 u32Param);
void Scheduler();
void ProcessTheMessage();
void PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl);
void ShowObjName(uint16 obj, bool showhide);
void FormattingSuperString();
void FormattingOneString();
void CharacterSay(uint16 i);
void CharacterTalk(const char *s);
void CharacterTalkInAction(uint16 ss);
void CharacterContinueTalk();
void CharacterMute();
void SomeoneTalk(uint16 s, uint16 Person, uint16 NewAnim);
void SomeoneContinueTalk();
void someoneMute();
void doString();

#endif
} // End of namespace Trecision
